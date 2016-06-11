#pragma once

#include <array>
#include <cassert>

#include "Utils.h"

enum class OperationType {
    Bypass,
    Sqrt,
    Ln
};

enum class ComparatorType {
    LessThan,
    GreaterThan
};

struct TreeNode {
    TreeNode() = default;
    ~TreeNode() = default;

    TreeNode(float bias,
             OperationType op,
             ComparatorType comp,
             unsigned long featureIdx)
        : Bias(bias)
        , Op(op)
        , Comp(comp)
        , DataSetFeatureIdx(featureIdx) {}

    unsigned long getFalseChildIdx() const {
        return FalseChildNodesIdx;
    }

    unsigned long getTrueChildIdx() const {
        return TrueChildNodesIdx;
    }

    bool isLeaf() const {
        assert((getFalseChildIdx() == 0) == (getTrueChildIdx() == 0) &&
               "There must either both or no child nodes");
        return getFalseChildIdx() == 0;
    }

    float Bias;
    OperationType Op;
    ComparatorType Comp;
    unsigned long DataSetFeatureIdx;
    unsigned long TrueChildNodesIdx = 0;
    unsigned long FalseChildNodesIdx = 0;
};

// for expected input range [0, 1)
float makeBalancedBias(OperationType op) {
    switch (op) {
        case OperationType::Bypass: return 0.5f;
        case OperationType::Sqrt: return std::sqrtf(0.5f);
        case OperationType::Ln: return std::log(0.5f);
    };
}

template<unsigned long DataSetFeatures_>
TreeNode makeDecisionTreeNode() {
    auto op = (OperationType)makeRandomInt<0, 2>();
    auto comp = (ComparatorType)makeRandomInt<0, 1>();
    int featureIdx = makeRandomInt<0, DataSetFeatures_>();
    float bias = makeBalancedBias(op);

    return TreeNode(bias, op, comp, featureIdx);
}

template<unsigned long TreeDepth_, unsigned long DataSetFeatures_>
std::array<TreeNode, TreeSize(TreeDepth_)> makeDecisionTree() {
    std::array<TreeNode, TreeSize(TreeDepth_)> tree;

    tree[0] = makeDecisionTreeNode<DataSetFeatures_>(); // root

    int parentIdx = 0;
    int parentBranch = 0;
    auto registerChild = [&tree, &parentIdx, &parentBranch](unsigned long childIdx) {
        if (parentBranch == 0) {
            tree[parentIdx].FalseChildNodesIdx = childIdx;
            parentBranch = 1;
        }
        else {
            tree[parentIdx].TrueChildNodesIdx = childIdx;
            parentBranch = 0;
            parentIdx++;
        }
    };

    for (unsigned long i = 1; i < TreeSize(TreeDepth_); i++) {
        tree[i] = makeDecisionTreeNode<DataSetFeatures_>();
        registerChild(i);
    }

    return tree;
};
