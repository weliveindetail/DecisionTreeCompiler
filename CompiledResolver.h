#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Value.h>

#include "DataSet.h"
#include "DecisionTree.h"

class SimpleOrcJit;

class CompiledResolver {
public:
  CompiledResolver(const DecisionTree_t &tree, int dataSetFeatures,
                   int compiledFunctionDepth, int compiledFunctionSwitchDepth);
  ~CompiledResolver();

  int64_t run(const DecisionTree_t &tree, const DataSet_t &dataSet);

private:
  llvm::LLVMContext Ctx;
  llvm::IRBuilder<> Builder;
  std::unique_ptr<llvm::Module> TheModule;
  std::unique_ptr<SimpleOrcJit> TheCompiler;

  const DecisionTree_t &DecisionTree;

  using SubtreeEvaluator_f = int64_t(const float *);
  using SubtreeEvals_t = std::unordered_map<int64_t, SubtreeEvaluator_f *>;
  SubtreeEvals_t CompiledEvaluators;

  int64_t getNumCompiledEvaluators(int compiledFunctionDepth);

  SubtreeEvals_t loadEvaluators(int nodeLevelsPerFunction,
                                std::string objFileName);

  SubtreeEvals_t compileEvaluators(int nodeLevelsPerFunction,
                                   int nodeLevelsPerSwitch,
                                   std::string objFileName);

  SubtreeEvals_t collectEvaluatorFunctions(int nodeLevelsPerFunction,
                                           std::string nameStub);

  int64_t getNodeIdxForSubtreeBitOffset(int64_t subtreeRootIdx,
                                        int subtreeLevels, unsigned bitOffset);

  void buildSubtreeLeafNodePathsBitsMapsRecursively(
      int64_t nodeIdx, int remainingLevels,
      const std::unordered_map<int64_t, unsigned> &nodeIdxBitOffsets,
      std::vector<std::pair<int64_t, std::unordered_map<unsigned, bool>>>
          &result);

  unsigned buildFixedConditionVectorTemplate(
      const std::unordered_map<unsigned, bool> &leafNodePathBitsMap);

  void buildCanonicalConditionVectorVariants(
      int64_t conditionVectorSize, unsigned fixedBitsTemplate,
      const std::unordered_map<unsigned, bool> &leafNodePathBitsMap,
      std::vector<unsigned> &result);

  void buildCanonicalConditionVectorVariantsRecursively(
      unsigned conditionVector, const std::vector<unsigned> &variableBitOffsets,
      int bitToVaryIdx, std::vector<unsigned> &result);

  llvm::Function *getUnaryIntrinsic(llvm::Intrinsic::ID id, llvm::Type *opTy);

  llvm::Function *emitFunctionDeclaration(std::string name);
  llvm::Value *emitOperator(OperationType op, llvm::Value *value);
  llvm::Value *emitComparison(ComparatorType comp, float bias,
                              llvm::Value *value);
  llvm::Value *emitSingleNodeEvaluaton(const TreeNode &node,
                                       llvm::Value *dataSetPtr);

  llvm::Value *emitComputeConditionVector(
      int64_t rootNodeIdx, int subtreeLevels, llvm::Value *dataSetPtr,
      int64_t numNodes, std::unordered_map<int64_t, unsigned int> &bitOffsets);

  llvm::Value *emitSubtreeSwitchesRecursively(
      int64_t switchRootNodeIdx, int switchLevels, llvm::Function *function,
      llvm::BasicBlock *switchBB, llvm::Value *dataSetPtr, int nestedSwitches);

  void emitSubtreeEvaluators(int subtreeLevels, int switchLevels,
                             const std::string &nameStub);

  static std::unique_ptr<SimpleOrcJit> makeCompiler();
  static std::unique_ptr<llvm::Module> makeModule(std::string name,
                                                  llvm::LLVMContext &ctx);
};
