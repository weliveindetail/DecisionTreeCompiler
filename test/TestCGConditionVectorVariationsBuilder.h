#pragma once

#include <initializer_list>

#include <gtest/gtest.h>

#include "codegen/utility/CGConditionVectorVariationsBuilder.h"
#include "codegen/utility/CGEvaluationPathsBuilder.h"
#include "data/DecisionSubtreeRef.h"
#include "data/DecisionTree.h"

// helpers
void checkUniqueness(std::vector<uint32_t> actualVariations, int pathNum);

void checkVariations(std::vector<uint32_t> actualVariations,
                     std::vector<uint32_t> expectedVariations, int pathNum);

std::vector<uint32_t> flattenToVector(
    std::initializer_list<std::initializer_list<unsigned int>> nested);

// -----------------------------------------------------------------------------

TEST(CGConditionVectorVariationsBuilder, RegularTree1) {
  // create tree:
  //             0
  //        1         2   (implicit result nodes)

  DecisionTree tree = (DecisionTreeFactory()).makeRandomRegular(1, 100);
  DecisionSubtreeRef subtree = tree.getSubtreeRef(/*root*/ 0, /*levels*/ 1);

  CGEvaluationPathsBuilder pathsBuilder(subtree);
  std::vector<CGEvaluationPath> paths = pathsBuilder.run();
  CGConditionVectorVariationsBuilder variationsBuilder(subtree);

  EXPECT_EQ(tree.getNode(0), variationsBuilder.getNodeForBitOffset(0));
  EXPECT_EQ(DecisionTreeNode(), variationsBuilder.getNodeForBitOffset(1));

  EXPECT_EQ(0, variationsBuilder.getBitOffsetForNode(tree.getNode(0)));
  EXPECT_EQ(0xFFFFFFFF, variationsBuilder.getBitOffsetForNode(tree.getNode(1)));

  checkVariations(variationsBuilder.run(paths[0]), {0b0}, 0); // 0 -> 1
  checkVariations(variationsBuilder.run(paths[1]), {0b1}, 1); // 0 -> 2
}

TEST(CGConditionVectorVariationsBuilder, RegularTree2) {
  // create tree:
  //              0
  //        1            2
  //    3     4        5     6
  //  7 8    9 10    11 12  13 14   (implicit result nodes)
  //                 ^^^^^^^^^^^^
  //           looking at this subtree

  DecisionTree tree = (DecisionTreeFactory()).makeRandomRegular(3, 100);
  DecisionSubtreeRef subtree = tree.getSubtreeRef(/*root*/ 2, /*levels*/ 2);

  CGEvaluationPathsBuilder pathsBuilder(subtree);
  std::vector<CGEvaluationPath> paths = pathsBuilder.run();
  CGConditionVectorVariationsBuilder variationsBuilder(subtree);

  EXPECT_EQ(tree.getNode(2), variationsBuilder.getNodeForBitOffset(0));
  EXPECT_EQ(tree.getNode(5), variationsBuilder.getNodeForBitOffset(1));
  EXPECT_EQ(tree.getNode(6), variationsBuilder.getNodeForBitOffset(2));
  EXPECT_EQ(DecisionTreeNode(), variationsBuilder.getNodeForBitOffset(3));

  EXPECT_EQ(0, variationsBuilder.getBitOffsetForNode(tree.getNode(2)));
  EXPECT_EQ(1, variationsBuilder.getBitOffsetForNode(tree.getNode(5)));
  EXPECT_EQ(2, variationsBuilder.getBitOffsetForNode(tree.getNode(6)));
  EXPECT_EQ(0xFFFFFFFF, variationsBuilder.getBitOffsetForNode(tree.getNode(1)));

  // check variations for all paths
  // variation encoding: 0bCBA with node indices A=2, B=5 and C=6
  ASSERT_EQ(tree.getNode(2), paths[0].getSrcNode());
  ASSERT_EQ(tree.getNode(11), paths[0].getDestNode());
  checkVariations(variationsBuilder.run(paths[0]), {0b000, 0b100}, 0);

  ASSERT_EQ(tree.getNode(2), paths[1].getSrcNode());
  ASSERT_EQ(tree.getNode(12), paths[1].getDestNode());
  checkVariations(variationsBuilder.run(paths[1]), {0b010, 0b110}, 1);

  ASSERT_EQ(tree.getNode(2), paths[2].getSrcNode());
  ASSERT_EQ(tree.getNode(13), paths[2].getDestNode());
  checkVariations(variationsBuilder.run(paths[2]), {0b001, 0b011}, 2);

  ASSERT_EQ(tree.getNode(2), paths[3].getSrcNode());
  ASSERT_EQ(tree.getNode(14), paths[3].getDestNode());
  checkVariations(variationsBuilder.run(paths[3]), {0b101, 0b111}, 3);

  // check uniqueness for all paths
  for (size_t i = 0; i < paths.size(); i++) {
    checkUniqueness(variationsBuilder.run(paths[i]), i);
  }
}

