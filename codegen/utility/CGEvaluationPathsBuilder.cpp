#include "codegen/utility/CGEvaluationPathsBuilder.h"
#include "Utils.h"

std::vector<CGEvaluationPath> CGEvaluationPathsBuilder::run() {
  std::list<CGEvaluationPath> paths =
      buildPathsRecursively(Subtree.Root, Subtree.Levels);

  assert(paths.size() == Subtree.getContinuationNodeCount());
  return moveToVector(std::move(paths));
}

std::list<CGEvaluationPath>
CGEvaluationPathsBuilder::buildPathsRecursively(DecisionTreeNode node,
                                                uint8_t remainingLevels) {
  // subtree continuation nodes insert a new path
  if (node.isLeaf() || remainingLevels == 0)
    return {CGEvaluationPath(Subtree, node)};

  // subtree nodes add themselves to all child continuation node paths
  std::list<CGEvaluationPath> paths;

  paths.splice(paths.end(),
               recurseToChildNode(NodeEvaluation::ContinueZeroLeft, node,
                                  remainingLevels));

  paths.splice(paths.end(),
               recurseToChildNode(NodeEvaluation::ContinueOneRight, node,
                                  remainingLevels));

  return paths;
}

std::list<CGEvaluationPath>
CGEvaluationPathsBuilder::recurseToChildNode(NodeEvaluation eval,
                                             DecisionTreeNode node,
                                             uint8_t remainingLevels) {
  if (!node.hasChildForEvaluation(eval))
    return {};

  std::list<CGEvaluationPath> paths =
      buildPathsRecursively(node.getChild(eval), remainingLevels - 1);

  for (CGEvaluationPath &p : paths)
    p.addParent(node, eval);

  return paths;
}
