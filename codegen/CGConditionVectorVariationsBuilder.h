#pragma once

#include <cstdint>
#include <list>
#include <vector>

#include "resolver/Driver.h"
#include "codegen/CGBase.h"
#include "codegen/CGEvaluationPath.h"

class CGConditionVectorVariationsBuilder {
public:
  CGConditionVectorVariationsBuilder(DecisionSubtreeRef subtreeRef)
      : Subtree(std::move(subtreeRef))
      , NodeIdxs(Subtree.collectNodeIndices()) {}

  std::vector<uint32_t> run(CGEvaluationPath pathInfo);

private:
  const DecisionSubtreeRef Subtree;
  const std::vector<uint64_t> NodeIdxs;

  uint32_t buildFixedBitsTemplate(CGEvaluationPath path);
  std::vector<uint8_t> collectVariableBitOffsets(
      CGEvaluationPath path);

  std::list<uint32_t> buildVariantsRecursively(
      uint32_t conditionVector,
      const std::vector<uint8_t> &variableBitOffsets,
      uint8_t bitToVaryIdx);

  template<class T>
  static std::vector<T> copyListToVector(std::list<T> l) {
    std::vector<T> v;
    v.reserve(l.size());
    std::copy(std::begin(l), std::end(l), std::back_inserter(v));
    return v;
  }
};
