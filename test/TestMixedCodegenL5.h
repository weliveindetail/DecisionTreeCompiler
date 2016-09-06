#pragma once

#include <gtest/gtest.h>

#include "codegen/CodeGeneratorSelector.h"
#include "codegen/LXSubtreeSwitch.h"
#include "data/DataSetFactory.h"
#include "data/DecisionTree.h"
#include "driver/JitDriver.h"

TEST(MixedCodegenL5, L2SubtreeSwitch_L3SubtreeSwitchAVX) {
  DecisionTreeFactory factory;
  JitDriver jitDriver;

  jitDriver.setCodegenSelector(makeLambdaSelector(
      [](const CompilerSession &session, int remainingLevels) -> CodeGenerator * {
        static LXSubtreeSwitch codegenSubtreeSwitch(2);
        static L3SubtreeSwitchAVX codegenSubtreeSwitchAVX;
        switch (remainingLevels) {
          case 5: return &codegenSubtreeSwitch;
          case 3: return &codegenSubtreeSwitchAVX;
        }
        llvm_unreachable("invalid remaining levels");
      }));

  { // test with single data-set feature
    DecisionTree tree = factory.makePerfectTrivialGradientTree(5);
    JitCompileResult result = jitDriver.run(std::move(tree));

    DataSetFactory data;
    auto *fp = result.EvaluatorFunction;

    EXPECT_EQ(31, fp(data.makeTrivialDataSet(1.0f / 64).data()));
    EXPECT_EQ(32, fp(data.makeTrivialDataSet(3.0f / 64).data()));
    EXPECT_EQ(33, fp(data.makeTrivialDataSet(5.0f / 64).data()));
    EXPECT_EQ(34, fp(data.makeTrivialDataSet(7.0f / 64).data()));
    EXPECT_EQ(35, fp(data.makeTrivialDataSet(9.0f / 64).data()));
    EXPECT_EQ(36, fp(data.makeTrivialDataSet(11.0f / 64).data()));
    EXPECT_EQ(37, fp(data.makeTrivialDataSet(13.0f / 64).data()));
    EXPECT_EQ(38, fp(data.makeTrivialDataSet(15.0f / 64).data()));
    EXPECT_EQ(39, fp(data.makeTrivialDataSet(17.0f / 64).data()));
    EXPECT_EQ(40, fp(data.makeTrivialDataSet(19.0f / 64).data()));
    EXPECT_EQ(41, fp(data.makeTrivialDataSet(21.0f / 64).data()));
    EXPECT_EQ(42, fp(data.makeTrivialDataSet(23.0f / 64).data()));
    EXPECT_EQ(43, fp(data.makeTrivialDataSet(25.0f / 64).data()));
    EXPECT_EQ(44, fp(data.makeTrivialDataSet(27.0f / 64).data()));
    EXPECT_EQ(45, fp(data.makeTrivialDataSet(29.0f / 64).data()));
    EXPECT_EQ(46, fp(data.makeTrivialDataSet(31.0f / 64).data()));
    EXPECT_EQ(47, fp(data.makeTrivialDataSet(33.0f / 64).data()));
    EXPECT_EQ(48, fp(data.makeTrivialDataSet(35.0f / 64).data()));
    EXPECT_EQ(49, fp(data.makeTrivialDataSet(37.0f / 64).data()));
    EXPECT_EQ(50, fp(data.makeTrivialDataSet(39.0f / 64).data()));
    EXPECT_EQ(51, fp(data.makeTrivialDataSet(41.0f / 64).data()));
    EXPECT_EQ(52, fp(data.makeTrivialDataSet(43.0f / 64).data()));
    EXPECT_EQ(53, fp(data.makeTrivialDataSet(45.0f / 64).data()));
    EXPECT_EQ(54, fp(data.makeTrivialDataSet(47.0f / 64).data()));
    EXPECT_EQ(55, fp(data.makeTrivialDataSet(49.0f / 64).data()));
    EXPECT_EQ(56, fp(data.makeTrivialDataSet(51.0f / 64).data()));
    EXPECT_EQ(57, fp(data.makeTrivialDataSet(53.0f / 64).data()));
    EXPECT_EQ(58, fp(data.makeTrivialDataSet(55.0f / 64).data()));
    EXPECT_EQ(59, fp(data.makeTrivialDataSet(57.0f / 64).data()));
    EXPECT_EQ(60, fp(data.makeTrivialDataSet(59.0f / 64).data()));
    EXPECT_EQ(61, fp(data.makeTrivialDataSet(61.0f / 64).data()));
    EXPECT_EQ(62, fp(data.makeTrivialDataSet(63.0f / 64).data()));
  }
  { // test with individual data-set features
    DecisionTree tree = factory.makePerfectDistinctGradientTree(5);
    JitCompileResult result = jitDriver.run(std::move(tree));

    auto *fp = result.EvaluatorFunction;
    DataSetFactory data(std::move(result.Tree), 31);

    auto le = NodeEvaluation::ContinueZeroLeft;
    auto ri = NodeEvaluation::ContinueOneRight;

    EXPECT_EQ(31, fp(data.makeDistinctDataSet(le, le, le, le, le).data()));
    EXPECT_EQ(32, fp(data.makeDistinctDataSet(le, le, le, le, ri).data()));
    EXPECT_EQ(33, fp(data.makeDistinctDataSet(le, le, le, ri, le).data()));
    EXPECT_EQ(34, fp(data.makeDistinctDataSet(le, le, le, ri, ri).data()));
    EXPECT_EQ(35, fp(data.makeDistinctDataSet(le, le, ri, le, le).data()));
    EXPECT_EQ(36, fp(data.makeDistinctDataSet(le, le, ri, le, ri).data()));
    EXPECT_EQ(37, fp(data.makeDistinctDataSet(le, le, ri, ri, le).data()));
    EXPECT_EQ(38, fp(data.makeDistinctDataSet(le, le, ri, ri, ri).data()));
    EXPECT_EQ(39, fp(data.makeDistinctDataSet(le, ri, le, le, le).data()));
    EXPECT_EQ(40, fp(data.makeDistinctDataSet(le, ri, le, le, ri).data()));
    EXPECT_EQ(41, fp(data.makeDistinctDataSet(le, ri, le, ri, le).data()));
    EXPECT_EQ(42, fp(data.makeDistinctDataSet(le, ri, le, ri, ri).data()));
    EXPECT_EQ(43, fp(data.makeDistinctDataSet(le, ri, ri, le, le).data()));
    EXPECT_EQ(44, fp(data.makeDistinctDataSet(le, ri, ri, le, ri).data()));
    EXPECT_EQ(45, fp(data.makeDistinctDataSet(le, ri, ri, ri, le).data()));
    EXPECT_EQ(46, fp(data.makeDistinctDataSet(le, ri, ri, ri, ri).data()));
    EXPECT_EQ(47, fp(data.makeDistinctDataSet(ri, le, le, le, le).data()));
    EXPECT_EQ(48, fp(data.makeDistinctDataSet(ri, le, le, le, ri).data()));
    EXPECT_EQ(49, fp(data.makeDistinctDataSet(ri, le, le, ri, le).data()));
    EXPECT_EQ(50, fp(data.makeDistinctDataSet(ri, le, le, ri, ri).data()));
    EXPECT_EQ(51, fp(data.makeDistinctDataSet(ri, le, ri, le, le).data()));
    EXPECT_EQ(52, fp(data.makeDistinctDataSet(ri, le, ri, le, ri).data()));
    EXPECT_EQ(53, fp(data.makeDistinctDataSet(ri, le, ri, ri, le).data()));
    EXPECT_EQ(54, fp(data.makeDistinctDataSet(ri, le, ri, ri, ri).data()));
    EXPECT_EQ(55, fp(data.makeDistinctDataSet(ri, ri, le, le, le).data()));
    EXPECT_EQ(56, fp(data.makeDistinctDataSet(ri, ri, le, le, ri).data()));
    EXPECT_EQ(57, fp(data.makeDistinctDataSet(ri, ri, le, ri, le).data()));
    EXPECT_EQ(58, fp(data.makeDistinctDataSet(ri, ri, le, ri, ri).data()));
    EXPECT_EQ(59, fp(data.makeDistinctDataSet(ri, ri, ri, le, le).data()));
    EXPECT_EQ(60, fp(data.makeDistinctDataSet(ri, ri, ri, le, ri).data()));
    EXPECT_EQ(61, fp(data.makeDistinctDataSet(ri, ri, ri, ri, le).data()));
    EXPECT_EQ(62, fp(data.makeDistinctDataSet(ri, ri, ri, ri, ri).data()));
  }
}

