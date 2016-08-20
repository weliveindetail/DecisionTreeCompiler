#include "resolver/DecisionTree.h"

#include <llvm/Support/ErrorOr.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Path.h>

#include "Utils.h"

DecisionSubtreeRef::DecisionSubtreeRef(const DecisionTree *tree,
                                       uint64_t rootIndex, uint8_t levels)
    : Tree(tree), RootIndex(rootIndex), Levels(levels) {
  assert(Levels > 0);
  assert(Levels <= std::min<uint8_t>(4, tree->Levels)); // max node count is 31

  assert(RootIndex >= 0);
  assert(RootIndex < DecisionTree::getFirstNodeIdxOnLevel(tree->Levels - Levels + 1));
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

  uint8_t rootLevel = DecisionTree::getLevelForNodeIdx(RootIndex);
  uint64_t rootOffset = RootIndex - DecisionTree::getFirstNodeIdxOnLevel(rootLevel);
  uint64_t nodeOffset = rootOffset * numSubtreeNodesOnLevel;

  uint8_t treeLevel = rootLevel + subtreeLevel;
  return DecisionTree::getFirstNodeIdxOnLevel(treeLevel) + nodeOffset;
}

DecisionSubtreeRef DecisionTree::getSubtreeRef(uint64_t rootIndex,
                                               uint8_t levels) const {
  assert(Finalized);
  return DecisionSubtreeRef(this, rootIndex, levels);
}

void DecisionTree::finalize() {
  assert(Nodes.size() == TreeNodes(Levels));
  assert(!Finalized);

  auto idxLess = [](const auto &lhsPair, const auto &rhsPair) {
    assert(lhsPair.first == lhsPair.second.NodeIdx);
    assert(rhsPair.first == rhsPair.second.NodeIdx);
    return lhsPair.first < rhsPair.first;
  };

  auto addResultNodeIfNecessary = [this](uint64_t idx) {
    if (idx != DecisionTreeNode::NoNodeIdx && idx >= FirstResultIdx) {
      Nodes.emplace(idx, DecisionTreeNode(idx));
    }
  };

  auto maxIdxIt = std::max_element(Nodes.begin(), Nodes.end(), idxLess);
  FirstResultIdx = maxIdxIt->second.NodeIdx + 1;

  for (const auto &pairIdxNode : Nodes) {
    addResultNodeIfNecessary(pairIdxNode.second.FalseChildNodeIdx);
    addResultNodeIfNecessary(pairIdxNode.second.TrueChildNodeIdx);
  }

  Finalized = true;
}

DecisionTreeFactory::DecisionTreeFactory(std::string cacheDirName)
    : CacheDir(initCacheDir(std::move(cacheDirName))) {
}

std::string DecisionTreeFactory::initCacheDir(std::string cacheDirName) {
  if (cacheDirName.empty()) {
    llvm::SmallString<256> defaultCacheDirName;
    if (llvm::sys::path::user_cache_directory(defaultCacheDirName, "libEvalTreeJit", "cache")) {
      cacheDirName = defaultCacheDirName.str();
    }
    else {
      llvm::sys::fs::current_path(defaultCacheDirName);
      cacheDirName = defaultCacheDirName.str().str() + "/cache";
    }
  }

  // todo: handle errors
  std::error_code EC = llvm::sys::fs::create_directories(cacheDirName);
  assert(!EC);

  // todo: save and check version file in cache directory

  return cacheDirName;
}

std::unique_ptr<DecisionTree> DecisionTreeFactory::makeRandomRegular(
    uint8_t levels, uint32_t dataSetFeatures) {
  uint64_t nodes = TreeNodes(levels);
  DecisionTree tree(levels, nodes);

  for (uint64_t level = 0; level < levels; level++) {
    uint64_t firstIdx = DecisionTree::getFirstNodeIdxOnLevel(level);
    uint64_t firstChildIdx = DecisionTree::getFirstNodeIdxOnLevel(level + 1);

    for (uint64_t i = 0; i < PowerOf2(level); i++) {
      float bias = makeBalancedBias(OperationType::Bypass);
      auto featureIdx = makeRandomInt<uint32_t>(0, dataSetFeatures);

      DecisionTreeNode node(firstIdx + i,
                            bias, featureIdx,
                            firstChildIdx + 2 * i,
                            firstChildIdx + 2 * i + 1);

      tree.Nodes.emplace(firstIdx + i, std::move(node));
    }
  }

  tree.finalize();
  return std::make_unique<DecisionTree>(std::move(tree));
}
