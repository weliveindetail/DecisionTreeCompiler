#pragma once

#include <benchmark/benchmark.h>

#include <driver/JitDriver.h>

#include "benchmark/Shared.h"

auto BenchmarkCodegenAdaptive = [](::benchmark::State& st, int id, int depth, int features) {
  DecisionTree tree = selectDecisionTree(id, depth, features);
  std::vector<float> dataSet = selectDataSet(id, features);

  JitDriver jitDriver;
  JitCompileResult jitResult = jitDriver.run(std::move(tree));
  JitCompileResult::Evaluator_f *compiledResover = jitResult.EvaluatorFunction;

  while (st.KeepRunning()) {
    benchmark::DoNotOptimize(compiledResover(dataSet.data()));
  }
};
