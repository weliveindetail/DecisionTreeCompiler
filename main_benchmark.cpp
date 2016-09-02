#include <memory>
#include <vector>

#include "benchmark/benchmark.h"

#include "DataSet.h"
#include "driver/utility/Interpreter.h"
#include "RegularResolver.h"
#include "CompiledResolver.h"

#include "data/DataSetFactory.h"
#include "data/DecisionTree.h"

#include "driver/JitDriver.h"

uint8_t treeDepth = 16;
uint32_t dataSetFeatures = 100;

uint64_t InvalidNodeIdx = DecisionTreeNode().getIdx();

DecisionTreeFactory treeFactory;
DecisionTree tree = treeFactory.makeRandomRegular(treeDepth, dataSetFeatures);

DataSetFactory dataSetFactory(tree.copy(), dataSetFeatures);
std::vector<DataSet_t> dataSets = dataSetFactory.makeRandomDataSets(100);
std::vector<uint64_t> results(dataSets.size(), InvalidNodeIdx);

std::unique_ptr<Interpreter> interpreter;
JitCompileResult::Evaluator_f *compiledResolver;

size_t selectRandomDataSetIdx() {
  assert(!dataSets.empty());
  return makeRandomInt(0ul, dataSets.size() - 1);
}

void submitDataSetInterpretedResult(size_t idx, uint64_t result) {
  assert(results[idx] == InvalidNodeIdx || results[idx] == result);
  results[idx] = result;
}

static void BM_RegularEvaluation(benchmark::State& state) {
  size_t idx = selectRandomDataSetIdx();
  int64_t result = 0;

  while (state.KeepRunning()) {
    result = interpreter->run(tree, dataSets[idx]);
  }

  submitDataSetInterpretedResult(idx, result);
}

/*static void BM_CompiledEvaluation(benchmark::State& state) {
  DataSet_t& dataSet = selectRandomDataSet();
  int64_t result = 0;

  while (state.KeepRunning()) {
    result = compiledResolver->run(tree, dataSet);
  }

  assert(dataSet[dataSetFeatures] == (float)result);
}*/

BENCHMARK(BM_RegularEvaluation);
//BENCHMARK(BM_CompiledEvaluation);

int main(int argc, char** argv) {
  interpreter = std::make_unique<Interpreter>();

  //compiledResolver = std::make_unique<CompiledResolver>(
  //    tree, dataSetFeatures, compiledFunctionLevels,
  //    compiledFunctionSwitchLevels);

  ::benchmark::Initialize(&argc, argv);
  ::benchmark::RunSpecifiedBenchmarks();
}
