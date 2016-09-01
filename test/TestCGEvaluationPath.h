#pragma once

#include <gtest/gtest.h>

#include "codegen/utility/CGEvaluationPath.h"
#include "data/DecisionSubtreeRef.h"
#include "data/DecisionTree.h"

TEST(CGEvaluationPath, RegularTree1) {
  // create tree:
  //             0
  //        1         2   (implicit result nodes)

  DecisionTree tree = (DecisionTreeFactory()).makeRandomRegular(1, 100);
  DecisionSubtreeRef subtree = tree.getSubtreeRef(/*root*/ 0, /*levels*/ 1);

  // test path 0 -> 2
  {
    CGEvaluationPath path(subtree, tree.getNode(2));
    path.addParent(tree.getNode(0), NodeEvaluation::ContinueOneRight);

    EXPECT_EQ(tree.getNode(0), path.getSrcNode());
    EXPECT_EQ(tree.getNode(0), path.getStep(0).getSrcNode());
    EXPECT_EQ(tree.getNode(2), path.getStep(0).getDestNode());
    EXPECT_EQ(tree.getNode(2), path.getDestNode());
  }
}

TEST(CGEvaluationPath, RegularTree2) {
  // create tree:
  //             0
  //        1         2
  //    3     4     5     6
  //  7 8   9 10  11 12  13 14   (implicit result nodes)
  //              ^^^^^^^^^^^^
  //        looking at this subtree

  DecisionTree tree = (DecisionTreeFactory()).makeRandomRegular(3, 100);
  DecisionSubtreeRef subtree = tree.getSubtreeRef(/*root*/ 2, /*levels*/ 2);

  // test path 2 -> 5 -> 12
  {
    CGEvaluationPath path(subtree, tree.getNode(12));
    path.addParent(tree.getNode(5), NodeEvaluation::ContinueOneRight);
    path.addParent(tree.getNode(2), NodeEvaluation::ContinueZeroLeft);

    EXPECT_EQ(tree.getNode(2), path.getSrcNode());
    EXPECT_EQ(tree.getNode(2), path.getStep(0).getSrcNode());
    EXPECT_EQ(tree.getNode(5), path.getStep(0).getDestNode());
    EXPECT_EQ(tree.getNode(5), path.getStep(1).getSrcNode());
    EXPECT_EQ(tree.getNode(12), path.getStep(1).getDestNode());
    EXPECT_EQ(tree.getNode(12), path.getDestNode());
  }

  // test path 2 -> 6 -> 14
  {
    CGEvaluationPath path(subtree, tree.getNode(14));
    path.addParent(tree.getNode(6), NodeEvaluation::ContinueOneRight);
    path.addParent(tree.getNode(2), NodeEvaluation::ContinueOneRight);

    EXPECT_EQ(tree.getNode(2), path.getSrcNode());
    EXPECT_EQ(tree.getNode(2), path.getStep(0).getSrcNode());
    EXPECT_EQ(tree.getNode(6), path.getStep(0).getDestNode());
    EXPECT_EQ(tree.getNode(6), path.getStep(1).getSrcNode());
    EXPECT_EQ(tree.getNode(14), path.getStep(1).getDestNode());
    EXPECT_EQ(tree.getNode(14), path.getDestNode());
  }
}

TEST(CGEvaluationPath, RegularTree3) {
  // create tree:
  //             0
  //        1         2
  //    3     4     5     6
  //  7 8   9 10  11 12  13 14   (implicit result nodes)

  DecisionTree tree = (DecisionTreeFactory()).makeRandomRegular(3, 100);
  DecisionSubtreeRef subtree = tree.getSubtreeRef(/*root*/ 0, /*levels*/ 3);

  // test path 0 -> 1 -> 3 -> 8
  {
    CGEvaluationPath path(subtree, tree.getNode(8));
    path.addParent(tree.getNode(3), NodeEvaluation::ContinueOneRight);
    path.addParent(tree.getNode(1), NodeEvaluation::ContinueZeroLeft);
    path.addParent(tree.getNode(0), NodeEvaluation::ContinueZeroLeft);

    EXPECT_EQ(tree.getNode(0), path.getSrcNode());
    EXPECT_EQ(tree.getNode(0), path.getStep(0).getSrcNode());
    EXPECT_EQ(tree.getNode(1), path.getStep(0).getDestNode());
    EXPECT_EQ(tree.getNode(1), path.getStep(1).getSrcNode());
    EXPECT_EQ(tree.getNode(3), path.getStep(1).getDestNode());
    EXPECT_EQ(tree.getNode(3), path.getStep(2).getSrcNode());
    EXPECT_EQ(tree.getNode(8), path.getStep(2).getDestNode());
    EXPECT_EQ(tree.getNode(8), path.getDestNode());
  }

  // test path 0 -> 2 -> 6 -> 14
  {
    CGEvaluationPath path(subtree, tree.getNode(14));
    path.addParent(tree.getNode(6), NodeEvaluation::ContinueOneRight);
    path.addParent(tree.getNode(2), NodeEvaluation::ContinueOneRight);
    path.addParent(tree.getNode(0), NodeEvaluation::ContinueOneRight);

    EXPECT_EQ(tree.getNode(0), path.getSrcNode());
    EXPECT_EQ(tree.getNode(0), path.getStep(0).getSrcNode());
    EXPECT_EQ(tree.getNode(2), path.getStep(0).getDestNode());
    EXPECT_EQ(tree.getNode(2), path.getStep(1).getSrcNode());
    EXPECT_EQ(tree.getNode(6), path.getStep(1).getDestNode());
    EXPECT_EQ(tree.getNode(6), path.getStep(2).getSrcNode());
    EXPECT_EQ(tree.getNode(14), path.getStep(2).getDestNode());
    EXPECT_EQ(tree.getNode(14), path.getDestNode());
  }
}
