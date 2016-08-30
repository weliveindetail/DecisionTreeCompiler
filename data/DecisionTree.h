#pragma once

#include <limits>
#include <memory>
#include <system_error>
#include <unordered_map>

#include "Utils.h"

enum class NodeEvaluation { ContinueZeroLeft = 0, ContinueOneRight = 1 };

class DecisionSubtreeRef;
class DecisionTree;

struct DecisionTreeNode {
  DecisionTreeNode() = default;
  DecisionTreeNode(DecisionTreeNode &&) = default;
  DecisionTreeNode(const DecisionTreeNode &) = default;
  DecisionTreeNode &operator=(DecisionTreeNode &&) = default;
  DecisionTreeNode &operator=(const DecisionTreeNode &) = default;

  DecisionTreeNode(uint64_t nodeIdx, float bias, uint32_t dataSetFeatureIdx,
                   uint64_t zeroFalseChildIdx, uint64_t oneTrueChildIdx)
      : NodeIdx(nodeIdx), DataSetFeatureIdx(dataSetFeatureIdx), Bias(bias),
        FalseChildNodeIdx(zeroFalseChildIdx),
        TrueChildNodeIdx(oneTrueChildIdx) {}

  friend bool operator==(const DecisionTreeNode &lhs,
                         const DecisionTreeNode &rhs) {
    if (lhs.NodeIdx == rhs.NodeIdx) {
#    ifndef NDEBUG
      assert(lhs.isImplicit() == rhs.isImplicit());
      assert(lhs.DataSetFeatureIdx == rhs.DataSetFeatureIdx);
      assert(lhs.TrueChildNodeIdx == rhs.TrueChildNodeIdx);
      assert(lhs.FalseChildNodeIdx == rhs.FalseChildNodeIdx);
#    endif
      return true;
    }

    return false;
  }

  friend bool operator!=(const DecisionTreeNode &lhs,
                         const DecisionTreeNode &rhs) {
    return !(lhs == rhs);
  }

  bool hasChildFor(NodeEvaluation evaluation) const {
    return (evaluation == NodeEvaluation::ContinueZeroLeft)
               ? this->hasLeftChild()
               : this->hasRightChild();
  }

  DecisionTreeNode getChildFor(NodeEvaluation evaluation,
                               DecisionSubtreeRef subtree) const;

  bool isLeaf() const { return !hasLeftChild() && !hasRightChild(); }

  bool isImplicit() const {
    bool allDefaulted =
        (std::isnan(Bias) && DataSetFeatureIdx == NoFeatureIdx &&
         TrueChildNodeIdx == NoNodeIdx && FalseChildNodeIdx == NoNodeIdx);

    return allDefaulted && NodeIdx != NoNodeIdx;
  }

  bool hasLeftChild() const { return FalseChildNodeIdx != NoNodeIdx; }
  bool hasRightChild() const { return TrueChildNodeIdx != NoNodeIdx; }

  uint64_t getIdx() const { return NodeIdx; }
  uint32_t getFeatureIdx() const { return DataSetFeatureIdx; }
  float getFeatureBias() const { return Bias; }

private:
  uint64_t NodeIdx = NoNodeIdx;
  uint64_t TrueChildNodeIdx = NoNodeIdx;
  uint64_t FalseChildNodeIdx = NoNodeIdx;

  uint32_t DataSetFeatureIdx = NoFeatureIdx;
  float Bias = NoBias;

  uint64_t getChildIdxFor(NodeEvaluation evaluation) const {
    return (evaluation == NodeEvaluation::ContinueZeroLeft)
                       ? FalseChildNodeIdx
                       : TrueChildNodeIdx;
  }

  static constexpr uint32_t NoFeatureIdx = 0xFFFFFFFF;
  static constexpr uint64_t NoNodeIdx = 0xFFFFFFFFFFFFFFFF;
  static constexpr float NoBias = std::numeric_limits<float>::quiet_NaN();

  friend class DecisionTree;
};

class DecisionTree {
public:
  DecisionTree() = default;
  DecisionTree(DecisionTree &&) = default;
  DecisionTree &operator=(DecisionTree &&) = default;

  // no copies as they'd be too expensive
  DecisionTree(const DecisionTree &) = delete;
  DecisionTree &operator=(const DecisionTree &) = delete;

  DecisionTree(uint8_t levels, uint64_t nodes) {
    Levels = levels;
    Nodes.reserve(nodes);
    Finalized = false;
  }

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

inline DecisionTreeNode
DecisionTreeNode::getChildFor(NodeEvaluation evaluation,
                              DecisionSubtreeRef subtree) const {
  return subtree.getNode(getChildIdxFor(evaluation));
}
