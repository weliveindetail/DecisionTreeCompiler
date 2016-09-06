#pragma once

#include <gtest/gtest.h>

#include "codegen/CodeGeneratorSelector.h"
#include "codegen/L1IfThenElse.h"
#include "data/DataSetFactory.h"
#include "data/DecisionTree.h"
#include "driver/JitDriver.h"

TEST(SingleCodegenL1, L1IfThenElse) {
  DecisionTreeFactory factory;
  JitDriver jitDriver;

  jitDriver.setCodegenSelector(makeLambdaSelector(
      [](const CompilerSession &session, int remainingLevels) {
        static L1IfThenElse codegen;
        return &codegen;
      }));

  { // test with single data-set feature
    DecisionTree tree = factory.makePerfectTrivialGradientTree(1);
    JitCompileResult result = jitDriver.run(std::move(tree));

    DataSetFactory data;
    auto *fp = result.EvaluatorFunction;

    EXPECT_EQ(1, fp(data.makeTrivialDataSet(1.0f / 4).data()));
    EXPECT_EQ(2, fp(data.makeTrivialDataSet(3.0f / 4).data()));
  }
  { // test with individual data-set features
    DecisionTree tree = factory.makePerfectDistinctGradientTree(1);
    JitCompileResult result = jitDriver.run(std::move(tree));

    auto *fp = result.EvaluatorFunction;
    DataSetFactory data(std::move(result.Tree), 1);

    auto left = NodeEvaluation::ContinueZeroLeft;
    auto right = NodeEvaluation::ContinueOneRight;

    EXPECT_EQ(1, fp(data.makeDistinctDataSet(left).data()));
    EXPECT_EQ(2, fp(data.makeDistinctDataSet(right).data()));
  }
}

TEST(SingleCodegenL1, L1SubtreeSwitch) {
  DecisionTreeFactory factory;
  JitDriver jitDriver;

  jitDriver.setCodegenSelector(makeLambdaSelector(
      [](const CompilerSession &session, int remainingLevels) {
        static LXSubtreeSwitch codegen(1);
        return &codegen;
      }));

  { // test with single data-set feature
    DecisionTree tree = factory.makePerfectTrivialGradientTree(1);
    JitCompileResult result = jitDriver.run(std::move(tree));

    DataSetFactory data;
    auto *fp = result.EvaluatorFunction;

    EXPECT_EQ(1, fp(data.makeTrivialDataSet(1.0f / 4).data()));
    EXPECT_EQ(2, fp(data.makeTrivialDataSet(3.0f / 4).data()));
  }
  { // test with individual data-set features
    DecisionTree tree = factory.makePerfectDistinctGradientTree(1);
    JitCompileResult result = jitDriver.run(std::move(tree));

    auto *fp = result.EvaluatorFunction;
    DataSetFactory data(std::move(result.Tree), 1);

    auto left = NodeEvaluation::ContinueZeroLeft;
    auto right = NodeEvaluation::ContinueOneRight;

    EXPECT_EQ(1, fp(data.makeDistinctDataSet(left).data()));
    EXPECT_EQ(2, fp(data.makeDistinctDataSet(right).data()));
  }
}