TEST(CGConditionVectorVariationsBuilder, RegularTree3) {
  // create tree:
  //              0
  //        1            2
  //    3     4        5     6
  //  7 8    9 10    11 12  13 14   (implicit result nodes)

  DecisionTree tree = (DecisionTreeFactory()).makeRandomRegular(3, 100);
  DecisionSubtreeRef subtree = tree.getSubtreeRef(/*root*/ 0, /*levels*/ 3);

  CGEvaluationPathsBuilder pathsBuilder(subtree);
  std::vector<CGEvaluationPath> paths = pathsBuilder.run();
  CGConditionVectorVariationsBuilder variationsBuilder(subtree);

  // nodes are collected in a pre-order traversal
  EXPECT_EQ(tree.getNode(0), variationsBuilder.getNodeForBitOffset(0));
  EXPECT_EQ(tree.getNode(1), variationsBuilder.getNodeForBitOffset(1));
  EXPECT_EQ(tree.getNode(3), variationsBuilder.getNodeForBitOffset(2));
  EXPECT_EQ(tree.getNode(4), variationsBuilder.getNodeForBitOffset(3));
  EXPECT_EQ(tree.getNode(2), variationsBuilder.getNodeForBitOffset(4));
  EXPECT_EQ(tree.getNode(5), variationsBuilder.getNodeForBitOffset(5));
  EXPECT_EQ(tree.getNode(6), variationsBuilder.getNodeForBitOffset(6));

  EXPECT_EQ(0, variationsBuilder.getBitOffsetForNode(tree.getNode(0)));
  EXPECT_EQ(1, variationsBuilder.getBitOffsetForNode(tree.getNode(1)));
  EXPECT_EQ(4, variationsBuilder.getBitOffsetForNode(tree.getNode(2)));
  EXPECT_EQ(2, variationsBuilder.getBitOffsetForNode(tree.getNode(3)));
  EXPECT_EQ(3, variationsBuilder.getBitOffsetForNode(tree.getNode(4)));
  EXPECT_EQ(5, variationsBuilder.getBitOffsetForNode(tree.getNode(5)));
  EXPECT_EQ(6, variationsBuilder.getBitOffsetForNode(tree.getNode(6)));

  {
    CGEvaluationPath path = paths[0]; // 0 -> 1 -> 3 -> 7
    ASSERT_EQ(tree.getNode(0), path.getSrcNode());
    ASSERT_EQ(tree.getNode(7), path.getDestNode());

    // fixed bits: 0, 1 and 2 (all zero)
    // variable bits: 3, 4, 5, 6
    //
    std::initializer_list<std::initializer_list<unsigned int>>
        expectedVariations = {
            {0b0000000},
            {0b0001000, 0b0010000, 0b0100000, 0b1000000},
            {0b0011000, 0b0101000, 0b1001000, 0b0110000, 0b1010000, 0b1100000},
            {0b0111000, 0b1011000, 0b1101000, 0b1110000},
            {0b1111000}};

    checkVariations(variationsBuilder.run(path),
                    flattenToVector(expectedVariations), 0);
  }

  // check uniqueness for all paths
  for (size_t i = 0; i < paths.size(); i++) {
    checkUniqueness(variationsBuilder.run(paths[i]), i);
  }
}

// -----------------------------------------------------------------------------

void checkVariations(std::vector<uint32_t> actualVariations,
                     std::vector<uint32_t> expectedVariations, int pathNum) {
  std::string testName =
      ::testing::UnitTest::GetInstance()->current_test_info()->name();

  auto countActualInstances = [&](uint32_t variation) {
    return std::count(actualVariations.begin(), actualVariations.end(),
                      variation);
  };

  for (uint32_t variation : expectedVariations) {
    EXPECT_EQ(1, countActualInstances(variation))
        << "Missing variation " << std::bitset<32>(variation) << " in "
        << testName << " path " << pathNum;
  }
}

void checkUniqueness(std::vector<uint32_t> actualVariations, int pathNum) {
  size_t sizeBefore = actualVariations.size();

  // there must be no duplicates
  std::sort(actualVariations.begin(), actualVariations.end());
  std::unique(actualVariations.begin(), actualVariations.end());

  std::string testName =
      ::testing::UnitTest::GetInstance()->current_test_info()->name();

  EXPECT_EQ(sizeBefore, actualVariations.size())
      << "Duplicate variations in " << testName << " path " << pathNum;
}

std::vector<uint32_t> flattenToVector(
    std::initializer_list<std::initializer_list<unsigned int>> nested) {
  std::vector<uint32_t> v;
  for (auto list : nested)
    for (auto item : list)
      v.push_back(item);
  return v;
}
