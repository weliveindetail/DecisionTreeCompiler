#pragma once

#include "DataSet.h"
#include "DecisionTree.h"

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
    float biasedFeatureValue = applyOperator(currentNode.Op, featureValue);

    bool evaluatesTrue = applyComparison(currentNode.Comp, currentNode.Bias,
                                         biasedFeatureValue);
    /*
    printf("%f %s %f -> %s\n",
           comparableFeatureValue,
           currentNode.Comp == ComparatorType::LessThan ? "<" : ">",
           currentNode.Bias,
           evaluatesTrue ? "true" : "false");
    //*/

    return evaluatesTrue ? currentNode.TrueChildNodeIdx
                         : currentNode.FalseChildNodeIdx;
  }

  float applyOperator(OperationType op, float value) {
    switch (op) {
      case OperationType::Bypass:
        return value;
      case OperationType::Sqrt:
        return std::sqrtf(value);
      case OperationType::Ln:
        return std::log(value);
    }
  };

  bool applyComparison(ComparatorType comp, float bias, float value) {
    switch (comp) {
      case ComparatorType::LessThan:
        return value < bias;
      case ComparatorType::GreaterThan:
        return value > bias;
    }
  };
};
