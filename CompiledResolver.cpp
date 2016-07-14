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
  int treeDepth = Log2(DecisionTree.size() + 1);

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

int64_t CompiledResolver::getNumCompiledEvaluators(int nodeLevelsPerFunction) {
  int64_t expectedEvaluators = 0;

  int treeDepth = Log2(DecisionTree.size() + 1);
  for (int level = 0; level < treeDepth; level += nodeLevelsPerFunction)
    expectedEvaluators += PowerOf2(level);

  return expectedEvaluators;
}

CompiledResolver::SubtreeEvals_t
CompiledResolver::loadEvaluators(int nodeLevelsPerFunction,
                                 std::string objFileName) {
  int64_t evaluators = getNumCompiledEvaluators(nodeLevelsPerFunction);

  printf("Loading %lld evaluators for %lu nodes from file %s\n\n", evaluators,
         DecisionTree.size(), objFileName.c_str());
  fflush(stdout);

  // load module from cache
  TheCompiler->submitModule(std::move(TheModule));

  return collectEvaluatorFunctions(nodeLevelsPerFunction, "nodeEvaluator_");
}

CompiledResolver::SubtreeEvals_t
CompiledResolver::compileEvaluators(int nodeLevelsPerFunction,
                                    int nodeLevelsPerSwitch,
                                    std::string objFileName) {
  int64_t evaluators = getNumCompiledEvaluators(nodeLevelsPerFunction);

  printf("Generating %lld evaluators for %lu nodes and cache it in file %s",
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
    int64_t firstNodeIdxOnLevel = TreeNodes(level);
    int64_t numNodesOnLevel = PowerOf2(level);

    for (int64_t offset = 0; offset < numNodesOnLevel; offset++) {
      int64_t nodeIdx = firstNodeIdxOnLevel + offset;

      std::string name = functionNameStub + std::to_string(nodeIdx);
      evaluators[nodeIdx] = TheCompiler->getEvaluatorFnPtr(name);
    }
  }

  assert(evaluators.size() == getNumCompiledEvaluators(nodeLevelsPerFunction));
  return evaluators;
}

