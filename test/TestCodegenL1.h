#pragma once

#include <gtest/gtest.h>

#include "data/DecisionTree.h"
#include "driver/JitDriver.h"

TEST(L1IfThenElse, RegularTree2) {
  DecisionTreeNode rootNode(0, 0.5f, 0, 1, 2);
  DecisionTreeNode leftChildNode(1, 0.25f, 1, 3, 4);
  DecisionTreeNode rightChildNode(2, 0.75f, 2, 5, 6);

  DecisionTree tree(/*levels*/ 2, /*nodes*/ 3);
  {
    tree.addNode(rootNode.getIdx(), rootNode);
    tree.addNode(leftChildNode.getIdx(), leftChildNode);
    tree.addNode(rightChildNode.getIdx(), rightChildNode);
    tree.finalize();
  }

  // tree:
  //               ds[0] > 0.5
  //   ds[1] > 0.25           ds[2] > 0.75
  // 3              4       5              6  (implicit result nodes)

  JitDriver jitDriver;
  auto *fp = jitDriver.run(std::move(tree));
  {
    float dataSet[]{0.1f, 0.2f, 0.0f};
    EXPECT_EQ(3, fp(dataSet));
  }
  {
    float dataSet[]{0.4f, 0.3f, 0.0f};
    EXPECT_EQ(4, fp(dataSet));
  }
  {
    float dataSet[]{0.6f, 0.0f, 0.4f};
    EXPECT_EQ(5, fp(dataSet));
  }
  {
    float dataSet[]{0.6f, 0.0f, 0.8f};
    EXPECT_EQ(6, fp(dataSet));
  }
}
