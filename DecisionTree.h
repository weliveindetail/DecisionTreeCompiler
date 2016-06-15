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
        TrueChildNodesIdx(0), FalseChildNodesIdx(0) {}

  TreeNode(float bias, OperationType op, ComparatorType comp, int featureIdx,
           int64_t trueChildNodesIdx, int64_t falseChildNodesIdx)
      : Bias(bias), Op(op), Comp(comp), DataSetFeatureIdx(featureIdx),
        TrueChildNodesIdx(trueChildNodesIdx),
        FalseChildNodesIdx(falseChildNodesIdx) {}

  int64_t getFalseChildIdx() const { return FalseChildNodesIdx; }

  int64_t getTrueChildIdx() const { return TrueChildNodesIdx; }

  bool isLeaf() const {
    assert((getFalseChildIdx() == 0) == (getTrueChildIdx() == 0) &&
           "There must either both or no child nodes");
    return getFalseChildIdx() == 0;
  }

  float Bias;
  OperationType Op;
  ComparatorType Comp;
  int DataSetFeatureIdx;
  int64_t TrueChildNodesIdx;
  int64_t FalseChildNodesIdx;
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

DecisionTree makeDecisionTree(int treeDepth, int dataSetFeatures, std::string fileName) {
  DecisionTree tree;

  tree.reserve(TreeSize(treeDepth));

  /*int64_t parentIdx = 0;
  int parentBranch = 0;
  auto registerChild = [&tree, &parentIdx, &parentBranch](int64_t childIdx) {
    if (parentBranch == 0) {
      tree[parentIdx].FalseChildNodesIdx = childIdx;
      parentBranch = 1;
    } else {
      tree[parentIdx].TrueChildNodesIdx = childIdx;
      parentBranch = 0;
      parentIdx++;
    }
  };*/

  nlohmann::json treeData = nlohmann::json::array();
  int64_t nodes = TreeSize(treeDepth);
  int64_t firstLeafIdx = TreeSize(treeDepth - 1);

  int64_t nodeIdx = 0;
  for (int level = 0; level < treeDepth; level++) {
    int64_t firstChildNodeIdx = TreeSize(level + 1);
    int numNodesOnLevel = (1 << level);

    for (int offset = 0; offset < numNodesOnLevel; offset++) {
      auto node = makeDecisionTreeNode(dataSetFeatures);

      if (nodeIdx < firstLeafIdx) {
        node.TrueChildNodesIdx = firstChildNodeIdx++;
        node.FalseChildNodesIdx = firstChildNodeIdx++;
      }

      treeData.push_back({{"Bias", node.Bias},
                          {"Op", (int)node.Op},
                          {"Comp", (int)node.Comp},
                          {"DataSetFeatureIdx", node.DataSetFeatureIdx},
                          {"TrueChildNodesIdx", node.TrueChildNodesIdx},
                          {"FalseChildNodesIdx", node.FalseChildNodesIdx}});

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

TreeNode loadDecisionTreeNode(nlohmann::json nodeData) {
  float bias = nodeData["Bias"];
  int op = nodeData["Op"];
  int comp = nodeData["Comp"];
  int featureIdx = nodeData["DataSetFeatureIdx"];
  int64_t trueChildNodesIdx = nodeData["TrueChildNodesIdx"];
  int64_t falseChildNodesIdx = nodeData["FalseChildNodesIdx"];

  return TreeNode(bias, (OperationType)op, (ComparatorType)comp, featureIdx,
                  trueChildNodesIdx, falseChildNodesIdx);
}

DecisionTree loadDecisionTree(std::string fileName) {
  llvm::ErrorOr<std::unique_ptr<llvm::MemoryBuffer>> fileBuffer =
      llvm::MemoryBuffer::getFile(fileName.c_str());

  if (!fileBuffer) {
    assert(false);
    return (DecisionTree());
  }
  DecisionTree tree;

  llvm::StringRef fileContents = fileBuffer.get()->getBuffer();
  nlohmann::json treeData = nlohmann::json::parse(fileContents.data());

  size_t nodes = treeData.size();
  tree.reserve(nodes);

  int64_t i = 0;
  for (auto nodeData : treeData) {
    tree[i++] = loadDecisionTreeNode(std::move(nodeData));
  }

  assert(i == nodes);
  return tree;
}

std::string getTreeFileNameFromModuleId(std::string moduleId) {
  size_t pos = moduleId.rfind('.');
  moduleId.replace(pos, moduleId.length() - pos, ".t");

  std::string Prefix("file:");
  size_t PrefixLength = Prefix.length();
  return "cache/" + moduleId.substr(PrefixLength);
}