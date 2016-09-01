#pragma once

#include <gtest/gtest.h>

#include "data/DecisionSubtreeRef.h"
#include "data/DecisionTree.h"

TEST(DecisionTreeFactory, getLevelForNodeIdx) {
  EXPECT_EQ(0, DecisionTree::getLevelForNodeIdx(0));
  EXPECT_EQ(1, DecisionTree::getLevelForNodeIdx(1));
  EXPECT_EQ(1, DecisionTree::getLevelForNodeIdx(2));
  EXPECT_EQ(2, DecisionTree::getLevelForNodeIdx(3));
  EXPECT_EQ(2, DecisionTree::getLevelForNodeIdx(6));
  EXPECT_EQ(3, DecisionTree::getLevelForNodeIdx(7));
}

TEST(DecisionTreeFactory, getFirstNodeIdxOnLevel) {
  EXPECT_EQ(0, DecisionTree::getFirstNodeIdxOnLevel(0));
  EXPECT_EQ(1, DecisionTree::getFirstNodeIdxOnLevel(1));
  EXPECT_EQ(3, DecisionTree::getFirstNodeIdxOnLevel(2));
  EXPECT_EQ(7, DecisionTree::getFirstNodeIdxOnLevel(3));

  for (int level = 0; level < 25; level++) {
    auto fstIdx = DecisionTree::getFirstNodeIdxOnLevel(level);
    EXPECT_EQ(level, DecisionTree::getLevelForNodeIdx(fstIdx));
  }
}

TEST(DecisionSubtreeRef, collectNodes) {
  DecisionTreeFactory treeFactory;
  DecisionTree tree = treeFactory.makeRandomRegular(4, 100);

  // tree:
  //                        0
  //            1                       2
  //      3           4            5         6
  //   7     8     9     10    11    12   13    14
  // 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30  (implicit result nodes)

  {
    // 1-level subtree from root node
    auto nodes = tree.getSubtreeRef(0, 1).collectNodesPreOrder();
    ASSERT_EQ(1, nodes.size());
    EXPECT_EQ(tree.getNode(0), nodes.front());
  }
  {
    // 2-level subtree from node 1
    auto nodes = tree.getSubtreeRef(1, 2).collectNodesPreOrder();
    ASSERT_EQ(3, nodes.size());

    auto it = nodes.begin();
    EXPECT_EQ(tree.getNode(1), *(it++));
    EXPECT_EQ(tree.getNode(3), *(it++));
    EXPECT_EQ(tree.getNode(4), *(it++));
  }
  {
    // 3-level subtree from node 2
    auto nodes = tree.getSubtreeRef(2, 3).collectNodesPreOrder();
    ASSERT_EQ(7, nodes.size());

    auto it = nodes.begin();
    EXPECT_EQ(tree.getNode(2), *(it++));
    EXPECT_EQ(tree.getNode(5), *(it++));
    EXPECT_EQ(tree.getNode(11), *(it++));
    EXPECT_EQ(tree.getNode(12), *(it++));
    EXPECT_EQ(tree.getNode(6), *(it++));
    EXPECT_EQ(tree.getNode(13), *(it++));
    EXPECT_EQ(tree.getNode(14), *(it++));
  }
  {
    // 4-level subtree from root node (entire tree)
    auto nodes = tree.getSubtreeRef(0, 4).collectNodesPreOrder();
    ASSERT_EQ(15, nodes.size());

    auto it = nodes.begin();
    EXPECT_EQ(tree.getNode(0), *(it++));
    EXPECT_EQ(tree.getNode(1), *(it++));
    EXPECT_EQ(tree.getNode(3), *(it++));
    EXPECT_EQ(tree.getNode(7), *(it++));
    EXPECT_EQ(tree.getNode(8), *(it++));
    EXPECT_EQ(tree.getNode(4), *(it++));
    EXPECT_EQ(tree.getNode(9), *(it++));
    EXPECT_EQ(tree.getNode(10), *(it++));
    EXPECT_EQ(tree.getNode(2), *(it++));
    EXPECT_EQ(tree.getNode(5), *(it++));
    EXPECT_EQ(tree.getNode(11), *(it++));
    EXPECT_EQ(tree.getNode(12), *(it++));
    EXPECT_EQ(tree.getNode(6), *(it++));
    EXPECT_EQ(tree.getNode(13), *(it++));
    EXPECT_EQ(tree.getNode(14), *(it++));
  }
}

TEST(DecisionTreeNode, isImplicit) {
  // create tree:
  //             0
  //        1         2
  //    3     4     5     6
  //  7 8    9 10 11 12  13 14   (implicit result nodes)

  DecisionTree tree = (DecisionTreeFactory()).makeRandomRegular(3, 100);
  DecisionSubtreeRef subtree = tree.getSubtreeRef(0, 3);

  for (int i = 0; i < 7; i++)
    EXPECT_FALSE(tree.getNode(i).isImplicit());

  for (int i = 7; i < 15; i++)
    EXPECT_TRUE(tree.getNode(i).isImplicit());
}

TEST(DecisionTreeNode, isLeaf) {
  // create tree:
  //             0
  //        1         2
  //    3     4     5     6
  //  7 8    9 10 11 12  13 14   (implicit result nodes)

  DecisionTree tree = (DecisionTreeFactory()).makeRandomRegular(3, 100);
  DecisionSubtreeRef subtree = tree.getSubtreeRef(0, 3);

  for (int i = 0; i < 7; i++)
    EXPECT_FALSE(tree.getNode(i).isLeaf());

  for (int i = 7; i < 15; i++)
    EXPECT_TRUE(tree.getNode(i).isLeaf());
}
