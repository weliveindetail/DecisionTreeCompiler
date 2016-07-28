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

  uint64_t run(const DecisionTree_t &tree, const DataSet_t &dataSet);

private:
  llvm::LLVMContext Ctx;
  llvm::IRBuilder<> Builder;
  std::unique_ptr<llvm::Module> TheModule;
  std::unique_ptr<SimpleOrcJit> TheCompiler;

  const DecisionTree_t &DecisionTree;

  using SubtreeEvaluator_f = uint64_t(const float *);
  using SubtreeEvals_t = std::unordered_map<uint64_t, SubtreeEvaluator_f *>;
  SubtreeEvals_t CompiledEvaluators;

  // 3 tree levels = 7 nodes = 128 switch cases
  // 5 tree levels = 31 nodes = 2.14 mio switch cases
  constexpr static uint8_t MaxSwitchLevels = 3;

  using PathBitsMap_t = std::unordered_map<uint8_t, bool>;
  using SubtreePathsMap_t = std::pair<uint64_t, PathBitsMap_t>;

  uint64_t getNumCompiledEvaluators(uint8_t nodeLevelsPerFunction);

  SubtreeEvals_t loadEvaluators(uint8_t nodeLevelsPerFunction,
                                std::string objFileName);

  SubtreeEvals_t compileEvaluators(uint8_t nodeLevelsPerFunction,
                                   uint8_t nodeLevelsPerSwitch,
                                   std::string objFileName);

  SubtreeEvals_t collectEvaluatorFunctions(int nodeLevelsPerFunction,
                                           std::string nameStub);

  std::vector<uint64_t> collectSubtreeNodeIdxs(
      uint64_t subtreeRootIdx, uint8_t subtreeNodes);

  std::vector<SubtreePathsMap_t> buildSubtreeChildNodePaths(
      uint64_t subtreeRootIdx, uint8_t subtreeLevels);

  void buildSubtreeChildNodePathsRecursively(
      uint64_t nodeIdx, uint8_t remainingLevels,
      const std::unordered_map<uint64_t, uint8_t> &bitOffsets,
      std::vector<SubtreePathsMap_t> &result);

  uint32_t buildFixedBitsConditionVectorTemplate(
      const PathBitsMap_t &subtreePaths);

  std::vector<uint32_t> buildCanonicalConditionVectorVariants(
      uint8_t numNodes, uint32_t fixedBitsTemplate,
      const PathBitsMap_t &subtreePaths);

  void buildCanonicalConditionVectorVariantsRecursively(
      uint32_t conditionVector, const std::vector<uint8_t> &variableBitOffsets,
      uint8_t bitToVaryIdx, std::vector<uint32_t> &result);

  llvm::Function *emitFunctionDeclaration(std::string name);

  llvm::Value *emitNodeLoad(const TreeNode &node, llvm::Value *dataSetPtr);
  llvm::Value *emitNodeCompare(const TreeNode &node, llvm::Value *dataSetFeatureVal);

  llvm::Value *emitComputeConditionVector(
      llvm::Value *dataSetPtr, uint64_t subtreeRootIdx, uint8_t subtreeLevels);

  llvm::Value *emitCollectDataSetValues(
      const std::vector<uint64_t> &nodeIdxs, llvm::Value *dataSetPtr);

  llvm::Value *emitDefineTreeNodeValues(
      const std::vector<uint64_t> &nodeIdxs);

  llvm::Value *emitDefineBitShiftMaskValues(uint64_t numNodes);

  llvm::Value *emitComputeCompareAvx(
      llvm::Value *lhs, llvm::Value *rhs, uint8_t items);

  llvm::Value *emitComputeBitShiftsAvx(
      llvm::Value *avxPackedCmpResults, llvm::Value *bitShiftValues, uint8_t items);

  llvm::Value *emitComputeHorizontalOrAvx(
      llvm::Value *avxPackedInts, uint8_t items);

  llvm::Value *emitSubtreeSwitchesRecursively(
      uint64_t subtreeRootIdx, uint8_t subtreeLevels, llvm::Function *function,
      llvm::BasicBlock *switchBB, llvm::Value *dataSetPtr, uint8_t remainingNestedSwitches);

  llvm::BasicBlock *emitSubtreeSwitchTargetAndRecurse(
      uint64_t nodeIdx, uint8_t subtreeLevels, llvm::Function *function,
      llvm::Value *dataSetPtr, uint8_t remainingNestedSwitches,
      llvm::BasicBlock *returnBB, llvm::Value *evalResultPtr);

  void emitSubtreeSwitchCaseLabels(
      llvm::BasicBlock *switchBB, llvm::SwitchInst *switchInst,
      llvm::BasicBlock *nodeBB,  uint8_t subtreeNodes,
      const CompiledResolver::PathBitsMap_t &pathBitsMap);

  void emitSubtreeEvaluators(uint8_t subtreeLevels, uint8_t switchLevels,
                             const std::string &nameStub);

  static std::unique_ptr<SimpleOrcJit> makeCompiler();
  static std::unique_ptr<llvm::Module> makeModule(std::string name,
                                                  llvm::LLVMContext &ctx);
};
