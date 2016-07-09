#pragma once

#include <cassert>
#include <unistd.h>
#include <unordered_map>

#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Path.h>

#include "Utils.h"
#include "json/src/json.hpp"

enum class OperationType { Bypass, Sqrt, Ln };

enum class ComparatorType { LessThan, GreaterThan };

struct TreeNode {
  TreeNode() = default;
  ~TreeNode() = default;

  TreeNode(float bias, OperationType op, ComparatorType comp, int featureIdx)
      : Bias(bias), Op(op), Comp(comp), DataSetFeatureIdx(featureIdx),
        TrueChildNodeIdx(0), FalseChildNodeIdx(0) {}

  TreeNode(float bias, OperationType op, ComparatorType comp, int featureIdx,
           int64_t trueChildNodesIdx, int64_t falseChildNodesIdx)
      : Bias(bias), Op(op), Comp(comp), DataSetFeatureIdx(featureIdx),
        TrueChildNodeIdx(trueChildNodesIdx),
        FalseChildNodeIdx(falseChildNodesIdx) {}

  float Bias;
  OperationType Op;
  ComparatorType Comp;
  int DataSetFeatureIdx;

  bool IsGlobalLeaf;
  int64_t TrueChildNodeIdx;
  int64_t FalseChildNodeIdx;
};

using DecisionTree = std::unordered_map<int64_t, TreeNode>;

// for expected input range [0, 1)
float makeBalancedBias(OperationType op) {
  switch (op) {
  case OperationType::Bypass:
    return 0.5f;
  case OperationType::Sqrt:
    return std::sqrtf(0.5f);
  case OperationType::Ln:
    return std::log(0.5f);
  };
}

TreeNode makeDecisionTreeNode(int dataSetFeatures) {
  auto op = (OperationType)makeRandomInt(0, 2);
  auto comp = (ComparatorType)makeRandomInt(0, 1);
  int featureIdx = makeRandomInt(0, dataSetFeatures);
  float bias = makeBalancedBias(op);

  return TreeNode(bias, op, comp, featureIdx);
}

DecisionTree makeDecisionTree(int treeDepth, int dataSetFeatures,
                              std::string fileName) {
  DecisionTree tree;
  tree.reserve(TreeNodes(treeDepth));

  nlohmann::json treeData = nlohmann::json::array();
  int64_t nodes = TreeNodes(treeDepth);
  int64_t firstLeafIdx = TreeNodes(treeDepth - 1);

  int64_t nodeIdx = 0;
  for (int level = 0; level < treeDepth; level++) {
    int64_t childNodeIdx = TreeNodes(level + 1);
    int numNodesOnLevel = PowerOf2(level);

    for (int offset = 0; offset < numNodesOnLevel; offset++) {
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

TreeNode loadDecisionTreeNode(const nlohmann::json &nodeData) {
  float bias = nodeData.at("Bias");
  int op = nodeData.at("Op");
  int comp = nodeData.at("Comp");
  int featureIdx = nodeData.at("DataSetFeatureIdx");
  int64_t trueChildNodeIdx = nodeData.at("TrueChildNodeIdx");
  int64_t falseChildNodeIdx = nodeData.at("FalseChildNodeIdx");

  return TreeNode(bias, (OperationType)op, (ComparatorType)comp, featureIdx,
                  trueChildNodeIdx, falseChildNodeIdx);
}

DecisionTree loadDecisionTree(int treeDepth, std::string fileName) {
  llvm::ErrorOr<std::unique_ptr<llvm::MemoryBuffer>> fileBuffer =
      llvm::MemoryBuffer::getFile(fileName.c_str());

  if (!fileBuffer) {
    assert(false);
    return (DecisionTree());
  }

  int64_t expectedNodes = TreeNodes(treeDepth);
  llvm::StringRef fileContents = fileBuffer.get()->getBuffer();
  nlohmann::json treeData = nlohmann::json::parse(fileContents.data());

  DecisionTree tree;
  tree.reserve(expectedNodes);

  int64_t i = 0;
  for (const auto &nodeData : treeData)
    tree[i++] = loadDecisionTreeNode(nodeData);

  assert(i == expectedNodes);
  return tree;
}
