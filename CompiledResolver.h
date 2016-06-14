#pragma once

#include <string>
#include <queue>
#include <forward_list>
#include <unordered_map>

#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/GenericValue.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"

#include "DecisionTree.h"
#include "SimpleOrcJit.h"

llvm::LLVMContext Ctx;
llvm::IRBuilder<> Builder(Ctx);
std::unique_ptr<llvm::Module> TheModule;
std::unique_ptr<SimpleOrcJit> TheCompiler;

using compiledNodeEvaluator_f = unsigned long(const float *);
using compiledNodeEvaluatorsMap_t =
    std::unordered_map<unsigned long, compiledNodeEvaluator_f *>;

compiledNodeEvaluatorsMap_t compiledNodeEvaluators;

std::unique_ptr<llvm::Module> setupModule() {
  auto module = std::make_unique<llvm::Module>("test", Ctx);

  auto *targetMachine = llvm::EngineBuilder().selectTarget();
  module->setDataLayout(targetMachine->createDataLayout());

  return module;
}

std::unique_ptr<SimpleOrcJit> setupCompiler() {
  auto *targetMachine = llvm::EngineBuilder().selectTarget();
  return std::make_unique<SimpleOrcJit>(*targetMachine);
}

void initializeLLVM() {
  llvm::InitializeNativeTarget();
  llvm::InitializeNativeTargetAsmPrinter();
  llvm::InitializeNativeTargetAsmParser();
  TheCompiler = setupCompiler();
  TheModule = setupModule();
}

void shutdownLLVM() {
  TheModule.reset();
  TheCompiler.reset();
  llvm::llvm_shutdown();
}

llvm::Function* emitFunctionDeclaration(std::string name) {
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

llvm::Value *emitNodeEvaluatonsRecursively(const DecisionTree &tree,
                                           unsigned long nodeIdx,
                                           llvm::Function *function,
                                           llvm::Value *dataSetPtr,
                                           int remainingLevels,
                                           std::queue<unsigned long>& scheduledNodes) {
  using namespace llvm;

  Type *returnTy = Type::getInt64Ty(Ctx);
  const TreeNode &node = tree.at(nodeIdx);

  if (node.isLeaf()) {
    return ConstantInt::get(returnTy, nodeIdx);
  }

  Value *comparisonResult = emitSingleNodeEvaluaton(node, dataSetPtr);

  unsigned long trueIdx = node.getTrueChildIdx();
  unsigned long falseIdx = node.getFalseChildIdx();

  if (remainingLevels == 0) {
    scheduledNodes.push(trueIdx);
    scheduledNodes.push(falseIdx);

    return Builder.CreateSelect(comparisonResult,
                                ConstantInt::get(returnTy, trueIdx),
                                ConstantInt::get(returnTy, falseIdx));
  }
  else {
    std::string trueBBName = "node_" + std::to_string(trueIdx);
    auto* trueBB = llvm::BasicBlock::Create(Ctx, trueBBName, function);

    std::string falseBBName = "node_" + std::to_string(falseIdx);
    auto* falseBB = llvm::BasicBlock::Create(Ctx, falseBBName, function);

    std::string mergeBBName = "merge_" + std::to_string(trueIdx)
                                 + "_" + std::to_string(falseIdx);
    auto* mergeBB = llvm::BasicBlock::Create(Ctx, mergeBBName, function);

    Builder.CreateCondBr(comparisonResult, trueBB, falseBB);

    Builder.SetInsertPoint(trueBB);
    Value *trueResult = emitNodeEvaluatonsRecursively(
        tree, trueIdx, function, dataSetPtr, remainingLevels - 1, scheduledNodes);
    Builder.CreateBr(mergeBB);
    trueBB = Builder.GetInsertBlock();

    Builder.SetInsertPoint(falseBB);
    Value *falseResult = emitNodeEvaluatonsRecursively(
        tree, falseIdx, function, dataSetPtr, remainingLevels - 1, scheduledNodes);
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

void compileEvaluators(const DecisionTree &tree, int nodeLevelsPerFunction) {
  using namespace llvm;

  printf("%% ");
  //unsigned long nodesProcessed = 0;
  //unsigned long quaterPercentStep = tree.size() / 25;

  std::string nameStub = "nodeEvaluator_";
  std::queue<unsigned long> scheduledNodes;
  std::forward_list<unsigned long> processedNodes;
  scheduledNodes.push(0);

  while (!scheduledNodes.empty()) {
    unsigned long nodeIdx = scheduledNodes.front();
    const TreeNode &node = tree.at(nodeIdx);

    std::string name = nameStub + std::to_string(nodeIdx);
    Function *evalFn = emitFunctionDeclaration(std::move(name));
    Value *dataSetPtr = &*evalFn->arg_begin();

    Builder.SetInsertPoint(llvm::BasicBlock::Create(Ctx, "entry", evalFn));
    Value *nextNodeIdx = emitNodeEvaluatonsRecursively(
        tree, nodeIdx, evalFn, dataSetPtr, nodeLevelsPerFunction - 1, scheduledNodes);

    Builder.CreateRet(nextNodeIdx);
    llvm::verifyFunction(*evalFn);

    /*nodesProcessed += TreeSize(nodeLevelsPerFunction);
    if (nodesProcessed >= quaterPercentStep) {
      nodesProcessed -= quaterPercentStep;
      printf(".");
      fflush(stdout);
    }*/

    processedNodes.push_front(nodeIdx);
    scheduledNodes.pop();
  }

  //llvm::outs() << "We just constructed this LLVM module:\n\n";
  //llvm::outs() << *TheModule.get() << "\n\n";

  // submit for jit compilation
  TheCompiler->submitModule(std::move(TheModule));

  for (int i = 0; i < 50; i++)
    printf(".");

  fflush(stdout);

  // collect evaluators
  while (!processedNodes.empty()) {
    unsigned long nodeIdx = processedNodes.front();
    std::string nodeFunctionName = nameStub + std::to_string(nodeIdx);

    compiledNodeEvaluators[nodeIdx] =
        TheCompiler->getEvaluatorFnPtr(nodeFunctionName);

    /*if (++nodesProcessed > quaterPercentStep) {
      nodesProcessed -= quaterPercentStep;
      printf(".");
      fflush(stdout);
    }*/

    processedNodes.pop_front();
  }
}

template <unsigned long DataSetFeatures_>
unsigned long computeLeafNodeIdxForDataSetCompiled(
    const DecisionTree &tree,
    const std::array<float, DataSetFeatures_> &dataSet) {
  int64_t treeNodeIdx = 0;

  while (!tree.at(treeNodeIdx).isLeaf()) {
    auto compiledEvaluator = compiledNodeEvaluators[treeNodeIdx];
    treeNodeIdx = compiledEvaluator(dataSet.data());
  }

  return treeNodeIdx;
}