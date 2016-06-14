#include <array>
#include <cassert>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <thread>

#include "CompiledResolver.h"
#include "RegularResolver.h"
#include "Utils.h"

template <int DataSetFeatures_>
std::array<float, DataSetFeatures_> makeRandomDataSet() {
  std::array<float, DataSetFeatures_> dataSet;

  for (int i = 0; i < DataSetFeatures_; i++)
    dataSet[i] = makeRandomFloat(); // range [0, 1)

  return dataSet;
};

template <int DataSetFeatures_>
std::tuple<int64_t, std::chrono::nanoseconds>
runBenchmarkEvalRegular(const DecisionTree &tree,
                        const std::array<float, DataSetFeatures_> &dataSet) {
  using namespace std::chrono;
  auto start = high_resolution_clock::now();

  int64_t leafIdx =
      computeLeafNodeIdxForDataSet<DataSetFeatures_>(tree, dataSet);

  auto end = high_resolution_clock::now();
  return std::make_tuple(leafIdx, duration_cast<nanoseconds>(end - start));
};

template <int DataSetFeatures_>
std::tuple<int64_t, std::chrono::nanoseconds>
runBenchmarkEvalCompiled(const DecisionTree &tree,
                         const std::array<float, DataSetFeatures_> &dataSet) {
  using namespace std::chrono;
  auto start = high_resolution_clock::now();

  int64_t leafIdx =
      computeLeafNodeIdxForDataSetCompiled<DataSetFeatures_>(tree, dataSet);

  auto end = high_resolution_clock::now();
  return std::make_tuple(leafIdx, duration_cast<nanoseconds>(end - start));
};

template <int TreeDepth_, int DataSetFeatures_>
void runBenchmark(int repetitions, int compiledFunctionDepth) {
  printf("Building decision tree with depth %d\n", TreeDepth_);
  auto tree = makeDecisionTree<TreeDepth_, DataSetFeatures_>();

  int64_t expectedEvaluators =
      getNumCompiledEvaluators<TreeDepth_>(compiledFunctionDepth);
  printf("Compiling %lld evaluators for %lu nodes", expectedEvaluators,
         tree.size());

  initializeLLVM();
  int actualEvaluators = compileEvaluators(tree, compiledFunctionDepth);
  assert(expectedEvaluators == actualEvaluators);

  {
    printf("\n\nBenchmarking: %d runs with %d features\n", repetitions,
           DataSetFeatures_);

    int64_t resultRegular;
    int64_t resultCompiled;

    std::chrono::nanoseconds runtimeRegular;
    std::chrono::nanoseconds runtimeCompiled;

    float totalRuntimeRegular = 0;
    float totalRuntimeCompiled = 0;

    for (int i = 0; i < repetitions; i++) {
      auto dataSet = makeRandomDataSet<DataSetFeatures_>();

      std::tie(resultRegular, runtimeRegular) =
          runBenchmarkEvalRegular<DataSetFeatures_>(tree, dataSet);

      std::tie(resultCompiled, runtimeCompiled) =
          runBenchmarkEvalCompiled<DataSetFeatures_>(tree, dataSet);

      assert(resultRegular == resultCompiled);

      totalRuntimeRegular += runtimeRegular.count();
      totalRuntimeCompiled += runtimeCompiled.count();
    }

    float averageRuntimeRegular = totalRuntimeRegular / repetitions;
    printf("Average evaluation time regular: %fns\n", averageRuntimeRegular);

    float averageRuntimeCompiled = totalRuntimeCompiled / repetitions;
    printf("Average evaluation time compiled: %fns\n", averageRuntimeCompiled);
  }

  shutdownLLVM();
}

int main() {
  int repetitions = 1000;
  int compiledFunctionDepth = 10;
  constexpr int treeDepth = 12; // depth 25 ~ 1GB data
  constexpr int dataSetFeatures = 100;

  runBenchmark<treeDepth, dataSetFeatures>(repetitions, compiledFunctionDepth);

  return 0;
}