void CompiledResolver::emitSubtreeEvaluators(int subtreeLevels,
                                             int switchLevels,
                                             const std::string &nameStub) {
  assert(subtreeLevels % switchLevels == 0);

  int treeDepth = Log2(DecisionTree.size() + 1);
  assert(treeDepth % subtreeLevels == 0);

  for (int level = 0; level < treeDepth; level += subtreeLevels) {
    int64_t firstIdxOnLevel = TreeNodes(level);
    int64_t firstIdxOnNextLevel = TreeNodes(level + 1);

    for (int64_t nodeIdx = firstIdxOnLevel; nodeIdx < firstIdxOnNextLevel;
         nodeIdx++) {
      const TreeNode &node = DecisionTree.at(nodeIdx);

      std::string name = nameStub + std::to_string(nodeIdx);
      llvm::Function *evalFn = emitFunctionDeclaration(move(name));
      llvm::Value *dataSetPtr = &*evalFn->arg_begin();

      Builder.SetInsertPoint(llvm::BasicBlock::Create(Ctx, "entry", evalFn));
      auto *entryBB = Builder.GetInsertBlock();

      llvm::Value *nextNodeIdx = emitSubtreeSwitchesRecursively(
          nodeIdx, switchLevels, evalFn, entryBB, dataSetPtr,
          subtreeLevels / switchLevels - 1);

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
CompiledResolver::emitSingleNodeEvaluaton(const TreeNode &node,
                                          llvm::Value *dataSetPtr) {
  using namespace llvm;

  Value *dataSetFeaturePtr =
      Builder.CreateConstGEP1_32(dataSetPtr, node.DataSetFeatureIdx);
  Value *dataSetFeatureVal = Builder.CreateLoad(dataSetFeaturePtr);

  Value *comparableFeatureVal = emitOperator(node.Op, dataSetFeatureVal);
  return emitComparison(node.Comp, node.Bias, comparableFeatureVal);
}

llvm::Value *CompiledResolver::emitOperator(OperationType op,
                                            llvm::Value *value) {
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

llvm::Function *CompiledResolver::getUnaryIntrinsic(llvm::Intrinsic::ID id,
                                                    llvm::Type *opTy) {
  return llvm::Intrinsic::getDeclaration(TheModule.get(), id, {opTy});
}

llvm::Value *CompiledResolver::emitComparison(ComparatorType comp, float bias,
                                              llvm::Value *value) {
  llvm::Constant *biasConst = llvm::ConstantFP::get(value->getType(), bias);
  switch (comp) {
  case ComparatorType::LessThan:
    return Builder.CreateFCmpOLT(value, biasConst);

  case ComparatorType::GreaterThan:
    return Builder.CreateFCmpOGT(value, biasConst);
  }
}

llvm::Value *CompiledResolver::emitSubtreeSwitchesRecursively(
    int64_t switchRootNodeIdx, int switchLevels, llvm::Function *function,
    llvm::BasicBlock *switchBB, llvm::Value *dataSetPtr, int nestedSwitches) {
  using namespace llvm;
  Type *returnTy = Type::getInt64Ty(Ctx);
  int64_t numNodes = TreeNodes(switchLevels);
  int64_t numContinuations = PowerOf2(switchLevels);

  std::unordered_map<int64_t, unsigned> subtreeNodeIdxBitOffsets;
  subtreeNodeIdxBitOffsets.reserve(numNodes);

  llvm::Value *conditionVector =
      emitComputeConditionVector(switchRootNodeIdx, switchLevels, dataSetPtr,
                                 numNodes, subtreeNodeIdxBitOffsets);

  std::string returnBBLabel =
      "return_switch_node_" + std::to_string(switchRootNodeIdx);
  auto *returnBB = BasicBlock::Create(Ctx, returnBBLabel, function);

  std::string evalResultLabel =
      "result_switch_node_" + std::to_string(switchRootNodeIdx);
  Value *evalResult = Builder.CreateAlloca(returnTy, nullptr, evalResultLabel);
  Value *conditionVectorVal = Builder.CreateLoad(conditionVector);

  auto *switchInst = Builder.CreateSwitch(conditionVectorVal, returnBB,
                                          PowerOf2(numNodes - 1));

  using PathBitsMap_t = std::unordered_map<unsigned, bool>;
  using LeafNodePathBitsMap_t = std::pair<int64_t, PathBitsMap_t>;
  std::vector<LeafNodePathBitsMap_t> leafNodePathBitsMaps;

  leafNodePathBitsMaps.reserve(numContinuations);
  buildSubtreeLeafNodePathsBitsMapsRecursively(switchRootNodeIdx, switchLevels,
                                               subtreeNodeIdxBitOffsets,
                                               leafNodePathBitsMaps);
  assert(leafNodePathBitsMaps.size() == numContinuations);

  // iterate leaf nodes
  for (const auto &leafNodeInfo : leafNodePathBitsMaps) {
    int64_t leafNodeIdx = leafNodeInfo.first;
    const PathBitsMap_t &pathBitsMap = leafNodeInfo.second;

    std::string nodeBBLabel = "switch_node_" + std::to_string(leafNodeIdx);
    auto *nodeBB = llvm::BasicBlock::Create(Ctx, nodeBBLabel, function);
    {
      Builder.SetInsertPoint(nodeBB);

      if (nestedSwitches > 0) {
        Value *subSwitchResult = emitSubtreeSwitchesRecursively(
            leafNodeIdx, switchLevels, function, nodeBB, dataSetPtr,
            nestedSwitches - 1);
        Builder.CreateStore(subSwitchResult, evalResult);
      } else {
        Builder.CreateStore(ConstantInt::get(returnTy, leafNodeIdx),
                            evalResult);
      }

      Builder.CreateBr(returnBB);
    }

    unsigned conditionVectorTemplate =
        buildFixedConditionVectorTemplate(pathBitsMap);

    std::vector<unsigned> canonicalVariants;
    buildCanonicalConditionVectorVariants(numNodes, conditionVectorTemplate,
                                          pathBitsMap, canonicalVariants);

    Builder.SetInsertPoint(switchBB);
    IntegerType *conditionVectorTy = IntegerType::get(Ctx, numNodes + 1);
    for (unsigned conditionVector : canonicalVariants) {
      ConstantInt *caseVal =
          ConstantInt::get(conditionVectorTy, conditionVector);
      switchInst->addCase(caseVal, nodeBB);
    }
  }

  Builder.SetInsertPoint(returnBB);
  return Builder.CreateLoad(evalResult);
}

llvm::Value *CompiledResolver::emitComputeConditionVector(
    int64_t rootNodeIdx, int subtreeLevels, llvm::Value *dataSetPtr,
    int64_t numNodes, std::unordered_map<int64_t, unsigned int> &bitOffsets) {
  using namespace llvm;
  Type *returnTy = Type::getInt64Ty(Ctx);

  Value *conditionVector =
      Builder.CreateAlloca(returnTy, nullptr, "conditionVector");
  Builder.CreateStore(ConstantInt::get(returnTy, 0), conditionVector);

  for (unsigned bitOffset = 0; bitOffset < numNodes; bitOffset++) {
    int64_t nodeIdx =
        getNodeIdxForSubtreeBitOffset(rootNodeIdx, subtreeLevels, bitOffset);

    // remember bit offset for each node index
    bitOffsets[nodeIdx] = bitOffset;

    const TreeNode &node = DecisionTree.at(nodeIdx);
    Value *evalResultBit = emitSingleNodeEvaluaton(node, dataSetPtr);
    Value *evalResultInt = Builder.CreateZExt(evalResultBit, returnTy);
    Value *vectorBit = Builder.CreateShl(evalResultInt, APInt(6, bitOffset));

    Value *conditionVectorOld = Builder.CreateLoad(conditionVector);
    Value *conditionVectorNew = Builder.CreateOr(conditionVectorOld, vectorBit);
    Builder.CreateStore(conditionVectorNew, conditionVector);
  }

  return conditionVector;
}

int64_t CompiledResolver::getNodeIdxForSubtreeBitOffset(int64_t subtreeRootIdx,
                                                        int subtreeLevels,
                                                        unsigned bitOffset) {
  int subtreeRootIdxLevel = Log2(subtreeRootIdx + 1);
  int nodeLevelInSubtree = Log2(bitOffset + 1);

  int64_t firstIdxOnRootLevel = TreeNodes(subtreeRootIdxLevel);
  int64_t firstIdxOnNodeLevel =
      TreeNodes(subtreeRootIdxLevel + nodeLevelInSubtree);

  int64_t subtreeRootIdxOffset = subtreeRootIdx - firstIdxOnRootLevel;
  int64_t numSubtreeNodesOnLevel = PowerOf2(nodeLevelInSubtree);
  int64_t firstSubtreeIdxOnNodeLevel =
      firstIdxOnNodeLevel + subtreeRootIdxOffset * numSubtreeNodesOnLevel;

  int nodeOffsetInSubtreeLevel = bitOffset - (PowerOf2(nodeLevelInSubtree) - 1);
  return firstSubtreeIdxOnNodeLevel + nodeOffsetInSubtreeLevel;
}

void CompiledResolver::buildSubtreeLeafNodePathsBitsMapsRecursively(
    int64_t nodeIdx, int remainingLevels,
    const std::unordered_map<int64_t, unsigned> &nodeIdxBitOffsets,
    std::vector<std::pair<int64_t, std::unordered_map<unsigned, bool>>>
        &result) {
  if (remainingLevels == 0) {
    // subtree leaf nodes create empty path maps
    std::unordered_map<unsigned, bool> pathBitsMap;
    result.push_back({nodeIdx, pathBitsMap});
  } else {
    // subtree non-leaf nodes add their offsets to their leafs' path maps
    const TreeNode &node = DecisionTree.at(nodeIdx);
    unsigned thisBitOffset = nodeIdxBitOffsets.at(nodeIdx);
    unsigned numChildLeafPaths = PowerOf2(remainingLevels);
    unsigned numChildsPerCond = numChildLeafPaths / 2;

    {
      buildSubtreeLeafNodePathsBitsMapsRecursively(node.TrueChildNodeIdx,
                                                   remainingLevels - 1,
                                                   nodeIdxBitOffsets, result);

      for (unsigned i = 0; i < numChildsPerCond; i++) {
        result[result.size() - i - 1].second[thisBitOffset] = true;
      }
    }
    {
      buildSubtreeLeafNodePathsBitsMapsRecursively(node.FalseChildNodeIdx,
                                                   remainingLevels - 1,
                                                   nodeIdxBitOffsets, result);

      for (unsigned i = 0; i < numChildsPerCond; i++) {
        result[result.size() - i - 1].second[thisBitOffset] = false;
      }
    }
  }
}

unsigned CompiledResolver::buildFixedConditionVectorTemplate(
    const std::unordered_map<unsigned, bool> &leafNodePathBitsMap) {
  unsigned fixedBitsVector = 0;
  for (const auto &mapEntry : leafNodePathBitsMap) {
    unsigned bit = mapEntry.second ? 1 : 0;
    unsigned vectorBit = bit << mapEntry.first;
    fixedBitsVector |= vectorBit;
  }

  return fixedBitsVector;
}

void CompiledResolver::buildCanonicalConditionVectorVariants(
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
  } else {
    unsigned expectedVariants = PowerOf2(variableBitOffsets.size());
    result.reserve(expectedVariants);

    buildCanonicalConditionVectorVariantsRecursively(
        fixedBitsTemplate, variableBitOffsets, 0, result);

    assert(result.size() == expectedVariants);
  }
}

void CompiledResolver::buildCanonicalConditionVectorVariantsRecursively(
    unsigned conditionVector, const std::vector<unsigned> &variableBitOffsets,
    int bitToVaryIdx, std::vector<unsigned> &result) {
  if (bitToVaryIdx < variableBitOffsets.size()) {
    unsigned bitToVary = variableBitOffsets.at(bitToVaryIdx);
    unsigned vectorTrueBit = 1u << bitToVary;

    // bit must still be in default zero state
    assert((conditionVector & ~vectorTrueBit) == conditionVector);

    // true variation
    buildCanonicalConditionVectorVariantsRecursively(
        conditionVector | vectorTrueBit, variableBitOffsets, bitToVaryIdx + 1,
        result);

    // false variation
    buildCanonicalConditionVectorVariantsRecursively(
        conditionVector, variableBitOffsets, bitToVaryIdx + 1, result);
  } else {
    result.push_back(conditionVector);
  }
}

int64_t CompiledResolver::run(const DecisionTree_t &tree,
                              const DataSet_t &dataSet) {
  assert(&tree == &DecisionTree);

  int64_t idx = 0;
  int64_t firstResultIdx = DecisionTree.size();
  const float *data = dataSet.data();

  while (idx < firstResultIdx) {
    SubtreeEvaluator_f *compiledEvaluator = CompiledEvaluators.at(idx);
    idx = compiledEvaluator(data);
  }

  return idx;
}
