#include <memory>
#include <unordered_map>
#include <vector>

#include "benchmark/benchmark.h"

#include "Utils.h"
#include "DecisionTree.h"
#include "RegularResolver.h"
#include "CompiledResolver.h"

int treeDepth = 6;
int dataSetFeatures = 100;
int compiledFunctionLevels = 6;
int compiledFunctionSwitchLevels = 3;

using DataSet_t = std::vector<float>;

std::unique_ptr<RegularResolver> regularResolver;
std::unique_ptr<CompiledResolver> compiledResolver;

DecisionTree_t tree = prepareDecisionTree(treeDepth, dataSetFeatures);
std::vector<DataSet_t> dataSets = prepareRandomDataSets(100, dataSetFeatures);

DataSet_t& selectRandomDataSet() {
  return dataSets[makeRandomInt(0, dataSets.size() - 1)];
};

static void BM_RegularEvaluation(benchmark::State& state) {
  while (state.KeepRunning()) {
    regularResolver->run(tree, selectRandomDataSet());
  }
}

static void BM_CompiledEvaluation(benchmark::State& state) {
  while (state.KeepRunning()) {
    compiledResolver->run(tree, selectRandomDataSet());
  }
}

BENCHMARK(BM_RegularEvaluation);
BENCHMARK(BM_CompiledEvaluation);

int main(int argc, char** argv) {
  regularResolver = std::make_unique<RegularResolver>();
  compiledResolver = std::make_unique<CompiledResolver>(
      tree, dataSetFeatures, compiledFunctionLevels,
      compiledFunctionSwitchLevels);

  ::benchmark::Initialize(&argc, argv);
  ::benchmark::RunSpecifiedBenchmarks();
}
