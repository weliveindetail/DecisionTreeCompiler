#pragma once

#include <cassert>
#include <unordered_map>

#include "Utils.h"

enum class OperationType { Bypass, Sqrt, Ln };

enum class ComparatorType { LessThan, GreaterThan };

struct TreeNode {
  TreeNode() = default;
  ~TreeNode() = default;

  TreeNode(float bias, OperationType op, ComparatorType comp, int featureIdx)
      : Bias(bias), Op(op), Comp(comp), DataSetFeatureIdx(featureIdx) {}

  int64_t getFalseChildIdx() const { return FalseChildNodesIdx; }

  int64_t getTrueChildIdx() const { return TrueChildNodesIdx; }

  bool isLeaf() const {
    assert((getFalseChildIdx() == 0) == (getTrueChildIdx() == 0) &&
           "There must either both or no child nodes");
    return getFalseChildIdx() == 0;
  }

  float Bias;
  OperationType Op;
  ComparatorType Comp;
  int DataSetFeatureIdx;
  int64_t TrueChildNodesIdx = 0;
  int64_t FalseChildNodesIdx = 0;
};

using DecisionTree = std::unordered_map<int64_t, TreeNode>;

// for expected input range [0, 1)
float makeBalancedBias(OperationType op) {
  switch (op) {
  case OperationType::Bypass:
    return 0.5f;
  case OperationType::Sqrt:
    return std::sqrtf(0.5f);
  case OperationType::Ln:
    return std::log(0.5f);
  };
}

template <int DataSetFeatures_> TreeNode makeDecisionTreeNode() {
  auto op = (OperationType)makeRandomInt<0, 2>();
  auto comp = (ComparatorType)makeRandomInt<0, 1>();
  int featureIdx = makeRandomInt<0, DataSetFeatures_>();
  float bias = makeBalancedBias(op);

  return TreeNode(bias, op, comp, featureIdx);
}

template <int TreeDepth_, int DataSetFeatures_>
DecisionTree makeDecisionTree() {
  DecisionTree tree;

  tree.reserve(TreeSize(TreeDepth_));
  tree[0] = makeDecisionTreeNode<DataSetFeatures_>(); // root

  int64_t parentIdx = 0;
  int parentBranch = 0;
  auto registerChild = [&tree, &parentIdx, &parentBranch](int64_t childIdx) {
    if (parentBranch == 0) {
      tree[parentIdx].FalseChildNodesIdx = childIdx;
      parentBranch = 1;
    } else {
      tree[parentIdx].TrueChildNodesIdx = childIdx;
      parentBranch = 0;
      parentIdx++;
    }
  };

  constexpr int64_t nodes = TreeSize(TreeDepth_);
  for (int64_t i = 1; i < nodes; i++) {
    tree[i] = makeDecisionTreeNode<DataSetFeatures_>();
    registerChild(i);
  }

  return tree;
};
