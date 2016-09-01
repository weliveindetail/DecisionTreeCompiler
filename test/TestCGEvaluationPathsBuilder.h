#pragma once

#include <gtest/gtest.h>

#include "codegen/utility/CGEvaluationPathsBuilder.h"
#include "data/DecisionSubtreeRef.h"
#include "data/DecisionTree.h"

// helpers
std::pair<DecisionTreeNode, size_t> bubbleDown(DecisionSubtreeRef tree,
                                               CGEvaluationPath path);

// ----------------------------------------------------------------------------

TEST(CGEvaluationPathsBuilder, RegularTree1) {
  // create tree:
  //             0
  //        1         2   (implicit result nodes)

  DecisionTree tree = (DecisionTreeFactory()).makeRandomRegular(1, 100);
  DecisionSubtreeRef subtree = tree.getSubtreeRef(/*root*/ 0, /*levels*/ 1);

  CGEvaluationPathsBuilder builder(subtree);
  std::vector<CGEvaluationPath> paths = builder.run();

  size_t expectedPaths = 2;
  size_t expectedNodesPerPath = 2;

  ASSERT_EQ(expectedPaths, paths.size());
  ASSERT_EQ(expectedNodesPerPath, paths[0].getNumNodes());
  ASSERT_EQ(expectedNodesPerPath, paths[1].getNumNodes());

  EXPECT_EQ(tree.getNode(0), paths[0].getSrcNode());
  EXPECT_EQ(tree.getNode(0), paths[0].getStep(0).getSrcNode());
  EXPECT_EQ(tree.getNode(1), paths[0].getStep(0).getDestNode());
  EXPECT_EQ(tree.getNode(1), paths[0].getDestNode());

  EXPECT_EQ(tree.getNode(0), paths[1].getSrcNode());
  EXPECT_EQ(tree.getNode(0), paths[1].getStep(0).getSrcNode());
  EXPECT_EQ(tree.getNode(2), paths[1].getStep(0).getDestNode());
  EXPECT_EQ(tree.getNode(2), paths[1].getDestNode());
}

TEST(CGEvaluationPathsBuilder, RegularTree2) {
  // create tree:
  //             0
  //        1            2
  //    3     4        5     6
  //  7 8    9 10    11 12  13 14   (implicit result nodes)
  //                 ^^^^^^^^^^^^
  //           looking at this subtree

  DecisionTree tree = (DecisionTreeFactory()).makeRandomRegular(3, 100);
  DecisionSubtreeRef subtree = tree.getSubtreeRef(/*root*/ 2, /*levels*/ 2);

  CGEvaluationPathsBuilder builder(subtree);
  std::vector<CGEvaluationPath> paths = builder.run();

  size_t expectedPaths = 4;
  size_t expectedNodesPerPath = 3;
  ASSERT_EQ(expectedPaths, paths.size());

  // check paths by bubbling through and checking results
  int sumResultNodeIdxs = 0;
  for (auto path : paths) {
    DecisionTreeNode node;
    size_t nodeCount;
    std::tie(node, nodeCount) = bubbleDown(subtree, std::move(path));

    EXPECT_EQ(expectedNodesPerPath, nodeCount);
    sumResultNodeIdxs += node.getIdx();
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

  DecisionTree tree = (DecisionTreeFactory()).makeRandomRegular(4, 100);
  DecisionSubtreeRef subtree = tree.getSubtreeRef(/*root*/ 0, /*levels*/ 4);

  CGEvaluationPathsBuilder builder(subtree);
  std::vector<CGEvaluationPath> paths = builder.run();

  size_t expectedPaths = 16;
  size_t expectedNodesPerPath = 5;
  ASSERT_EQ(expectedPaths, paths.size());

  // check paths by bubbling through and checking results
  int sumResultNodeIdxs = 0;
  for (auto path : paths) {
    DecisionTreeNode node;
    size_t nodeCount;
    std::tie(node, nodeCount) = bubbleDown(subtree, std::move(path));

    EXPECT_EQ(expectedNodesPerPath, nodeCount);
    sumResultNodeIdxs += node.getIdx();
  }

  EXPECT_EQ(8 * (15 + 30), sumResultNodeIdxs);
}

// ----------------------------------------------------------------------------

std::pair<DecisionTreeNode, size_t> bubbleDown(DecisionSubtreeRef tree,
                                               CGEvaluationPath path) {
  DecisionTreeNode node = path.getSrcNode();
  size_t nodeCount = 1;

  while (node != path.getDestNode()) {
    EXPECT_EQ(path.getStep(nodeCount - 1).getSrcNode(), node);

    if (auto stepOrNull = path.findStepFromNode(node)) {
      node = stepOrNull->getDestNode();
      EXPECT_EQ(path.getStep(nodeCount - 1).getDestNode(), node);
      nodeCount++;
    }
    else {
      break;
    }
  }

  return std::make_pair(node, nodeCount);
}
