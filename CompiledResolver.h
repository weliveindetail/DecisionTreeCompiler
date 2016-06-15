#pragma once

#include <forward_list>
#include <queue>
#include <string>
#include <unordered_map>

#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/GenericValue.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/ManagedStatic.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_ostream.h>

#include "DecisionTree.h"
#include "SimpleObjectCache.h"
#include "SimpleOrcJit.h"

llvm::LLVMContext Ctx;
llvm::IRBuilder<> Builder(Ctx);
std::unique_ptr<llvm::Module> TheModule;
std::unique_ptr<SimpleOrcJit> TheCompiler;

using compiledNodeEvaluator_f = int64_t(const float *);
using compiledNodeEvaluatorsMap_t =
    std::unordered_map<int64_t, compiledNodeEvaluator_f *>;

compiledNodeEvaluatorsMap_t compiledNodeEvaluators;

void setupModule(std::string name) {
  TheModule = std::make_unique<llvm::Module>(name, Ctx);

  auto *targetMachine = llvm::EngineBuilder().selectTarget();
  TheModule->setDataLayout(targetMachine->createDataLayout());
}

std::unique_ptr<SimpleOrcJit> setupCompiler() {
  auto targetMachine = llvm::EngineBuilder().selectTarget();
  auto cache = std::make_unique<SimpleObjectCache>();

  return std::make_unique<SimpleOrcJit>(*targetMachine, std::move(cache));
}

void initializeLLVM() {
  llvm::InitializeNativeTarget();
  llvm::InitializeNativeTargetAsmPrinter();
  llvm::InitializeNativeTargetAsmParser();
  TheCompiler = setupCompiler();
}

void shutdownLLVM() {
  TheModule.reset();
  TheCompiler.reset();
  llvm::llvm_shutdown();
}

llvm::Function *emitFunctionDeclaration(std::string name) {
  using namespace llvm;

  auto returnTy = Type::getInt64Ty(Ctx);
  auto argTy = Type::getFloatTy(Ctx)->getPointerTo();
  auto signature = FunctionType::get(returnTy, {argTy}, false);
  auto linkage = Function::ExternalLinkage;

  Function *evalFn =
      Function::Create(signature, linkage, name, TheModule.get());

  evalFn->setName(name);
  return evalFn;
}

llvm::Function *getUnaryIntrinsic(llvm::Intrinsic::ID id, llvm::Type *opTy) {
  return llvm::Intrinsic::getDeclaration(TheModule.get(), id, {opTy});
}

llvm::Value *emitOperator(OperationType op, llvm::Value *value) {
  switch (op) {
  case OperationType::Bypass:
    return value;

  case OperationType::Sqrt: {
    llvm::Function *sqrtFn =
        getUnaryIntrinsic(llvm::Intrinsic::sqrt, value->getType());
    return Builder.CreateCall(sqrtFn, {value});
  }

  case OperationType::Ln: {
    llvm::Function *lnFn =
        getUnaryIntrinsic(llvm::Intrinsic::log, value->getType());
    return Builder.CreateCall(lnFn, {value});
  }
  }
};

llvm::Value *emitComparison(ComparatorType comp, float bias,
                            llvm::Value *value) {
  llvm::Constant *biasConst = llvm::ConstantFP::get(value->getType(), bias);
  switch (comp) {
  case ComparatorType::LessThan:
    return Builder.CreateFCmpOLT(value, biasConst);

  case ComparatorType::GreaterThan:
    return Builder.CreateFCmpOGT(value, biasConst);
  }
}

llvm::Value *emitSingleNodeEvaluaton(const TreeNode &node,
                                     llvm::Value *dataSetPtr) {
  using namespace llvm;

  Value *dataSetFeaturePtr =
      Builder.CreateConstGEP1_32(dataSetPtr, node.DataSetFeatureIdx);
  Value *dataSetFeatureVal = Builder.CreateLoad(dataSetFeaturePtr);

  Value *comparableFeatureVal = emitOperator(node.Op, dataSetFeatureVal);
  return emitComparison(node.Comp, node.Bias, comparableFeatureVal);
}

