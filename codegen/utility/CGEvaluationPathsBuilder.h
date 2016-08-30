#pragma once

#include <list>
#include <vector>

#include "codegen/utility/CGEvaluationPath.h"
#include "data/DecisionSubtreeRef.h"

class CGEvaluationPathsBuilder {
public:
  CGEvaluationPathsBuilder(DecisionSubtreeRef subtree)
      : Subtree(std::move(subtree)){};

  std::vector<CGEvaluationPath> run();

private:
  DecisionSubtreeRef Subtree;

  std::list<CGEvaluationPath> buildPathsRecursively(DecisionTreeNode node,
                                                    uint8_t remainingLevels);

  std::list<CGEvaluationPath> recurseToChildNode(NodeEvaluation eval,
                                                 DecisionTreeNode node,
                                                 uint8_t remainingLevels);
};
