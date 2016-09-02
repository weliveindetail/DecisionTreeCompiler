#include "data/DecisionTree.h"

#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Path.h>

#include "data/DecisionSubtreeRef.h"
#include "data/DecisionTreeNode.h"

DecisionTree::DecisionTree(uint8_t levels, uint64_t nodes) {
  Levels = levels;
  Nodes.reserve(nodes);
  Finalized = false;
}

DecisionTree DecisionTree::copy() const {
  return *this;
}

void DecisionTree::finalize() {
  assert(Nodes.size() == TreeNodes(Levels));
  assert(!Finalized);

  auto idxLess = [](const auto &lhsPair, const auto &rhsPair) {
    assert(lhsPair.first == lhsPair.second.getIdx());
    assert(rhsPair.first == rhsPair.second.getIdx());
    return lhsPair.first < rhsPair.first;
  };

  auto addResultNodeIfNecessary = [this](uint64_t idx) {
    if (idx != DecisionTreeNode::NoNodeIdx && idx >= FirstResultIdx) {
      addImplicitNode(idx);
    }
  };

  auto maxIdxIt = std::max_element(Nodes.begin(), Nodes.end(), idxLess);
  FirstResultIdx = maxIdxIt->second.getIdx() + 1;

  for (const auto &pairIdxNode : Nodes) {
    addResultNodeIfNecessary(pairIdxNode.second.FalseChildNodeIdx);
    addResultNodeIfNecessary(pairIdxNode.second.TrueChildNodeIdx);
  }

  Finalized = true;
}

DecisionSubtreeRef DecisionTree::getSubtreeRef(uint64_t rootIndex,
                                               uint8_t levels) const {
  assert(Finalized);
  return DecisionSubtreeRef(this, getNode(rootIndex), levels);
}

DecisionTreeFactory::DecisionTreeFactory(std::string cacheDirName)
    : CacheDir(initCacheDir(std::move(cacheDirName))) {}

std::string DecisionTreeFactory::initCacheDir(std::string cacheDirName) {
  if (cacheDirName.empty()) {
    llvm::SmallString<256> defaultCacheDirName;
    if (llvm::sys::path::user_cache_directory(defaultCacheDirName,
                                              "libEvalTreeJit", "cache")) {
      cacheDirName = defaultCacheDirName.str();
    } else {
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

DecisionTree DecisionTreeFactory::makeRandomRegular(uint8_t levels,
                                                    uint32_t dataSetFeatures) {
  uint64_t nodes = TreeNodes(levels);
  DecisionTree tree(levels, nodes);

  for (uint64_t level = 0; level < levels; level++) {
    uint64_t firstIdx = DecisionTree::getFirstNodeIdxOnLevel(level);
    uint64_t firstChildIdx = DecisionTree::getFirstNodeIdxOnLevel(level + 1);

    for (uint64_t i = 0; i < PowerOf2(level); i++) {
      float bias = 0.4f + makeRandomFloat() / 5.0f; // 0.5 +/- 0.1
      auto featureIdx = makeRandomInt<uint32_t>(0, dataSetFeatures);

      DecisionTreeNode node(firstIdx + i, bias, featureIdx,
                            firstChildIdx + 2 * i, firstChildIdx + 2 * i + 1);

      tree.addNode(firstIdx + i, std::move(node));
    }
  }

  tree.finalize();
  return tree;
}
