#pragma once

#include <gtest/gtest.h>

#include "codegen/CodeGeneratorSelector.h"
#include "codegen/L1IfThenElse.h"
#include "data/DecisionTree.h"
#include "driver/JitDriver.h"
#include "Utils.h"

class DataSetFactory {
public:
  DataSetFactory(DecisionTree tree, uint32_t features)
      : Tree(std::move(tree)), Features(features) {}

  std::vector<float> makeDataSet(NodeEvaluation l1, NodeEvaluation l2) {
    std::vector<float> ds(Features, 0.0f);
    auto subtree = Tree.getSubtreeRef(Tree.getRootNodeIdx(), 2);

    ds[subtree.Root.getFeatureIdx()] = getAdjustedBias(subtree.Root, l1);

    auto node = subtree.Root.getChildFor(l1, subtree);
    ds[node.getFeatureIdx()] = getAdjustedBias(node, l2);

    return moveToVector(std::move(ds));
  }

private:
  DecisionTree Tree;
  uint32_t Features;

  float getAdjustedBias(DecisionTreeNode node, NodeEvaluation eval) const {
    bool addFraction = (eval == NodeEvaluation::ContinueOneRight);
    return node.getFeatureBias() + (addFraction ? 0.01f : -0.01f);
  }
};

TEST(L1IfThenElse, RegularTree2) {
  DecisionTreeNode n0(0, 0.5f, 0, 1, 2);
  DecisionTreeNode n1(1, 0.25f, 1, 3, 4);
  DecisionTreeNode n2(2, 0.75f, 2, 5, 6);

  DecisionTree tree(/*levels*/ 2, /*nodes*/ 3);
  tree.addNode(n0.getIdx(), n0);
  tree.addNode(n1.getIdx(), n1);
  tree.addNode(n2.getIdx(), n2);
  tree.finalize();

  // tree:
  //               ds[0] > 0.5
  //   ds[1] > 0.25           ds[2] > 0.75
  // 3              4       5              6  (implicit result nodes)

  JitDriver jitDriver;

  jitDriver.setCodegenSelector(makeLambdaSelector(
      [](const CompilerSession &session,
         int remainingLevels) {
    static L1IfThenElse codegen;
    return &codegen;
  }));

  auto left = NodeEvaluation::ContinueZeroLeft;
  auto right = NodeEvaluation::ContinueOneRight;

  DataSetFactory data(tree.copy(), 3);
  auto *fp = jitDriver.run(std::move(tree));

  EXPECT_EQ(3, fp(data.makeDataSet(left, left).data()));
  EXPECT_EQ(4, fp(data.makeDataSet(left, right).data()));
  EXPECT_EQ(5, fp(data.makeDataSet(right, left).data()));
  EXPECT_EQ(6, fp(data.makeDataSet(right, right).data()));
}