llvm::Value *
emitNodeEvaluationsRecursively(const DecisionTree &tree, int64_t nodeIdx,
                               llvm::Function *function,
                               llvm::Value *dataSetPtr, int remainingLevels,
                               std::queue<int64_t> &scheduledNodes) {
  using namespace llvm;

  Type *returnTy = Type::getInt64Ty(Ctx);
  const TreeNode &node = tree.at(nodeIdx);

  if (node.isLeaf()) {
    return ConstantInt::get(returnTy, nodeIdx);
  }

  Value *comparisonResult = emitSingleNodeEvaluaton(node, dataSetPtr);

  int64_t trueIdx = node.getTrueChildIdx();
  int64_t falseIdx = node.getFalseChildIdx();

  if (remainingLevels == 0) {
    scheduledNodes.push(trueIdx);
    scheduledNodes.push(falseIdx);

    return Builder.CreateSelect(comparisonResult,
                                ConstantInt::get(returnTy, trueIdx),
                                ConstantInt::get(returnTy, falseIdx));
  } else {
    std::string trueBBName = "node_" + std::to_string(trueIdx);
    auto *trueBB = llvm::BasicBlock::Create(Ctx, trueBBName, function);

    std::string falseBBName = "node_" + std::to_string(falseIdx);
    auto *falseBB = llvm::BasicBlock::Create(Ctx, falseBBName, function);

    std::string mergeBBName =
        "merge_" + std::to_string(trueIdx) + "_" + std::to_string(falseIdx);
    auto *mergeBB = llvm::BasicBlock::Create(Ctx, mergeBBName, function);

    Builder.CreateCondBr(comparisonResult, trueBB, falseBB);

    Builder.SetInsertPoint(trueBB);
    Value *trueResult =
        emitNodeEvaluationsRecursively(tree, trueIdx, function, dataSetPtr,
                                       remainingLevels - 1, scheduledNodes);
    Builder.CreateBr(mergeBB);
    trueBB = Builder.GetInsertBlock();

    Builder.SetInsertPoint(falseBB);
    Value *falseResult =
        emitNodeEvaluationsRecursively(tree, falseIdx, function, dataSetPtr,
                                       remainingLevels - 1, scheduledNodes);
    Builder.CreateBr(mergeBB);
    falseBB = Builder.GetInsertBlock();

    Builder.SetInsertPoint(mergeBB);
    std::string mergeResultName = mergeBBName + "tmp";
    PHINode *mergeResult = Builder.CreatePHI(returnTy, 2, mergeBBName);

    mergeResult->addIncoming(trueResult, trueBB);
    mergeResult->addIncoming(falseResult, falseBB);

    return mergeResult;
  }
}

template <int TreeDepth_>
int64_t loadEvaluators(const DecisionTree &tree, int nodeLevelsPerFunction) {
  using namespace llvm;

  // load module from cache
  TheCompiler->submitModule(std::move(TheModule));

  // figure out which evaluator functions we expect and collect them
  std::string nameStub = "nodeEvaluator_";

  int evaluatorDepth =
      ((TreeDepth_ + nodeLevelsPerFunction - 1) / nodeLevelsPerFunction);

  for (int level = 0; level < TreeDepth_; level += nodeLevelsPerFunction) {
    int64_t firstNodeIdxOnLevel = TreeSize(level);
    int numNodesOnLevel = (1 << level);

    for (int offset = 0; offset < numNodesOnLevel; offset++) {
      int64_t nodeIdx = firstNodeIdxOnLevel + offset;

      std::string nodeFunctionName = nameStub + std::to_string(nodeIdx);

      compiledNodeEvaluators[nodeIdx] =
          TheCompiler->getEvaluatorFnPtr(nodeFunctionName);
    }
  }

  return compiledNodeEvaluators.size();
}

template <int TreeDepth_>
int64_t compileEvaluators(const DecisionTree &tree, int nodeLevelsPerFunction) {
  using namespace llvm;

  std::string nameStub = "nodeEvaluator_";
  std::forward_list<int64_t> processedNodes;

  std::queue<int64_t> scheduledNodes;
  scheduledNodes.push(0);

  while (!scheduledNodes.empty()) {
    int64_t nodeIdx = scheduledNodes.front();
    const TreeNode &node = tree.at(nodeIdx);

    std::string name = nameStub + std::to_string(nodeIdx);
    Function *evalFn = emitFunctionDeclaration(std::move(name));
    Value *dataSetPtr = &*evalFn->arg_begin();

    Builder.SetInsertPoint(llvm::BasicBlock::Create(Ctx, "entry", evalFn));
    Value *nextNodeIdx = emitNodeEvaluationsRecursively(
        tree, nodeIdx, evalFn, dataSetPtr, nodeLevelsPerFunction - 1,
        scheduledNodes);

    Builder.CreateRet(nextNodeIdx);
    llvm::verifyFunction(*evalFn);

    processedNodes.push_front(nodeIdx);
    scheduledNodes.pop();
  }

  printf("\nCompiling..");
  fflush(stdout);

  // llvm::outs() << "We just constructed this LLVM module:\n\n";
  // llvm::outs() << *TheModule.get() << "\n\n";

  // submit for jit compilation
  TheCompiler->submitModule(std::move(TheModule));
  printf(".");
  fflush(stdout);

  // collect evaluators
  while (!processedNodes.empty()) {
    int64_t nodeIdx = processedNodes.front();
    std::string nodeFunctionName = nameStub + std::to_string(nodeIdx);

    compiledNodeEvaluators[nodeIdx] =
        TheCompiler->getEvaluatorFnPtr(nodeFunctionName);

    processedNodes.pop_front();
  }

  return compiledNodeEvaluators.size();
}

template <int TreeDepth_>
int64_t getNumCompiledEvaluators(int compiledFunctionDepth) {
  int64_t expectedEvaluators = 0;
  int evaluatorDepth =
      ((TreeDepth_ + compiledFunctionDepth - 1) / compiledFunctionDepth);

  for (int i = 0; i < evaluatorDepth; i++)
    expectedEvaluators += 1 << (compiledFunctionDepth * i);

  return expectedEvaluators;
}

template <int DataSetFeatures_>
int64_t computeLeafNodeIdxForDataSetCompiled(
    const DecisionTree &tree,
    const std::vector<float> &dataSet) {
  int64_t treeNodeIdx = 0;

  while (!tree.at(treeNodeIdx).isLeaf()) {
    auto compiledEvaluator = compiledNodeEvaluators[treeNodeIdx];
    treeNodeIdx = compiledEvaluator(dataSet.data());
  }

  return treeNodeIdx;
}