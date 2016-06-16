#pragma once

#include <chrono>
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

void collectSubtreeNodeIdxsRecursively(
    const DecisionTree &tree, int64_t nodeIdx, int remainingLevels,
    std::vector<int64_t> &result) {
  result.push_back(nodeIdx);

  if (remainingLevels > 0) {
    const TreeNode &node = tree.at(nodeIdx);
    collectSubtreeNodeIdxsRecursively(tree, node.TrueChildNodeIdx, remainingLevels - 1, result);
    collectSubtreeNodeIdxsRecursively(tree, node.FalseChildNodeIdx, remainingLevels - 1, result);
  }
}

void collectSubtreeLeafNodePathNodeBitOffsetsRecursively(
    const DecisionTree &tree, int64_t nodeIdx, int remainingLevels,
    const std::unordered_map<int64_t, unsigned> &nodeIdxBitOffsets,
    std::vector<std::vector<int64_t>> &result) {
  if (remainingLevels == 0) {
    // subtree leaf nodes create path vector
    result.push_back({ nodeIdxBitOffsets.at(nodeIdx) });
    return;
  }

  const TreeNode &node = tree.at(nodeIdx);
  collectSubtreeLeafNodePathNodeBitOffsetsRecursively(
      tree, node.TrueChildNodeIdx, remainingLevels - 1,
      nodeIdxBitOffsets, result);
  collectSubtreeLeafNodePathNodeBitOffsetsRecursively(
      tree, node.FalseChildNodeIdx, remainingLevels - 1,
      nodeIdxBitOffsets, result);

  // subtree non-leaf nodes add their offsets to all of their own child leafs
  unsigned thisBitOffset = nodeIdxBitOffsets.at(nodeIdx);
  unsigned firstChildLeafPath = result.size() - PowerOf2(remainingLevels);
  for (unsigned i = firstChildLeafPath; i < result.size(); i++) {
    result[i].push_back(thisBitOffset);
  }
}
/*{
  if (remainingLevels == 0) {
    result.push_back({ nodeIdx });
    return;
  }

  const TreeNode &node = tree.at(nodeIdx);
  collectSubtreeLeafNodePathNodeBitOffsetsRecursively(tree, node.TrueChildNodeIdx, remainingLevels - 1, result);
  collectSubtreeLeafNodePathNodeBitOffsetsRecursively(tree, node.FalseChildNodeIdx, remainingLevels - 1, result);

  unsigned firstChildLeafPath = result.size() - PowerOf2(remainingLevels - 1);
  for (unsigned i = firstChildLeafPath; i < result.size(), i++) {
    result[i].push_back(nodeIdx);
  }
}*/

void buildCanonicalConditionVectorVariantsRecursively(
    int subtreeNodes, const std::vector<int64_t> &leafNodePathIdxs,
    std::vector<std::vector<int64_t>> &result) {
  std::vector<int64_t> leafNodeNonPathIdxs;
  for (int i = 0; i < subtreeNodes; i++) {
    //if (leafNodePathIdxs.find(i))
  }
  for (int i = 0; i < subtreeNodes; i++) {
    std::vector<int64_t> canonicalVariant = leafNodePathIdxs; // copy

  }
}


