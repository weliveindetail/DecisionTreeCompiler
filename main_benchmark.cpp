#include <benchmark/benchmark.h>

#include "benchmark/BenchmarkInterpreter.h"
#include "benchmark/BenchmarkSingleCodegen.h"
#include "benchmark/BenchmarkMixedCodegen.h"
#include "benchmark/Shared.h"

int BenchmarkId = 0;

std::string makeBenchmarkName(const char *target, int depth, int features) {
  std::string name(target);
  name.resize(23, ' ');
  name += std::to_string(depth);
  name.resize(30, ' ');
  name += std::to_string(features);
  name.resize(39, ' ');
  return name;
}

template <class Benchmark_f>
void addBenchmark(Benchmark_f lambda, const char *name, int depth, int features) {
  auto caption = makeBenchmarkName(name, depth, features);
  auto benchmark = ::benchmark::RegisterBenchmark(caption.data(), lambda,
                                                  BenchmarkId++, depth,
                                                  features);
  benchmark->MinTime(3.0)->Threads(2)->UseRealTime();
}

int main(int argc, char** argv) {
  printf("Target                 Depth  Features Flags\n");

  int f = 10000;
  std::vector<int> treeDepths{2, 3, 12};
  initializeSharedData(treeDepths, {f});

  addBenchmark(BMInterpreter, "Interpreter", 12, f);
  addBenchmark(BMInterpreterValueBased, "InterpreterVB", 12, f);
  addBenchmark(BMCodegenAdaptive, "AdaptiveCodegen", 12, f);
  addBenchmark(BMCodegenL1IfThenElse, "PureL1IfThenElse", 12, f);

  addBenchmark(BMCodegenL1IfThenElse, "PureL1IfThenElse", 3, f);
  addBenchmark(BMCodegenL3SubtreeSwitchAVX, "PureL3SubtreeSwitchAVX", 3, f);

  addBenchmark(BMCodegenL1IfThenElse, "PureL1IfThenElse", 2, f);
  addBenchmark(BMCodegenL2SubtreeSwitch, "PureL2SubtreeSwitch", 2, f);

  /*
  std::vector<int> treeDepths{6, 9, 12, 15};
  std::vector<int> dataSetFeatures {5, 10000};
  initializeSharedData(treeDepths, dataSetFeatures);

  for (int f : dataSetFeatures) {
    for (int d : treeDepths) {
      addBenchmark(BMInterpreter, "Interpreter", d, f);
      addBenchmark(BMInterpreterValueBased, "InterpreterVB", d, f);
      addBenchmark(BMCodegenAdaptive, "AdaptiveCodegen", d, f);
      addBenchmark(BMCodegenL1IfThenElse, "PureL1IfThenElse", d, f);

      if (d % 2 == 0)
        addBenchmark(BMCodegenL2SubtreeSwitch,
                     "PureL2SubtreeSwitch", d, f);

      if (d % 3 == 0)
        addBenchmark(BMCodegenL3SubtreeSwitchAVX,
                     "PureL3SubtreeSwitchAVX", d, f);
    }
  }*/

  ::benchmark::Initialize(&argc, argv);
  ::benchmark::RunSpecifiedBenchmarks();
}
