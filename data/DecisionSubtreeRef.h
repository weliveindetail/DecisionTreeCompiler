#pragma once

#include <list>

#include "data/DecisionTree.h"
#include "data/DecisionTreeNode.h"

#include "Utils.h"

struct DecisionSubtreeRef {
  DecisionSubtreeRef() = default;
  DecisionSubtreeRef(DecisionSubtreeRef &&) = default;
  DecisionSubtreeRef(const DecisionSubtreeRef &) = default;
  DecisionSubtreeRef &operator=(DecisionSubtreeRef &&) = default;
  DecisionSubtreeRef &operator=(const DecisionSubtreeRef &) = default;

  DecisionSubtreeRef(const DecisionTree *tree, DecisionTreeNode root,
                     uint8_t levels);

  bool isComplete() const {
    // todo: implement this for non-regular trees
    return true;
  }

  DecisionTreeNode getNode(uint64_t idx) const {
    return Tree->getNode(idx);
  }

  uint8_t getNodeCount() const { return (uint8_t)(PowerOf2(Levels) - 1); }
  uint8_t getContinuationNodeCount() const { return PowerOf2<uint8_t>(Levels); }

  std::list<DecisionTreeNode> collectNodesPreOrder() const;

  DecisionTreeNode Root;
  uint8_t Levels;

private:
  const DecisionTree *Tree;

  std::list<DecisionTreeNode> collectNodesRecursively(
      DecisionTreeNode n, int levels) const;
};

inline DecisionSubtreeRef::DecisionSubtreeRef(const DecisionTree *tree,
                                              DecisionTreeNode root,
                                              uint8_t levels)
    : Tree(tree), Root(std::move(root)), Levels(levels) {
  assert(!Root.isImplicit());
  assert(Levels > 0 && Levels <= 4); // max node count is 31
  assert(Tree->getNode(Root.getIdx()) == Root);

  // make sure we have a complete subtree
  assert(collectNodesPreOrder().size() == TreeNodes(Levels));
}

inline std::list<DecisionTreeNode>
DecisionSubtreeRef::collectNodesPreOrder() const {
  std::list<DecisionTreeNode> nodes = collectNodesRecursively(Root, Levels - 1);
  nodes.push_front(std::move(Root));

  return nodes;
}

inline std::list<DecisionTreeNode>
DecisionSubtreeRef::collectNodesRecursively(DecisionTreeNode n,
                                            int levels) const {
  if (levels > 0) {
    std::list<DecisionTreeNode> ns;

    if (n.hasLeftChild()) {
      ns.push_back(n.getChildFor(NodeEvaluation::ContinueZeroLeft, *this));
      ns.splice(ns.end(), collectNodesRecursively(ns.back(), levels - 1));
    }

    if (n.hasRightChild()) {
      ns.push_back(n.getChildFor(NodeEvaluation::ContinueOneRight, *this));
      ns.splice(ns.end(), collectNodesRecursively(ns.back(), levels - 1));
    }

    return ns;
  }

  return {};
}
