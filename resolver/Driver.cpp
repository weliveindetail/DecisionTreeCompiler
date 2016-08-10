#include "resolver/Driver.h"

DecisionSubtreeRef::DecisionSubtreeRef(const DecisionTree &tree,
                                       uint64_t rootIndex, uint8_t levels)
    : Tree(tree), RootIndex(rootIndex), Levels(levels) {
  assert(Levels > 0);
  assert(Levels <= std::min<uint8_t>(4, tree.Levels)); // max node count is 31

  assert(RootIndex >= 0);
  assert(RootIndex < Tree.getFirstNodeIdxOnLevel(tree.Levels - Levels));
}

std::vector<uint64_t> DecisionSubtreeRef::collectNodeIndices() const {
  std::vector<uint64_t> idxs;
  idxs.reserve(getNodeCount());

  for (uint8_t i = 0; i < Levels; i++) {
    auto idxsOnLevel = collectNodeIndicesOnSubtreeLevel(i);
    idxs.insert(idxs.end(), idxsOnLevel.begin(), idxsOnLevel.end());
  }

  return idxs;
}

std::vector<uint64_t> DecisionSubtreeRef::collectNodeIndicesOnSubtreeLevel(
    uint8_t level) const {
  std::vector<uint64_t> idxs(PowerOf2(level));

  uint64_t firstSubtreeIdxOnNodeLevel =
      getFirstSubtreeNodeIdxOnSubtreeLevel(level);

  std::iota(idxs.begin(), idxs.end(), firstSubtreeIdxOnNodeLevel);
  return idxs;
}

uint64_t DecisionSubtreeRef::getFirstSubtreeNodeIdxOnSubtreeLevel(
    uint8_t subtreeLevel) const {
  uint64_t numSubtreeNodesOnLevel = PowerOf2(subtreeLevel);

  uint8_t rootLevel = Tree.getLevelForNodeIdx(RootIndex);
  uint64_t rootOffset = RootIndex - Tree.getFirstNodeIdxOnLevel(rootLevel);
  uint64_t nodeOffset = rootOffset * numSubtreeNodesOnLevel;

  uint8_t treeLevel = rootLevel + subtreeLevel;
  return Tree.getFirstNodeIdxOnLevel(treeLevel) + nodeOffset;
}

DecisionSubtreeRef DecisionTree::getSubtreeRef(uint64_t rootIndex,
                                               uint8_t levels) const {
  return DecisionSubtreeRef(*this, rootIndex, levels);
}

std::vector<DecisionTreeEvaluationPath>
DecisionTreeCompiler::buildSubtreeEvaluationPaths(DecisionSubtreeRef subtree) {
  std::vector<DecisionTreeEvaluationPath> results;
  results.reserve(subtree.getContinuationNodeCount());

  std::vector<uint64_t> nodeIdxs = subtree.collectNodeIndices();
  buildSubtreeEvaluationPathsRecursively(subtree, subtree.RootIndex,
                                         subtree.Levels, nodeIdxs, results);

  assert(results.size() == subtree.getContinuationNodeCount());
  return results;
}

// todo: this is still over-complicated
void DecisionTreeCompiler::buildSubtreeEvaluationPathsRecursively(
    DecisionSubtreeRef subtree,
    uint64_t nodeIdx, uint8_t remainingLevels,
    const std::vector<uint64_t> &nodeIdxs,
    std::vector<DecisionTreeEvaluationPath> &result) {
  if (remainingLevels > 1) {
    // non-leaf subtree nodes add their offsets to their leafs' child path maps
    const DecisionTreeNode &node = subtree.getNode(nodeIdx);
    uint8_t numChildLeafPaths = PowerOf2<uint8_t>(remainingLevels);
    uint8_t numChildsPerCond = numChildLeafPaths / 2;

    buildSubtreeEvaluationPathsRecursively(subtree, node.FalseChildNodeIdx,
                                          remainingLevels - 1,
                                          nodeIdxs, result);

    for (uint8_t i = 0; i < numChildsPerCond; i++) {
      result[result.size() - 1 - i].addParent(node.FalseChildNodeIdx,
                                              NodeEvaluation_t::ContinueZeroLeft);
    }

    buildSubtreeEvaluationPathsRecursively(subtree, node.TrueChildNodeIdx,
                                          remainingLevels - 1,
                                          nodeIdxs, result);

    for (uint8_t i = 0; i < numChildsPerCond; i++) {
      result[result.size() - 1 - i].addParent(node.TrueChildNodeIdx,
                                              NodeEvaluation_t::ContinueOneRight);
    }
  }
  else {
    // subtree leaf nodes create empty path maps
    const DecisionTreeNode &node = subtree.getNode(nodeIdx);

    DecisionTreeEvaluationPath zeroLeftChildPath(subtree);
    DecisionTreeEvaluationPath oneRightChildPath(subtree);

    zeroLeftChildPath.initPathTarget(node.FalseChildNodeIdx,
                                     NodeEvaluation_t::ContinueZeroLeft);

    oneRightChildPath.initPathTarget(node.TrueChildNodeIdx,
                                     NodeEvaluation_t::ContinueOneRight);

    result.push_back(zeroLeftChildPath);
    result.push_back(oneRightChildPath);
  }
}
