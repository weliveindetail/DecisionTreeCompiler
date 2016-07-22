#pragma once

#include <cassert>
#include <unistd.h>
#include <unordered_map>

#include <llvm/Support/ErrorOr.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/Path.h>

#include "Utils.h"
#include "json/src/json.hpp"

struct TreeNode;
using DecisionTree_t = std::unordered_map<uint64_t, TreeNode>;

enum class OperationType { Bypass, Sqrt, Ln };

enum class ComparatorType { LessThan, GreaterThan };

struct TreeNode {
  TreeNode() = default;
  ~TreeNode() = default;

  TreeNode(float bias, OperationType op, ComparatorType comp, uint32_t featureIdx)
      : Bias(bias), Op(op), Comp(comp), DataSetFeatureIdx(featureIdx),
        TrueChildNodeIdx(0), FalseChildNodeIdx(0) {}

  TreeNode(float bias, OperationType op, ComparatorType comp, uint32_t featureIdx,
           uint64_t trueChildNodesIdx, uint64_t falseChildNodesIdx)
      : Bias(bias), Op(op), Comp(comp), DataSetFeatureIdx(featureIdx),
        TrueChildNodeIdx(trueChildNodesIdx),
        FalseChildNodeIdx(falseChildNodesIdx) {}

  float Bias;
  OperationType Op;
  ComparatorType Comp;
  uint32_t DataSetFeatureIdx;

  bool IsGlobalLeaf;
  uint64_t TrueChildNodeIdx;
  uint64_t FalseChildNodeIdx;
};

// for expected input range [0, 1)
static float makeBalancedBias(OperationType op) {
  switch (op) {
  case OperationType::Bypass:
    return 0.5f;
  case OperationType::Sqrt:
    return std::sqrtf(0.5f);
  case OperationType::Ln:
    return std::log(0.5f);
  };
}

static TreeNode makeDecisionTreeNode(uint32_t dataSetFeatures) {
  auto op = (OperationType)makeRandomInt(0, 2);
  auto comp = (ComparatorType)makeRandomInt(0, 1);
  auto featureIdx = makeRandomInt<uint32_t>(0, dataSetFeatures);
  float bias = makeBalancedBias(op);

  return TreeNode(bias, op, comp, featureIdx);
}

static DecisionTree_t makeDecisionTree(uint32_t treeDepth, uint32_t dataSetFeatures,
                                       std::string fileName) {
  DecisionTree_t tree;
  tree.reserve(TreeNodes(treeDepth));

  nlohmann::json treeData = nlohmann::json::array();
  uint64_t nodes = TreeNodes(treeDepth);
  uint64_t firstLeafIdx = TreeNodes(treeDepth - 1);

  uint64_t nodeIdx = 0;
  for (uint32_t level = 0; level < treeDepth; level++) {
    uint64_t childNodeIdx = TreeNodes(level + 1);
    uint64_t numNodesOnLevel = PowerOf2(level);

    for (uint64_t offset = 0; offset < numNodesOnLevel; offset++) {
      auto node = makeDecisionTreeNode(dataSetFeatures);

      node.TrueChildNodeIdx = childNodeIdx++;
      node.FalseChildNodeIdx = childNodeIdx++;
      node.IsGlobalLeaf = (level == treeDepth - 1);

      treeData.push_back({{"Bias", node.Bias},
                          {"Op", (int)node.Op},
                          {"Comp", (int)node.Comp},
                          {"DataSetFeatureIdx", node.DataSetFeatureIdx},
                          {"TrueChildNodeIdx", node.TrueChildNodeIdx},
                          {"FalseChildNodeIdx", node.FalseChildNodeIdx}});

      tree[nodeIdx] = std::move(node);
      nodeIdx++;
    }
  }

  {
    llvm::StringRef cacheDir = llvm::sys::path::parent_path(fileName);

    if (!cacheDir.empty())
      llvm::sys::fs::create_directories(cacheDir);

    std::error_code EC;
    llvm::raw_fd_ostream outfile(fileName, EC, llvm::sys::fs::F_Text);

    std::string treeDataStr = treeData.dump(2);
    outfile.write(treeDataStr.data(), treeDataStr.size());
    outfile.close();
  }

  return tree;
};

static TreeNode loadDecisionTreeNode(const nlohmann::json &nodeData) {
  float bias = nodeData.at("Bias");
  int op = nodeData.at("Op");
  int comp = nodeData.at("Comp");
  uint32_t featureIdx = nodeData.at("DataSetFeatureIdx");
  uint64_t trueChildNodeIdx = nodeData.at("TrueChildNodeIdx");
  uint64_t falseChildNodeIdx = nodeData.at("FalseChildNodeIdx");

  return TreeNode(bias, (OperationType)op, (ComparatorType)comp, featureIdx,
                  trueChildNodeIdx, falseChildNodeIdx);
}

static DecisionTree_t loadDecisionTree(int treeDepth, std::string fileName) {
  llvm::ErrorOr<std::unique_ptr<llvm::MemoryBuffer>> fileBuffer =
      llvm::MemoryBuffer::getFile(fileName.c_str());

  if (!fileBuffer) {
    assert(false);
    return (DecisionTree_t());
  }

  uint64_t expectedNodes = TreeNodes(treeDepth);
  llvm::StringRef fileContents = fileBuffer.get()->getBuffer();
  nlohmann::json treeData = nlohmann::json::parse(fileContents.data());

  DecisionTree_t tree;
  tree.reserve(expectedNodes);

  uint64_t i = 0;
  for (const auto &nodeData : treeData)
    tree[i++] = loadDecisionTreeNode(nodeData);

  assert(i == expectedNodes);
  return tree;
}

static DecisionTree_t prepareDecisionTree(uint32_t treeDepth, uint32_t dataSetFeatures) {
  std::string cachedTreeFile = makeTreeFileName(treeDepth, dataSetFeatures);
  bool isTreeFileCached = isFileInCache(cachedTreeFile);

  if (isTreeFileCached) {
    printf("Loading decision tree with depth %d from file %s\n", treeDepth,
           cachedTreeFile.c_str());
    return loadDecisionTree(treeDepth, std::move(cachedTreeFile));
  } else {
    printf("Building decision tree with depth %d and cache it in file %s\n",
           treeDepth, cachedTreeFile.c_str());
    return
        makeDecisionTree(treeDepth, dataSetFeatures, std::move(cachedTreeFile));
  }
}
