#pragma once

#include <gtest/gtest.h>

#include "codegen/CodeGeneratorSelector.h"
#include "codegen/L1IfThenElse.h"
#include "data/DataSetFactory.h"
#include "data/DecisionTree.h"
#include "driver/JitDriver.h"

TEST(L1IfThenElse, RegularTree2) {
  DecisionTreeNode n0(0, 0.5f, 0, 1, 2);
  DecisionTreeNode n1(1, 0.25f, 1, 3, 4);
  DecisionTreeNode n2(2, 0.75f, 2, 5, 6);

  DecisionTree tree(/*levels*/ 2, /*nodes*/ 3);
  tree.addNodes(n0, n1, n2);
  tree.finalize();

  // tree:
  //           0
  //     1            2
  // 3     4        5     6   (implicit result nodes)

  JitDriver jitDriver;

  jitDriver.setCodegenSelector(makeLambdaSelector(
      [](const CompilerSession &session, int remainingLevels) {
        static L1IfThenElse codegen;
        return &codegen;
      }));

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

TEST(L1IfThenElse, RegularTree3) {
  DecisionTreeNode n0(0, 0.5f, 0, 1, 2);
  DecisionTreeNode n1(1, 0.25f, 1, 3, 4);
  DecisionTreeNode n2(2, 0.75f, 2, 5, 6);
  DecisionTreeNode n3(3, 0.2f, 3, 7, 8);
  DecisionTreeNode n4(4, 0.3f, 4, 9, 10);
  DecisionTreeNode n5(5, 0.7f, 5, 11, 12);
  DecisionTreeNode n6(6, 0.8f, 6, 13, 14);

  DecisionTree tree(/*levels*/ 3, /*nodes*/ 7);
  tree.addNodes(n0, n1, n2, n3, n4, n5, n6);
  tree.finalize();

  // tree:
  //              0
  //        1            2
  //    3     4        5     6
  //  7 8    9 10    11 12  13 14   (implicit result nodes)

  JitDriver jitDriver;

  jitDriver.setCodegenSelector(makeLambdaSelector(
      [](const CompilerSession &session, int remainingLevels) {
        static L1IfThenElse codegen;
        return &codegen;
      }));

  JitCompileResult result = jitDriver.run(std::move(tree));

  auto *fp = result.EvaluatorFunction;
  DataSetFactory data(std::move(result.Tree), 7);

  auto left = NodeEvaluation::ContinueZeroLeft;
  auto right = NodeEvaluation::ContinueOneRight;

  EXPECT_EQ(7, fp(data.makeDistinctDataSet(left, left, left).data()));
  EXPECT_EQ(8, fp(data.makeDistinctDataSet(left, left, right).data()));
  EXPECT_EQ(9, fp(data.makeDistinctDataSet(left, right, left).data()));
  EXPECT_EQ(10, fp(data.makeDistinctDataSet(left, right, right).data()));
  EXPECT_EQ(11, fp(data.makeDistinctDataSet(right, left, left).data()));
  EXPECT_EQ(12, fp(data.makeDistinctDataSet(right, left, right).data()));
  EXPECT_EQ(13, fp(data.makeDistinctDataSet(right, right, left).data()));
  EXPECT_EQ(14, fp(data.makeDistinctDataSet(right, right, right).data()));
}
