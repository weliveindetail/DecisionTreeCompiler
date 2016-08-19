#pragma once

#include "LegacyDecisionTree.h"

enum class NodeEvaluation_t {
  ContinueZeroLeft = 0,
  ContinueOneRight = 1
};

class DecisionSubtreeRef;

struct DecisionTreeNode {
  uint64_t NodeIdx = NoNodeIdx; // same as index as long as tree is regular

  uint32_t DataSetFeatureIdx;
  float Bias;

  uint64_t TrueChildNodeIdx;
  uint64_t FalseChildNodeIdx;

  bool hasChildForEvaluation(NodeEvaluation_t evaluation) const {
    return (evaluation == NodeEvaluation_t::ContinueZeroLeft)
           ? hasLeftChild()
           : hasRightChild();
  }

  uint64_t getChildIdx(NodeEvaluation_t evaluation) const {
    return (evaluation == NodeEvaluation_t::ContinueZeroLeft)
        ? FalseChildNodeIdx
        : TrueChildNodeIdx;
  }

  bool isLeaf() const {
    return !hasLeftChild() && !hasRightChild();
  }

  bool hasLeftChild() const { return FalseChildNodeIdx != NoNodeIdx; }
  bool hasRightChild() const { return TrueChildNodeIdx != NoNodeIdx; }

  static constexpr uint64_t NoNodeIdx = 0xFFFFFFFFFFFFFFFF;
  static DecisionTreeNode NoNode;
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
  DecisionSubtreeRef() = default;
  DecisionSubtreeRef(DecisionSubtreeRef &&) = default;
  DecisionSubtreeRef(const DecisionSubtreeRef &) = default;
  DecisionSubtreeRef &operator=(DecisionSubtreeRef &&) = default;
  DecisionSubtreeRef &operator=(const DecisionSubtreeRef &) = default;

  DecisionSubtreeRef(const DecisionTree &tree, uint64_t rootIndex, uint8_t levels);

  const DecisionTreeNode& getNode(uint64_t idx) const {
    return Tree->Nodes.at(idx);
  }

  uint8_t getNodeCount() const {
    return (uint8_t)(PowerOf2(Levels) - 1);
  }

  uint8_t getContinuationNodeCount() const {
    return PowerOf2<uint8_t>(Levels);
  }

  std::vector<uint64_t> collectNodeIndices() const;

  const DecisionTree *Tree;
  uint64_t RootIndex;
  uint8_t Levels;

private:
  std::vector<uint64_t> collectNodeIndicesOnSubtreeLevel(uint8_t level) const;
  uint64_t getFirstSubtreeNodeIdxOnSubtreeLevel(uint8_t subtreeLevel) const;

};
