#include "CGEvaluationPathsBuilder.h"

std::vector<CGEvaluationPath>
    CGEvaluationPathsBuilder::run() {
  std::list<CGEvaluationPath> paths =
    buildPathsRecursively(Subtree.RootIndex, Subtree.Levels);

  assert(paths.size() == Subtree.getContinuationNodeCount());
  return copyListToVector(std::move(paths));
}

std::list<CGEvaluationPath>
CGEvaluationPathsBuilder::buildPathsRecursively(
    uint64_t nodeIdx, uint8_t remainingLevels) {
  const DecisionTreeNode &node = Subtree.getNode(nodeIdx);

  // subtree continuation nodes insert a new path
  if (node.isLeaf() || remainingLevels == 0)
    return {CGEvaluationPath(Subtree, node)};

  // subtree nodes add themselves to all child continuation node paths
  std::list<CGEvaluationPath> paths;

  paths.splice(paths.end(), recurseToChildNode(
      NodeEvaluation_t::ContinueZeroLeft, node, remainingLevels));

  paths.splice(paths.end(), recurseToChildNode(
      NodeEvaluation_t::ContinueOneRight, node, remainingLevels));

  return paths;
}

std::list<CGEvaluationPath>
CGEvaluationPathsBuilder::recurseToChildNode(
    NodeEvaluation_t eval, const DecisionTreeNode &node,
    uint8_t remainingLevels) {
  if (!node.hasChildForEvaluation(eval))
    return {};

  std::list<CGEvaluationPath> paths =
      buildPathsRecursively(node.getChildIdx(eval), remainingLevels - 1);

  for (CGEvaluationPath &p : paths)
    p.addParent(node, eval);

  return paths;
}
