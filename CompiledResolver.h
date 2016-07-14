#pragma once

#include <chrono>
#include <forward_list>
#include <queue>
#include <string>
#include <unordered_map>
#include <utility>

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

#include "DataSet.h"
#include "DecisionTree.h"
#include "SimpleObjectCache.h"
#include "SimpleOrcJit.h"
#include "Utils.h"

class CompiledResolver {
public:
  CompiledResolver(const DecisionTree_t &tree,
                   int dataSetFeatures,
                   int compiledFunctionDepth,
                   int compiledFunctionSwitchDepth);
  ~CompiledResolver();

  int64_t run(const DecisionTree_t &tree,
              const DataSet_t &dataSet);

private:
  llvm::LLVMContext Ctx;
  llvm::IRBuilder<> Builder;
  std::unique_ptr<llvm::Module> TheModule;
  std::unique_ptr<SimpleOrcJit> TheCompiler;

  using compiledNodeEvaluator_f = int64_t(const float *);
  std::unordered_map<int64_t, compiledNodeEvaluator_f *> CompiledNodeEvaluators;

  void setupModule(std::string name);
  llvm::Function *emitFunctionDeclaration(std::string name);
  llvm::Function *getUnaryIntrinsic(llvm::Intrinsic::ID id, llvm::Type *opTy);
  llvm::Value *emitOperator(OperationType op, llvm::Value *value);
  llvm::Value *emitComparison(ComparatorType comp, float bias, llvm::Value *value);
  llvm::Value *emitSingleNodeEvaluaton(const TreeNode &node, llvm::Value *dataSetPtr);
  int64_t getNodeIdxForSubtreeBitOffset(int64_t subtreeRootIdx, int subtreeLevels,
                                        unsigned bitOffset);
  void buildSubtreeLeafNodePathsBitsMapsRecursively(
      const DecisionTree_t &tree, int64_t nodeIdx, int remainingLevels,
      const std::unordered_map<int64_t, unsigned> &nodeIdxBitOffsets,
      std::vector<std::pair<int64_t, std::unordered_map<unsigned, bool>>>
          &result);
  unsigned buildFixedConditionVectorTemplate(
      const std::unordered_map<unsigned, bool> &leafNodePathBitsMap);
  void buildCanonicalConditionVectorVariantsRecursively(
      unsigned conditionVector, const std::vector<unsigned> &variableBitOffsets,
      int bitToVaryIdx, std::vector<unsigned> &result);

  void buildCanonicalConditionVectorVariants(
      int64_t conditionVectorSize, unsigned fixedBitsTemplate,
      const std::unordered_map<unsigned, bool> &leafNodePathBitsMap,
      std::vector<unsigned> &result);

  llvm::Value *emitComputeConditionVector(
      const DecisionTree_t &tree, int64_t rootNodeIdx, int subtreeLevels,
      llvm::Value *dataSetPtr, int64_t numNodes,
      std::unordered_map<int64_t, unsigned int> &bitOffsets);

  llvm::Value *emitSubtreeSwitchesRecursively(
      const DecisionTree_t &tree, int64_t switchRootNodeIdx, int switchLevels,
      llvm::Function *function, llvm::BasicBlock *switchBB,
      llvm::Value *dataSetPtr, int nestedSwitches);
  llvm::Value *emitSubtreeEvaluation(const DecisionTree_t &tree,
                                     int64_t rootNodeIdx, int subtreeLevels,
                                     int switchLevels, llvm::Function *function,
                                     llvm::Value *dataSetPtr);
  int64_t loadEvaluators(const DecisionTree_t &tree, int treeDepth,
                         int nodeLevelsPerFunction);
  int64_t compileEvaluators(const DecisionTree_t &tree, int treeDepth,
                            int nodeLevelsPerFunction, int nodeLevelsPerSwitch);
  int64_t getNumCompiledEvaluators(int treeDepth, int compiledFunctionDepth);
};
