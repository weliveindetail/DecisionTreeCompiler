#pragma once

#include <list>
#include <vector>

#include "CGEvaluationPath.h"
#include "data/DecisionTree.h"

class CGEvaluationPathsBuilder {
public:
  CGEvaluationPathsBuilder(DecisionSubtreeRef subtree)
      : Subtree(std::move(subtree)){};

  std::vector<CGEvaluationPath> run();

private:
  DecisionSubtreeRef Subtree;

  std::list<CGEvaluationPath> buildPathsRecursively(uint64_t nodeIdx,
                                                    uint8_t remainingLevels);

  std::list<CGEvaluationPath> recurseToChildNode(NodeEvaluation eval,
                                                 const DecisionTreeNode &node,
                                                 uint8_t remainingLevels);
};
