#include <array>
#include <cassert>
#include <chrono>
#include <cstdio>
#include <thread>

#include "CompiledResolver.h"
#include "RegularResolver.h"
#include "Utils.h"

template <unsigned long DataSetFeatures_>
std::array<float, DataSetFeatures_> makeRandomDataSet() {
  std::array<float, DataSetFeatures_> dataSet;

  for (int i = 0; i < DataSetFeatures_; i++)
    dataSet[i] = makeRandomFloat(); // range [0, 1)

  return dataSet;
};

template <unsigned long TreeDepth_, unsigned long DataSetFeatures_>
std::tuple<unsigned long, std::chrono::nanoseconds>
runBenchmarkEvalRegular(const DecisionTree &tree,
                        const std::array<float, DataSetFeatures_> &dataSet) {
  using namespace std::chrono;
  auto start = high_resolution_clock::now();

  unsigned long leafIdx =
      computeLeafNodeIdxForDataSet<TreeDepth_, DataSetFeatures_>(tree, dataSet);

  auto end = high_resolution_clock::now();
  return std::make_tuple(leafIdx, duration_cast<nanoseconds>(end - start));
};

template <unsigned long TreeDepth_, unsigned long DataSetFeatures_>
std::tuple<unsigned long, std::chrono::nanoseconds>
runBenchmarkEvalCompiled(const DecisionTree &tree,
                         const std::array<float, DataSetFeatures_> &dataSet) {
  using namespace std::chrono;
  auto start = high_resolution_clock::now();

  unsigned long leafIdx =
      computeLeafNodeIdxForDataSetCompiled<DataSetFeatures_>(tree, dataSet);

  auto end = high_resolution_clock::now();
  return std::make_tuple(leafIdx, duration_cast<nanoseconds>(end - start));
};

template <unsigned long TreeDepth_, unsigned long DataSetFeatures_>
void runBenchmark(int repetitions) {
  printf("Building decision tree with depth %lu\n", TreeDepth_);
  auto tree = makeDecisionTree<TreeDepth_, DataSetFeatures_>();

  printf("\nCompiling evaluators for %lu nodes\n", tree.size());
  initializeLLVM();
  compileEvaluators(tree, 3);

  {
    printf("\n\nBenchmarking: %d runs with %lu features\n", repetitions,
           DataSetFeatures_);

    unsigned long resultRegular;
    unsigned long resultCompiled;

    std::chrono::nanoseconds runtimeRegular;
    std::chrono::nanoseconds runtimeCompiled;

    float totalRuntimeRegular = 0;
    float totalRuntimeCompiled = 0;

    for (int i = 0; i < repetitions; i++) {
      auto dataSet = makeRandomDataSet<DataSetFeatures_>();

      std::tie(resultRegular, runtimeRegular) =
          runBenchmarkEvalRegular<TreeDepth_, DataSetFeatures_>(tree, dataSet);

      std::tie(resultCompiled, runtimeCompiled) =
          runBenchmarkEvalCompiled<TreeDepth_, DataSetFeatures_>(tree, dataSet);

      assert(resultRegular == resultCompiled);

      totalRuntimeRegular += runtimeRegular.count();
      totalRuntimeCompiled += runtimeCompiled.count();
    }

    float averageRuntimeRegular = totalRuntimeRegular / repetitions;
    printf("Average evaluation time regular: %fns\n", averageRuntimeRegular);

    float averageRuntimeCompiled = totalRuntimeCompiled / repetitions;
    printf("Average evaluation time compiled: %fns\n\n",
           averageRuntimeCompiled);
  }

  shutdownLLVM();
}

int main() {
  {
    int repetitions = 1000;
    constexpr auto treeDepth = 15; // depth 25 ~ 1GB data
    constexpr auto dataSetFeatures = 100;
    runBenchmark<treeDepth, dataSetFeatures>(repetitions);
  }

  return 0;
}
