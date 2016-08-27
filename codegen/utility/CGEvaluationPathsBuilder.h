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

  std::list<CGEvaluationPath> recurseToChildNode(NodeEvaluation_t eval,
                                                 const DecisionTreeNode &node,
                                                 uint8_t remainingLevels);

  template <class T> static std::vector<T> copyListToVector(std::list<T> l) {
    std::vector<T> v;
    v.reserve(l.size());
    std::copy(std::begin(l), std::end(l), std::back_inserter(v));
    return v;
  }
};
