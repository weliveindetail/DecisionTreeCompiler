#include <benchmark/benchmark.h>

#include "benchmark/BenchmarkInterpreter.h"
#include "benchmark/BenchmarkSingleCodegen.h"
#include "benchmark/BenchmarkMixedCodegen.h"
#include "benchmark/Shared.h"

int BenchmarkId = 0;

std::string makeBenchmarkName(const char *target, int depth, int features) {
  std::string name(target);
  name.resize(20, ' ');
  name += std::to_string(depth);
  name.resize(27, ' ');
  name += std::to_string(features);
  name.resize(36, ' ');
  return name;
}

template <class Benchmark_f>
void addBenchmark(Benchmark_f lambda, const char *name, int depth, int features) {
  ::benchmark::RegisterBenchmark(
      makeBenchmarkName(name, depth, features).data(),
      lambda, BenchmarkId++, depth, features)->Threads(2);
}

int main(int argc, char** argv) {
  printf("Target              Depth  Features\n");

  std::vector<int> treeDepths{3, 4, 6, 8, 9, 12};
  std::vector<int> dataSetFeatures {5, 10000};
  initializeSharedData(treeDepths, dataSetFeatures);

  for (int f : dataSetFeatures) {
    for (int d : treeDepths) {
      addBenchmark(BenchmarkInterpreter, "Interpreter", d, f);
      addBenchmark(BenchmarkInterpreterValueBased, "InterpreterVB", d, f);
      addBenchmark(BenchmarkCodegenAdaptive, "AdaptiveCodgen", d, f);
      addBenchmark(BenchmarkCodegenL1IfThenElse, "L1IfThenElse", d, f);

      if (d % 2 == 0)
        addBenchmark(BenchmarkCodegenL2SubtreeSwitch, "L2SubtreeSwitch", d, f);

      if (d % 3 == 0)
        addBenchmark(BenchmarkCodegenL3SubtreeSwitchAVX, "L3SubtreeSwitchAVX", d, f);
    }
  }

  ::benchmark::Initialize(&argc, argv);
  ::benchmark::RunSpecifiedBenchmarks();
}