TEST(MixedCodegenL5, L3SubtreeSwitchAVX_L2SubtreeSwitch) {
  DecisionTreeFactory factory;
  JitDriver jitDriver;

  jitDriver.setCodegenSelector(makeLambdaSelector(
      [](const CompilerSession &session, int remainingLevels) -> CodeGenerator * {
        static LXSubtreeSwitch codegenSubtreeSwitch(2);
        static L3SubtreeSwitchAVX codegenSubtreeSwitchAVX;
        switch (remainingLevels) {
          case 5: return &codegenSubtreeSwitchAVX;
          case 2: return &codegenSubtreeSwitch;
        }
        llvm_unreachable("invalid remaining levels");
      }));

  { // test with single data-set feature
    DecisionTree tree = factory.makePerfectTrivialGradientTree(5);
    JitCompileResult result = jitDriver.run(std::move(tree));

    DataSetFactory data;
    auto *fp = result.EvaluatorFunction;

    EXPECT_EQ(31, fp(data.makeTrivialDataSet(1.0f / 64).data()));
    EXPECT_EQ(32, fp(data.makeTrivialDataSet(3.0f / 64).data()));
    EXPECT_EQ(33, fp(data.makeTrivialDataSet(5.0f / 64).data()));
    EXPECT_EQ(34, fp(data.makeTrivialDataSet(7.0f / 64).data()));
    EXPECT_EQ(35, fp(data.makeTrivialDataSet(9.0f / 64).data()));
    EXPECT_EQ(36, fp(data.makeTrivialDataSet(11.0f / 64).data()));
    EXPECT_EQ(37, fp(data.makeTrivialDataSet(13.0f / 64).data()));
    EXPECT_EQ(38, fp(data.makeTrivialDataSet(15.0f / 64).data()));
    EXPECT_EQ(39, fp(data.makeTrivialDataSet(17.0f / 64).data()));
    EXPECT_EQ(40, fp(data.makeTrivialDataSet(19.0f / 64).data()));
    EXPECT_EQ(41, fp(data.makeTrivialDataSet(21.0f / 64).data()));
    EXPECT_EQ(42, fp(data.makeTrivialDataSet(23.0f / 64).data()));
    EXPECT_EQ(43, fp(data.makeTrivialDataSet(25.0f / 64).data()));
    EXPECT_EQ(44, fp(data.makeTrivialDataSet(27.0f / 64).data()));
    EXPECT_EQ(45, fp(data.makeTrivialDataSet(29.0f / 64).data()));
    EXPECT_EQ(46, fp(data.makeTrivialDataSet(31.0f / 64).data()));
    EXPECT_EQ(47, fp(data.makeTrivialDataSet(33.0f / 64).data()));
    EXPECT_EQ(48, fp(data.makeTrivialDataSet(35.0f / 64).data()));
    EXPECT_EQ(49, fp(data.makeTrivialDataSet(37.0f / 64).data()));
    EXPECT_EQ(50, fp(data.makeTrivialDataSet(39.0f / 64).data()));
    EXPECT_EQ(51, fp(data.makeTrivialDataSet(41.0f / 64).data()));
    EXPECT_EQ(52, fp(data.makeTrivialDataSet(43.0f / 64).data()));
    EXPECT_EQ(53, fp(data.makeTrivialDataSet(45.0f / 64).data()));
    EXPECT_EQ(54, fp(data.makeTrivialDataSet(47.0f / 64).data()));
    EXPECT_EQ(55, fp(data.makeTrivialDataSet(49.0f / 64).data()));
    EXPECT_EQ(56, fp(data.makeTrivialDataSet(51.0f / 64).data()));
    EXPECT_EQ(57, fp(data.makeTrivialDataSet(53.0f / 64).data()));
    EXPECT_EQ(58, fp(data.makeTrivialDataSet(55.0f / 64).data()));
    EXPECT_EQ(59, fp(data.makeTrivialDataSet(57.0f / 64).data()));
    EXPECT_EQ(60, fp(data.makeTrivialDataSet(59.0f / 64).data()));
    EXPECT_EQ(61, fp(data.makeTrivialDataSet(61.0f / 64).data()));
    EXPECT_EQ(62, fp(data.makeTrivialDataSet(63.0f / 64).data()));
  }
  { // test with individual data-set features
    DecisionTree tree = factory.makePerfectDistinctGradientTree(5);
    JitCompileResult result = jitDriver.run(std::move(tree));

    auto *fp = result.EvaluatorFunction;
    DataSetFactory data(std::move(result.Tree), 31);

    auto le = NodeEvaluation::ContinueZeroLeft;
    auto ri = NodeEvaluation::ContinueOneRight;

    EXPECT_EQ(31, fp(data.makeDistinctDataSet(le, le, le, le, le).data()));
    EXPECT_EQ(32, fp(data.makeDistinctDataSet(le, le, le, le, ri).data()));
    EXPECT_EQ(33, fp(data.makeDistinctDataSet(le, le, le, ri, le).data()));
    EXPECT_EQ(34, fp(data.makeDistinctDataSet(le, le, le, ri, ri).data()));
    EXPECT_EQ(35, fp(data.makeDistinctDataSet(le, le, ri, le, le).data()));
    EXPECT_EQ(36, fp(data.makeDistinctDataSet(le, le, ri, le, ri).data()));
    EXPECT_EQ(37, fp(data.makeDistinctDataSet(le, le, ri, ri, le).data()));
    EXPECT_EQ(38, fp(data.makeDistinctDataSet(le, le, ri, ri, ri).data()));
    EXPECT_EQ(39, fp(data.makeDistinctDataSet(le, ri, le, le, le).data()));
    EXPECT_EQ(40, fp(data.makeDistinctDataSet(le, ri, le, le, ri).data()));
    EXPECT_EQ(41, fp(data.makeDistinctDataSet(le, ri, le, ri, le).data()));
    EXPECT_EQ(42, fp(data.makeDistinctDataSet(le, ri, le, ri, ri).data()));
    EXPECT_EQ(43, fp(data.makeDistinctDataSet(le, ri, ri, le, le).data()));
    EXPECT_EQ(44, fp(data.makeDistinctDataSet(le, ri, ri, le, ri).data()));
    EXPECT_EQ(45, fp(data.makeDistinctDataSet(le, ri, ri, ri, le).data()));
    EXPECT_EQ(46, fp(data.makeDistinctDataSet(le, ri, ri, ri, ri).data()));
    EXPECT_EQ(47, fp(data.makeDistinctDataSet(ri, le, le, le, le).data()));
    EXPECT_EQ(48, fp(data.makeDistinctDataSet(ri, le, le, le, ri).data()));
    EXPECT_EQ(49, fp(data.makeDistinctDataSet(ri, le, le, ri, le).data()));
    EXPECT_EQ(50, fp(data.makeDistinctDataSet(ri, le, le, ri, ri).data()));
    EXPECT_EQ(51, fp(data.makeDistinctDataSet(ri, le, ri, le, le).data()));
    EXPECT_EQ(52, fp(data.makeDistinctDataSet(ri, le, ri, le, ri).data()));
    EXPECT_EQ(53, fp(data.makeDistinctDataSet(ri, le, ri, ri, le).data()));
    EXPECT_EQ(54, fp(data.makeDistinctDataSet(ri, le, ri, ri, ri).data()));
    EXPECT_EQ(55, fp(data.makeDistinctDataSet(ri, ri, le, le, le).data()));
    EXPECT_EQ(56, fp(data.makeDistinctDataSet(ri, ri, le, le, ri).data()));
    EXPECT_EQ(57, fp(data.makeDistinctDataSet(ri, ri, le, ri, le).data()));
    EXPECT_EQ(58, fp(data.makeDistinctDataSet(ri, ri, le, ri, ri).data()));
    EXPECT_EQ(59, fp(data.makeDistinctDataSet(ri, ri, ri, le, le).data()));
    EXPECT_EQ(60, fp(data.makeDistinctDataSet(ri, ri, ri, le, ri).data()));
    EXPECT_EQ(61, fp(data.makeDistinctDataSet(ri, ri, ri, ri, le).data()));
    EXPECT_EQ(62, fp(data.makeDistinctDataSet(ri, ri, ri, ri, ri).data()));
  }
}

