#include <memory>
#include <vector>

#include "benchmark/benchmark.h"

#include "DataSet.h"
#include "DecisionTree.h"
#include "RegularResolver.h"
#include "CompiledResolver.h"

int treeDepth = 16;
int dataSetFeatures = 100;
int compiledFunctionLevels = 16;
int compiledFunctionSwitchLevels = 2;

std::unique_ptr<RegularResolver> regularResolver;
std::unique_ptr<CompiledResolver> compiledResolver;

DecisionTree_t tree = prepareDecisionTree(treeDepth, dataSetFeatures);
std::vector<DataSet_t> dataSets = makeRandomDataSets(100, dataSetFeatures, tree); // hack

DataSet_t& selectRandomDataSet() {
  return dataSets[makeRandomInt(0, dataSets.size() - 1)];
};

static void BM_RegularEvaluation(benchmark::State& state) {
  DataSet_t& dataSet = selectRandomDataSet();
  int64_t result = 0;

  while (state.KeepRunning()) {
    result = regularResolver->run(tree, dataSet);
  }

  assert(dataSet[dataSetFeatures] == (float)result);
}

static void BM_CompiledEvaluation(benchmark::State& state) {
  DataSet_t& dataSet = selectRandomDataSet();
  int64_t result = 0;

  while (state.KeepRunning()) {
    result = compiledResolver->run(tree, dataSet);
  }

  assert(dataSet[dataSetFeatures] == (float)result);
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
