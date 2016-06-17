#pragma once

#include <chrono>
#include <forward_list>
#include <utility>
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

void collectSubtreeNodeIdxsLevelwise(
    const DecisionTree &tree, int64_t subtreeRootIdx, int subtreeRootIdxLevel, int levels,
    std::vector<int64_t> &result) {
  int64_t firstIdxOnRootLevel = TreeNodes(subtreeRootIdxLevel);
  int64_t subtreeRootIdxOffset = subtreeRootIdx - firstIdxOnRootLevel;

  for (int level = 0; level < levels; level++) {
    int64_t numSubtreeNodesOnLevel = PowerOf2(level);
    int64_t firstIdxOnLevel = TreeNodes(subtreeRootIdxLevel + level);
    int64_t subtreeIdxOffset = subtreeRootIdxOffset * numSubtreeNodesOnLevel;

    for (int64_t offset = 0; offset < numSubtreeNodesOnLevel; offset++) {
      int64_t nodeIdx = firstIdxOnLevel + subtreeIdxOffset + offset;
      result.push_back(nodeIdx);
    }
  }
}

void buildSubtreeLeafNodePathsBitsMapsRecursively(
    const DecisionTree &tree, int64_t nodeIdx, int remainingLevels,
    const std::unordered_map<int64_t, unsigned> &nodeIdxBitOffsets,
    std::vector<std::pair<int64_t, std::unordered_map<unsigned, bool>>> &result) {
  if (remainingLevels == 0) {
    // subtree leaf nodes create empty path maps
    std::unordered_map<unsigned, bool> pathBitsMap;
    result.push_back({ nodeIdx, pathBitsMap });
  }
  else {
    // subtree non-leaf nodes add their offsets to their leafs' path maps
    const TreeNode &node = tree.at(nodeIdx);
    unsigned thisBitOffset = nodeIdxBitOffsets.at(nodeIdx);
    unsigned numChildLeafPaths = PowerOf2(remainingLevels);
    unsigned numChildsPerCond = numChildLeafPaths / 2;

    {
      buildSubtreeLeafNodePathsBitsMapsRecursively(
          tree, node.TrueChildNodeIdx, remainingLevels - 1,
          nodeIdxBitOffsets, result);

      for (unsigned i = 0; i < numChildsPerCond; i++) {
        result[result.size() - i - 1].second[thisBitOffset] = true;
      }
    }
    {
      buildSubtreeLeafNodePathsBitsMapsRecursively(
          tree, node.FalseChildNodeIdx, remainingLevels - 1,
          nodeIdxBitOffsets, result);

      for (unsigned i = 0; i < numChildsPerCond; i++) {
        result[result.size() - i - 1].second[thisBitOffset] = false;
      }
    }
  }
}

unsigned buildFixedConditionVectorTemplate(
    const std::unordered_map<unsigned, bool> &leafNodePathBitsMap) {
  unsigned fixedBitsVector = 0;
  for (const auto &mapEntry : leafNodePathBitsMap) {
    unsigned bit = mapEntry.second ? 1 : 0;
    unsigned vectorBit = bit << mapEntry.first;
    fixedBitsVector |= vectorBit;
  }

  return fixedBitsVector;
}

void buildCanonicalConditionVectorVariantsRecursively(
    unsigned conditionVector, const std::vector<unsigned> &variableBitOffsets,
    int bitToVaryIdx, std::vector<unsigned> &result) {
  if (bitToVaryIdx < variableBitOffsets.size()) {
    unsigned bitToVary = variableBitOffsets.at(bitToVaryIdx);
    unsigned vectorTrueBit = 1u << bitToVary;

    // bit must still be in default zero state
    assert((conditionVector & ~vectorTrueBit) == conditionVector);

    // true variation
    result.push_back(conditionVector| vectorTrueBit);
    buildCanonicalConditionVectorVariantsRecursively(
        conditionVector | vectorTrueBit, variableBitOffsets, bitToVary + 1, result);

    // false variation
    result.push_back(conditionVector);
    buildCanonicalConditionVectorVariantsRecursively(
        conditionVector, variableBitOffsets, bitToVary + 1, result);
  }
}

