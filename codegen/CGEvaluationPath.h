#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include <experimental/optional>

#include "codegen/CGBase.h"

struct DecisionTreeEvaluationPathNode {
  DecisionTreeEvaluationPathNode() = default;
  DecisionTreeEvaluationPathNode(DecisionTreeEvaluationPathNode &&) = default;
  DecisionTreeEvaluationPathNode(const DecisionTreeEvaluationPathNode &) = default;
  DecisionTreeEvaluationPathNode &operator=(DecisionTreeEvaluationPathNode &&) = default;
  DecisionTreeEvaluationPathNode &operator=(const DecisionTreeEvaluationPathNode &) = default;

  DecisionTreeEvaluationPathNode(const DecisionTreeNode &currentNode,
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

struct DecisionTreeEvaluationPath {
  using Data_t = std::vector<DecisionTreeEvaluationPathNode>;

  DecisionTreeEvaluationPath() = default;
  DecisionTreeEvaluationPath(DecisionTreeEvaluationPath &&) = default;
  DecisionTreeEvaluationPath(const DecisionTreeEvaluationPath &) = default;
  DecisionTreeEvaluationPath &operator=(DecisionTreeEvaluationPath &&) = default;
  DecisionTreeEvaluationPath &operator=(const DecisionTreeEvaluationPath &) = default;

  DecisionTreeEvaluationPath(DecisionSubtreeRef subtree,
                             const DecisionTreeNode &continuationNode)
      : Nodes(subtree.Levels)
      , ContinuationNode(&continuationNode)
      , InsertPos(Nodes.rend()) {}

  void addParent(const DecisionTreeNode &node, NodeEvaluation_t evaluation) {
    const DecisionTreeNode &child =
        (InsertPos == Nodes.rend()) ? getContinuationNode() : InsertPos->getNodeData();

    Data_t::const_iterator pos = std::prev(InsertPos).base();
    Nodes.emplace(pos, node, child, evaluation);
  }

  bool hasNodeIdx(uint64_t idx) const {
    return (bool)findNode(idx);
  }

  std::experimental::optional<DecisionTreeEvaluationPathNode> findNode(uint64_t idx) const {
    auto findIdx = [=](const DecisionTreeEvaluationPathNode &node) {
      return node.getNodeData().NodeIdx == idx;
    };

    auto it = std::find_if(Nodes.begin(), Nodes.end(), findIdx);
    return (it == Nodes.end()) ? std::experimental::optional<DecisionTreeEvaluationPathNode>() : *it;
  }

  const DecisionTreeNode &getContinuationNode() const {
    return *ContinuationNode;
  }

  Data_t Nodes;

private:
  const DecisionTreeNode *ContinuationNode;
  mutable Data_t::reverse_iterator InsertPos;
};
