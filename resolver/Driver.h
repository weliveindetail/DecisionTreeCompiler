#pragma once

#include <experimental/optional>

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>

#include "DecisionTree.h"

enum class NodeEvaluation_t {
  ContinueZeroLeft = 0,
  ContinueOneRight = 1
};

class DecisionSubtreeRef;

struct DecisionTreeNode {
  uint64_t NodeIdx = NoNodeIdx; // same as index as long as tree is regular

  uint32_t DataSetFeatureIdx;
  float Bias;

  uint64_t TrueChildNodeIdx;
  uint64_t FalseChildNodeIdx;

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

  bool hasLeftChild() const { return FalseChildNodeIdx != NoNodeIdx; }
  bool hasRightChild() const { return TrueChildNodeIdx != NoNodeIdx; }

  static constexpr uint64_t NoNodeIdx = 0xFFFFFFFFFFFFFFFF;
};

class DecisionTree {
public:
  DecisionSubtreeRef getSubtreeRef(uint64_t rootIndex, uint8_t levels) const;

  uint8_t getLevelForNodeIdx(uint64_t nodeIdx) const {
    return Log2(nodeIdx + 1);
  }

  uint8_t getFirstNodeIdxOnLevel(uint8_t level) const {
    return (uint8_t)(PowerOf2(level) - 1);
  }

  uint8_t Levels;
  std::unordered_map<uint64_t, DecisionTreeNode> Nodes;
};

struct DecisionSubtreeRef {
  DecisionSubtreeRef() = default;
  DecisionSubtreeRef(DecisionSubtreeRef &&) = default;
  DecisionSubtreeRef(const DecisionSubtreeRef &) = default;
  DecisionSubtreeRef &operator=(DecisionSubtreeRef &&) = default;
  DecisionSubtreeRef &operator=(const DecisionSubtreeRef &) = default;

  DecisionSubtreeRef(const DecisionTree &tree, uint64_t rootIndex, uint8_t levels);

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

struct DecisionTreeEvaluationPathNode {
  DecisionTreeEvaluationPathNode() = default;
  DecisionTreeEvaluationPathNode(DecisionTreeEvaluationPathNode &&) = default;
  DecisionTreeEvaluationPathNode(const DecisionTreeEvaluationPathNode &) = default;
  DecisionTreeEvaluationPathNode &operator=(DecisionTreeEvaluationPathNode &&) = default;
  DecisionTreeEvaluationPathNode &operator=(const DecisionTreeEvaluationPathNode &) = default;

  DecisionTreeEvaluationPathNode(const DecisionTreeNode &currentNode,
                                 const DecisionTreeNode &nextNode,
                                 NodeEvaluation_t evaluation)
      : Node(&currentNode), ChildNode(&nextNode), Evaluation(evaluation) {}

  uint8_t getEvaluationValue() const {
    return (uint8_t)Evaluation;
  }

  const DecisionTreeNode &getNodeData() const {
    return *Node;
  }

  const DecisionTreeNode &getChildNodeData() const {
    return *ChildNode;
  }

private:
  const DecisionTreeNode *Node;
  const DecisionTreeNode *ChildNode;
  NodeEvaluation_t Evaluation;
};

struct DecisionTreeEvaluationPath {
  using Data_t = std::vector<DecisionTreeEvaluationPathNode>;

  DecisionTreeEvaluationPath() = default;
  DecisionTreeEvaluationPath(DecisionTreeEvaluationPath &&) = default;
  DecisionTreeEvaluationPath(const DecisionTreeEvaluationPath &) = default;
  DecisionTreeEvaluationPath &operator=(DecisionTreeEvaluationPath &&) = default;
  DecisionTreeEvaluationPath &operator=(const DecisionTreeEvaluationPath &) = default;

  DecisionTreeEvaluationPath(DecisionSubtreeRef subtree,
                             const DecisionTreeNode &continuationNode)
      : Nodes(subtree.Levels)
      , ContinuationNode(&continuationNode)
      , InsertPos(Nodes.rend()) {}

  void addParent(const DecisionTreeNode &node, NodeEvaluation_t evaluation) {
    const DecisionTreeNode &child =
        (InsertPos == Nodes.rend()) ? getContinuationNode() : InsertPos->getNodeData();

    Data_t::const_iterator pos = std::prev(InsertPos).base();
    Nodes.emplace(pos, node, child, evaluation);
  }

  bool hasNodeIdx(uint64_t idx) const {
    return (bool)findNode(idx);
  }

  std::experimental::optional<DecisionTreeEvaluationPathNode> findNode(uint64_t idx) const {
    auto findIdx = [=](const DecisionTreeEvaluationPathNode &node) {
      return node.getNodeData().NodeIdx == idx;
    };

    auto it = std::find_if(Nodes.begin(), Nodes.end(), findIdx);
    return (it == Nodes.end()) ? std::experimental::optional<DecisionTreeEvaluationPathNode>() : *it;
  }

  const DecisionTreeNode &getContinuationNode() const {
    return *ContinuationNode;
  }

  Data_t Nodes;

private:
  const DecisionTreeNode *ContinuationNode;
  mutable Data_t::reverse_iterator InsertPos;
};

class DecisionTreeCompiler {
public:
  llvm::LLVMContext Ctx;
  llvm::IRBuilder<> Builder;
  std::unique_ptr<llvm::Module> TheModule;
  //std::unique_ptr<SimpleOrcJit> TheCompiler;

  const DecisionTree_t &DecisionTreeLegacyData;
  const DecisionTree &DecisionTreeData;

  llvm::Value *emitNodeLoad(
      const DecisionTreeNode &node, llvm::Value *dataSetPtr) {
    llvm::Value *dataSetFeaturePtr =
        Builder.CreateConstGEP1_32(dataSetPtr, node.DataSetFeatureIdx);

    return Builder.CreateLoad(dataSetFeaturePtr);
  }
};
