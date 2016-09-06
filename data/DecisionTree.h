#pragma once

#include <unordered_map>

#include "data/DecisionTreeNode.h"
#include "Utils.h"

class DecisionTree {
public:
  DecisionTree() = default;
  DecisionTree(DecisionTree &&) = default;
  DecisionTree &operator=(DecisionTree &&) = default;

  DecisionTree(uint8_t levels, uint64_t nodes);
  void finalize();

  DecisionTree copy() const;

  uint8_t getNumLevels() const { return Levels; }
  uint64_t getRootNodeIdx() const { return 0; }

  DecisionSubtreeRef getSubtreeRef(uint64_t rootIndex, uint8_t levels) const;

  DecisionTreeNode getNode(uint64_t idx) const {
    assert(Nodes.find(idx) != Nodes.end());
    return Nodes.at(idx);
  }

  DecisionTreeNode getRootNode() const {
    return getNode(getRootNodeIdx());
  }

  DecisionTreeNode getChildNodeFor(DecisionTreeNode node,
                                   NodeEvaluation eval) const {
    return getNode(eval == NodeEvaluation::ContinueZeroLeft
                   ? node.FalseChildNodeIdx
                   : node.TrueChildNodeIdx);
  }

  void addNode(DecisionTreeNode node) {
    assert(!Finalized);
    assert(Nodes.find(node.getIdx()) == Nodes.end());
    uint64_t idx = node.getIdx(); // avoid move before read!
    Nodes.emplace(idx, std::move(node));
  }

  template <typename ...Args_tt>
  void addNodes(DecisionTreeNode node, Args_tt... args) {
    addNode(node);
    addNodes(args...);
  }

  template <typename ...Args_tt>
  void addNodes(DecisionTreeNode node) {
    addNode(node);
  }

  static uint8_t getLevelForNodeIdx(uint64_t nodeIdx) {
    return Log2(nodeIdx + 1);
  }

  static uint64_t getFirstNodeIdxOnLevel(uint8_t level) {
    return PowerOf2(level) - 1;
  }

private:
  bool Finalized = false;
  uint8_t Levels = 0;
  uint64_t FirstResultIdx = DecisionTreeNode::NoNodeIdx;
  std::unordered_map<uint64_t, DecisionTreeNode> Nodes;

  // no implicit copies as they'd be too expensive, use copy() instead
  DecisionTree(const DecisionTree &) = default;
  DecisionTree &operator=(const DecisionTree &) = default;

  void addImplicitNode(uint64_t nodeIdx) {
    DecisionTreeNode node;
    node.NodeIdx = nodeIdx;
    assert(node.isImplicit());
    Nodes.emplace(nodeIdx, std::move(node));
  }
};

class DecisionTreeFactory {
public:
  DecisionTreeFactory(std::string cacheDirName = std::string{});

  DecisionTree makePerfectTrivialUniformTree(uint8_t levels);
  DecisionTree makePerfectDistinctUniformTree(uint8_t levels);

private:
  std::string CacheDir;
  std::string initCacheDir(std::string cacheDirName);
};
