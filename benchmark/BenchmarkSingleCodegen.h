#pragma once

#include <benchmark/benchmark.h>

#include <codegen/CodeGeneratorSelector.h>
#include <codegen/L1IfThenElse.h>
#include <codegen/L3SubtreeSwitchAVX.h>
#include <codegen/LXSubtreeSwitch.h>
#include <driver/JitDriver.h>

#include "benchmark/Shared.h"

auto BenchmarkCodegenL1IfThenElse = [](::benchmark::State& st, int id, int depth, int features) {
  DecisionTree tree = selectDecisionTree(id, depth, features);
  std::vector<float> dataSet = selectDataSet(id, features);

  JitDriver jitDriver;

  jitDriver.setCodegenSelector(makeLambdaSelector(
      [](const CompilerSession &session, int remainingLevels) {
        static L1IfThenElse codegen;
        return &codegen;
      }));

  JitCompileResult jitResult = jitDriver.run(std::move(tree));
  JitCompileResult::Evaluator_f *compiledResover = jitResult.EvaluatorFunction;

  while (st.KeepRunning()) {
    benchmark::DoNotOptimize(compiledResover(dataSet.data()));
  }
};

auto BenchmarkCodegenL2SubtreeSwitch = [](::benchmark::State& st, int id, int depth, int features) {
  DecisionTree tree = selectDecisionTree(id, depth, features);
  std::vector<float> dataSet = selectDataSet(id, features);

  JitDriver jitDriver;

  jitDriver.setCodegenSelector(makeLambdaSelector(
      [](const CompilerSession &session, int remainingLevels) {
        static LXSubtreeSwitch codegen(2);
        return &codegen;
      }));

  JitCompileResult jitResult = jitDriver.run(std::move(tree));
  JitCompileResult::Evaluator_f *compiledResover = jitResult.EvaluatorFunction;

  while (st.KeepRunning()) {
    benchmark::DoNotOptimize(compiledResover(dataSet.data()));
  }
};

auto BenchmarkCodegenL3SubtreeSwitchAVX = [](::benchmark::State& st, int id, int depth, int features) {
  DecisionTree tree = selectDecisionTree(id, depth, features);
  std::vector<float> dataSet = selectDataSet(id, features);

  JitDriver jitDriver;

  jitDriver.setCodegenSelector(makeLambdaSelector(
      [](const CompilerSession &session, int remainingLevels) {
        static L3SubtreeSwitchAVX codegen;
        return &codegen;
      }));

  JitCompileResult jitResult = jitDriver.run(std::move(tree));
  JitCompileResult::Evaluator_f *compiledResover = jitResult.EvaluatorFunction;

  while (st.KeepRunning()) {
    benchmark::DoNotOptimize(compiledResover(dataSet.data()));
  }
};
