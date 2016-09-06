#pragma once

#include <gtest/gtest.h>

#include "codegen/CodeGeneratorSelector.h"
#include "codegen/L1IfThenElse.h"
#include "codegen/L3SubtreeSwitchAVX.h"
#include "codegen/LXSubtreeSwitch.h"
#include "data/DataSetFactory.h"
#include "data/DecisionTree.h"
#include "driver/JitDriver.h"

TEST(SingleCodegenL3, L1IfThenElse) {
  DecisionTreeFactory factory;
  JitDriver jitDriver;

  jitDriver.setCodegenSelector(makeLambdaSelector(
      [](const CompilerSession &session, int remainingLevels) {
        static L1IfThenElse codegen;
        return &codegen;
      }));

  { // test with single data-set feature
    DecisionTree tree = factory.makePerfectTrivialGradientTree(3);
    JitCompileResult result = jitDriver.run(std::move(tree));

    DataSetFactory data;
    auto *fp = result.EvaluatorFunction;

    EXPECT_EQ(7, fp(data.makeTrivialDataSet(1.0f / 16).data()));
    EXPECT_EQ(8, fp(data.makeTrivialDataSet(3.0f / 16).data()));
    EXPECT_EQ(9, fp(data.makeTrivialDataSet(5.0f / 16).data()));
    EXPECT_EQ(10, fp(data.makeTrivialDataSet(7.0f / 16).data()));
    EXPECT_EQ(11, fp(data.makeTrivialDataSet(9.0f / 16).data()));
    EXPECT_EQ(12, fp(data.makeTrivialDataSet(11.0f / 16).data()));
    EXPECT_EQ(13, fp(data.makeTrivialDataSet(13.0f / 16).data()));
    EXPECT_EQ(14, fp(data.makeTrivialDataSet(15.0f / 16).data()));
  }
  { // test with individual data-set features
    DecisionTree tree = factory.makePerfectDistinctGradientTree(3);
    JitCompileResult result = jitDriver.run(std::move(tree));

    auto *fp = result.EvaluatorFunction;
    DataSetFactory data(std::move(result.Tree), 7);

    auto le = NodeEvaluation::ContinueZeroLeft;
    auto ri = NodeEvaluation::ContinueOneRight;

    EXPECT_EQ(7, fp(data.makeDistinctDataSet(le, le, le).data()));
    EXPECT_EQ(8, fp(data.makeDistinctDataSet(le, le, ri).data()));
    EXPECT_EQ(9, fp(data.makeDistinctDataSet(le, ri, le).data()));
    EXPECT_EQ(10, fp(data.makeDistinctDataSet(le, ri, ri).data()));
    EXPECT_EQ(11, fp(data.makeDistinctDataSet(ri, le, le).data()));
    EXPECT_EQ(12, fp(data.makeDistinctDataSet(ri, le, ri).data()));
    EXPECT_EQ(13, fp(data.makeDistinctDataSet(ri, ri, le).data()));
    EXPECT_EQ(14, fp(data.makeDistinctDataSet(ri, ri, ri).data()));
  }
}

TEST(SingleCodegenL3, L1SubtreeSwitch) {
  DecisionTreeFactory factory;
  JitDriver jitDriver;

  jitDriver.setCodegenSelector(makeLambdaSelector(
      [](const CompilerSession &session, int remainingLevels) {
        static LXSubtreeSwitch codegen(1);
        return &codegen;
      }));

  { // test with single data-set feature
    DecisionTree tree = factory.makePerfectTrivialGradientTree(3);
    JitCompileResult result = jitDriver.run(std::move(tree));

    DataSetFactory data;
    auto *fp = result.EvaluatorFunction;

    EXPECT_EQ(7, fp(data.makeTrivialDataSet(1.0f / 16).data()));
    EXPECT_EQ(8, fp(data.makeTrivialDataSet(3.0f / 16).data()));
    EXPECT_EQ(9, fp(data.makeTrivialDataSet(5.0f / 16).data()));
    EXPECT_EQ(10, fp(data.makeTrivialDataSet(7.0f / 16).data()));
    EXPECT_EQ(11, fp(data.makeTrivialDataSet(9.0f / 16).data()));
    EXPECT_EQ(12, fp(data.makeTrivialDataSet(11.0f / 16).data()));
    EXPECT_EQ(13, fp(data.makeTrivialDataSet(13.0f / 16).data()));
    EXPECT_EQ(14, fp(data.makeTrivialDataSet(15.0f / 16).data()));
  }
  { // test with individual data-set features
    DecisionTree tree = factory.makePerfectDistinctGradientTree(3);
    JitCompileResult result = jitDriver.run(std::move(tree));

    auto *fp = result.EvaluatorFunction;
    DataSetFactory data(std::move(result.Tree), 7);

    auto le = NodeEvaluation::ContinueZeroLeft;
    auto ri = NodeEvaluation::ContinueOneRight;

    EXPECT_EQ(7, fp(data.makeDistinctDataSet(le, le, le).data()));
    EXPECT_EQ(8, fp(data.makeDistinctDataSet(le, le, ri).data()));
    EXPECT_EQ(9, fp(data.makeDistinctDataSet(le, ri, le).data()));
    EXPECT_EQ(10, fp(data.makeDistinctDataSet(le, ri, ri).data()));
    EXPECT_EQ(11, fp(data.makeDistinctDataSet(ri, le, le).data()));
    EXPECT_EQ(12, fp(data.makeDistinctDataSet(ri, le, ri).data()));
    EXPECT_EQ(13, fp(data.makeDistinctDataSet(ri, ri, le).data()));
    EXPECT_EQ(14, fp(data.makeDistinctDataSet(ri, ri, ri).data()));
  }
}

