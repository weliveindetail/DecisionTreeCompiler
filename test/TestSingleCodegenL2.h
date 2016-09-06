#pragma once

#include <gtest/gtest.h>

#include "codegen/CodeGeneratorSelector.h"
#include "codegen/LXSubtreeSwitch.h"
#include "data/DataSetFactory.h"
#include "data/DecisionTree.h"
#include "driver/JitDriver.h"

TEST(SingleCodegenL2, L1IfThenElse) {
  DecisionTreeFactory factory;
  JitDriver jitDriver;

  jitDriver.setCodegenSelector(makeLambdaSelector(
      [](const CompilerSession &session, int remainingLevels) {
        static L1IfThenElse codegen;
        return &codegen;
      }));

  { // test with single data-set feature
    DecisionTree tree = factory.makePerfectTrivialGradientTree(2);
    JitCompileResult result = jitDriver.run(std::move(tree));

    DataSetFactory data;
    auto *fp = result.EvaluatorFunction;

    EXPECT_EQ(3, fp(data.makeTrivialDataSet(1.0f / 8).data()));
    EXPECT_EQ(4, fp(data.makeTrivialDataSet(3.0f / 8).data()));
    EXPECT_EQ(5, fp(data.makeTrivialDataSet(5.0f / 8).data()));
    EXPECT_EQ(6, fp(data.makeTrivialDataSet(7.0f / 8).data()));
  }
  { // test with individual data-set features
    DecisionTree tree = factory.makePerfectDistinctGradientTree(2);
    JitCompileResult result = jitDriver.run(std::move(tree));

    auto *fp = result.EvaluatorFunction;
    DataSetFactory data(std::move(result.Tree), 3);

    auto left = NodeEvaluation::ContinueZeroLeft;
    auto right = NodeEvaluation::ContinueOneRight;

    EXPECT_EQ(3, fp(data.makeDistinctDataSet(left, left).data()));
    EXPECT_EQ(4, fp(data.makeDistinctDataSet(left, right).data()));
    EXPECT_EQ(5, fp(data.makeDistinctDataSet(right, left).data()));
    EXPECT_EQ(6, fp(data.makeDistinctDataSet(right, right).data()));
  }
}

TEST(SingleCodegenL2, L1SubtreeSwitch) {
  DecisionTreeFactory factory;
  JitDriver jitDriver;

  jitDriver.setCodegenSelector(makeLambdaSelector(
      [](const CompilerSession &session, int remainingLevels) {
        static LXSubtreeSwitch codegen(1);
        return &codegen;
      }));

  { // test with single data-set feature
    DecisionTree tree = factory.makePerfectTrivialGradientTree(2);
    JitCompileResult result = jitDriver.run(std::move(tree));

    DataSetFactory data;
    auto *fp = result.EvaluatorFunction;

    EXPECT_EQ(3, fp(data.makeTrivialDataSet(1.0f / 8).data()));
    EXPECT_EQ(4, fp(data.makeTrivialDataSet(3.0f / 8).data()));
    EXPECT_EQ(5, fp(data.makeTrivialDataSet(5.0f / 8).data()));
    EXPECT_EQ(6, fp(data.makeTrivialDataSet(7.0f / 8).data()));
  }
  { // test with individual data-set features
    DecisionTree tree = factory.makePerfectDistinctGradientTree(2);
    JitCompileResult result = jitDriver.run(std::move(tree));

    auto *fp = result.EvaluatorFunction;
    DataSetFactory data(std::move(result.Tree), 3);

    auto left = NodeEvaluation::ContinueZeroLeft;
    auto right = NodeEvaluation::ContinueOneRight;

    EXPECT_EQ(3, fp(data.makeDistinctDataSet(left, left).data()));
    EXPECT_EQ(4, fp(data.makeDistinctDataSet(left, right).data()));
    EXPECT_EQ(5, fp(data.makeDistinctDataSet(right, left).data()));
    EXPECT_EQ(6, fp(data.makeDistinctDataSet(right, right).data()));
  }
}

TEST(SingleCodegenL2, L2SubtreeSwitch) {
  DecisionTreeFactory factory;
  JitDriver jitDriver;

  jitDriver.setCodegenSelector(makeLambdaSelector(
      [](const CompilerSession &session, int remainingLevels) {
        static LXSubtreeSwitch codegen(2);
        return &codegen;
      }));

  { // test with single data-set feature
    DecisionTree tree = factory.makePerfectTrivialGradientTree(2);
    JitCompileResult result = jitDriver.run(std::move(tree));

    DataSetFactory data;
    auto *fp = result.EvaluatorFunction;

    EXPECT_EQ(3, fp(data.makeTrivialDataSet(1.0f / 8).data()));
    EXPECT_EQ(4, fp(data.makeTrivialDataSet(3.0f / 8).data()));
    EXPECT_EQ(5, fp(data.makeTrivialDataSet(5.0f / 8).data()));
    EXPECT_EQ(6, fp(data.makeTrivialDataSet(7.0f / 8).data()));
  }
  { // test with individual data-set features
    DecisionTree tree = factory.makePerfectDistinctGradientTree(2);
    JitCompileResult result = jitDriver.run(std::move(tree));

    auto *fp = result.EvaluatorFunction;
    DataSetFactory data(std::move(result.Tree), 3);

    auto left = NodeEvaluation::ContinueZeroLeft;
    auto right = NodeEvaluation::ContinueOneRight;

    EXPECT_EQ(3, fp(data.makeDistinctDataSet(left, left).data()));
    EXPECT_EQ(4, fp(data.makeDistinctDataSet(left, right).data()));
    EXPECT_EQ(5, fp(data.makeDistinctDataSet(right, left).data()));
    EXPECT_EQ(6, fp(data.makeDistinctDataSet(right, right).data()));
  }
}
