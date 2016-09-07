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

/// Create a decision tree with the given number of levels and the following
/// special properties:
/// * Perfect: all leaf nodes have equal depth
/// * Trivial: all nodes read the same feature value
/// * Gradient: within one level nodes have constantly raising bias values
///
/// Example: makePerfectTrivialGradientTree(4)
///
/// Indices:
///                            0
///              1                           2
///       3             4             5             6
///   7      8      9      10     11    12      13     14
/// 15 16  17 18  19 20  21 22  23 24  25 26  27 28  29 30 (results)
///
///
/// Bias values:
///                           0.5
///             0.25                        0.75
///     0.125         0.375         0.625         0.875
/// 0.0625 0.1875 0.3125 0.4375 0.5625 0.6875 0.8125 0.9375
///
///
/// Feature indices:
///                            0
///              0                           0
///       0             0             0             0
///   0      0      0      0      0      0      0      0
///
DecisionTree
DecisionTreeFactory::makePerfectTrivialGradientTree(uint8_t levels) {
  uint64_t nodes = TreeNodes(levels);
  DecisionTree tree(levels, nodes);

  for (uint8_t level = 0; level < levels; level++) {
    uint64_t firstIdx = DecisionTree::getFirstNodeIdxOnLevel(level);
    uint64_t firstChildIdx = DecisionTree::getFirstNodeIdxOnLevel(level + 1);

    size_t nodeCount = PowerOf2(level);
    float biasStep = 1.0f / nodeCount;
    float biasBase = biasStep / 2.0f;

    for (uint64_t i = 0; i < nodeCount; i++) {
      float bias = biasBase + i * biasStep;
      uint32_t featureIdx = 0;

      tree.addNode(DecisionTreeNode(firstIdx + i, bias, featureIdx,
                                    firstChildIdx + 2 * i,
                                    firstChildIdx + 2 * i + 1));
    }
  }

  tree.finalize();
  return tree;
}

/// Create a decision tree with the given number of levels and the following
/// special properties:
/// * Perfect: all leaf nodes have equal depth
/// * Distinct: every node reads its own distinct feature value
/// * Gradient: within one level nodes have constantly raising bias values
///
/// Example: makePerfectDistinctGradientTree(4)
///
/// Indices == Feature Indices:
///                            0
///              1                           2
///       3             4             5             6
///   7      8      9      10     11    12      13     14
/// 15 16  17 18  19 20  21 22  23 24  25 26  27 28  29 30 (results)
///
///
/// Bias values:
///                           0.5
///             0.25                        0.75
///     0.125         0.375         0.625         0.875
/// 0.0625 0.1875 0.3125 0.4375 0.5625 0.6875 0.8125 0.9375
///
DecisionTree
DecisionTreeFactory::makePerfectDistinctGradientTree(uint8_t levels) {
  uint64_t nodes = TreeNodes(levels);
  DecisionTree tree(levels, nodes);

  for (uint8_t level = 0; level < levels; level++) {
    uint64_t firstIdx = DecisionTree::getFirstNodeIdxOnLevel(level);
    uint64_t firstChildIdx = DecisionTree::getFirstNodeIdxOnLevel(level + 1);

    size_t nodeCount = PowerOf2(level);
    float biasStep = 1.0f / nodeCount;
    float biasBase = biasStep / 2.0f;

    for (uint64_t i = 0; i < nodeCount; i++) {
      float bias = biasBase + i * biasStep;
      uint32_t featureIdx = firstIdx + i;

      tree.addNode(DecisionTreeNode(firstIdx + i, bias, featureIdx,
                                    firstChildIdx + 2 * i,
                                    firstChildIdx + 2 * i + 1));
    }
  }

  tree.finalize();
  return tree;
}

