#pragma once

#include <benchmark/benchmark.h>
#include <driver/JitDriver.h>

#include "benchmark/Shared.h"

auto BMCodegenAdaptive = [](::benchmark::State& st, int id, int depth, int features) {
  DecisionTree tree = selectDecisionTree(id, depth, features);

  JitDriver jitDriver;
  JitCompileResult jitResult = jitDriver.run(std::move(tree));
  JitCompileResult::Evaluator_f *compiledResover = jitResult.EvaluatorFunction;

  float *data1 = selectRandomDataSet(id, features);
  float *data2 = selectRandomDataSet(id, features);
  float *data3 = selectRandomDataSet(id, features);

  while (st.KeepRunning()) {
    benchmark::DoNotOptimize(compiledResover(data1));
    benchmark::DoNotOptimize(compiledResover(data2));
    benchmark::DoNotOptimize(compiledResover(data3));
  }
};
