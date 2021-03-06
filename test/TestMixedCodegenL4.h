#pragma once

#include <gtest/gtest.h>

#include "codegen/CodeGeneratorSelector.h"
#include "codegen/LXSubtreeSwitch.h"
#include "data/DataSetFactory.h"
#include "data/DecisionTree.h"
#include "driver/JitDriver.h"

TEST(MixedCodegenL4, L1IfThenElse_L3SubtreeSwitch) {
  DecisionTreeFactory factory;
  JitDriver jitDriver;

  jitDriver.setCodegenSelector(makeLambdaSelector(
      [](const CompilerSession &session, int remainingLevels) -> CodeGenerator * {
        static L1IfThenElse codegenIfThenElse;
        static LXSubtreeSwitch codegenSubtreeSwitch(3);
        switch (remainingLevels) {
          case 4: return &codegenIfThenElse;
          case 3: return &codegenSubtreeSwitch;
        }
        llvm_unreachable("invalid remaining levels");
        return nullptr;
      }));

  { // test with single data-set feature
    DecisionTree tree = factory.makePerfectTrivialGradientTree(4);
    JitCompileResult result = jitDriver.run(std::move(tree));

    DataSetFactory data;
    auto *fp = result.EvaluatorFunction;

    EXPECT_EQ(15, fp(data.makeTrivialDataSet(1.0f / 32).data()));
    EXPECT_EQ(16, fp(data.makeTrivialDataSet(3.0f / 32).data()));
    EXPECT_EQ(17, fp(data.makeTrivialDataSet(5.0f / 32).data()));
    EXPECT_EQ(18, fp(data.makeTrivialDataSet(7.0f / 32).data()));
    EXPECT_EQ(19, fp(data.makeTrivialDataSet(9.0f / 32).data()));
    EXPECT_EQ(20, fp(data.makeTrivialDataSet(11.0f / 32).data()));
    EXPECT_EQ(21, fp(data.makeTrivialDataSet(13.0f / 32).data()));
    EXPECT_EQ(22, fp(data.makeTrivialDataSet(15.0f / 32).data()));
    EXPECT_EQ(23, fp(data.makeTrivialDataSet(17.0f / 32).data()));
    EXPECT_EQ(24, fp(data.makeTrivialDataSet(19.0f / 32).data()));
    EXPECT_EQ(25, fp(data.makeTrivialDataSet(21.0f / 32).data()));
    EXPECT_EQ(26, fp(data.makeTrivialDataSet(23.0f / 32).data()));
    EXPECT_EQ(27, fp(data.makeTrivialDataSet(25.0f / 32).data()));
    EXPECT_EQ(28, fp(data.makeTrivialDataSet(27.0f / 32).data()));
    EXPECT_EQ(29, fp(data.makeTrivialDataSet(29.0f / 32).data()));
    EXPECT_EQ(30, fp(data.makeTrivialDataSet(31.0f / 32).data()));
  }
  { // test with individual data-set features
    DecisionTree tree = factory.makePerfectDistinctGradientTree(4);
    JitCompileResult result = jitDriver.run(std::move(tree));

    auto *fp = result.EvaluatorFunction;
    DataSetFactory data(std::move(result.Tree), 15);

    auto le = NodeEvaluation::ContinueZeroLeft;
    auto ri = NodeEvaluation::ContinueOneRight;

    EXPECT_EQ(15, fp(data.makeDistinctDataSet(le, le, le, le).data()));
    EXPECT_EQ(16, fp(data.makeDistinctDataSet(le, le, le, ri).data()));
    EXPECT_EQ(17, fp(data.makeDistinctDataSet(le, le, ri, le).data()));
    EXPECT_EQ(18, fp(data.makeDistinctDataSet(le, le, ri, ri).data()));
    EXPECT_EQ(19, fp(data.makeDistinctDataSet(le, ri, le, le).data()));
    EXPECT_EQ(20, fp(data.makeDistinctDataSet(le, ri, le, ri).data()));
    EXPECT_EQ(21, fp(data.makeDistinctDataSet(le, ri, ri, le).data()));
    EXPECT_EQ(22, fp(data.makeDistinctDataSet(le, ri, ri, ri).data()));
    EXPECT_EQ(23, fp(data.makeDistinctDataSet(ri, le, le, le).data()));
    EXPECT_EQ(24, fp(data.makeDistinctDataSet(ri, le, le, ri).data()));
    EXPECT_EQ(25, fp(data.makeDistinctDataSet(ri, le, ri, le).data()));
    EXPECT_EQ(26, fp(data.makeDistinctDataSet(ri, le, ri, ri).data()));
    EXPECT_EQ(27, fp(data.makeDistinctDataSet(ri, ri, le, le).data()));
    EXPECT_EQ(28, fp(data.makeDistinctDataSet(ri, ri, le, ri).data()));
    EXPECT_EQ(29, fp(data.makeDistinctDataSet(ri, ri, ri, le).data()));
    EXPECT_EQ(30, fp(data.makeDistinctDataSet(ri, ri, ri, ri).data()));
  }
}

