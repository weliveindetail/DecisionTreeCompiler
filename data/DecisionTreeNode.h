#pragma once

#include <cstdint>
#include <limits>

class DecisionSubtreeRef;
class DecisionTree;

enum class NodeEvaluation { ContinueZeroLeft = 0, ContinueOneRight = 1 };

struct DecisionTreeNode {
  DecisionTreeNode() = default;
  DecisionTreeNode(DecisionTreeNode &&) = default;
  DecisionTreeNode(const DecisionTreeNode &) = default;
  DecisionTreeNode &operator=(DecisionTreeNode &&) = default;
  DecisionTreeNode &operator=(const DecisionTreeNode &) = default;

  DecisionTreeNode(uint64_t nodeIdx, float bias, uint32_t dataSetFeatureIdx,
                   uint64_t zeroFalseChildIdx, uint64_t oneTrueChildIdx);

  friend bool operator==(const DecisionTreeNode &, const DecisionTreeNode &);
  friend bool operator!=(const DecisionTreeNode &, const DecisionTreeNode &);

  bool isImplicit() const;
  bool isLeaf() const { return !hasLeftChild() && !hasRightChild(); }
  bool hasLeftChild() const { return FalseChildNodeIdx != NoNodeIdx; }
  bool hasRightChild() const { return TrueChildNodeIdx != NoNodeIdx; }

  bool hasChildFor(NodeEvaluation evaluation) const;
  DecisionTreeNode getChildFor(NodeEvaluation evaluation,
                               DecisionSubtreeRef subtree) const;

  uint64_t getIdx() const { return NodeIdx; }
  uint32_t getFeatureIdx() const { return DataSetFeatureIdx; }
  float getFeatureBias() const { return Bias; }

private:
  uint64_t NodeIdx = NoNodeIdx;
  uint64_t TrueChildNodeIdx = NoNodeIdx;
  uint64_t FalseChildNodeIdx = NoNodeIdx;

  uint32_t DataSetFeatureIdx = NoFeatureIdx;
  float Bias = NoBias;

  uint64_t getChildIdxFor(NodeEvaluation evaluation) const;

  static constexpr uint32_t NoFeatureIdx = 0xFFFFFFFF;
  static constexpr uint64_t NoNodeIdx = 0xFFFFFFFFFFFFFFFF;
  static constexpr float NoBias = std::numeric_limits<float>::quiet_NaN();

  friend class DecisionTree;
};
