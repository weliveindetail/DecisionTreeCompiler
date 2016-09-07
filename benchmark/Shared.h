#pragma once

#include <cstdint>
#include <unordered_map>
#include <vector>

#include <data/DataSetFactory.h>
#include <data/DecisionTree.h>
#include <data/DecisionTreeNode.h>
#include <Utils.h>

std::unordered_map<int, DecisionTree> RegularDecisionTrees;
std::unordered_map<int, std::vector<std::vector<float>>> DataSetCollections;

std::mutex DataSetIdxsAccess;
std::unordered_map<uint64_t, std::unordered_map<int, size_t>> DataSetIdxs;

int makeKeyForDecisionTree(int depth, int features) {
  assert(depth > 0 && depth < 100);
  assert(features > 0 && features < (int)((1 << 30) / 100));
  return features * 100 + depth;
}

void initializeSharedData(std::vector<int> treeDepths,
                          std::vector<int> dataSetFeatures) {
  DecisionTreeFactory treeFactory;
  DecisionTree unused;

  for (int features : dataSetFeatures) {
    DataSetFactory dsFactory(unused.copy(), features);
    DataSetCollections[features] = dsFactory.makeRandomDataSets(10);

    for (int depth : treeDepths) {
      int key = makeKeyForDecisionTree(depth, features);
      RegularDecisionTrees[key] = treeFactory.makePerfectRandomTree(depth, features);
    }
  }
}

DecisionTree selectDecisionTree(int benchmarkId, int depth, int features) {
  int key = makeKeyForDecisionTree(depth, features);
  return RegularDecisionTrees[key].copy();
}

float *selectRandomDataSet(int benchmarkId, int features) {
  auto &collection = DataSetCollections[features];
  auto idx = makeRandomInt<size_t>(0, collection.size() - 1);
  return collection[idx].data();
}