TEST(MixedCodegenL4, L3SubtreeSwitch_L1IfThenElse) {
  DecisionTreeFactory factory;
  JitDriver jitDriver;

  jitDriver.setCodegenSelector(makeLambdaSelector(
      [](const CompilerSession &session, int remainingLevels) -> CodeGenerator * {
        static L1IfThenElse codegenIfThenElse;
        static LXSubtreeSwitch codegenSubtreeSwitch(3);
        switch (remainingLevels) {
          case 4: return &codegenSubtreeSwitch;
          case 1: return &codegenIfThenElse;
        }
        llvm_unreachable("invalid remaining levels");
        return nullptr;
      }));

  { // test with single data-set feature
    DecisionTree tree = factory.makePerfectTrivialGradientTree(4);
    JitCompileResult result = jitDriver.run(std::move(tree));

    DataSetFactory data;
    auto *fp = result.EvaluatorFunction;

    EXPECT_EQ(15, fp(data.makeTrivialDataSet(1.0f / 32).data()));
    EXPECT_EQ(16, fp(data.makeTrivialDataSet(3.0f / 32).data()));
    EXPECT_EQ(17, fp(data.makeTrivialDataSet(5.0f / 32).data()));
    EXPECT_EQ(18, fp(data.makeTrivialDataSet(7.0f / 32).data()));
    EXPECT_EQ(19, fp(data.makeTrivialDataSet(9.0f / 32).data()));
    EXPECT_EQ(20, fp(data.makeTrivialDataSet(11.0f / 32).data()));
    EXPECT_EQ(21, fp(data.makeTrivialDataSet(13.0f / 32).data()));
    EXPECT_EQ(22, fp(data.makeTrivialDataSet(15.0f / 32).data()));
    EXPECT_EQ(23, fp(data.makeTrivialDataSet(17.0f / 32).data()));
    EXPECT_EQ(24, fp(data.makeTrivialDataSet(19.0f / 32).data()));
    EXPECT_EQ(25, fp(data.makeTrivialDataSet(21.0f / 32).data()));
    EXPECT_EQ(26, fp(data.makeTrivialDataSet(23.0f / 32).data()));
    EXPECT_EQ(27, fp(data.makeTrivialDataSet(25.0f / 32).data()));
    EXPECT_EQ(28, fp(data.makeTrivialDataSet(27.0f / 32).data()));
    EXPECT_EQ(29, fp(data.makeTrivialDataSet(29.0f / 32).data()));
    EXPECT_EQ(30, fp(data.makeTrivialDataSet(31.0f / 32).data()));
  }
  { // test with individual data-set features
    DecisionTree tree = factory.makePerfectDistinctGradientTree(4);
    JitCompileResult result = jitDriver.run(std::move(tree));

    auto *fp = result.EvaluatorFunction;
    DataSetFactory data(std::move(result.Tree), 15);

    auto le = NodeEvaluation::ContinueZeroLeft;
    auto ri = NodeEvaluation::ContinueOneRight;

    EXPECT_EQ(15, fp(data.makeDistinctDataSet(le, le, le, le).data()));
    EXPECT_EQ(16, fp(data.makeDistinctDataSet(le, le, le, ri).data()));
    EXPECT_EQ(17, fp(data.makeDistinctDataSet(le, le, ri, le).data()));
    EXPECT_EQ(18, fp(data.makeDistinctDataSet(le, le, ri, ri).data()));
    EXPECT_EQ(19, fp(data.makeDistinctDataSet(le, ri, le, le).data()));
    EXPECT_EQ(20, fp(data.makeDistinctDataSet(le, ri, le, ri).data()));
    EXPECT_EQ(21, fp(data.makeDistinctDataSet(le, ri, ri, le).data()));
    EXPECT_EQ(22, fp(data.makeDistinctDataSet(le, ri, ri, ri).data()));
    EXPECT_EQ(23, fp(data.makeDistinctDataSet(ri, le, le, le).data()));
    EXPECT_EQ(24, fp(data.makeDistinctDataSet(ri, le, le, ri).data()));
    EXPECT_EQ(25, fp(data.makeDistinctDataSet(ri, le, ri, le).data()));
    EXPECT_EQ(26, fp(data.makeDistinctDataSet(ri, le, ri, ri).data()));
    EXPECT_EQ(27, fp(data.makeDistinctDataSet(ri, ri, le, le).data()));
    EXPECT_EQ(28, fp(data.makeDistinctDataSet(ri, ri, le, ri).data()));
    EXPECT_EQ(29, fp(data.makeDistinctDataSet(ri, ri, ri, le).data()));
    EXPECT_EQ(30, fp(data.makeDistinctDataSet(ri, ri, ri, ri).data()));
  }
}

