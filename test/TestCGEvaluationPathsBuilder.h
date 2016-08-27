#pragma once

#include <gtest/gtest.h>

#include "codegen/utility/CGEvaluationPathsBuilder.h"
#include "data/DecisionTree.h"

// helpers
std::pair<uint64_t, size_t> bubbleDown(DecisionSubtreeRef tree,
                                       CGEvaluationPath path);

TEST(CGEvaluationPathsBuilder, RegularTree1) {
  // create tree:
  //             0
  //        1         2   (implicit result nodes)

  auto treeData = (DecisionTreeFactory()).makeRandomRegular(1, 100);
  auto tree = treeData->getSubtreeRef(/*root*/0, /*levels*/1);

  CGEvaluationPathsBuilder builder(tree);
  std::vector<CGEvaluationPath> paths = builder.run();

  size_t expectedPaths = 2;
  size_t expectedNodesPerPath = 2;

  ASSERT_EQ(expectedPaths, paths.size());
  ASSERT_EQ(expectedNodesPerPath, paths[0].Nodes.size() + 1); //
  ASSERT_EQ(expectedNodesPerPath, paths[1].Nodes.size() + 1); // +1 for continuation node

  EXPECT_EQ(&tree.getNode(0), &paths[0].Nodes[0].getNodeData());
  EXPECT_EQ(&tree.getNode(1), &paths[0].Nodes[0].getChildNodeData());

  EXPECT_EQ(&tree.getNode(0), &paths[1].Nodes[0].getNodeData());
  EXPECT_EQ(&tree.getNode(2), &paths[1].Nodes[0].getChildNodeData());
}

TEST(CGEvaluationPathsBuilder, RegularTree2) {
  // create tree:
  //             0
  //        1            2
  //    3     4        5     6
  //  7 8    9 10    11 12  13 14   (implicit result nodes)
  //                 ^^^^^^^^^^^^
  //           looking at this subtree

  auto treeData = (DecisionTreeFactory()).makeRandomRegular(3, 100);
  auto tree = treeData->getSubtreeRef(/*root*/2, /*levels*/2);

  CGEvaluationPathsBuilder builder(tree);
  std::vector<CGEvaluationPath> paths = builder.run();

  size_t expectedPaths = 4;
  size_t expectedNodesPerPath = 3;
  ASSERT_EQ(expectedPaths, paths.size());

  // check paths by bubbling through and checking results
  int sumResultNodeIdxs = 0;
  for (auto path : paths) {
    uint64_t nodeIdx; size_t nodeCount;
    std::tie(nodeIdx, nodeCount) = bubbleDown(tree, std::move(path));

    EXPECT_EQ(expectedNodesPerPath, nodeCount);
    sumResultNodeIdxs += nodeIdx;
  }

  EXPECT_EQ(11 + 12 + 13 + 14, sumResultNodeIdxs);
}

TEST(CGEvaluationPathsBuilder, RegularTree4) {
  // create tree:
  //                        0
  //             1                     2
  //       3          4           5          6
  //   7     8     9     10   11    12    13    14
  // 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30  (implicit result nodes)

  auto treeData = (DecisionTreeFactory()).makeRandomRegular(4, 100);
  auto tree = treeData->getSubtreeRef(/*root*/0, /*levels*/4);

  CGEvaluationPathsBuilder builder(tree);
  std::vector<CGEvaluationPath> paths = builder.run();

  size_t expectedPaths = 16;
  size_t expectedNodesPerPath = 5;
  ASSERT_EQ(expectedPaths, paths.size());

  // check paths by bubbling through and checking results
  int sumResultNodeIdxs = 0;
  for (auto path : paths) {
    uint64_t nodeIdx; size_t nodeCount;
    std::tie(nodeIdx, nodeCount) = bubbleDown(tree, std::move(path));

    EXPECT_EQ(expectedNodesPerPath, nodeCount);
    sumResultNodeIdxs += nodeIdx;
  }

  EXPECT_EQ(8 * (15 + 30), sumResultNodeIdxs);
}

// ----------------------------------------------------------------------------

std::pair<uint64_t, size_t> bubbleDown(DecisionSubtreeRef tree,
                                       CGEvaluationPath path) {
  auto pathIt = path.Nodes.begin();
  uint64_t nodeIdx = pathIt->getNodeData().NodeIdx;
  size_t nodeCount = 1;

  while (!tree.getNode(nodeIdx).isLeaf()) {
    uint64_t expectedChildIdx =
        path.findNode(nodeIdx)->getChildNodeData().NodeIdx;

    if (pathIt->getEvaluationValue() == 0) {
      nodeIdx = tree.getNode(nodeIdx).FalseChildNodeIdx;
    }
    else {
      nodeIdx = tree.getNode(nodeIdx).TrueChildNodeIdx;
    }

    EXPECT_EQ(nodeIdx, expectedChildIdx);
    ++nodeCount;
    ++pathIt;
  }

  return std::make_pair(nodeIdx, nodeCount);
}
