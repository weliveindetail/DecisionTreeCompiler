#pragma once

#include <gtest/gtest.h>

#include "codegen/CodeGeneratorSelector.h"
#include "codegen/L1IfThenElse.h"
#include "codegen/LXSubtreeSwitch.h"
#include "data/DataSetFactory.h"
#include "data/DecisionTree.h"
#include "driver/JitDriver.h"

TEST(SingleCodegenL6, L3SubtreeSwitch) {
  DecisionTreeFactory factory;
  JitDriver jitDriver;

  jitDriver.setCodegenSelector(makeLambdaSelector(
      [](const CompilerSession &session, int remainingLevels) {
        static LXSubtreeSwitch codegen(3);
        return &codegen;
      }));

  { // test with single data-set feature
    DecisionTree tree = factory.makePerfectTrivialGradientTree(6);
    JitCompileResult result = jitDriver.run(std::move(tree));

    DataSetFactory data;
    auto *fp = result.EvaluatorFunction;

    EXPECT_EQ(63, fp(data.makeTrivialDataSet(1.0f / 128).data()));
    EXPECT_EQ(64, fp(data.makeTrivialDataSet(3.0f / 128).data()));
    EXPECT_EQ(65, fp(data.makeTrivialDataSet(5.0f / 128).data()));
    EXPECT_EQ(66, fp(data.makeTrivialDataSet(7.0f / 128).data()));
    EXPECT_EQ(67, fp(data.makeTrivialDataSet(9.0f / 128).data()));
    EXPECT_EQ(68, fp(data.makeTrivialDataSet(11.0f / 128).data()));
    EXPECT_EQ(69, fp(data.makeTrivialDataSet(13.0f / 128).data()));
    EXPECT_EQ(70, fp(data.makeTrivialDataSet(15.0f / 128).data()));
    EXPECT_EQ(71, fp(data.makeTrivialDataSet(17.0f / 128).data()));
    EXPECT_EQ(72, fp(data.makeTrivialDataSet(19.0f / 128).data()));
    EXPECT_EQ(73, fp(data.makeTrivialDataSet(21.0f / 128).data()));
    EXPECT_EQ(74, fp(data.makeTrivialDataSet(23.0f / 128).data()));
    EXPECT_EQ(75, fp(data.makeTrivialDataSet(25.0f / 128).data()));
    EXPECT_EQ(76, fp(data.makeTrivialDataSet(27.0f / 128).data()));
    EXPECT_EQ(77, fp(data.makeTrivialDataSet(29.0f / 128).data()));
    EXPECT_EQ(78, fp(data.makeTrivialDataSet(31.0f / 128).data()));
    EXPECT_EQ(79, fp(data.makeTrivialDataSet(33.0f / 128).data()));
    EXPECT_EQ(80, fp(data.makeTrivialDataSet(35.0f / 128).data()));
    EXPECT_EQ(81, fp(data.makeTrivialDataSet(37.0f / 128).data()));
    EXPECT_EQ(82, fp(data.makeTrivialDataSet(39.0f / 128).data()));
    EXPECT_EQ(83, fp(data.makeTrivialDataSet(41.0f / 128).data()));
    EXPECT_EQ(84, fp(data.makeTrivialDataSet(43.0f / 128).data()));
    EXPECT_EQ(85, fp(data.makeTrivialDataSet(45.0f / 128).data()));
    EXPECT_EQ(86, fp(data.makeTrivialDataSet(47.0f / 128).data()));
    EXPECT_EQ(87, fp(data.makeTrivialDataSet(49.0f / 128).data()));
    EXPECT_EQ(88, fp(data.makeTrivialDataSet(51.0f / 128).data()));
    EXPECT_EQ(89, fp(data.makeTrivialDataSet(53.0f / 128).data()));
    EXPECT_EQ(90, fp(data.makeTrivialDataSet(55.0f / 128).data()));
    EXPECT_EQ(91, fp(data.makeTrivialDataSet(57.0f / 128).data()));
    EXPECT_EQ(92, fp(data.makeTrivialDataSet(59.0f / 128).data()));
    EXPECT_EQ(93, fp(data.makeTrivialDataSet(61.0f / 128).data()));
    EXPECT_EQ(94, fp(data.makeTrivialDataSet(63.0f / 128).data()));
    EXPECT_EQ(95, fp(data.makeTrivialDataSet(65.0f / 128).data()));
    EXPECT_EQ(96, fp(data.makeTrivialDataSet(67.0f / 128).data()));
    EXPECT_EQ(97, fp(data.makeTrivialDataSet(69.0f / 128).data()));
    EXPECT_EQ(98, fp(data.makeTrivialDataSet(71.0f / 128).data()));
    EXPECT_EQ(99, fp(data.makeTrivialDataSet(73.0f / 128).data()));
    EXPECT_EQ(100, fp(data.makeTrivialDataSet(75.0f / 128).data()));
    EXPECT_EQ(101, fp(data.makeTrivialDataSet(77.0f / 128).data()));
    EXPECT_EQ(102, fp(data.makeTrivialDataSet(79.0f / 128).data()));
    EXPECT_EQ(103, fp(data.makeTrivialDataSet(81.0f / 128).data()));
    EXPECT_EQ(104, fp(data.makeTrivialDataSet(83.0f / 128).data()));
    EXPECT_EQ(105, fp(data.makeTrivialDataSet(85.0f / 128).data()));
    EXPECT_EQ(106, fp(data.makeTrivialDataSet(87.0f / 128).data()));
    EXPECT_EQ(107, fp(data.makeTrivialDataSet(89.0f / 128).data()));
    EXPECT_EQ(108, fp(data.makeTrivialDataSet(91.0f / 128).data()));
    EXPECT_EQ(109, fp(data.makeTrivialDataSet(93.0f / 128).data()));
    EXPECT_EQ(110, fp(data.makeTrivialDataSet(95.0f / 128).data()));
    EXPECT_EQ(111, fp(data.makeTrivialDataSet(97.0f / 128).data()));
    EXPECT_EQ(112, fp(data.makeTrivialDataSet(99.0f / 128).data()));
    EXPECT_EQ(113, fp(data.makeTrivialDataSet(101.0f / 128).data()));
    EXPECT_EQ(114, fp(data.makeTrivialDataSet(103.0f / 128).data()));
    EXPECT_EQ(115, fp(data.makeTrivialDataSet(105.0f / 128).data()));
    EXPECT_EQ(116, fp(data.makeTrivialDataSet(107.0f / 128).data()));
    EXPECT_EQ(117, fp(data.makeTrivialDataSet(109.0f / 128).data()));
    EXPECT_EQ(118, fp(data.makeTrivialDataSet(111.0f / 128).data()));
    EXPECT_EQ(119, fp(data.makeTrivialDataSet(113.0f / 128).data()));
    EXPECT_EQ(120, fp(data.makeTrivialDataSet(115.0f / 128).data()));
    EXPECT_EQ(121, fp(data.makeTrivialDataSet(117.0f / 128).data()));
    EXPECT_EQ(122, fp(data.makeTrivialDataSet(119.0f / 128).data()));
    EXPECT_EQ(123, fp(data.makeTrivialDataSet(121.0f / 128).data()));
    EXPECT_EQ(124, fp(data.makeTrivialDataSet(123.0f / 128).data()));
    EXPECT_EQ(125, fp(data.makeTrivialDataSet(125.0f / 128).data()));
    EXPECT_EQ(126, fp(data.makeTrivialDataSet(127.0f / 128).data()));
  }
  { // test with individual data-set features
    DecisionTree tree = factory.makePerfectDistinctGradientTree(6);
    JitCompileResult result = jitDriver.run(std::move(tree));

    auto *fp = result.EvaluatorFunction;
    DataSetFactory data(std::move(result.Tree), 63);

    auto le = NodeEvaluation::ContinueZeroLeft;
    auto ri = NodeEvaluation::ContinueOneRight;

    EXPECT_EQ(63, fp(data.makeDistinctDataSet(le, le, le, le, le, le).data()));
    EXPECT_EQ(64, fp(data.makeDistinctDataSet(le, le, le, le, le, ri).data()));
    EXPECT_EQ(65, fp(data.makeDistinctDataSet(le, le, le, le, ri, le).data()));
    EXPECT_EQ(66, fp(data.makeDistinctDataSet(le, le, le, le, ri, ri).data()));
    EXPECT_EQ(67, fp(data.makeDistinctDataSet(le, le, le, ri, le, le).data()));
    EXPECT_EQ(68, fp(data.makeDistinctDataSet(le, le, le, ri, le, ri).data()));
    EXPECT_EQ(69, fp(data.makeDistinctDataSet(le, le, le, ri, ri, le).data()));
    EXPECT_EQ(70, fp(data.makeDistinctDataSet(le, le, le, ri, ri, ri).data()));
    EXPECT_EQ(71, fp(data.makeDistinctDataSet(le, le, ri, le, le, le).data()));
    EXPECT_EQ(72, fp(data.makeDistinctDataSet(le, le, ri, le, le, ri).data()));
    EXPECT_EQ(73, fp(data.makeDistinctDataSet(le, le, ri, le, ri, le).data()));
    EXPECT_EQ(74, fp(data.makeDistinctDataSet(le, le, ri, le, ri, ri).data()));
    EXPECT_EQ(75, fp(data.makeDistinctDataSet(le, le, ri, ri, le, le).data()));
    EXPECT_EQ(76, fp(data.makeDistinctDataSet(le, le, ri, ri, le, ri).data()));
    EXPECT_EQ(77, fp(data.makeDistinctDataSet(le, le, ri, ri, ri, le).data()));
    EXPECT_EQ(78, fp(data.makeDistinctDataSet(le, le, ri, ri, ri, ri).data()));
    EXPECT_EQ(79, fp(data.makeDistinctDataSet(le, ri, le, le, le, le).data()));
    EXPECT_EQ(80, fp(data.makeDistinctDataSet(le, ri, le, le, le, ri).data()));
    EXPECT_EQ(81, fp(data.makeDistinctDataSet(le, ri, le, le, ri, le).data()));
    EXPECT_EQ(82, fp(data.makeDistinctDataSet(le, ri, le, le, ri, ri).data()));
    EXPECT_EQ(83, fp(data.makeDistinctDataSet(le, ri, le, ri, le, le).data()));
    EXPECT_EQ(84, fp(data.makeDistinctDataSet(le, ri, le, ri, le, ri).data()));
    EXPECT_EQ(85, fp(data.makeDistinctDataSet(le, ri, le, ri, ri, le).data()));
    EXPECT_EQ(86, fp(data.makeDistinctDataSet(le, ri, le, ri, ri, ri).data()));
    EXPECT_EQ(87, fp(data.makeDistinctDataSet(le, ri, ri, le, le, le).data()));
    EXPECT_EQ(88, fp(data.makeDistinctDataSet(le, ri, ri, le, le, ri).data()));
    EXPECT_EQ(89, fp(data.makeDistinctDataSet(le, ri, ri, le, ri, le).data()));
    EXPECT_EQ(90, fp(data.makeDistinctDataSet(le, ri, ri, le, ri, ri).data()));
    EXPECT_EQ(91, fp(data.makeDistinctDataSet(le, ri, ri, ri, le, le).data()));
    EXPECT_EQ(92, fp(data.makeDistinctDataSet(le, ri, ri, ri, le, ri).data()));
    EXPECT_EQ(93, fp(data.makeDistinctDataSet(le, ri, ri, ri, ri, le).data()));
    EXPECT_EQ(94, fp(data.makeDistinctDataSet(le, ri, ri, ri, ri, ri).data()));
    EXPECT_EQ(95, fp(data.makeDistinctDataSet(ri, le, le, le, le, le).data()));
    EXPECT_EQ(96, fp(data.makeDistinctDataSet(ri, le, le, le, le, ri).data()));
    EXPECT_EQ(97, fp(data.makeDistinctDataSet(ri, le, le, le, ri, le).data()));
    EXPECT_EQ(98, fp(data.makeDistinctDataSet(ri, le, le, le, ri, ri).data()));
    EXPECT_EQ(99, fp(data.makeDistinctDataSet(ri, le, le, ri, le, le).data()));
    EXPECT_EQ(100, fp(data.makeDistinctDataSet(ri, le, le, ri, le, ri).data()));
    EXPECT_EQ(101, fp(data.makeDistinctDataSet(ri, le, le, ri, ri, le).data()));
    EXPECT_EQ(102, fp(data.makeDistinctDataSet(ri, le, le, ri, ri, ri).data()));
    EXPECT_EQ(103, fp(data.makeDistinctDataSet(ri, le, ri, le, le, le).data()));
    EXPECT_EQ(104, fp(data.makeDistinctDataSet(ri, le, ri, le, le, ri).data()));
    EXPECT_EQ(105, fp(data.makeDistinctDataSet(ri, le, ri, le, ri, le).data()));
    EXPECT_EQ(106, fp(data.makeDistinctDataSet(ri, le, ri, le, ri, ri).data()));
    EXPECT_EQ(107, fp(data.makeDistinctDataSet(ri, le, ri, ri, le, le).data()));
    EXPECT_EQ(108, fp(data.makeDistinctDataSet(ri, le, ri, ri, le, ri).data()));
    EXPECT_EQ(109, fp(data.makeDistinctDataSet(ri, le, ri, ri, ri, le).data()));
    EXPECT_EQ(110, fp(data.makeDistinctDataSet(ri, le, ri, ri, ri, ri).data()));
    EXPECT_EQ(111, fp(data.makeDistinctDataSet(ri, ri, le, le, le, le).data()));
    EXPECT_EQ(112, fp(data.makeDistinctDataSet(ri, ri, le, le, le, ri).data()));
    EXPECT_EQ(113, fp(data.makeDistinctDataSet(ri, ri, le, le, ri, le).data()));
    EXPECT_EQ(114, fp(data.makeDistinctDataSet(ri, ri, le, le, ri, ri).data()));
    EXPECT_EQ(115, fp(data.makeDistinctDataSet(ri, ri, le, ri, le, le).data()));
    EXPECT_EQ(116, fp(data.makeDistinctDataSet(ri, ri, le, ri, le, ri).data()));
    EXPECT_EQ(117, fp(data.makeDistinctDataSet(ri, ri, le, ri, ri, le).data()));
    EXPECT_EQ(118, fp(data.makeDistinctDataSet(ri, ri, le, ri, ri, ri).data()));
    EXPECT_EQ(119, fp(data.makeDistinctDataSet(ri, ri, ri, le, le, le).data()));
    EXPECT_EQ(120, fp(data.makeDistinctDataSet(ri, ri, ri, le, le, ri).data()));
    EXPECT_EQ(121, fp(data.makeDistinctDataSet(ri, ri, ri, le, ri, le).data()));
    EXPECT_EQ(122, fp(data.makeDistinctDataSet(ri, ri, ri, le, ri, ri).data()));
    EXPECT_EQ(123, fp(data.makeDistinctDataSet(ri, ri, ri, ri, le, le).data()));
    EXPECT_EQ(124, fp(data.makeDistinctDataSet(ri, ri, ri, ri, le, ri).data()));
    EXPECT_EQ(125, fp(data.makeDistinctDataSet(ri, ri, ri, ri, ri, le).data()));
    EXPECT_EQ(126, fp(data.makeDistinctDataSet(ri, ri, ri, ri, ri, ri).data()));
  }
}


