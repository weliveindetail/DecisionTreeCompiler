#pragma once

#include <gtest/gtest.h>

#include "codegen/utility/CGEvaluationPathsBuilder.h"
#include "codegen/utility/CGConditionVectorVariationsBuilder.h"
#include "data/DecisionTree.h"

// helpers
void checkUniqueness(int pathNum,
                     std::vector<uint32_t> actualVariations);

void checkVariations(int pathNum,
                     std::vector<uint32_t> actualVariations,
                     std::vector<uint32_t> expectedVariations);

// ----------------------------------------------------------------------------

TEST(CGConditionVectorVariationsBuilder, RegularTree1) {
  // create tree:
  //             0
  //        1         2   (implicit result nodes)

  auto treeData = (DecisionTreeFactory()).makeRandomRegular(1, 100);
  auto tree = treeData->getSubtreeRef(/*root*/0, /*levels*/1);

  CGEvaluationPathsBuilder pathsBuilder(tree);
  std::vector<CGEvaluationPath> paths = pathsBuilder.run();
  CGConditionVectorVariationsBuilder variationsBuilder(tree);

  checkVariations(0, variationsBuilder.run(paths[0]), {0b0}); // 0 -> 1
  checkVariations(1, variationsBuilder.run(paths[1]), {0b1}); // 0 -> 2
}

// ----------------------------------------------------------------------------

TEST(CGConditionVectorVariationsBuilder, RegularTree2) {
  // create tree:
  //              0
  //        1            2
  //    3     4        5     6
  //  7 8    9 10    11 12  13 14   (implicit result nodes)
  //                 ^^^^^^^^^^^^
  //           looking at this subtree

  auto treeData = (DecisionTreeFactory()).makeRandomRegular(3, 100);
  auto tree = treeData->getSubtreeRef(/*root*/2, /*levels*/2);

  CGEvaluationPathsBuilder pathsBuilder(tree);
  std::vector<CGEvaluationPath> paths = pathsBuilder.run();
  CGConditionVectorVariationsBuilder variationsBuilder(tree);

  // check variations for all paths
  // variation encoding: 0bCBA with node indices A=2, B=5 and C=6
  checkVariations(0, variationsBuilder.run(paths[0]), {0b000, 0b100}); // 2 -> 5 -> 11
  checkVariations(1, variationsBuilder.run(paths[1]), {0b010, 0b110}); // 2 -> 5 -> 12
  checkVariations(2, variationsBuilder.run(paths[2]), {0b001, 0b011}); // 2 -> 6 -> 13
  checkVariations(3, variationsBuilder.run(paths[3]), {0b101, 0b111}); // 2 -> 6 -> 14

  // check uniqueness for all paths
  for (size_t i = 0; i < paths.size(); i++) {
    checkUniqueness(i, variationsBuilder.run(paths[i]));
  }
}

// ----------------------------------------------------------------------------

TEST(CGConditionVectorVariationsBuilder, RegularTree3) {
  // create tree:
  //              0
  //        1            2
  //    3     4        5     6
  //  7 8    9 10    11 12  13 14   (implicit result nodes)

  auto treeData = (DecisionTreeFactory()).makeRandomRegular(3, 100);
  auto tree = treeData->getSubtreeRef(/*root*/0, /*levels*/3);

  CGEvaluationPathsBuilder pathsBuilder(tree);
  std::vector<CGEvaluationPath> paths = pathsBuilder.run();
  CGConditionVectorVariationsBuilder variationsBuilder(tree);

  // check variation for path 0 -> 1 -> 3 -> 7
  //
  // variation encoding: 0b GFED CB A
  //  with node indices:    6..3 21 0
  //
  checkVariations(0, variationsBuilder.run(paths[0]), {
      //___D_BA     ___D_BA     ___D_BA     ___D_BA     ___D_BA     ___D_BA
      0b0000000,
      0b0000100,  0b0010000,  0b0100000,  0b1000000,
      0b0010100,  0b0100100,  0b1000100,  0b0110000,  0b1010000,  0b1100000,
      0b0110100,  0b1010100,  0b1100100,  0b1110000,
      0b1110100
  });

  // check uniqueness for all paths
  for (size_t i = 0; i < paths.size(); i++) {
    checkUniqueness(i, variationsBuilder.run(paths[i]));
  }
}

// ----------------------------------------------------------------------------

void checkVariations(int pathNum,
                     std::vector<uint32_t> actualVariations,
                     std::vector<uint32_t> expectedVariations) {
  std::string testName =
      ::testing::UnitTest::GetInstance()->current_test_info()->name();

  auto countActualInstances = [&](uint32_t variation) {
    return std::count(actualVariations.begin(),
                      actualVariations.end(), variation);
  };

  for (uint32_t variation : expectedVariations) {
    EXPECT_EQ(1, countActualInstances(variation))
              << "Missing variation " << std::bitset<32>(variation)
              << " in " << testName << " path " << pathNum;
  }
}

void checkUniqueness(int pathNum, std::vector<uint32_t> actualVariations) {
  size_t sizeBefore = actualVariations.size();

  // there must be no duplicates
  std::sort(actualVariations.begin(), actualVariations.end());
  std::unique(actualVariations.begin(), actualVariations.end());

  std::string testName =
      ::testing::UnitTest::GetInstance()->current_test_info()->name();

  EXPECT_EQ(sizeBefore, actualVariations.size())
            << "Duplicate variations in " << testName << " path " << pathNum;
}