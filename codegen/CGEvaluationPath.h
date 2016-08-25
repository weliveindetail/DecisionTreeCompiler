#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include <experimental/optional>

#include "resolver/DecisionTree.h"

struct CGEvaluationPathNode {
  CGEvaluationPathNode() = default;
  CGEvaluationPathNode(CGEvaluationPathNode &&) = default;
  CGEvaluationPathNode(const CGEvaluationPathNode &) = default;
  CGEvaluationPathNode &operator=(CGEvaluationPathNode &&) = default;
  CGEvaluationPathNode &operator=(const CGEvaluationPathNode &) = default;

  CGEvaluationPathNode(const DecisionTreeNode &currentNode,
                       const DecisionTreeNode &nextNode,
                       NodeEvaluation_t evaluation)
      : Node(&currentNode), ChildNode(&nextNode), Evaluation(evaluation) {}

  uint8_t getEvaluationValue() const {
    return (uint8_t)Evaluation;
  }

  const DecisionTreeNode &getNodeData() const {
    return *Node;
  }

  const DecisionTreeNode &getChildNodeData() const {
    return *ChildNode;
  }

private:
  const DecisionTreeNode *Node;
  const DecisionTreeNode *ChildNode;
  NodeEvaluation_t Evaluation;
};

struct CGEvaluationPath {
  CGEvaluationPath() = default;
  CGEvaluationPath(CGEvaluationPath &&) = default;
  CGEvaluationPath(const CGEvaluationPath &) = default;
  CGEvaluationPath &operator=(CGEvaluationPath &&) = default;
  CGEvaluationPath &operator=(const CGEvaluationPath &) = default;

  using Data_t = std::vector<CGEvaluationPathNode>;

  CGEvaluationPath(DecisionSubtreeRef subtree,
                   const DecisionTreeNode &continuationNode)
      : Nodes(subtree.Levels)
      , ContinuationNode(&continuationNode)
      , InsertIdx(Nodes.size()) {}

  void addParent(const DecisionTreeNode &node, NodeEvaluation_t evaluation) {
    const DecisionTreeNode &child =
        (InsertIdx == Nodes.size()) ? getContinuationNode() : Nodes[InsertIdx].getNodeData();

    Nodes[--InsertIdx] = CGEvaluationPathNode(node, child, evaluation);
  }

  bool hasNodeIdx(uint64_t idx) const {
    return (bool)findNode(idx);
  }

  std::experimental::optional<CGEvaluationPathNode> findNode(uint64_t idx) const {
    auto findIdx = [=](const CGEvaluationPathNode &node) {
      return node.getNodeData().NodeIdx == idx;
    };

    auto it = std::find_if(Nodes.begin(), Nodes.end(), findIdx);
    return (it == Nodes.end()) ? std::experimental::optional<CGEvaluationPathNode>() : *it;
  }

  const DecisionTreeNode &getContinuationNode() const {
    return *ContinuationNode;
  }

  Data_t Nodes;

private:
  const DecisionTreeNode *ContinuationNode;
  Data_t::size_type InsertIdx;
};
