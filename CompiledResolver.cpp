#include "CompiledResolver.h"

#include <string>

#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/GenericValue.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/ManagedStatic.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_ostream.h>

#include "SimpleObjectCache.h"
#include "SimpleOrcJit.h"
#include "Utils.h"

void llvm::ObjectCache::anchor() {}

CompiledResolver::CompiledResolver(const DecisionTree_t &tree,
                                   int dataSetFeatures, int functionDepth,
                                   int switchDepth)
    : Builder(Ctx), DecisionTree(tree) {
  llvm::InitializeNativeTarget();
  llvm::InitializeNativeTargetAsmPrinter();
  llvm::InitializeNativeTargetAsmParser();

  assert(isPowerOf2(DecisionTree.size() + 1));
  uint8_t treeDepth = Log2(DecisionTree.size() + 1);

  assert(dataSetFeatures > 0);
  assert(functionDepth > 0 && functionDepth <= treeDepth);
  assert(switchDepth > 0 && switchDepth <= MaxSwitchLevels);

  // current restrictions:
  assert(treeDepth % functionDepth == 0);
  assert(functionDepth % switchDepth == 0);

  std::string cachedTreeFile = makeTreeFileName(treeDepth, dataSetFeatures);
  std::string cachedObjFile =
      makeObjFileName(treeDepth, dataSetFeatures, functionDepth, switchDepth);

  bool inCache = isFileInCache(cachedTreeFile) && isFileInCache(cachedObjFile);

  TheModule = makeModule(cachedObjFile, Ctx);
  TheCompiler = makeCompiler();

  CompiledEvaluators =
      (inCache) ? loadEvaluators(functionDepth, cachedObjFile)
                : compileEvaluators(functionDepth, switchDepth, cachedObjFile);
}

CompiledResolver::~CompiledResolver() { llvm::llvm_shutdown(); }

std::unique_ptr<llvm::Module>
CompiledResolver::makeModule(std::string name, llvm::LLVMContext &ctx) {
  auto M = std::make_unique<llvm::Module>("file:" + name, ctx);

  auto *targetMachine = llvm::EngineBuilder().selectTarget();
  M->setDataLayout(targetMachine->createDataLayout());

  return M;
}

std::unique_ptr<SimpleOrcJit> CompiledResolver::makeCompiler() {
  auto targetMachine = llvm::EngineBuilder().selectTarget();
  auto cache = std::make_unique<SimpleObjectCache>();

  return std::make_unique<SimpleOrcJit>(*targetMachine, std::move(cache));
}

uint64_t CompiledResolver::getNumCompiledEvaluators(uint8_t nodeLevelsPerFunction) {
  uint64_t expectedEvaluators = 0;

  uint8_t treeDepth = Log2(DecisionTree.size() + 1);
  for (uint8_t level = 0; level < treeDepth; level += nodeLevelsPerFunction)
    expectedEvaluators += PowerOf2(level);

  return expectedEvaluators;
}

CompiledResolver::SubtreeEvals_t
CompiledResolver::loadEvaluators(uint8_t nodeLevelsPerFunction,
                                 std::string objFileName) {
  uint64_t evaluators = getNumCompiledEvaluators(nodeLevelsPerFunction);

  printf("Loading %llu evaluators for %lu nodes from file %s\n\n", evaluators,
         DecisionTree.size(), objFileName.c_str());
  fflush(stdout);

  // load module from cache
  TheCompiler->loadModuleFromCache(std::move(TheModule));

  return collectEvaluatorFunctions(nodeLevelsPerFunction, "nodeEvaluator_");
}