TEST(SingleCodegenL6, L3SubtreeSwitchAVX) {
  DecisionTreeFactory factory;
  JitDriver jitDriver;

  jitDriver.setCodegenSelector(makeLambdaSelector(
      [](const CompilerSession &session, int remainingLevels) {
        static L3SubtreeSwitchAVX codegen;
        return &codegen;
      }));

  { // test with single data-set feature
    DecisionTree tree = factory.makePerfectTrivialGradientTree(6);
    JitCompileResult result = jitDriver.run(std::move(tree));

    DataSetFactory data;
    auto *fp = result.EvaluatorFunction;

    EXPECT_EQ(63, fp(data.makeTrivialDataSet(1.0f / 128).data()));
    EXPECT_EQ(64, fp(data.makeTrivialDataSet(3.0f / 128).data()));
    EXPECT_EQ(65, fp(data.makeTrivialDataSet(5.0f / 128).data()));
    EXPECT_EQ(66, fp(data.makeTrivialDataSet(7.0f / 128).data()));
    EXPECT_EQ(67, fp(data.makeTrivialDataSet(9.0f / 128).data()));
    EXPECT_EQ(68, fp(data.makeTrivialDataSet(11.0f / 128).data()));
    EXPECT_EQ(69, fp(data.makeTrivialDataSet(13.0f / 128).data()));
    EXPECT_EQ(70, fp(data.makeTrivialDataSet(15.0f / 128).data()));
    EXPECT_EQ(71, fp(data.makeTrivialDataSet(17.0f / 128).data()));
    EXPECT_EQ(72, fp(data.makeTrivialDataSet(19.0f / 128).data()));
    EXPECT_EQ(73, fp(data.makeTrivialDataSet(21.0f / 128).data()));
    EXPECT_EQ(74, fp(data.makeTrivialDataSet(23.0f / 128).data()));
    EXPECT_EQ(75, fp(data.makeTrivialDataSet(25.0f / 128).data()));
    EXPECT_EQ(76, fp(data.makeTrivialDataSet(27.0f / 128).data()));
    EXPECT_EQ(77, fp(data.makeTrivialDataSet(29.0f / 128).data()));
    EXPECT_EQ(78, fp(data.makeTrivialDataSet(31.0f / 128).data()));
    EXPECT_EQ(79, fp(data.makeTrivialDataSet(33.0f / 128).data()));
    EXPECT_EQ(80, fp(data.makeTrivialDataSet(35.0f / 128).data()));
    EXPECT_EQ(81, fp(data.makeTrivialDataSet(37.0f / 128).data()));
    EXPECT_EQ(82, fp(data.makeTrivialDataSet(39.0f / 128).data()));
    EXPECT_EQ(83, fp(data.makeTrivialDataSet(41.0f / 128).data()));
    EXPECT_EQ(84, fp(data.makeTrivialDataSet(43.0f / 128).data()));
    EXPECT_EQ(85, fp(data.makeTrivialDataSet(45.0f / 128).data()));
    EXPECT_EQ(86, fp(data.makeTrivialDataSet(47.0f / 128).data()));
    EXPECT_EQ(87, fp(data.makeTrivialDataSet(49.0f / 128).data()));
    EXPECT_EQ(88, fp(data.makeTrivialDataSet(51.0f / 128).data()));
    EXPECT_EQ(89, fp(data.makeTrivialDataSet(53.0f / 128).data()));
    EXPECT_EQ(90, fp(data.makeTrivialDataSet(55.0f / 128).data()));
    EXPECT_EQ(91, fp(data.makeTrivialDataSet(57.0f / 128).data()));
    EXPECT_EQ(92, fp(data.makeTrivialDataSet(59.0f / 128).data()));
    EXPECT_EQ(93, fp(data.makeTrivialDataSet(61.0f / 128).data()));
    EXPECT_EQ(94, fp(data.makeTrivialDataSet(63.0f / 128).data()));
    EXPECT_EQ(95, fp(data.makeTrivialDataSet(65.0f / 128).data()));
    EXPECT_EQ(96, fp(data.makeTrivialDataSet(67.0f / 128).data()));
    EXPECT_EQ(97, fp(data.makeTrivialDataSet(69.0f / 128).data()));
    EXPECT_EQ(98, fp(data.makeTrivialDataSet(71.0f / 128).data()));
    EXPECT_EQ(99, fp(data.makeTrivialDataSet(73.0f / 128).data()));
    EXPECT_EQ(100, fp(data.makeTrivialDataSet(75.0f / 128).data()));
    EXPECT_EQ(101, fp(data.makeTrivialDataSet(77.0f / 128).data()));
    EXPECT_EQ(102, fp(data.makeTrivialDataSet(79.0f / 128).data()));
    EXPECT_EQ(103, fp(data.makeTrivialDataSet(81.0f / 128).data()));
    EXPECT_EQ(104, fp(data.makeTrivialDataSet(83.0f / 128).data()));
    EXPECT_EQ(105, fp(data.makeTrivialDataSet(85.0f / 128).data()));
    EXPECT_EQ(106, fp(data.makeTrivialDataSet(87.0f / 128).data()));
    EXPECT_EQ(107, fp(data.makeTrivialDataSet(89.0f / 128).data()));
    EXPECT_EQ(108, fp(data.makeTrivialDataSet(91.0f / 128).data()));
    EXPECT_EQ(109, fp(data.makeTrivialDataSet(93.0f / 128).data()));
    EXPECT_EQ(110, fp(data.makeTrivialDataSet(95.0f / 128).data()));
    EXPECT_EQ(111, fp(data.makeTrivialDataSet(97.0f / 128).data()));
    EXPECT_EQ(112, fp(data.makeTrivialDataSet(99.0f / 128).data()));
    EXPECT_EQ(113, fp(data.makeTrivialDataSet(101.0f / 128).data()));
    EXPECT_EQ(114, fp(data.makeTrivialDataSet(103.0f / 128).data()));
    EXPECT_EQ(115, fp(data.makeTrivialDataSet(105.0f / 128).data()));
    EXPECT_EQ(116, fp(data.makeTrivialDataSet(107.0f / 128).data()));
    EXPECT_EQ(117, fp(data.makeTrivialDataSet(109.0f / 128).data()));
    EXPECT_EQ(118, fp(data.makeTrivialDataSet(111.0f / 128).data()));
    EXPECT_EQ(119, fp(data.makeTrivialDataSet(113.0f / 128).data()));
    EXPECT_EQ(120, fp(data.makeTrivialDataSet(115.0f / 128).data()));
    EXPECT_EQ(121, fp(data.makeTrivialDataSet(117.0f / 128).data()));
    EXPECT_EQ(122, fp(data.makeTrivialDataSet(119.0f / 128).data()));
    EXPECT_EQ(123, fp(data.makeTrivialDataSet(121.0f / 128).data()));
    EXPECT_EQ(124, fp(data.makeTrivialDataSet(123.0f / 128).data()));
    EXPECT_EQ(125, fp(data.makeTrivialDataSet(125.0f / 128).data()));
    EXPECT_EQ(126, fp(data.makeTrivialDataSet(127.0f / 128).data()));
  }
  { // test with individual data-set features
    DecisionTree tree = factory.makePerfectDistinctGradientTree(6);
    JitCompileResult result = jitDriver.run(std::move(tree));

    auto *fp = result.EvaluatorFunction;
    DataSetFactory data(std::move(result.Tree), 63);

    auto le = NodeEvaluation::ContinueZeroLeft;
    auto ri = NodeEvaluation::ContinueOneRight;

    EXPECT_EQ(63, fp(data.makeDistinctDataSet(le, le, le, le, le, le).data()));
    EXPECT_EQ(64, fp(data.makeDistinctDataSet(le, le, le, le, le, ri).data()));
    EXPECT_EQ(65, fp(data.makeDistinctDataSet(le, le, le, le, ri, le).data()));
    EXPECT_EQ(66, fp(data.makeDistinctDataSet(le, le, le, le, ri, ri).data()));
    EXPECT_EQ(67, fp(data.makeDistinctDataSet(le, le, le, ri, le, le).data()));
    EXPECT_EQ(68, fp(data.makeDistinctDataSet(le, le, le, ri, le, ri).data()));
    EXPECT_EQ(69, fp(data.makeDistinctDataSet(le, le, le, ri, ri, le).data()));
    EXPECT_EQ(70, fp(data.makeDistinctDataSet(le, le, le, ri, ri, ri).data()));
    EXPECT_EQ(71, fp(data.makeDistinctDataSet(le, le, ri, le, le, le).data()));
    EXPECT_EQ(72, fp(data.makeDistinctDataSet(le, le, ri, le, le, ri).data()));
    EXPECT_EQ(73, fp(data.makeDistinctDataSet(le, le, ri, le, ri, le).data()));
    EXPECT_EQ(74, fp(data.makeDistinctDataSet(le, le, ri, le, ri, ri).data()));
    EXPECT_EQ(75, fp(data.makeDistinctDataSet(le, le, ri, ri, le, le).data()));
    EXPECT_EQ(76, fp(data.makeDistinctDataSet(le, le, ri, ri, le, ri).data()));
    EXPECT_EQ(77, fp(data.makeDistinctDataSet(le, le, ri, ri, ri, le).data()));
    EXPECT_EQ(78, fp(data.makeDistinctDataSet(le, le, ri, ri, ri, ri).data()));
    EXPECT_EQ(79, fp(data.makeDistinctDataSet(le, ri, le, le, le, le).data()));
    EXPECT_EQ(80, fp(data.makeDistinctDataSet(le, ri, le, le, le, ri).data()));
    EXPECT_EQ(81, fp(data.makeDistinctDataSet(le, ri, le, le, ri, le).data()));
    EXPECT_EQ(82, fp(data.makeDistinctDataSet(le, ri, le, le, ri, ri).data()));
    EXPECT_EQ(83, fp(data.makeDistinctDataSet(le, ri, le, ri, le, le).data()));
    EXPECT_EQ(84, fp(data.makeDistinctDataSet(le, ri, le, ri, le, ri).data()));
    EXPECT_EQ(85, fp(data.makeDistinctDataSet(le, ri, le, ri, ri, le).data()));
    EXPECT_EQ(86, fp(data.makeDistinctDataSet(le, ri, le, ri, ri, ri).data()));
    EXPECT_EQ(87, fp(data.makeDistinctDataSet(le, ri, ri, le, le, le).data()));
    EXPECT_EQ(88, fp(data.makeDistinctDataSet(le, ri, ri, le, le, ri).data()));
    EXPECT_EQ(89, fp(data.makeDistinctDataSet(le, ri, ri, le, ri, le).data()));
    EXPECT_EQ(90, fp(data.makeDistinctDataSet(le, ri, ri, le, ri, ri).data()));
    EXPECT_EQ(91, fp(data.makeDistinctDataSet(le, ri, ri, ri, le, le).data()));
    EXPECT_EQ(92, fp(data.makeDistinctDataSet(le, ri, ri, ri, le, ri).data()));
    EXPECT_EQ(93, fp(data.makeDistinctDataSet(le, ri, ri, ri, ri, le).data()));
    EXPECT_EQ(94, fp(data.makeDistinctDataSet(le, ri, ri, ri, ri, ri).data()));
    EXPECT_EQ(95, fp(data.makeDistinctDataSet(ri, le, le, le, le, le).data()));
    EXPECT_EQ(96, fp(data.makeDistinctDataSet(ri, le, le, le, le, ri).data()));
    EXPECT_EQ(97, fp(data.makeDistinctDataSet(ri, le, le, le, ri, le).data()));
    EXPECT_EQ(98, fp(data.makeDistinctDataSet(ri, le, le, le, ri, ri).data()));
    EXPECT_EQ(99, fp(data.makeDistinctDataSet(ri, le, le, ri, le, le).data()));
    EXPECT_EQ(100, fp(data.makeDistinctDataSet(ri, le, le, ri, le, ri).data()));
    EXPECT_EQ(101, fp(data.makeDistinctDataSet(ri, le, le, ri, ri, le).data()));
    EXPECT_EQ(102, fp(data.makeDistinctDataSet(ri, le, le, ri, ri, ri).data()));
    EXPECT_EQ(103, fp(data.makeDistinctDataSet(ri, le, ri, le, le, le).data()));
    EXPECT_EQ(104, fp(data.makeDistinctDataSet(ri, le, ri, le, le, ri).data()));
    EXPECT_EQ(105, fp(data.makeDistinctDataSet(ri, le, ri, le, ri, le).data()));
    EXPECT_EQ(106, fp(data.makeDistinctDataSet(ri, le, ri, le, ri, ri).data()));
    EXPECT_EQ(107, fp(data.makeDistinctDataSet(ri, le, ri, ri, le, le).data()));
    EXPECT_EQ(108, fp(data.makeDistinctDataSet(ri, le, ri, ri, le, ri).data()));
    EXPECT_EQ(109, fp(data.makeDistinctDataSet(ri, le, ri, ri, ri, le).data()));
    EXPECT_EQ(110, fp(data.makeDistinctDataSet(ri, le, ri, ri, ri, ri).data()));
    EXPECT_EQ(111, fp(data.makeDistinctDataSet(ri, ri, le, le, le, le).data()));
    EXPECT_EQ(112, fp(data.makeDistinctDataSet(ri, ri, le, le, le, ri).data()));
    EXPECT_EQ(113, fp(data.makeDistinctDataSet(ri, ri, le, le, ri, le).data()));
    EXPECT_EQ(114, fp(data.makeDistinctDataSet(ri, ri, le, le, ri, ri).data()));
    EXPECT_EQ(115, fp(data.makeDistinctDataSet(ri, ri, le, ri, le, le).data()));
    EXPECT_EQ(116, fp(data.makeDistinctDataSet(ri, ri, le, ri, le, ri).data()));
    EXPECT_EQ(117, fp(data.makeDistinctDataSet(ri, ri, le, ri, ri, le).data()));
    EXPECT_EQ(118, fp(data.makeDistinctDataSet(ri, ri, le, ri, ri, ri).data()));
    EXPECT_EQ(119, fp(data.makeDistinctDataSet(ri, ri, ri, le, le, le).data()));
    EXPECT_EQ(120, fp(data.makeDistinctDataSet(ri, ri, ri, le, le, ri).data()));
    EXPECT_EQ(121, fp(data.makeDistinctDataSet(ri, ri, ri, le, ri, le).data()));
    EXPECT_EQ(122, fp(data.makeDistinctDataSet(ri, ri, ri, le, ri, ri).data()));
    EXPECT_EQ(123, fp(data.makeDistinctDataSet(ri, ri, ri, ri, le, le).data()));
    EXPECT_EQ(124, fp(data.makeDistinctDataSet(ri, ri, ri, ri, le, ri).data()));
    EXPECT_EQ(125, fp(data.makeDistinctDataSet(ri, ri, ri, ri, ri, le).data()));
    EXPECT_EQ(126, fp(data.makeDistinctDataSet(ri, ri, ri, ri, ri, ri).data()));
  }
}
