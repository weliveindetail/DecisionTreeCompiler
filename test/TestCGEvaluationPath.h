#pragma once

#include <gtest/gtest.h>

#include "codegen/utility/CGEvaluationPath.h"
#include "data/DecisionTree.h"

TEST(CGEvaluationPath, RegularTree1) {
  // create tree:
  //             0
  //        1         2   (implicit result nodes)

  DecisionTree tree = (DecisionTreeFactory()).makeRandomRegular(1, 100);
  DecisionSubtreeRef s = tree.getSubtreeRef(/*root*/ 0, /*levels*/ 1);

  // create path 0 -> 2
  {
    CGEvaluationPath p(s, s.getNode(2));
    p.addParent(s.getNode(0), NodeEvaluation::ContinueOneRight);

    EXPECT_EQ(s.getNode(0), p.getStep(0).getSrcNode());
    EXPECT_EQ(s.getNode(2), p.getStep(0).getDestNode());
  }
}

TEST(CGEvaluationPath, RegularTree2) {
  // create tree:
  //             0
  //        1         2
  //    3     4     5     6
  //  7 8   9 10  11 12  13 14   (implicit result nodes)
  //                 ^^^^^^^^^^^^
  //           looking at this subtree

  DecisionTree tree = (DecisionTreeFactory()).makeRandomRegular(3, 100);
  DecisionSubtreeRef s = tree.getSubtreeRef(/*root*/ 2, /*levels*/ 2);

  // create path 2 -> 5 -> 12
  {
    CGEvaluationPath p(s, s.getNode(12));
    p.addParent(s.getNode(5), NodeEvaluation::ContinueOneRight);
    p.addParent(s.getNode(2), NodeEvaluation::ContinueZeroLeft);

    EXPECT_EQ(s.getNode(2), p.getStep(0).getSrcNode());
    EXPECT_EQ(s.getNode(5), p.getStep(0).getDestNode());
    EXPECT_EQ(s.getNode(5), p.getStep(1).getSrcNode());
    EXPECT_EQ(s.getNode(12), p.getStep(1).getDestNode());
  }

  // create path 2 -> 6 -> 14
  {
    CGEvaluationPath p(s, s.getNode(14));
    p.addParent(s.getNode(6), NodeEvaluation::ContinueOneRight);
    p.addParent(s.getNode(2), NodeEvaluation::ContinueOneRight);

    EXPECT_EQ(s.getNode(2), p.getStep(0).getSrcNode());
    EXPECT_EQ(s.getNode(6), p.getStep(0).getDestNode());
    EXPECT_EQ(s.getNode(6), p.getStep(1).getSrcNode());
    EXPECT_EQ(s.getNode(14), p.getStep(1).getDestNode());
  }
}

TEST(CGEvaluationPath, RegularTree3) {
  // create tree:
  //             0
  //        1         2
  //    3     4     5     6
  //  7 8   9 10  11 12  13 14   (implicit result nodes)

  DecisionTree tree = (DecisionTreeFactory()).makeRandomRegular(3, 100);
  DecisionSubtreeRef s = tree.getSubtreeRef(/*root*/ 0, /*levels*/ 3);

  // create path 0 -> 1 -> 3 -> 8
  {
    CGEvaluationPath p(s, s.getNode(8));
    p.addParent(s.getNode(3), NodeEvaluation::ContinueOneRight);
    p.addParent(s.getNode(1), NodeEvaluation::ContinueZeroLeft);
    p.addParent(s.getNode(0), NodeEvaluation::ContinueZeroLeft);

    EXPECT_EQ(s.getNode(0), p.getStep(0).getSrcNode());
    EXPECT_EQ(s.getNode(1), p.getStep(0).getDestNode());
    EXPECT_EQ(s.getNode(1), p.getStep(1).getSrcNode());
    EXPECT_EQ(s.getNode(3), p.getStep(1).getDestNode());
    EXPECT_EQ(s.getNode(3), p.getStep(2).getSrcNode());
    EXPECT_EQ(s.getNode(8), p.getStep(2).getDestNode());
  }

  // create path 0 -> 2 -> 6 -> 14
  {
    CGEvaluationPath p(s, s.getNode(14));
    p.addParent(s.getNode(6), NodeEvaluation::ContinueOneRight);
    p.addParent(s.getNode(2), NodeEvaluation::ContinueOneRight);
    p.addParent(s.getNode(0), NodeEvaluation::ContinueOneRight);

    EXPECT_EQ(s.getNode(0), p.getStep(0).getSrcNode());
    EXPECT_EQ(s.getNode(2), p.getStep(0).getDestNode());
    EXPECT_EQ(s.getNode(2), p.getStep(1).getSrcNode());
    EXPECT_EQ(s.getNode(6), p.getStep(1).getDestNode());
    EXPECT_EQ(s.getNode(6), p.getStep(2).getSrcNode());
    EXPECT_EQ(s.getNode(14), p.getStep(2).getDestNode());
  }
}