CompiledResolver::SubtreeEvals_t
CompiledResolver::compileEvaluators(uint8_t nodeLevelsPerFunction,
                                    uint8_t nodeLevelsPerSwitch,
                                    std::string objFileName) {
  uint64_t evaluators = getNumCompiledEvaluators(nodeLevelsPerFunction);

  printf("Generating %llu evaluators for %lu nodes and cache it in file %s",
         evaluators, DecisionTree.size(), objFileName.c_str());

  std::string nameStub = "nodeEvaluator_";

  {
    printf("\nComposing...");
    fflush(stdout);

    using namespace std::chrono;
    auto start = high_resolution_clock::now();

    emitSubtreeEvaluators(nodeLevelsPerFunction, nodeLevelsPerSwitch, nameStub);

    auto end = high_resolution_clock::now();
    auto dur = duration_cast<seconds>(end - start);

    printf(" took %lld seconds", dur.count());
    fflush(stdout);
  }

#ifndef NDEBUG
  llvm::outs() << "\n\nWe just constructed this LLVM module:\n\n";
  llvm::outs() << *TheModule.get() << "\n\n";
#endif

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

  printf("\nCollecting...");
  fflush(stdout);

  return collectEvaluatorFunctions(nodeLevelsPerFunction, nameStub);
}

CompiledResolver::SubtreeEvals_t
CompiledResolver::collectEvaluatorFunctions(int nodeLevelsPerFunction,
                                            std::string functionNameStub) {
  SubtreeEvals_t evaluators;
  int treeDepth = Log2(DecisionTree.size() + 1);

  for (int level = 0; level < treeDepth; level += nodeLevelsPerFunction) {
    uint64_t firstNodeIdxOnLevel = TreeNodes(level);
    uint64_t numNodesOnLevel = PowerOf2(level);

    for (uint64_t offset = 0; offset < numNodesOnLevel; offset++) {
      uint64_t nodeIdx = firstNodeIdxOnLevel + offset;

      std::string name = functionNameStub + std::to_string(nodeIdx);
      evaluators[nodeIdx] = TheCompiler->getFnPtr<SubtreeEvaluator_f>(name);
    }
  }

  assert(evaluators.size() == getNumCompiledEvaluators(nodeLevelsPerFunction));
  return evaluators;
}

void CompiledResolver::emitSubtreeEvaluators(uint8_t subtreeLevels,
                                             uint8_t switchLevels,
                                             const std::string &nameStub) {
  uint8_t treeDepth = Log2(DecisionTree.size() + 1);

  for (uint8_t level = 0; level < treeDepth; level += subtreeLevels) {
    uint64_t firstIdxOnLevel = TreeNodes(level);
    uint64_t firstIdxOnNextLevel = TreeNodes(level + 1);

    for (uint64_t nodeIdx = firstIdxOnLevel; nodeIdx < firstIdxOnNextLevel;
         nodeIdx++) {
      const TreeNode &node = DecisionTree.at(nodeIdx);

      std::string name = nameStub + std::to_string(nodeIdx);
      llvm::Function *evalFn = emitFunctionDeclaration(move(name));
      llvm::Value *dataSetPtr = &*evalFn->arg_begin();

      Builder.SetInsertPoint(llvm::BasicBlock::Create(Ctx, "entry", evalFn));
      auto *entryBB = Builder.GetInsertBlock();

      int numNestedSwitches = subtreeLevels / switchLevels - 1;
      llvm::Value *nextNodeIdx = emitSubtreeSwitchesRecursively(
          nodeIdx, switchLevels, evalFn, entryBB, dataSetPtr,
          (uint8_t)numNestedSwitches);

      Builder.CreateRet(nextNodeIdx);
      verifyFunction(*evalFn);
    }
  }
}

