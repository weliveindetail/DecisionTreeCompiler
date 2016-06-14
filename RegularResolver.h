#pragma once

#include "DecisionTree.h"

template <unsigned long DataSetFeatures_>
int64_t
computeChildNodeForDataSet(const TreeNode &currentNode,
                           const std::array<float, DataSetFeatures_> &dataSet) {

  auto applyOperator = [](OperationType op, float value) {
    switch (op) {
    case OperationType::Bypass:
      return value;
    case OperationType::Sqrt:
      return std::sqrtf(value);
    case OperationType::Ln:
      return std::log(value);
    }
  };

  auto applyComparison = [](ComparatorType comp, float bias, float value) {
    switch (comp) {
    case ComparatorType::LessThan:
      return value < bias;
    case ComparatorType::GreaterThan:
      return value > bias;
    }
  };

  float featureValue = dataSet[currentNode.DataSetFeatureIdx];
  float comparableFeatureValue = applyOperator(currentNode.Op, featureValue);

  bool evaluatesTrue = applyComparison(currentNode.Comp, currentNode.Bias,
                                       comparableFeatureValue);
  /*
  printf("%f %s %f -> %s\n",
         comparableFeatureValue,
         currentNode.Comp == ComparatorType::LessThan ? "<" : ">",
         currentNode.Bias,
         evaluatesTrue ? "true" : "false");
  //*/

  return evaluatesTrue ? currentNode.getTrueChildIdx()
                       : currentNode.getFalseChildIdx();
}

template <unsigned long DataSetFeatures_>
int64_t computeLeafNodeIdxForDataSet(
    const DecisionTree &tree,
    const std::array<float, DataSetFeatures_> &dataSet) {
  unsigned long treeNodeIdx = 0;

  while (!tree.at(treeNodeIdx).isLeaf()) {
    treeNodeIdx = computeChildNodeForDataSet(tree.at(treeNodeIdx), dataSet);
  }

  return treeNodeIdx;
}
