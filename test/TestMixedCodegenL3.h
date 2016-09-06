#pragma once

#include <gtest/gtest.h>

#include "codegen/CodeGeneratorSelector.h"
#include "codegen/LXSubtreeSwitch.h"
#include "data/DataSetFactory.h"
#include "data/DecisionTree.h"
#include "driver/JitDriver.h"

TEST(MixedCodegenL3, L1IfThenElse_L1SubtreeSwitch_L1IfThenElse) {
  DecisionTreeFactory factory;
  JitDriver jitDriver;

  jitDriver.setCodegenSelector(makeLambdaSelector(
      [](const CompilerSession &session, int remainingLevels) -> CodeGenerator * {
        static L1IfThenElse codegenIfThenElse;
        static LXSubtreeSwitch codegenSubtreeSwitch(1);
        switch (remainingLevels) {
          case 3: return &codegenIfThenElse;
          case 2: return &codegenSubtreeSwitch;
          case 1: return &codegenIfThenElse;
        }
        llvm_unreachable("invalid remaining levels");
        return nullptr;
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

TEST(MixedCodegenL3, L1SubtreeSwitch_L1IfThenElse_L1SubtreeSwitch) {
  DecisionTreeFactory factory;
  JitDriver jitDriver;

  jitDriver.setCodegenSelector(makeLambdaSelector(
      [](const CompilerSession &session, int remainingLevels) -> CodeGenerator * {
        static L1IfThenElse codegenIfThenElse;
        static LXSubtreeSwitch codegenSubtreeSwitch(1);
        switch (remainingLevels) {
          case 3: return &codegenSubtreeSwitch;
          case 2: return &codegenIfThenElse;
          case 1: return &codegenSubtreeSwitch;
        }
        llvm_unreachable("invalid remaining levels");
        return nullptr;
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

TEST(MixedCodegenL3, L1IfThenElse_L2SubtreeSwitch) {
  DecisionTreeFactory factory;
  JitDriver jitDriver;

  jitDriver.setCodegenSelector(makeLambdaSelector(
      [](const CompilerSession &session, int remainingLevels) -> CodeGenerator * {
        static L1IfThenElse codegenIfThenElse;
        static LXSubtreeSwitch codegenSubtreeSwitch(2);
        switch (remainingLevels) {
          case 3: return &codegenIfThenElse;
          case 2: return &codegenSubtreeSwitch;
        }
        llvm_unreachable("invalid remaining levels");
        return nullptr;
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

TEST(MixedCodegenL3, L2SubtreeSwitch_L1IfThenElse) {
  DecisionTreeFactory factory;
  JitDriver jitDriver;

  jitDriver.setCodegenSelector(makeLambdaSelector(
      [](const CompilerSession &session, int remainingLevels) -> CodeGenerator * {
        static L1IfThenElse codegenIfThenElse;
        static LXSubtreeSwitch codegenSubtreeSwitch(2);
        switch (remainingLevels) {
          case 3: return &codegenSubtreeSwitch;
          case 1: return &codegenIfThenElse;
        }
        llvm_unreachable("invalid remaining levels");
        return nullptr;
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
