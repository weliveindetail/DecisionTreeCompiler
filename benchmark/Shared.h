#pragma once

#include <cstdint>
#include <unordered_map>
#include <vector>

#include <data/DataSetFactory.h>
#include <data/DecisionTree.h>
#include <data/DecisionTreeNode.h>

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

std::vector<float> selectDataSet(int benchmarkId, int features) {
  auto &collection = DataSetCollections[features];
  size_t idx;

  {
    std::lock_guard<std::mutex> lock(DataSetIdxsAccess);

    idx = DataSetIdxs[benchmarkId][features];
    DataSetIdxs[benchmarkId][features] = (idx + 1) % collection.size();
  }

  return collection[idx];
}

/*template <int N>
struct flag
{
  friend constexpr int adl_flag (flag<N>);
};

template <int N, int = adl_flag(flag<N>{})>
constexpr int reader(int, flag<N>)
{
  return N;
}

template <int N>
constexpr int reader(
    float, flag<N>, int R = reader(0, flag<N-1>{}))
{
  return R;
}

constexpr int reader(float, flag<0>)
{
  return 0;
}

template <int N>
struct writer
{
  friend constexpr int adl_flag (flag<N>)
  {
    return N;
  }
  static constexpr int value = N;
};

template<int N = 1, int C = reader (0, flag<32> ())>
int constexpr nextBenchmarkId (int R = writer<C + N>::value) {
  return R;
}*/
