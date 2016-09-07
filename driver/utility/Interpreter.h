#pragma once

#include "data/DecisionTree.h"
#include "data/DecisionTreeNode.h"

class Interpreter {
public:
  uint64_t run(const DecisionTree &tree,
               const std::vector<float> &dataSet) {
    uint64_t nodeIdx = tree.getRootNodeIdx();
    const DecisionTreeNode *nodePtr = tree.getNodePtr(nodeIdx);

    while (!nodePtr->isImplicit()) {
      float featureValue = dataSet[nodePtr->getFeatureIdx()];
      uint64_t childIdx = featureValue > nodePtr->getFeatureBias()
                          ? nodePtr->getRightChildIdx()
                          : nodePtr->getLeftChildIdx();

      nodePtr = tree.getNodePtr(childIdx);
    }

    return nodePtr->getIdx();
  }

  uint64_t runValueBased(const DecisionTree &tree,
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