TEST(MixedCodegenL4, L1IfThenElse_L3SubtreeSwitchAVX) {
  DecisionTreeFactory factory;
  JitDriver jitDriver;

  jitDriver.setCodegenSelector(makeLambdaSelector(
      [](const CompilerSession &session, int remainingLevels) -> CodeGenerator * {
        static L1IfThenElse codegenIfThenElse;
        static L3SubtreeSwitchAVX codegenSubtreeSwitch;
        switch (remainingLevels) {
          case 4: return &codegenIfThenElse;
          case 3: return &codegenSubtreeSwitch;
        }
        llvm_unreachable("invalid remaining levels");
        return nullptr;
      }));

  { // test with single data-set feature
    DecisionTree tree = factory.makePerfectTrivialGradientTree(4);
    JitCompileResult result = jitDriver.run(std::move(tree));

    DataSetFactory data;
    auto *fp = result.EvaluatorFunction;

    EXPECT_EQ(15, fp(data.makeTrivialDataSet(1.0f / 32).data()));
    EXPECT_EQ(16, fp(data.makeTrivialDataSet(3.0f / 32).data()));
    EXPECT_EQ(17, fp(data.makeTrivialDataSet(5.0f / 32).data()));
    EXPECT_EQ(18, fp(data.makeTrivialDataSet(7.0f / 32).data()));
    EXPECT_EQ(19, fp(data.makeTrivialDataSet(9.0f / 32).data()));
    EXPECT_EQ(20, fp(data.makeTrivialDataSet(11.0f / 32).data()));
    EXPECT_EQ(21, fp(data.makeTrivialDataSet(13.0f / 32).data()));
    EXPECT_EQ(22, fp(data.makeTrivialDataSet(15.0f / 32).data()));
    EXPECT_EQ(23, fp(data.makeTrivialDataSet(17.0f / 32).data()));
    EXPECT_EQ(24, fp(data.makeTrivialDataSet(19.0f / 32).data()));
    EXPECT_EQ(25, fp(data.makeTrivialDataSet(21.0f / 32).data()));
    EXPECT_EQ(26, fp(data.makeTrivialDataSet(23.0f / 32).data()));
    EXPECT_EQ(27, fp(data.makeTrivialDataSet(25.0f / 32).data()));
    EXPECT_EQ(28, fp(data.makeTrivialDataSet(27.0f / 32).data()));
    EXPECT_EQ(29, fp(data.makeTrivialDataSet(29.0f / 32).data()));
    EXPECT_EQ(30, fp(data.makeTrivialDataSet(31.0f / 32).data()));
  }
  { // test with individual data-set features
    DecisionTree tree = factory.makePerfectDistinctGradientTree(4);
    JitCompileResult result = jitDriver.run(std::move(tree));

    auto *fp = result.EvaluatorFunction;
    DataSetFactory data(std::move(result.Tree), 15);

    auto le = NodeEvaluation::ContinueZeroLeft;
    auto ri = NodeEvaluation::ContinueOneRight;

    EXPECT_EQ(15, fp(data.makeDistinctDataSet(le, le, le, le).data()));
    EXPECT_EQ(16, fp(data.makeDistinctDataSet(le, le, le, ri).data()));
    EXPECT_EQ(17, fp(data.makeDistinctDataSet(le, le, ri, le).data()));
    EXPECT_EQ(18, fp(data.makeDistinctDataSet(le, le, ri, ri).data()));
    EXPECT_EQ(19, fp(data.makeDistinctDataSet(le, ri, le, le).data()));
    EXPECT_EQ(20, fp(data.makeDistinctDataSet(le, ri, le, ri).data()));
    EXPECT_EQ(21, fp(data.makeDistinctDataSet(le, ri, ri, le).data()));
    EXPECT_EQ(22, fp(data.makeDistinctDataSet(le, ri, ri, ri).data()));
    EXPECT_EQ(23, fp(data.makeDistinctDataSet(ri, le, le, le).data()));
    EXPECT_EQ(24, fp(data.makeDistinctDataSet(ri, le, le, ri).data()));
    EXPECT_EQ(25, fp(data.makeDistinctDataSet(ri, le, ri, le).data()));
    EXPECT_EQ(26, fp(data.makeDistinctDataSet(ri, le, ri, ri).data()));
    EXPECT_EQ(27, fp(data.makeDistinctDataSet(ri, ri, le, le).data()));
    EXPECT_EQ(28, fp(data.makeDistinctDataSet(ri, ri, le, ri).data()));
    EXPECT_EQ(29, fp(data.makeDistinctDataSet(ri, ri, ri, le).data()));
    EXPECT_EQ(30, fp(data.makeDistinctDataSet(ri, ri, ri, ri).data()));
  }
}

