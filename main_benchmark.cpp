#include <memory>
#include <vector>

#include "benchmark/benchmark.h"

#include "data/DataSetFactory.h"
#include "data/DecisionTree.h"

#include "driver/JitDriver.h"
#include "driver/utility/Interpreter.h"

uint8_t TreeDepth = 12;
uint64_t InvalidNodeIdx = DecisionTreeNode().getIdx();

DecisionTree Tree;
std::vector<std::vector<float>> DataSets;
std::vector<uint64_t> Results;

std::unique_ptr<Interpreter> InterpreterResolver;
JitCompileResult::Evaluator_f *CompiledResolver;

void submitDataSetEvalResult(size_t idx, uint64_t result) {
  assert(Results[idx] == InvalidNodeIdx || Results[idx] == result);
  Results[idx] = result;
}

static void BM_RegularEvaluation(benchmark::State& state) {
  static size_t idx = 0;
  uint64_t result = 0;

  while (state.KeepRunning()) {
    result = InterpreterResolver->run(Tree, DataSets[idx]);
  }

  submitDataSetEvalResult(idx, result);
  if (idx < DataSets.size() - 1) idx++;
}

static void BM_CompiledEvaluation(benchmark::State& state) {
  static size_t idx = 0;
  uint64_t result = 0;

  while (state.KeepRunning()) {
    result = CompiledResolver(DataSets[idx].data());
  }

  submitDataSetEvalResult(idx, result);
  if (idx < DataSets.size() - 1) idx++;
}

BENCHMARK(BM_RegularEvaluation);
BENCHMARK(BM_CompiledEvaluation);

int main(int argc, char** argv) {
  printf("Preparing benchmark data..\n");
  DecisionTreeFactory treeFactory;
  Tree = treeFactory.makePerfectDistinctUniformTree(TreeDepth);
  uint32_t dataSetFeatures = TreeNodes(TreeDepth);

  DataSetFactory dsFactory(Tree.copy(), dataSetFeatures);
  DataSets = dsFactory.makeRandomDataSets(100);

  Results.resize(DataSets.size());
  std::fill(Results.begin(), Results.end(), InvalidNodeIdx);

  printf("Compiling decision tree..\n");
  JitDriver jitDriver;
  JitCompileResult result = jitDriver.run(std::move(Tree));
  CompiledResolver = result.EvaluatorFunction;
  Tree = std::move(result.Tree);

  InterpreterResolver = std::make_unique<Interpreter>();

  printf("\n");
  ::benchmark::Initialize(&argc, argv);
  ::benchmark::RunSpecifiedBenchmarks();
}
