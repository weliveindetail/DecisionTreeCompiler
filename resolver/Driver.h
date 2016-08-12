#pragma once

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>

#include "DecisionTree.h"

enum class NodeEvaluation_t {
  ContinueZeroLeft,
  ContinueOneRight
};

class DecisionSubtreeRef;

struct DecisionTreeNode {
  uint64_t NodeID; // same as index as long as tree is regular

  uint32_t DataSetFeatureIdx;
  float Bias;

  uint64_t TrueChildNodeIdx;
  uint64_t FalseChildNodeIdx;

  static constexpr uint64_t NoNodeIdx = 0xFFFFFFFFFFFFFFFF;
};

class DecisionTree {
public:
  DecisionSubtreeRef getSubtreeRef(uint64_t rootIndex, uint8_t levels) const;

  uint8_t getLevelForNodeIdx(uint64_t nodeIdx) const {
    return Log2(nodeIdx + 1);
  }

  uint8_t getFirstNodeIdxOnLevel(uint8_t level) const {
    return (uint8_t)(PowerOf2(level) - 1);
  }

  uint8_t Levels;
  std::unordered_map<uint64_t, DecisionTreeNode> Nodes;
};

struct DecisionSubtreeRef {
  DecisionSubtreeRef(const DecisionTree &tree, uint64_t rootIndex, uint8_t levels);

  const DecisionTreeNode& getNode(uint64_t idx) const {
    return Tree.Nodes.at(idx);
  }

  uint8_t getNodeCount() const {
    return (uint8_t)(PowerOf2(Levels) - 1);
  }

  uint8_t getContinuationNodeCount() const {
    return PowerOf2<uint8_t>(Levels);
  }

  std::vector<uint64_t> collectNodeIndices() const;

  const DecisionTree &Tree;
  uint64_t RootIndex;
  uint8_t Levels;

private:
  std::vector<uint64_t> collectNodeIndicesOnSubtreeLevel(uint8_t level) const;
  uint64_t getFirstSubtreeNodeIdxOnSubtreeLevel(uint8_t subtreeLevel) const;

};

struct DecisionTreeEvaluationPathNode {
  DecisionTreeEvaluationPathNode() = default;
  DecisionTreeEvaluationPathNode(DecisionTreeEvaluationPathNode &&) = default;
  DecisionTreeEvaluationPathNode(const DecisionTreeEvaluationPathNode &) = default;
  DecisionTreeEvaluationPathNode &operator=(DecisionTreeEvaluationPathNode &&) = default;
  DecisionTreeEvaluationPathNode &operator=(const DecisionTreeEvaluationPathNode &) = default;

  DecisionTreeEvaluationPathNode(uint64_t treeNodeIndex, NodeEvaluation_t parentEvaluation)
      : TreeNodeIndex(treeNodeIndex), ParentEvaluation(parentEvaluation) {}

  uint64_t TreeNodeIndex;
  NodeEvaluation_t ParentEvaluation;
};

class DecisionTreeEvaluationPath {
  using Data_t = std::vector<DecisionTreeEvaluationPathNode>;

public:
  DecisionTreeEvaluationPath() = default;
  DecisionTreeEvaluationPath(DecisionTreeEvaluationPath &&) = default;
  DecisionTreeEvaluationPath(const DecisionTreeEvaluationPath &) = default;
  DecisionTreeEvaluationPath &operator=(DecisionTreeEvaluationPath &&) = default;
  DecisionTreeEvaluationPath &operator=(const DecisionTreeEvaluationPath &) = default;

  DecisionTreeEvaluationPath(DecisionSubtreeRef subtree)
    : Nodes(subtree.Levels), FirstNonSubtreeIdx(subtree.getNodeCount()) {}

  void initPathTarget(uint64_t continuationNodeIdx, NodeEvaluation_t parentEval) {
    assert(continuationNodeIdx >= FirstNonSubtreeIdx);

    InsertPos = Nodes.rend();
    *InsertPos = DecisionTreeEvaluationPathNode(continuationNodeIdx, parentEval);
  }

  void addParent(uint64_t treeNodeIdx, NodeEvaluation_t parentEval) {
    assert(treeNodeIdx < FirstNonSubtreeIdx);

    --InsertPos;
    *InsertPos = DecisionTreeEvaluationPathNode(treeNodeIdx, parentEval);
  }

  uint64_t getTargetIdx() const {
    return Nodes.back().TreeNodeIndex;
  }

  Data_t Nodes;
  mutable Data_t::reverse_iterator InsertPos;
  const uint64_t FirstNonSubtreeIdx;
};

class DecisionTreeCompiler {
public:
  llvm::LLVMContext Ctx;
  llvm::IRBuilder<> Builder;
  std::unique_ptr<llvm::Module> TheModule;
  //std::unique_ptr<SimpleOrcJit> TheCompiler;

  const DecisionTree_t &DecisionTreeLegacyData;
  const DecisionTree &DecisionTreeData;

  llvm::Value *emitNodeLoad(
      const DecisionTreeNode &node, llvm::Value *dataSetPtr) {
    llvm::Value *dataSetFeaturePtr =
        Builder.CreateConstGEP1_32(dataSetPtr, node.DataSetFeatureIdx);

    return Builder.CreateLoad(dataSetFeaturePtr);
  }

  std::vector<DecisionTreeEvaluationPath> buildSubtreeEvaluationPaths(DecisionSubtreeRef subtree);
  void buildSubtreeEvaluationPathsRecursively(
      DecisionSubtreeRef subtree,
      uint64_t nodeIdx, uint8_t remainingLevels,
      const std::vector<uint64_t> &nodeIdxs,
      std::vector<DecisionTreeEvaluationPath> &result);
};