TEST(MixedCodegenL5, L1IfThenElse_L3SubtreeSwitchAVX_L1IfThenElse) {
  DecisionTreeFactory factory;
  JitDriver jitDriver;

  jitDriver.setCodegenSelector(makeLambdaSelector(
      [](const CompilerSession &session, int remainingLevels) -> CodeGenerator * {
        static L1IfThenElse codegenIfThenElse;
        static L3SubtreeSwitchAVX codegenSubtreeSwitchAVX;
        switch (remainingLevels) {
          case 5: return &codegenIfThenElse;
          case 4: return &codegenSubtreeSwitchAVX;
          case 1: return &codegenIfThenElse;
        }
        llvm_unreachable("invalid remaining levels");
      }));

  { // test with single data-set feature
    DecisionTree tree = factory.makePerfectTrivialGradientTree(5);
    JitCompileResult result = jitDriver.run(std::move(tree));

    DataSetFactory data;
    auto *fp = result.EvaluatorFunction;

    EXPECT_EQ(31, fp(data.makeTrivialDataSet(1.0f / 64).data()));
    EXPECT_EQ(32, fp(data.makeTrivialDataSet(3.0f / 64).data()));
    EXPECT_EQ(33, fp(data.makeTrivialDataSet(5.0f / 64).data()));
    EXPECT_EQ(34, fp(data.makeTrivialDataSet(7.0f / 64).data()));
    EXPECT_EQ(35, fp(data.makeTrivialDataSet(9.0f / 64).data()));
    EXPECT_EQ(36, fp(data.makeTrivialDataSet(11.0f / 64).data()));
    EXPECT_EQ(37, fp(data.makeTrivialDataSet(13.0f / 64).data()));
    EXPECT_EQ(38, fp(data.makeTrivialDataSet(15.0f / 64).data()));
    EXPECT_EQ(39, fp(data.makeTrivialDataSet(17.0f / 64).data()));
    EXPECT_EQ(40, fp(data.makeTrivialDataSet(19.0f / 64).data()));
    EXPECT_EQ(41, fp(data.makeTrivialDataSet(21.0f / 64).data()));
    EXPECT_EQ(42, fp(data.makeTrivialDataSet(23.0f / 64).data()));
    EXPECT_EQ(43, fp(data.makeTrivialDataSet(25.0f / 64).data()));
    EXPECT_EQ(44, fp(data.makeTrivialDataSet(27.0f / 64).data()));
    EXPECT_EQ(45, fp(data.makeTrivialDataSet(29.0f / 64).data()));
    EXPECT_EQ(46, fp(data.makeTrivialDataSet(31.0f / 64).data()));
    EXPECT_EQ(47, fp(data.makeTrivialDataSet(33.0f / 64).data()));
    EXPECT_EQ(48, fp(data.makeTrivialDataSet(35.0f / 64).data()));
    EXPECT_EQ(49, fp(data.makeTrivialDataSet(37.0f / 64).data()));
    EXPECT_EQ(50, fp(data.makeTrivialDataSet(39.0f / 64).data()));
    EXPECT_EQ(51, fp(data.makeTrivialDataSet(41.0f / 64).data()));
    EXPECT_EQ(52, fp(data.makeTrivialDataSet(43.0f / 64).data()));
    EXPECT_EQ(53, fp(data.makeTrivialDataSet(45.0f / 64).data()));
    EXPECT_EQ(54, fp(data.makeTrivialDataSet(47.0f / 64).data()));
    EXPECT_EQ(55, fp(data.makeTrivialDataSet(49.0f / 64).data()));
    EXPECT_EQ(56, fp(data.makeTrivialDataSet(51.0f / 64).data()));
    EXPECT_EQ(57, fp(data.makeTrivialDataSet(53.0f / 64).data()));
    EXPECT_EQ(58, fp(data.makeTrivialDataSet(55.0f / 64).data()));
    EXPECT_EQ(59, fp(data.makeTrivialDataSet(57.0f / 64).data()));
    EXPECT_EQ(60, fp(data.makeTrivialDataSet(59.0f / 64).data()));
    EXPECT_EQ(61, fp(data.makeTrivialDataSet(61.0f / 64).data()));
    EXPECT_EQ(62, fp(data.makeTrivialDataSet(63.0f / 64).data()));
  }
  { // test with individual data-set features
    DecisionTree tree = factory.makePerfectDistinctGradientTree(5);
    JitCompileResult result = jitDriver.run(std::move(tree));

    auto *fp = result.EvaluatorFunction;
    DataSetFactory data(std::move(result.Tree), 31);

    auto le = NodeEvaluation::ContinueZeroLeft;
    auto ri = NodeEvaluation::ContinueOneRight;

    EXPECT_EQ(31, fp(data.makeDistinctDataSet(le, le, le, le, le).data()));
    EXPECT_EQ(32, fp(data.makeDistinctDataSet(le, le, le, le, ri).data()));
    EXPECT_EQ(33, fp(data.makeDistinctDataSet(le, le, le, ri, le).data()));
    EXPECT_EQ(34, fp(data.makeDistinctDataSet(le, le, le, ri, ri).data()));
    EXPECT_EQ(35, fp(data.makeDistinctDataSet(le, le, ri, le, le).data()));
    EXPECT_EQ(36, fp(data.makeDistinctDataSet(le, le, ri, le, ri).data()));
    EXPECT_EQ(37, fp(data.makeDistinctDataSet(le, le, ri, ri, le).data()));
    EXPECT_EQ(38, fp(data.makeDistinctDataSet(le, le, ri, ri, ri).data()));
    EXPECT_EQ(39, fp(data.makeDistinctDataSet(le, ri, le, le, le).data()));
    EXPECT_EQ(40, fp(data.makeDistinctDataSet(le, ri, le, le, ri).data()));
    EXPECT_EQ(41, fp(data.makeDistinctDataSet(le, ri, le, ri, le).data()));
    EXPECT_EQ(42, fp(data.makeDistinctDataSet(le, ri, le, ri, ri).data()));
    EXPECT_EQ(43, fp(data.makeDistinctDataSet(le, ri, ri, le, le).data()));
    EXPECT_EQ(44, fp(data.makeDistinctDataSet(le, ri, ri, le, ri).data()));
    EXPECT_EQ(45, fp(data.makeDistinctDataSet(le, ri, ri, ri, le).data()));
    EXPECT_EQ(46, fp(data.makeDistinctDataSet(le, ri, ri, ri, ri).data()));
    EXPECT_EQ(47, fp(data.makeDistinctDataSet(ri, le, le, le, le).data()));
    EXPECT_EQ(48, fp(data.makeDistinctDataSet(ri, le, le, le, ri).data()));
    EXPECT_EQ(49, fp(data.makeDistinctDataSet(ri, le, le, ri, le).data()));
    EXPECT_EQ(50, fp(data.makeDistinctDataSet(ri, le, le, ri, ri).data()));
    EXPECT_EQ(51, fp(data.makeDistinctDataSet(ri, le, ri, le, le).data()));
    EXPECT_EQ(52, fp(data.makeDistinctDataSet(ri, le, ri, le, ri).data()));
    EXPECT_EQ(53, fp(data.makeDistinctDataSet(ri, le, ri, ri, le).data()));
    EXPECT_EQ(54, fp(data.makeDistinctDataSet(ri, le, ri, ri, ri).data()));
    EXPECT_EQ(55, fp(data.makeDistinctDataSet(ri, ri, le, le, le).data()));
    EXPECT_EQ(56, fp(data.makeDistinctDataSet(ri, ri, le, le, ri).data()));
    EXPECT_EQ(57, fp(data.makeDistinctDataSet(ri, ri, le, ri, le).data()));
    EXPECT_EQ(58, fp(data.makeDistinctDataSet(ri, ri, le, ri, ri).data()));
    EXPECT_EQ(59, fp(data.makeDistinctDataSet(ri, ri, ri, le, le).data()));
    EXPECT_EQ(60, fp(data.makeDistinctDataSet(ri, ri, ri, le, ri).data()));
    EXPECT_EQ(61, fp(data.makeDistinctDataSet(ri, ri, ri, ri, le).data()));
    EXPECT_EQ(62, fp(data.makeDistinctDataSet(ri, ri, ri, ri, ri).data()));
  }
}
