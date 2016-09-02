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
      [](const CompilerSession &session,
         int remainingLevels) {
    static L1IfThenElse codegen;
    return &codegen;
  }));

  auto left = NodeEvaluation::ContinueZeroLeft;
  auto right = NodeEvaluation::ContinueOneRight;

  DataSetFactory data(tree.copy(), 3);
  auto *fp = jitDriver.run(std::move(tree));

  EXPECT_EQ(3, fp(data.makeDataSet(left, left).data()));
  EXPECT_EQ(4, fp(data.makeDataSet(left, right).data()));
  EXPECT_EQ(5, fp(data.makeDataSet(right, left).data()));
  EXPECT_EQ(6, fp(data.makeDataSet(right, right).data()));
}
