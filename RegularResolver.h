#pragma once

#include "DataSet.h"
#include "LegacyDecisionTree.h"

class RegularResolver {
public:
    int64_t run(const DecisionTree_t &tree,
                const DataSet_t &dataSet) {
      int64_t idx = 0;
      int64_t firstResultIdx = tree.size();

      while (idx < firstResultIdx) {
        idx = computeChildNodeForDataSet(tree.at(idx), dataSet);
      }

      return idx;
    }

private:
  int64_t computeChildNodeForDataSet(const TreeNode &currentNode,
                                     const DataSet_t &dataSet) {
    float featureValue = dataSet[currentNode.DataSetFeatureIdx];
    return featureValue > currentNode.Bias ? currentNode.TrueChildNodeIdx
                                           : currentNode.FalseChildNodeIdx;
  }
};