TEST(MixedCodegenL4, L3SubtreeSwitchAVX_L1IfThenElse) {
  DecisionTreeFactory factory;
  JitDriver jitDriver;

  jitDriver.setCodegenSelector(makeLambdaSelector(
      [](const CompilerSession &session, int remainingLevels) -> CodeGenerator * {
        static L1IfThenElse codegenIfThenElse;
        static L3SubtreeSwitchAVX codegenSubtreeSwitch;
        switch (remainingLevels) {
          case 4: return &codegenSubtreeSwitch;
          case 1: return &codegenIfThenElse;
        }
        llvm_unreachable("invalid remaining levels");
        return nullptr;
      }));

  { // test with single data-set feature
    DecisionTree tree = factory.makePerfectTrivialGradientTree(4);
    JitCompileResult result = jitDriver.run(std::move(tree));

    DataSetFactory data;
    auto *fp = result.EvaluatorFunction;

    EXPECT_EQ(15, fp(data.makeTrivialDataSet(1.0f / 32).data()));
    EXPECT_EQ(16, fp(data.makeTrivialDataSet(3.0f / 32).data()));
    EXPECT_EQ(17, fp(data.makeTrivialDataSet(5.0f / 32).data()));
    EXPECT_EQ(18, fp(data.makeTrivialDataSet(7.0f / 32).data()));
    EXPECT_EQ(19, fp(data.makeTrivialDataSet(9.0f / 32).data()));
    EXPECT_EQ(20, fp(data.makeTrivialDataSet(11.0f / 32).data()));
    EXPECT_EQ(21, fp(data.makeTrivialDataSet(13.0f / 32).data()));
    EXPECT_EQ(22, fp(data.makeTrivialDataSet(15.0f / 32).data()));
    EXPECT_EQ(23, fp(data.makeTrivialDataSet(17.0f / 32).data()));
    EXPECT_EQ(24, fp(data.makeTrivialDataSet(19.0f / 32).data()));
    EXPECT_EQ(25, fp(data.makeTrivialDataSet(21.0f / 32).data()));
    EXPECT_EQ(26, fp(data.makeTrivialDataSet(23.0f / 32).data()));
    EXPECT_EQ(27, fp(data.makeTrivialDataSet(25.0f / 32).data()));
    EXPECT_EQ(28, fp(data.makeTrivialDataSet(27.0f / 32).data()));
    EXPECT_EQ(29, fp(data.makeTrivialDataSet(29.0f / 32).data()));
    EXPECT_EQ(30, fp(data.makeTrivialDataSet(31.0f / 32).data()));
  }
  { // test with individual data-set features
    DecisionTree tree = factory.makePerfectDistinctGradientTree(4);
    JitCompileResult result = jitDriver.run(std::move(tree));

    auto *fp = result.EvaluatorFunction;
    DataSetFactory data(std::move(result.Tree), 15);

    auto le = NodeEvaluation::ContinueZeroLeft;
    auto ri = NodeEvaluation::ContinueOneRight;

    EXPECT_EQ(15, fp(data.makeDistinctDataSet(le, le, le, le).data()));
    EXPECT_EQ(16, fp(data.makeDistinctDataSet(le, le, le, ri).data()));
    EXPECT_EQ(17, fp(data.makeDistinctDataSet(le, le, ri, le).data()));
    EXPECT_EQ(18, fp(data.makeDistinctDataSet(le, le, ri, ri).data()));
    EXPECT_EQ(19, fp(data.makeDistinctDataSet(le, ri, le, le).data()));
    EXPECT_EQ(20, fp(data.makeDistinctDataSet(le, ri, le, ri).data()));
    EXPECT_EQ(21, fp(data.makeDistinctDataSet(le, ri, ri, le).data()));
    EXPECT_EQ(22, fp(data.makeDistinctDataSet(le, ri, ri, ri).data()));
    EXPECT_EQ(23, fp(data.makeDistinctDataSet(ri, le, le, le).data()));
    EXPECT_EQ(24, fp(data.makeDistinctDataSet(ri, le, le, ri).data()));
    EXPECT_EQ(25, fp(data.makeDistinctDataSet(ri, le, ri, le).data()));
    EXPECT_EQ(26, fp(data.makeDistinctDataSet(ri, le, ri, ri).data()));
    EXPECT_EQ(27, fp(data.makeDistinctDataSet(ri, ri, le, le).data()));
    EXPECT_EQ(28, fp(data.makeDistinctDataSet(ri, ri, le, ri).data()));
    EXPECT_EQ(29, fp(data.makeDistinctDataSet(ri, ri, ri, le).data()));
    EXPECT_EQ(30, fp(data.makeDistinctDataSet(ri, ri, ri, ri).data()));
  }
}
