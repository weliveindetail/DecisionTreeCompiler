#pragma once

#include <cstdint>
#include <vector>

#include <experimental/optional>

#include "data/DecisionSubtreeRef.h"
#include "data/DecisionTreeNode.h"

struct CGEvaluationStep {
  CGEvaluationStep() = default;
  CGEvaluationStep(CGEvaluationStep &&) = default;
  CGEvaluationStep(const CGEvaluationStep &) = default;
  CGEvaluationStep &operator=(CGEvaluationStep &&) = default;
  CGEvaluationStep &operator=(const CGEvaluationStep &) = default;

  CGEvaluationStep(DecisionTreeNode fromNode, DecisionTreeNode toNode,
                   NodeEvaluation fromNodeEval)
      : FromNode(fromNode), ToNode(toNode), FromNodeEval(fromNodeEval) {}

  friend bool operator==(const CGEvaluationStep &lhs,
                         const CGEvaluationStep &rhs) {
    bool equal = lhs.FromNode == rhs.FromNode && lhs.ToNode == rhs.ToNode;
    assert(!equal || lhs.FromNodeEval == rhs.FromNodeEval);
    return equal;
  }

  friend bool operator!=(const CGEvaluationStep &lhs,
                         const CGEvaluationStep &rhs) {
    return !(lhs == rhs);
  }

  uint8_t getSrcNodeEvalValue() const { return (uint8_t)FromNodeEval; }

  DecisionTreeNode getSrcNode() const { return FromNode; }
  DecisionTreeNode getDestNode() const { return ToNode; }

private:
  DecisionTreeNode FromNode;
  DecisionTreeNode ToNode;
  NodeEvaluation FromNodeEval;
};

struct CGEvaluationPath {
  CGEvaluationPath() = default;
  CGEvaluationPath(CGEvaluationPath &&) = default;
  CGEvaluationPath(const CGEvaluationPath &) = default;
  CGEvaluationPath &operator=(CGEvaluationPath &&) = default;
  CGEvaluationPath &operator=(const CGEvaluationPath &) = default;

  CGEvaluationPath(DecisionSubtreeRef subtree,
                   DecisionTreeNode destinationNode)
      : Steps(subtree.Levels), DestinationNode(destinationNode),
        InsertIdx(Steps.size()) {}

  friend bool operator==(const CGEvaluationPath &lhs,
                         const CGEvaluationPath &rhs) {
    if (lhs.Steps.size() != rhs.Steps.size())
      return false;

    if (lhs.InsertIdx != rhs.InsertIdx)
      return false;

    for (size_t i = lhs.Steps.size() - 1; i >= lhs.InsertIdx; i++)
      if (lhs.Steps[i] != rhs.Steps[i])
        return false;

    return lhs.DestinationNode == rhs.DestinationNode;
  }

  friend bool operator!=(const CGEvaluationPath &lhs,
                         const CGEvaluationPath &rhs) {
    return !(lhs == rhs);
  }

  size_t getNumSteps() const { return Steps.size(); };
  size_t getNumNodes() const { return Steps.size() + 1; };

  void addParent(DecisionTreeNode node, NodeEvaluation evaluation) {
    assert(!isFinalized());
    DecisionTreeNode child = (InsertIdx == Steps.size())
                                        ? getDestNode()
                                        : getStep(InsertIdx).getSrcNode();

    Steps[--InsertIdx] = CGEvaluationStep(node, child, evaluation);
  }

  bool hasNode(DecisionTreeNode node) const {
    assert(isFinalized());
    for (CGEvaluationStep step : Steps)
      if (node == step.getSrcNode())
        return true;

    return (node == DestinationNode);
  }

  std::experimental::optional<CGEvaluationStep>
  findStepFromNode(DecisionTreeNode node) const {
    assert(isFinalized());
    for (CGEvaluationStep step : Steps)
      if (node == step.getSrcNode())
        return step;

    return std::experimental::optional<CGEvaluationStep>();
  }

  DecisionTreeNode getSrcNode() const {
    // source node is only valid if all parents were added
    assert(isFinalized());
    return Steps.front().getSrcNode();
  }

  DecisionTreeNode getDestNode() const {
    // destination node is always valid
    return DestinationNode;
  }

  CGEvaluationStep getStep(size_t idx) {
    assert(InsertIdx <= idx);
    return Steps[idx];
  }

  std::vector<CGEvaluationStep> Steps;

private:
  DecisionTreeNode DestinationNode;
  size_t InsertIdx;

  bool isFinalized() const { return InsertIdx == 0; }
};
