#pragma once

#include <unordered_map>

#include "data/DecisionTreeNode.h"
#include "Utils.h"

class DecisionTree {
public:
  DecisionTree() = default;
  DecisionTree(DecisionTree &&) = default;
  DecisionTree &operator=(DecisionTree &&) = default;

  // no copies as they'd be too expensive
  DecisionTree(const DecisionTree &) = delete;
  DecisionTree &operator=(const DecisionTree &) = delete;

  DecisionTree(uint8_t levels, uint64_t nodes);
  void finalize();

  uint8_t getNumLevels() const { return Levels; }
  uint64_t getRootNodeIdx() const { return 0; }
  DecisionSubtreeRef getSubtreeRef(uint64_t rootIndex, uint8_t levels) const;

  DecisionTreeNode getNode(uint64_t idx) const {
    assert(Nodes.find(idx) != Nodes.end());
    return Nodes.at(idx);
  }

  void addNode(uint64_t idx, DecisionTreeNode node) {
    assert(!Finalized);
    assert(Nodes.find(idx) == Nodes.end());
    Nodes.emplace(idx, std::move(node));
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
  DecisionTree makeRandomRegular(uint8_t levels, uint32_t dataSetFeatures);

private:
  std::string CacheDir;
  std::string initCacheDir(std::string cacheDirName);
};
