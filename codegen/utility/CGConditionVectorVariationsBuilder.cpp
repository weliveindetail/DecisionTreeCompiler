#include "CGConditionVectorVariationsBuilder.h"

std::vector<uint32_t>
CGConditionVectorVariationsBuilder::run(CGEvaluationPath path) {
  std::vector<uint8_t> variableBitOffsets = collectVariableBitOffsets(path);
  uint32_t fixedBitsTemplate = buildFixedBitsTemplate(path);

  if (variableBitOffsets.empty())
    return {fixedBitsTemplate};

  std::list<uint32_t> variants = buildVariantsRecursively(
      fixedBitsTemplate, variableBitOffsets, 0);

  assert(variants.size() == PowerOf2(variableBitOffsets.size()));
  return copyListToVector(std::move(variants));
}

uint32_t CGConditionVectorVariationsBuilder::buildFixedBitsTemplate(
    CGEvaluationPath path) {
  uint32_t fixedBits = 0;

  for (uint8_t bitOffset = 0; bitOffset < NodeIdxs.size(); bitOffset++) {
    uint64_t idx = NodeIdxs[bitOffset];
    if (auto node = path.findNode(idx)) {
      uint32_t bit = node->getEvaluationValue();
      uint32_t vectorBit = bit << bitOffset;
      fixedBits |= vectorBit;
    }
  }

  return fixedBits;
}

std::vector<uint8_t>
CGConditionVectorVariationsBuilder::collectVariableBitOffsets(
    CGEvaluationPath path) {
  std::vector<uint8_t> variableBitOffsets;

  for (uint8_t bitOffset = 0; bitOffset < NodeIdxs.size(); bitOffset++) {
    if (!path.hasNodeIdx(NodeIdxs[bitOffset])) {
      variableBitOffsets.push_back(bitOffset);
    }
  }

  return variableBitOffsets;
}

std::list<uint32_t> CGConditionVectorVariationsBuilder::buildVariantsRecursively(
    uint32_t conditionVector, const std::vector<uint8_t> &variableBitOffsets,
    uint8_t bitToVaryIdx) {
  if (bitToVaryIdx == variableBitOffsets.size())
    return {conditionVector};

  uint8_t bitToVaryOffset = variableBitOffsets.at(bitToVaryIdx);
  uint32_t vectorTrueBit = 1u << bitToVaryOffset;

  // bit must still be in default zero state
  assert((conditionVector & ~vectorTrueBit) == conditionVector);

  // false and true variations
  std::list<uint32_t> variations;

  variations.splice(variations.end(), buildVariantsRecursively(
      conditionVector, variableBitOffsets, bitToVaryIdx + 1));

  variations.splice(variations.end(), buildVariantsRecursively(
      conditionVector | vectorTrueBit, variableBitOffsets, bitToVaryIdx + 1));

  return variations;
}
