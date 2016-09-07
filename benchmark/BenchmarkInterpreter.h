#pragma once

#include <benchmark/benchmark.h>

#include <driver/utility/Interpreter.h>

#include "benchmark/Shared.h"

auto BenchmarkInterpreter = [](::benchmark::State& st, int id, int depth, int features) {
  DecisionTree tree = selectDecisionTree(id, depth, features);
  std::vector<float> dataSet = selectDataSet(id, features);
  Interpreter resolver;

  while (st.KeepRunning()) {
    benchmark::DoNotOptimize(resolver.run(tree, dataSet));
  }
};