llvm::Value *
emitSubtreeEvaluation(const DecisionTree &tree, int64_t rootNodeIdx,
                      llvm::Function *function,
                      llvm::Value *dataSetPtr, int levels,
                      std::queue<int64_t> &scheduledNodes) {
  using namespace llvm;

  Type *returnTy = Type::getInt64Ty(Ctx);
  int64_t numNodes = TreeNodes(levels);
  int64_t numLeafNodes = PowerOf2(levels - 1);

  std::vector<int64_t> subtreeNodeIdxs;
  subtreeNodeIdxs.reserve(numNodes);
  collectSubtreeNodeIdxsRecursively(tree, rootNodeIdx, levels - 1, subtreeNodeIdxs);
  assert(subtreeNodeIdxs.size() == numNodes);

  std::unordered_map<int64_t, unsigned> subtreeNodeIdxBitOffsets;
  subtreeNodeIdxBitOffsets.reserve(numNodes);

  Value *conditionVector = Builder.CreateAlloca(returnTy);
  for (unsigned bitOffset = 0; bitOffset < numNodes; bitOffset++) {
    int64_t nodeIdx = subtreeNodeIdxs[bitOffset];

    // remember nodeIdx at bit offset
    subtreeNodeIdxBitOffsets[nodeIdx] = bitOffset;

    const TreeNode &node = tree.at(nodeIdx);
    Value *comparisonResult = emitSingleNodeEvaluaton(node, dataSetPtr);
    Value *vectorBit = Builder.CreateShl(comparisonResult, APInt(6, bitOffset));

    Value *conditionVectorOld = Builder.CreateLoad(conditionVector);
    Value *conditionVectorNew = Builder.CreateOr(conditionVectorOld, vectorBit);
    Builder.CreateStore(conditionVectorNew, conditionVector);
  }

  auto *defaultBB = llvm::BasicBlock::Create(Ctx, "switch_default", function);
  Value *conditionVectorVal = Builder.CreateLoad(conditionVector);

  auto *switchInst = SwitchInst::Create(conditionVectorVal, defaultBB, PowerOf2(numNodes - 1));

  std::vector<std::vector<int64_t>> leafNodePathsNodeBitOffsets;
  leafNodePathsNodeBitOffsets.reserve(numLeafNodes);
  collectSubtreeLeafNodePathNodeBitOffsetsRecursively(
      tree, rootNodeIdx, levels - 1, subtreeNodeIdxBitOffsets,
      leafNodePathsNodeBitOffsets);
  assert(leafNodePathsNodeBitOffsets.size() == numLeafNodes);

  // iterate leaf numNodes
  for (std::vector<int64_t> &leafNodePathNodeBitOffsets : leafNodePathsNodeBitOffsets) {
    int64_t leafNodeIdx = subtreeNodeIdxs[leafNodePathNodeBitOffsets[0]];

    std::string nodeBBLabel = "switch_node_" + std::to_string(leafNodeIdx);
    auto *nodeBB = llvm::BasicBlock::Create(Ctx, nodeBBLabel, function);

    std::vector<std::vector<int64_t>> canonicalVariants;
    buildCanonicalConditionVectorVariantsRecursively(numNodes, leafNodePathNodeBitOffsets, canonicalVariants);

    //Constant *caseVal;
    //switchInst->addCase(caseVal, nodeBB);
  }
}

int64_t loadEvaluators(const DecisionTree &tree, int treeDepth,
                       int nodeLevelsPerFunction) {
  using namespace llvm;

  // load module from cache
  TheCompiler->submitModule(std::move(TheModule));

  // figure out which evaluator functions we expect and collect them
  std::string nameStub = "nodeEvaluator_";

  int evaluatorDepth =
      ((treeDepth + nodeLevelsPerFunction - 1) / nodeLevelsPerFunction);

  for (int level = 0; level < treeDepth; level += nodeLevelsPerFunction) {
    int64_t firstNodeIdxOnLevel = TreeNodes(level);
    int64_t numNodesOnLevel = PowerOf2(level);

    for (int64_t offset = 0; offset < numNodesOnLevel; offset++) {
      int64_t nodeIdx = firstNodeIdxOnLevel + offset;

      std::string nodeFunctionName = nameStub + std::to_string(nodeIdx);

      compiledNodeEvaluators[nodeIdx] =
          TheCompiler->getEvaluatorFnPtr(nodeFunctionName);
    }
  }

  return compiledNodeEvaluators.size();
}

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
    Value *nextNodeIdx = emitSubtreeEvaluation(
        tree, nodeIdx, evalFn, dataSetPtr, nodeLevelsPerFunction,
        scheduledNodes);

    Builder.CreateRet(nextNodeIdx);
    llvm::verifyFunction(*evalFn);

    processedNodes.push_front(nodeIdx);
    scheduledNodes.pop();
  }

  // llvm::outs() << "We just constructed this LLVM module:\n\n";
  // llvm::outs() << *TheModule.get() << "\n\n";

  // submit module for jit compilation
  {
    printf("\nCompiling...");
    fflush(stdout);

    using namespace std::chrono;
    auto start = high_resolution_clock::now();

    TheCompiler->submitModule(std::move(TheModule));

    auto end = high_resolution_clock::now();
    auto dur = duration_cast<seconds>(end - start);

    printf(" took %lld seconds", dur.count());
    fflush(stdout);
  }

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

int64_t getNumCompiledEvaluators(int treeDepth, int compiledFunctionDepth) {
  int64_t expectedEvaluators = 0;
  int evaluatorDepth =
      ((treeDepth + compiledFunctionDepth - 1) / compiledFunctionDepth);

  for (int i = 0; i < evaluatorDepth; i++)
    expectedEvaluators += PowerOf2(compiledFunctionDepth * i);

  return expectedEvaluators;
}

int64_t
computeLeafNodeIdxForDataSetCompiled(const DecisionTree &tree,
                                     const std::vector<float> &dataSet) {
  int64_t treeNodeIdx = 0;

  while (!tree.at(treeNodeIdx).isLeaf()) {
    auto compiledEvaluator = compiledNodeEvaluators[treeNodeIdx];
    treeNodeIdx = compiledEvaluator(dataSet.data());
  }

  return treeNodeIdx;
}