/// Create a decision tree with the given number of levels and the following
/// special properties:
/// * Perfect: all leaf nodes have equal depth
/// * Trivial: all nodes read the same feature value
/// * Uniform: all nodes have bias of 0.5, so all results have equal probability
///
/// Example: makePerfectTrivialUniformTree(4)
///
/// Indices == Feature Indices:
///                            0
///              1                           2
///       3             4             5             6
///   7      8      9      10     11    12      13     14
/// 15 16  17 18  19 20  21 22  23 24  25 26  27 28  29 30 (results)
///
///
/// Feature indices:
///                            0
///              0                           0
///       0             0             0             0
///   0      0      0      0      0      0      0      0
///
///
/// Bias values:
///                           0.5
///             0.5                         0.5
///      0.5           0.5           0.5           0.5
///   0.5   0.5     0.5   0.5     0.5   0.5     0.5   0.5
///
DecisionTree
DecisionTreeFactory::makePerfectTrivialUniformTree(uint8_t levels) {
  uint64_t nodes = TreeNodes(levels);
  DecisionTree tree(levels, nodes);

  for (uint64_t level = 0; level < levels; level++) {
    uint64_t firstIdx = DecisionTree::getFirstNodeIdxOnLevel(level);
    uint64_t firstChildIdx = DecisionTree::getFirstNodeIdxOnLevel(level + 1);

    for (uint64_t i = 0; i < PowerOf2(level); i++) {
      float bias = 0.5f;
      auto featureIdx = 0;

      tree.addNode(DecisionTreeNode(firstIdx + i, bias, featureIdx,
                                    firstChildIdx + 2 * i,
                                    firstChildIdx + 2 * i + 1));
    }
  }

  tree.finalize();
  return tree;
}

/// Create a decision tree with the given number of levels and the following
/// special properties:
/// * Perfect: all leaf nodes have equal depth
/// * Distinct: every node reads its own distinct feature value
/// * Uniform: all nodes have bias of 0.5, so all results have equal probability
///
/// Example: makePerfectDistinctUniformTree(4)
///
/// Indices == Feature Indices:
///                            0
///              1                           2
///       3             4             5             6
///   7      8      9      10     11    12      13     14
/// 15 16  17 18  19 20  21 22  23 24  25 26  27 28  29 30 (results)
///
///
/// Bias values:
///                           0.5
///             0.5                         0.5
///      0.5           0.5           0.5           0.5
///   0.5   0.5     0.5   0.5     0.5   0.5     0.5   0.5
///
DecisionTree
DecisionTreeFactory::makePerfectDistinctUniformTree(uint8_t levels) {
  uint64_t nodes = TreeNodes(levels);
  DecisionTree tree(levels, nodes);

  for (uint64_t level = 0; level < levels; level++) {
    uint64_t firstIdx = DecisionTree::getFirstNodeIdxOnLevel(level);
    uint64_t firstChildIdx = DecisionTree::getFirstNodeIdxOnLevel(level + 1);

    for (uint64_t i = 0; i < PowerOf2(level); i++) {
      float bias = 0.5f;
      auto featureIdx = firstIdx + i;

      tree.addNode(DecisionTreeNode(firstIdx + i, bias, featureIdx,
                                    firstChildIdx + 2 * i,
                                    firstChildIdx + 2 * i + 1));
    }
  }

  tree.finalize();
  return tree;
}

/// Create a decision tree with the given number of levels and the following
/// special properties:
/// * Perfect: all leaf nodes have equal depth
/// * Random: nodes have random bias and read random feature values
///
/// Example: makePerfectRandomTree(4)
///
/// Indices:
///                            0
///              1                           2
///       3             4             5             6
///   7      8      9      10     11    12      13     14
/// 15 16  17 18  19 20  21 22  23 24  25 26  27 28  29 30 (results)
///
DecisionTree
DecisionTreeFactory::makePerfectRandomTree(uint8_t levels,
                                           uint32_t dataSetFeatures) {
  uint64_t nodes = TreeNodes(levels);
  DecisionTree tree(levels, nodes);

  for (uint64_t level = 0; level < levels; level++) {
    uint64_t firstIdx = DecisionTree::getFirstNodeIdxOnLevel(level);
    uint64_t firstChildIdx = DecisionTree::getFirstNodeIdxOnLevel(level + 1);

    for (uint64_t i = 0; i < PowerOf2(level); i++) {
      float bias = makeRandomFloat();
      auto featureIdx = makeRandomInt<uint32_t>(0, dataSetFeatures - 1);

      tree.addNode(DecisionTreeNode(firstIdx + i, bias, featureIdx,
                                    firstChildIdx + 2 * i,
                                    firstChildIdx + 2 * i + 1));
    }
  }

  tree.finalize();
  return tree;
}