TEST(SingleCodegenL3, L3SubtreeSwitch) {
  DecisionTreeFactory factory;
  JitDriver jitDriver;

  jitDriver.setCodegenSelector(makeLambdaSelector(
      [](const CompilerSession &session, int remainingLevels) {
        static LXSubtreeSwitch codegen(3);
        return &codegen;
      }));

  { // test with single data-set feature
    DecisionTree tree = factory.makePerfectTrivialGradientTree(3);
    JitCompileResult result = jitDriver.run(std::move(tree));

    DataSetFactory data;
    auto *fp = result.EvaluatorFunction;

    EXPECT_EQ(7, fp(data.makeTrivialDataSet(1.0f / 16).data()));
    EXPECT_EQ(8, fp(data.makeTrivialDataSet(3.0f / 16).data()));
    EXPECT_EQ(9, fp(data.makeTrivialDataSet(5.0f / 16).data()));
    EXPECT_EQ(10, fp(data.makeTrivialDataSet(7.0f / 16).data()));
    EXPECT_EQ(11, fp(data.makeTrivialDataSet(9.0f / 16).data()));
    EXPECT_EQ(12, fp(data.makeTrivialDataSet(11.0f / 16).data()));
    EXPECT_EQ(13, fp(data.makeTrivialDataSet(13.0f / 16).data()));
    EXPECT_EQ(14, fp(data.makeTrivialDataSet(15.0f / 16).data()));
  }
  { // test with individual data-set features
    DecisionTree tree = factory.makePerfectDistinctGradientTree(3);
    JitCompileResult result = jitDriver.run(std::move(tree));

    auto *fp = result.EvaluatorFunction;
    DataSetFactory data(std::move(result.Tree), 7);

    auto le = NodeEvaluation::ContinueZeroLeft;
    auto ri = NodeEvaluation::ContinueOneRight;

    EXPECT_EQ(7, fp(data.makeDistinctDataSet(le, le, le).data()));
    EXPECT_EQ(8, fp(data.makeDistinctDataSet(le, le, ri).data()));
    EXPECT_EQ(9, fp(data.makeDistinctDataSet(le, ri, le).data()));
    EXPECT_EQ(10, fp(data.makeDistinctDataSet(le, ri, ri).data()));
    EXPECT_EQ(11, fp(data.makeDistinctDataSet(ri, le, le).data()));
    EXPECT_EQ(12, fp(data.makeDistinctDataSet(ri, le, ri).data()));
    EXPECT_EQ(13, fp(data.makeDistinctDataSet(ri, ri, le).data()));
    EXPECT_EQ(14, fp(data.makeDistinctDataSet(ri, ri, ri).data()));
  }
}

TEST(SingleCodegenL3, L3SubtreeSwitchAVX) {
  DecisionTreeFactory factory;
  JitDriver jitDriver;

  jitDriver.setCodegenSelector(makeLambdaSelector(
      [](const CompilerSession &session, int remainingLevels) {
        static L3SubtreeSwitchAVX codegen;
        return &codegen;
      }));

  { // test with single data-set feature
    DecisionTree tree = factory.makePerfectTrivialGradientTree(3);
    JitCompileResult result = jitDriver.run(std::move(tree));

    DataSetFactory data;
    auto *fp = result.EvaluatorFunction;

    EXPECT_EQ(7, fp(data.makeTrivialDataSet(1.0f / 16).data()));
    EXPECT_EQ(8, fp(data.makeTrivialDataSet(3.0f / 16).data()));
    EXPECT_EQ(9, fp(data.makeTrivialDataSet(5.0f / 16).data()));
    EXPECT_EQ(10, fp(data.makeTrivialDataSet(7.0f / 16).data()));
    EXPECT_EQ(11, fp(data.makeTrivialDataSet(9.0f / 16).data()));
    EXPECT_EQ(12, fp(data.makeTrivialDataSet(11.0f / 16).data()));
    EXPECT_EQ(13, fp(data.makeTrivialDataSet(13.0f / 16).data()));
    EXPECT_EQ(14, fp(data.makeTrivialDataSet(15.0f / 16).data()));
  }
  { // test with individual data-set features
    DecisionTree tree = factory.makePerfectDistinctGradientTree(3);
    JitCompileResult result = jitDriver.run(std::move(tree));

    auto *fp = result.EvaluatorFunction;
    DataSetFactory data(std::move(result.Tree), 7);

    auto le = NodeEvaluation::ContinueZeroLeft;
    auto ri = NodeEvaluation::ContinueOneRight;

    EXPECT_EQ(7, fp(data.makeDistinctDataSet(le, le, le).data()));
    EXPECT_EQ(8, fp(data.makeDistinctDataSet(le, le, ri).data()));
    EXPECT_EQ(9, fp(data.makeDistinctDataSet(le, ri, le).data()));
    EXPECT_EQ(10, fp(data.makeDistinctDataSet(le, ri, ri).data()));
    EXPECT_EQ(11, fp(data.makeDistinctDataSet(ri, le, le).data()));
    EXPECT_EQ(12, fp(data.makeDistinctDataSet(ri, le, ri).data()));
    EXPECT_EQ(13, fp(data.makeDistinctDataSet(ri, ri, le).data()));
    EXPECT_EQ(14, fp(data.makeDistinctDataSet(ri, ri, ri).data()));
  }
}
