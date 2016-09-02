#pragma once

#include <vector>

#include "data/DecisionTree.h"
#include "data/DecisionTreeNode.h"

class DataSetFactory {
public:
  DataSetFactory(DecisionTree tree, uint32_t features)
      : Tree(std::move(tree)), Features(features) {}

  template <typename ...Args_tt>
  std::vector<float> makeDataSet(NodeEvaluation l1, Args_tt... args) {
    std::vector<float> ds(Features, 0.0f);
    fillDataSetRecursively(ds, Tree.getRootNode(), l1, args...);
    return ds;
  }

private:
  DecisionTree Tree;
  uint32_t Features;

  template <typename ...Args_tt>
  void fillDataSetRecursively(std::vector<float> &ds, DecisionTreeNode node,
                              NodeEvaluation eval, Args_tt... childEvals) {
    ds[node.getFeatureIdx()] = getAdjustedBias(node, eval);

    assert(node.hasChildFor(eval));
    fillDataSetRecursively(ds, Tree.getChildNodeFor(node, eval), childEvals...);
  }

  template <typename ...Args_tt>
  void fillDataSetRecursively(std::vector<float> &ds, DecisionTreeNode node) {
    assert(node.isImplicit());
  }

  float getAdjustedBias(DecisionTreeNode node, NodeEvaluation eval) const {
    bool addFraction = (eval == NodeEvaluation::ContinueOneRight);
    return node.getFeatureBias() + (addFraction ? 0.01f : -0.01f);
  }
};
