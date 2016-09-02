#pragma once

#include "data/DecisionTree.h"
#include "data/DecisionTreeNode.h"

class Interpreter {
public:
    uint64_t run(const DecisionTree &tree,
                 const std::vector<float> &dataSet) {
      DecisionTreeNode node = tree.getRootNode();

      while (!node.isImplicit()) {
        float featureValue = dataSet[node.getFeatureIdx()];
        NodeEvaluation eval = featureValue > node.getFeatureBias()
                            ? NodeEvaluation::ContinueOneRight
                            : NodeEvaluation::ContinueZeroLeft;

        node = tree.getChildNodeFor(std::move(node), eval);
      }

      return node.getIdx();
    }
};
