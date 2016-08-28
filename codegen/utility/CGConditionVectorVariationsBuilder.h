#pragma once

#include <cstdint>
#include <list>
#include <vector>

#include "CGEvaluationPath.h"
#include "data/DecisionTree.h"

class CGConditionVectorVariationsBuilder {
public:
  CGConditionVectorVariationsBuilder(DecisionSubtreeRef subtreeRef)
      : Subtree(std::move(subtreeRef)),
        Nodes(moveToVector(Subtree.collectNodesPreOrder())) {}

  std::vector<uint32_t> run(CGEvaluationPath pathInfo);

private:
  const DecisionSubtreeRef Subtree;
  const std::vector<DecisionTreeNode> Nodes;

  uint32_t buildFixedBitsTemplate(CGEvaluationPath path) const;
  std::vector<uint8_t> collectVariableBitOffsets(CGEvaluationPath path) const;

  std::list<uint32_t>
  buildVariantsRecursively(uint32_t conditionVector,
                           const std::vector<uint8_t> &variableBitOffsets,
                           uint8_t bitToVaryIdx);
};
