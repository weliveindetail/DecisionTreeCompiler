#pragma once

#include <gtest/gtest.h>

#include "codegen/CGEvaluationPath.h"
#include "resolver/DecisionTree.h"

TEST(CGEvaluationPath, RegularTree1) {
  // create tree:
  //             0
  //        1         2   (implicit result nodes)

  DecisionTreeFactory treeFactory;
  auto tree = treeFactory.makeRandomRegular(1, 100);
  auto s = tree->getSubtreeRef(/*root*/0, /*levels*/1);

  // create path 0 -> 2
  {
    CGEvaluationPath p(s, s.getNode(2));
    p.addParent(s.getNode(0), NodeEvaluation_t::ContinueOneRight);

    EXPECT_EQ(&s.getNode(0), &p.Nodes[0].getNodeData());
    EXPECT_EQ(&s.getNode(2), &p.Nodes[0].getChildNodeData());
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

  auto tree = (DecisionTreeFactory()).makeRandomRegular(3, 100);
  auto s = tree->getSubtreeRef(/*root*/2, /*levels*/2);

  // create path 2 -> 5 -> 12
  {
    CGEvaluationPath p(s, s.getNode(12));
    p.addParent(s.getNode(5), NodeEvaluation_t::ContinueOneRight);
    p.addParent(s.getNode(2), NodeEvaluation_t::ContinueZeroLeft);

    EXPECT_EQ(s.getNode( 2).NodeIdx, p.Nodes[0].getNodeData().NodeIdx);
    EXPECT_EQ(s.getNode( 5).NodeIdx, p.Nodes[0].getChildNodeData().NodeIdx);
    EXPECT_EQ(s.getNode( 5).NodeIdx, p.Nodes[1].getNodeData().NodeIdx);
    EXPECT_EQ(s.getNode(12).NodeIdx, p.Nodes[1].getChildNodeData().NodeIdx);
  }

  // create path 2 -> 6 -> 14
  {
    CGEvaluationPath p(s, s.getNode(14));
    p.addParent(s.getNode(6), NodeEvaluation_t::ContinueOneRight);
    p.addParent(s.getNode(2), NodeEvaluation_t::ContinueOneRight);

    EXPECT_EQ(&s.getNode( 2), &p.Nodes[0].getNodeData());
    EXPECT_EQ(&s.getNode( 6), &p.Nodes[0].getChildNodeData());
    EXPECT_EQ(&s.getNode( 6), &p.Nodes[1].getNodeData());
    EXPECT_EQ(&s.getNode(14), &p.Nodes[1].getChildNodeData());
  }
}

TEST(CGEvaluationPath, RegularTree3) {
  // create tree:
  //             0
  //        1         2
  //    3     4     5     6
  //  7 8   9 10  11 12  13 14   (implicit result nodes)

  DecisionTreeFactory treeFactory;
  auto tree = treeFactory.makeRandomRegular(3, 100);
  auto s = tree->getSubtreeRef(/*root*/0, /*levels*/3);

  // create path 0 -> 1 -> 3 -> 8
  {
    CGEvaluationPath p(s, s.getNode(8));
    p.addParent(s.getNode(3), NodeEvaluation_t::ContinueOneRight);
    p.addParent(s.getNode(1), NodeEvaluation_t::ContinueZeroLeft);
    p.addParent(s.getNode(0), NodeEvaluation_t::ContinueZeroLeft);

    EXPECT_EQ(&s.getNode(0), &p.Nodes[0].getNodeData());
    EXPECT_EQ(&s.getNode(1), &p.Nodes[0].getChildNodeData());
    EXPECT_EQ(&s.getNode(1), &p.Nodes[1].getNodeData());
    EXPECT_EQ(&s.getNode(3), &p.Nodes[1].getChildNodeData());
    EXPECT_EQ(&s.getNode(3), &p.Nodes[2].getNodeData());
    EXPECT_EQ(&s.getNode(8), &p.Nodes[2].getChildNodeData());
  }

  // create path 0 -> 2 -> 6 -> 14
  {
    CGEvaluationPath p(s, s.getNode(14));
    p.addParent(s.getNode(6), NodeEvaluation_t::ContinueOneRight);
    p.addParent(s.getNode(2), NodeEvaluation_t::ContinueOneRight);
    p.addParent(s.getNode(0), NodeEvaluation_t::ContinueOneRight);

    EXPECT_EQ(&s.getNode( 0), &p.Nodes[0].getNodeData());
    EXPECT_EQ(&s.getNode( 2), &p.Nodes[0].getChildNodeData());
    EXPECT_EQ(&s.getNode( 2), &p.Nodes[1].getNodeData());
    EXPECT_EQ(&s.getNode( 6), &p.Nodes[1].getChildNodeData());
    EXPECT_EQ(&s.getNode( 6), &p.Nodes[2].getNodeData());
    EXPECT_EQ(&s.getNode(14), &p.Nodes[2].getChildNodeData());
  }
}
