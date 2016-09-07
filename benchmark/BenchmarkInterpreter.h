#pragma once

#include <benchmark/benchmark.h>
#include <driver/utility/Interpreter.h>

#include "benchmark/Shared.h"

auto BMInterpreter = [](::benchmark::State& st, int id, int depth, int features) {
  DecisionTree tree = selectDecisionTree(id, depth, features);
  Interpreter resolver;

  float *data1 = selectRandomDataSet(id, features);
  float *data2 = selectRandomDataSet(id, features);
  float *data3 = selectRandomDataSet(id, features);
  float *data4 = selectRandomDataSet(id, features);
  float *data5 = selectRandomDataSet(id, features);

  while (st.KeepRunning()) {
    benchmark::DoNotOptimize(resolver.run(tree, data1));
    benchmark::DoNotOptimize(resolver.run(tree, data2));
    benchmark::DoNotOptimize(resolver.run(tree, data3));
    benchmark::DoNotOptimize(resolver.run(tree, data4));
    benchmark::DoNotOptimize(resolver.run(tree, data5));
  }
};

auto BMInterpreterValueBased = [](::benchmark::State& st, int id, int depth, int features) {
  DecisionTree tree = selectDecisionTree(id, depth, features);
  Interpreter resolver;

  float *data1 = selectRandomDataSet(id, features);
  float *data2 = selectRandomDataSet(id, features);
  float *data3 = selectRandomDataSet(id, features);
  float *data4 = selectRandomDataSet(id, features);
  float *data5 = selectRandomDataSet(id, features);

  while (st.KeepRunning()) {
    benchmark::DoNotOptimize(resolver.runValueBased(tree, data1));
    benchmark::DoNotOptimize(resolver.runValueBased(tree, data2));
    benchmark::DoNotOptimize(resolver.runValueBased(tree, data3));
    benchmark::DoNotOptimize(resolver.runValueBased(tree, data4));
    benchmark::DoNotOptimize(resolver.runValueBased(tree, data5));
  }
};
