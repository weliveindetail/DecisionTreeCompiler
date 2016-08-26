#pragma once

#include <limits>
#include <memory>
#include <system_error>

#include <llvm/Support/ErrorOr.h>

#include "LegacyDecisionTree.h"

enum class NodeEvaluation_t {
  ContinueZeroLeft = 0,
  ContinueOneRight = 1
};

class DecisionSubtreeRef;

struct DecisionTreeNode {
  DecisionTreeNode() = default;
  DecisionTreeNode(DecisionTreeNode &&) = default;
  DecisionTreeNode(const DecisionTreeNode &) = default;
  DecisionTreeNode &operator=(DecisionTreeNode &&) = default;
  DecisionTreeNode &operator=(const DecisionTreeNode &) = default;

  uint64_t NodeIdx = NoNodeIdx; // same as index as long as tree is regular

  uint32_t DataSetFeatureIdx = NoFeatureIdx;
  float Bias = NoBias;

  uint64_t TrueChildNodeIdx = NoNodeIdx;
  uint64_t FalseChildNodeIdx = NoNodeIdx;

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

  bool isImplicit() const {
    bool allDefaulted = (std::isnan(Bias) &&
                         DataSetFeatureIdx == NoFeatureIdx &&
                         TrueChildNodeIdx == NoNodeIdx &&
                         FalseChildNodeIdx == NoNodeIdx);

    return allDefaulted && NodeIdx != NoNodeIdx;
  }

  bool hasLeftChild() const { return FalseChildNodeIdx != NoNodeIdx; }
  bool hasRightChild() const { return TrueChildNodeIdx != NoNodeIdx; }

private:
  DecisionTreeNode(uint64_t nodeIdx, float bias, uint32_t dataSetFeatureIdx,
                   uint64_t zeroFalseChildIdx, uint64_t oneTrueChildIdx)
      : NodeIdx(nodeIdx), DataSetFeatureIdx(dataSetFeatureIdx), Bias(bias)
      , FalseChildNodeIdx(zeroFalseChildIdx), TrueChildNodeIdx(oneTrueChildIdx) {}

  DecisionTreeNode(uint64_t nodeIdx) : NodeIdx(nodeIdx) {}

  static constexpr uint32_t NoFeatureIdx = 0xFFFFFFFF;
  static constexpr uint64_t NoNodeIdx = 0xFFFFFFFFFFFFFFFF;
  static constexpr float NoBias = std::numeric_limits<float>::quiet_NaN();

  friend class DecisionTree;
  friend class DecisionTreeFactory;
};

class DecisionTree {
public:
  DecisionTree() = default;
  DecisionTree(DecisionTree &&) = default;
  DecisionTree &operator=(DecisionTree &&) = default;

  DecisionTree(const DecisionTree &) = delete;
  DecisionTree &operator=(const DecisionTree &) = delete;

  DecisionSubtreeRef getSubtreeRef(uint64_t rootIndex, uint8_t levels) const;

  static uint8_t getLevelForNodeIdx(uint64_t nodeIdx) {
    return Log2(nodeIdx + 1);
  }

  static uint64_t getFirstNodeIdxOnLevel(uint8_t level) {
    return PowerOf2(level) - 1;
  }

private:
  DecisionTree(uint8_t levels, uint64_t nodes) {
    Levels = levels;
    Nodes.reserve(nodes);
    Finalized = false;
  }

  void finalize();

  bool Finalized = false;
  uint8_t Levels = 0;
  uint64_t FirstResultIdx = DecisionTreeNode::NoNodeIdx;
  std::unordered_map<uint64_t, DecisionTreeNode> Nodes;

  friend class DecisionTreeFactory;
  friend class DecisionSubtreeRef;
};

class DecisionTreeFactory {
public:
  DecisionTreeFactory(std::string cacheDirName = std::string{});

  DecisionTree makeRandomRegular(uint8_t levels, uint32_t dataSetFeatures);

private:
  std::string CacheDir;

  std::string initCacheDir(std::string cacheDirName);
};

struct DecisionSubtreeRef {
  DecisionSubtreeRef() = default;
  DecisionSubtreeRef(DecisionSubtreeRef &&) = default;
  DecisionSubtreeRef(const DecisionSubtreeRef &) = default;
  DecisionSubtreeRef &operator=(DecisionSubtreeRef &&) = default;
  DecisionSubtreeRef &operator=(const DecisionSubtreeRef &) = default;

  DecisionSubtreeRef(const DecisionTree *tree, uint64_t rootIndex, uint8_t levels);

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
