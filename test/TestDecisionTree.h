#pragma once

#include <gtest/gtest.h>

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

TEST(DecisionTreeFactory, makeRandomRegular) {
  DecisionTreeFactory treeFactory;
  uint32_t dsf = 100;

  {
    std::unique_ptr<DecisionTree> tree = treeFactory.makeRandomRegular(1, dsf);
    DecisionSubtreeRef rootNodeRef = tree->getSubtreeRef(0, 1);

    std::vector<uint64_t> idxs = rootNodeRef.collectNodeIndices();
    ASSERT_EQ(1, idxs.size());
    EXPECT_EQ(0, idxs[0]);
  }
  {
    std::unique_ptr<DecisionTree> tree = treeFactory.makeRandomRegular(3, dsf);
    DecisionSubtreeRef subtreeRef = tree->getSubtreeRef(1, 2);

    std::vector<uint64_t> idxs = subtreeRef.collectNodeIndices();
    ASSERT_EQ(3, idxs.size());
    EXPECT_EQ(1, idxs[0]);
    EXPECT_EQ(3, idxs[1]);
    EXPECT_EQ(4, idxs[2]);
  }
}

TEST(DecisionTreeNode, isImplicit) {
  // create tree:
  //             0
  //        1         2
  //    3     4     5     6
  //  7 8    9 10 11 12  13 14   (implicit result nodes)

  DecisionTreeFactory treeFactory;
  auto tree = treeFactory.makeRandomRegular(3, 100);
  auto s = tree->getSubtreeRef(0, 3);

  for (int i = 0; i < 7; i++)
    EXPECT_FALSE(s.getNode(i).isImplicit());

  for (int i = 7; i < 15; i++)
    EXPECT_TRUE(s.getNode(i).isImplicit());
}

TEST(DecisionTreeNode, isLeaf) {
  // create tree:
  //             0
  //        1         2
  //    3     4     5     6
  //  7 8    9 10 11 12  13 14   (implicit result nodes)

  DecisionTreeFactory treeFactory;
  auto tree = treeFactory.makeRandomRegular(3, 100);
  auto s = tree->getSubtreeRef(0, 3);

  for (int i = 0; i < 7; i++)
    EXPECT_FALSE(s.getNode(i).isLeaf());

  for (int i = 7; i < 15; i++)
    EXPECT_TRUE(s.getNode(i).isLeaf());
}