llvm::Function *CompiledResolver::emitFunctionDeclaration(std::string name) {
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

llvm::Value *
CompiledResolver::emitNodeLoad(const TreeNode &node,
                               llvm::Value *dataSetPtr) {
  llvm::Value *dataSetFeaturePtr =
      Builder.CreateConstGEP1_32(dataSetPtr, node.DataSetFeatureIdx);

  return Builder.CreateLoad(dataSetFeaturePtr);
}

llvm::Value *
CompiledResolver::emitNodeCompare(const TreeNode &node,
                                  llvm::Value *dataSetFeatureVal) {
  llvm::Constant *bias = llvm::ConstantFP::get(dataSetFeatureVal->getType(), node.Bias);

  return Builder.CreateFCmpOGT(dataSetFeatureVal, bias);
}

llvm::Value *CompiledResolver::emitSubtreeSwitchesRecursively(
    uint64_t switchRootNodeIdx, uint8_t switchLevels, llvm::Function *function,
    llvm::BasicBlock *switchBB, llvm::Value *dataSetPtr, uint8_t remainingNestedSwitches) {
  using namespace llvm;
  Type *returnTy = Type::getInt64Ty(Ctx);
  auto numNodes = TreeNodes<uint8_t>(switchLevels);
  auto numContinuations = PowerOf2<uint8_t>(switchLevels);

  std::unordered_map<uint64_t, uint8_t> subtreeNodeIdxBitOffsets;
  subtreeNodeIdxBitOffsets.reserve(numNodes);

  Value *conditionVector =
      emitComputeConditionVector(switchRootNodeIdx, dataSetPtr,
                                 numNodes, subtreeNodeIdxBitOffsets);

  std::string returnBBLabel =
      "switch" + std::to_string(switchRootNodeIdx) + "_return";
  auto *returnBB = BasicBlock::Create(Ctx, returnBBLabel, function);

  std::string defaultBBLabel =
      "switch" + std::to_string(switchRootNodeIdx) + "_default";
  auto *defaultBB = BasicBlock::Create(Ctx, defaultBBLabel, function);

  std::string evalResultLabel =
      "switch_" + std::to_string(switchRootNodeIdx) + "_value";
  Value *evalResult = Builder.CreateAlloca(returnTy, nullptr, evalResultLabel);
  Builder.CreateStore(ConstantInt::get(returnTy, 0), evalResult);

  Value *conditionVectorVal = Builder.CreateLoad(conditionVector);

  auto *switchInst = Builder.CreateSwitch(conditionVectorVal, defaultBB,
                                          PowerOf2<uint32_t>(numNodes - 1));

  std::vector<LeafNodePathBitsMap_t> leafNodePathBitsMaps;

  leafNodePathBitsMaps.reserve(numContinuations);
  buildSubtreeLeafNodePathsBitsMapsRecursively(switchRootNodeIdx, switchLevels,
                                               subtreeNodeIdxBitOffsets,
                                               leafNodePathBitsMaps);
  assert(leafNodePathBitsMaps.size() == numContinuations);

  // iterate leaf nodes
  BasicBlock *nodeBB;
  for (const auto &leafNodeInfo : leafNodePathBitsMaps) {
    uint64_t leafNodeIdx = leafNodeInfo.first;
    const PathBitsMap_t &pathBitsMap = leafNodeInfo.second;

    std::string nodeBBLabel = "n" + std::to_string(leafNodeIdx);
    nodeBB = BasicBlock::Create(Ctx, nodeBBLabel, function);
    {
      Builder.SetInsertPoint(nodeBB);

      if (remainingNestedSwitches > 0) {
        Value *subSwitchResult = emitSubtreeSwitchesRecursively(
            leafNodeIdx, switchLevels, function, nodeBB, dataSetPtr,
            remainingNestedSwitches - 1);
        Builder.CreateStore(subSwitchResult, evalResult);
      } else {
        Builder.CreateStore(ConstantInt::get(returnTy, leafNodeIdx),
                            evalResult);
      }

      Builder.CreateBr(returnBB);
    }

    uint32_t conditionVectorTemplate =
        buildFixedBitsConditionVectorTemplate(pathBitsMap);

    std::vector<uint32_t> canonicalVariants;
    buildCanonicalConditionVectorVariants(numNodes, conditionVectorTemplate,
                                          pathBitsMap, canonicalVariants);

    Builder.SetInsertPoint(switchBB);
    IntegerType *switchTy = IntegerType::get(Ctx, numNodes + 1);
    for (uint32_t variant : canonicalVariants) {
      ConstantInt *caseVal = ConstantInt::get(switchTy, variant);
      switchInst->addCase(caseVal, nodeBB);
    }
  }

  defaultBB->moveAfter(nodeBB);
  returnBB->moveAfter(defaultBB);

  Builder.SetInsertPoint(defaultBB);
  Builder.CreateBr(returnBB);

  Builder.SetInsertPoint(returnBB);
  return Builder.CreateLoad(evalResult);
}

llvm::Value *CompiledResolver::emitComputeConditionVector(
    uint64_t rootNodeIdx, llvm::Value *dataSetPtr, uint8_t numNodes,
    std::unordered_map<uint64_t, uint8_t> &bitOffsets) {
  using namespace llvm;
  IntegerType *switchTy = IntegerType::get(Ctx, numNodes + 1);

  Type *featureValTy = Type::getFloatTy(Ctx);
  ConstantInt *numNodesConst = ConstantInt::get(switchTy, numNodes);

  std::unordered_map<uint8_t, uint64_t> nodeIdxs;

  // remember bit offset for each node index and vice versa
  for (uint8_t bitOffset = 0; bitOffset < numNodes; bitOffset++) {
    uint64_t nodeIdx =
        getNodeIdxForSubtreeBitOffset(rootNodeIdx, bitOffset);

    bitOffsets[nodeIdx] = bitOffset;
    nodeIdxs[bitOffset] = nodeIdx;
  }

  Value *featureValues = Builder.CreateAlloca(
      featureValTy, numNodesConst, "featureValues");

  // load feature values for all nodes in the subtree
  for (uint8_t bitOffset = 0; bitOffset < numNodes; bitOffset++) {
    const TreeNode &node = DecisionTree.at(nodeIdxs[bitOffset]);
    Value *dataSetFeatureVal = emitNodeLoad(node, dataSetPtr);

    llvm::Value *featureValuePtr =
        Builder.CreateConstGEP1_32(featureValues, bitOffset);

    Builder.CreateStore(dataSetFeatureVal, featureValuePtr);
  }

  Value *conditionVector =
      Builder.CreateAlloca(switchTy, nullptr, "conditionVector");
  Builder.CreateStore(ConstantInt::get(switchTy, 0), conditionVector);

  // compare feature values for all nodes in the subtree
  for (uint8_t bitOffset = 0; bitOffset < numNodes; bitOffset++) {
    const TreeNode &node = DecisionTree.at(nodeIdxs[bitOffset]);

    llvm::Value *featureValuePtr =
        Builder.CreateConstGEP1_32(featureValues, bitOffset);

    Value *featureVal = Builder.CreateLoad(featureValuePtr);
    Value *evalResultBit = emitNodeCompare(node, featureVal);
    Value *evalResultInt = Builder.CreateZExt(evalResultBit, switchTy);
    Value *vectorBit = Builder.CreateShl(evalResultInt, APInt(8, bitOffset));

    Value *conditionVectorOld = Builder.CreateLoad(conditionVector);
    Value *conditionVectorNew = Builder.CreateOr(conditionVectorOld, vectorBit);
    Builder.CreateStore(conditionVectorNew, conditionVector);
  }

  return conditionVector;
}

uint64_t CompiledResolver::getNodeIdxForSubtreeBitOffset(uint64_t subtreeRootIdx,
                                                         uint32_t bitOffset) {
  int subtreeRootIdxLevel = Log2(subtreeRootIdx + 1);
  int nodeLevelInSubtree = Log2(bitOffset + 1);

  uint64_t firstIdxOnRootLevel = TreeNodes(subtreeRootIdxLevel);
  uint64_t firstIdxOnNodeLevel =
      TreeNodes(subtreeRootIdxLevel + nodeLevelInSubtree);

  uint64_t subtreeRootIdxOffset = subtreeRootIdx - firstIdxOnRootLevel;
  uint64_t numSubtreeNodesOnLevel = PowerOf2(nodeLevelInSubtree);
  uint64_t firstSubtreeIdxOnNodeLevel =
      firstIdxOnNodeLevel + subtreeRootIdxOffset * numSubtreeNodesOnLevel;

  uint32_t nodeOffsetInSubtreeLevel = bitOffset - (PowerOf2<uint32_t>(nodeLevelInSubtree) - 1);
  return firstSubtreeIdxOnNodeLevel + nodeOffsetInSubtreeLevel;
}

void CompiledResolver::buildSubtreeLeafNodePathsBitsMapsRecursively(
    uint64_t nodeIdx, uint8_t remainingLevels,
    const std::unordered_map<uint64_t, uint8_t> &nodeIdxBitOffsets,
    std::vector<LeafNodePathBitsMap_t> &result) {
  if (remainingLevels == 0) {
    // subtree leaf nodes create empty path maps
    std::unordered_map<uint8_t, bool> pathBitsMap;
    result.push_back({nodeIdx, pathBitsMap});
  } else {
    // subtree non-leaf nodes add their offsets to their leafs' path maps
    const TreeNode &node = DecisionTree.at(nodeIdx);
    uint8_t thisBitOffset = nodeIdxBitOffsets.at(nodeIdx);
    uint8_t numChildLeafPaths = PowerOf2<uint8_t>(remainingLevels);
    uint8_t numChildsPerCond = numChildLeafPaths / 2;

    {
      buildSubtreeLeafNodePathsBitsMapsRecursively(node.TrueChildNodeIdx,
                                                   remainingLevels - 1,
                                                   nodeIdxBitOffsets, result);

      for (uint8_t i = 0; i < numChildsPerCond; i++) {
        result[result.size() - i - 1].second[thisBitOffset] = true;
      }
    }
    {
      buildSubtreeLeafNodePathsBitsMapsRecursively(node.FalseChildNodeIdx,
                                                   remainingLevels - 1,
                                                   nodeIdxBitOffsets, result);

      for (uint8_t i = 0; i < numChildsPerCond; i++) {
        result[result.size() - i - 1].second[thisBitOffset] = false;
      }
    }
  }
}

uint32_t CompiledResolver::buildFixedBitsConditionVectorTemplate(
    const PathBitsMap_t &leafNodePathBitsMap) {
  uint32_t fixedBitsVector = 0;
  for (const auto &mapEntry : leafNodePathBitsMap) {
    uint32_t bit = mapEntry.second ? 1 : 0;
    uint32_t vectorBit = bit << mapEntry.first;
    fixedBitsVector |= vectorBit;
  }

  return fixedBitsVector;
}

void CompiledResolver::buildCanonicalConditionVectorVariants(
    uint8_t numNodes, uint32_t fixedBitsTemplate,
    const PathBitsMap_t &leafNodePathBitsMap,
    std::vector<uint32_t> &result) {
  std::vector<uint8_t> variableBitOffsets;
  for (uint8_t bitOffset = 0; bitOffset < numNodes; bitOffset++) {
    if (leafNodePathBitsMap.find(bitOffset) == leafNodePathBitsMap.end())
      variableBitOffsets.push_back(bitOffset);
  }

  if (variableBitOffsets.empty()) {
    result.push_back(fixedBitsTemplate);
  } else {
    auto expectedVariants = PowerOf2<uint32_t>(variableBitOffsets.size());
    result.reserve(expectedVariants);

    buildCanonicalConditionVectorVariantsRecursively(
        fixedBitsTemplate, variableBitOffsets, 0, result);

    assert(result.size() == expectedVariants);
  }
}

void CompiledResolver::buildCanonicalConditionVectorVariantsRecursively(
    uint32_t conditionVector, const std::vector<uint8_t> &variableBitOffsets,
    uint8_t bitToVaryIdx, std::vector<uint32_t> &result) {
  if (bitToVaryIdx < variableBitOffsets.size()) {
    uint8_t bitToVary = variableBitOffsets.at(bitToVaryIdx);
    uint32_t vectorTrueBit = 1u << bitToVary;

    // bit must still be in default zero state
    assert((conditionVector & ~vectorTrueBit) == conditionVector);
    auto nextBitIdx = (uint8_t)(bitToVaryIdx + 1);

    // true variation
    buildCanonicalConditionVectorVariantsRecursively(
        conditionVector | vectorTrueBit, variableBitOffsets, nextBitIdx,
        result);

    // false variation
    buildCanonicalConditionVectorVariantsRecursively(
        conditionVector, variableBitOffsets, nextBitIdx, result);
  } else {
    result.push_back(conditionVector);
  }
}

uint64_t CompiledResolver::run(const DecisionTree_t &tree,
                               const DataSet_t &dataSet) {
  assert(&tree == &DecisionTree);

  uint64_t idx = 0;
  uint64_t firstResultIdx = DecisionTree.size();
  const float *data = dataSet.data();

  while (idx < firstResultIdx) {
    SubtreeEvaluator_f *compiledEvaluator = CompiledEvaluators.at(idx);
    idx = compiledEvaluator(data);
  }

  return idx;
}