void buildCanonicalConditionVectorVariants(
    int64_t conditionVectorSize, unsigned fixedBitsTemplate,
    const std::unordered_map<unsigned, bool> &leafNodePathBitsMap,
    std::vector<unsigned> &result) {
  std::vector<unsigned> variableBitOffsets;
  for (int i = 0; i < conditionVectorSize; i++) {
    if (leafNodePathBitsMap.find(i) == leafNodePathBitsMap.end())
      variableBitOffsets.push_back(i);
  }

  if (variableBitOffsets.empty()) {
    result.push_back(fixedBitsTemplate);
  }
  else {
    unsigned expectedVariants = PowerOf2(variableBitOffsets.size());
    result.reserve(expectedVariants);

    buildCanonicalConditionVectorVariantsRecursively(
        fixedBitsTemplate, variableBitOffsets, 0, result);

    assert(result.size() == expectedVariants);
  }
}

llvm::Value *
emitSubtreeEvaluation(const DecisionTree &tree, int64_t rootNodeIdx,
                      int subtreeRootIdxLevel, int levels,
                      llvm::Function *function,
                      llvm::Value *dataSetPtr) {
  using namespace llvm;

  Type *returnTy = Type::getInt64Ty(Ctx);
  int64_t numNodes = TreeNodes(levels);
  int64_t numLeafNodes = PowerOf2(levels - 1);
  int64_t numNonLeafNodes = TreeNodes(levels - 1);
  assert(numNodes == numLeafNodes + numNonLeafNodes);

  std::vector<int64_t> subtreeNodeIdxs;
  subtreeNodeIdxs.reserve(numNodes);
  //collectSubtreeNodeIdxsRecursively(tree, rootNodeIdx, levels - 1, subtreeNodeIdxs);
  collectSubtreeNodeIdxsLevelwise(tree, rootNodeIdx, subtreeRootIdxLevel, levels, subtreeNodeIdxs);
  assert(subtreeNodeIdxs.size() == numNodes);

  std::unordered_map<int64_t, unsigned> subtreeNodeIdxBitOffsets;
  subtreeNodeIdxBitOffsets.reserve(numNonLeafNodes);

  Value *conditionVector = Builder.CreateAlloca(returnTy, nullptr, "conditionVector");
  for (unsigned bitOffset = 0; bitOffset < numNonLeafNodes; bitOffset++) {
    int64_t nodeIdx = subtreeNodeIdxs[bitOffset];

    // remember nodeIdx at bit offset
    subtreeNodeIdxBitOffsets[nodeIdx] = bitOffset;

    const TreeNode &node = tree.at(nodeIdx);
    Value *evalResultBit = emitSingleNodeEvaluaton(node, dataSetPtr);
    Value *evalResultInt = Builder.CreateZExt(evalResultBit, returnTy);
    Value *vectorBit = Builder.CreateShl(evalResultInt, APInt(6, bitOffset));

    Value *conditionVectorOld = Builder.CreateLoad(conditionVector);
    Value *conditionVectorNew = Builder.CreateOr(conditionVectorOld, vectorBit);
    Builder.CreateStore(conditionVectorNew, conditionVector);
  }

  auto *switchBB = Builder.GetInsertBlock();
  auto *returnBB = BasicBlock::Create(Ctx, "return", function);

  Value *evalResult = Builder.CreateAlloca(returnTy, nullptr, "result");
  Value *conditionVectorVal = Builder.CreateLoad(conditionVector);
  auto *switchInst = Builder.CreateSwitch(conditionVectorVal, returnBB,
                                          PowerOf2(numNodes - 1));

  //auto *switchInst = SwitchInst::Create(conditionVectorVal, defaultBB, PowerOf2(numNodes - 1));

  using PathBitsMap_t = std::unordered_map<unsigned, bool>;
  using LeafNodePathBitsMap_t = std::pair<int64_t, PathBitsMap_t>;
  std::vector<LeafNodePathBitsMap_t> leafNodePathBitsMaps;

  leafNodePathBitsMaps.reserve(numLeafNodes);
  buildSubtreeLeafNodePathsBitsMapsRecursively(
      tree, rootNodeIdx, levels - 1, subtreeNodeIdxBitOffsets,
      leafNodePathBitsMaps);
  assert(leafNodePathBitsMaps.size() == numLeafNodes);

  // iterate leaf nodes
  for (const LeafNodePathBitsMap_t &leafNodePathBitsMap : leafNodePathBitsMaps) {
    int64_t leafNodeIdx = leafNodePathBitsMap.first;
    const PathBitsMap_t &pathBitsMap = leafNodePathBitsMap.second;

    std::string nodeBBLabel = "switch_node_" + std::to_string(leafNodeIdx);
    auto *nodeBB = llvm::BasicBlock::Create(Ctx, nodeBBLabel, function);
    {
      Builder.SetInsertPoint(nodeBB);
      const TreeNode &node = tree.at(leafNodeIdx);

      bool isSubtreeNodeIsTreeLeaf = node.isLeaf();
      if (isSubtreeNodeIsTreeLeaf) {
        Builder.CreateStore(ConstantInt::get(returnTy, leafNodeIdx), evalResult);
      }
      else {
        Value *comparisonResult = emitSingleNodeEvaluaton(node, dataSetPtr);
        Value *nextSubtreeRoot =
            Builder.CreateSelect(comparisonResult,
                                 ConstantInt::get(returnTy, node.TrueChildNodeIdx),
                                 ConstantInt::get(returnTy, node.FalseChildNodeIdx));
        Builder.CreateStore(nextSubtreeRoot, evalResult);
      }
      Builder.CreateBr(returnBB);
    }

    unsigned conditionVectorTemplate = buildFixedConditionVectorTemplate(pathBitsMap);

    std::vector<unsigned> canonicalVariants;
    buildCanonicalConditionVectorVariants(
        numNonLeafNodes, conditionVectorTemplate, pathBitsMap, canonicalVariants);

    Builder.SetInsertPoint(switchBB);
    IntegerType *conditionVectorTy = IntegerType::get(Ctx, numNonLeafNodes + 1);
    for (unsigned conditionVector : canonicalVariants) {
      ConstantInt *caseVal = ConstantInt::get(conditionVectorTy, conditionVector);
      switchInst->addCase(caseVal, nodeBB);
    }
  }

  Builder.SetInsertPoint(returnBB);
  return Builder.CreateLoad(evalResult);
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

int64_t compileEvaluators(const DecisionTree &tree, int treeDepth, int nodeLevelsPerFunction) {
  using namespace llvm;

  std::string nameStub = "nodeEvaluator_";
  std::forward_list<int64_t> processedNodes;

  // figure out which evaluator functions we expect and collect them
  int evaluatorDepth =
      ((treeDepth + nodeLevelsPerFunction - 1) / nodeLevelsPerFunction);

  for (int level = 0; level < treeDepth; level += nodeLevelsPerFunction) {
    int64_t firstNodeIdxOnLevel = TreeNodes(level);
    int64_t numNodesOnLevel = PowerOf2(level);

    for (int64_t offset = 0; offset < numNodesOnLevel; offset++) {
      int64_t nodeIdx = firstNodeIdxOnLevel + offset;
      const TreeNode &node = tree.at(nodeIdx);

      std::string name = nameStub + std::to_string(nodeIdx);
      Function *evalFn = emitFunctionDeclaration(std::move(name));
      Value *dataSetPtr = &*evalFn->arg_begin();

      Builder.SetInsertPoint(llvm::BasicBlock::Create(Ctx, "entry", evalFn));
      Value *nextNodeIdx = emitSubtreeEvaluation(
          tree, nodeIdx, level, nodeLevelsPerFunction, evalFn, dataSetPtr);

      Builder.CreateRet(nextNodeIdx);
      llvm::verifyFunction(*evalFn);

      processedNodes.push_front(nodeIdx);
    }
  }

  llvm::outs() << "We just constructed this LLVM module:\n\n";
  llvm::outs() << *TheModule.get() << "\n\n";

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
    compiledNodeEvaluator_f *compiledEvaluator = compiledNodeEvaluators[treeNodeIdx];
    treeNodeIdx = compiledEvaluator(dataSet.data());
  }

  return treeNodeIdx;
}