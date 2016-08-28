#include "codegen/utility/CGConditionVectorVariationsBuilder.h"
#include "Utils.h"

std::vector<uint32_t>
CGConditionVectorVariationsBuilder::run(CGEvaluationPath path) {
  std::vector<uint8_t> variableBitOffsets = collectVariableBitOffsets(path);
  uint32_t fixedBitsTemplate = buildFixedBitsTemplate(path);

  if (variableBitOffsets.empty())
    return {fixedBitsTemplate};

  std::list<uint32_t> variants =
      buildVariantsRecursively(fixedBitsTemplate, variableBitOffsets, 0);

  assert(variants.size() == PowerOf2(variableBitOffsets.size()));
  return copyListToVector(std::move(variants));
}

uint32_t CGConditionVectorVariationsBuilder::buildFixedBitsTemplate(
    CGEvaluationPath path) const {
  uint32_t fixedBits = 0;

  for (uint8_t bitOffset = 0; bitOffset < Nodes.size(); bitOffset++) {
    if (auto step = path.findStepFromNode(Nodes.at(bitOffset))) {
      uint32_t bit = step->getSrcNodeEvalValue();
      uint32_t vectorBit = bit << bitOffset;
      fixedBits |= vectorBit;
    }
  }

  return fixedBits;
}

std::vector<uint8_t>
CGConditionVectorVariationsBuilder::collectVariableBitOffsets(
    CGEvaluationPath path) const {
  std::vector<uint8_t> variableBitOffsets;

  for (uint8_t bitOffset = 0; bitOffset < Nodes.size(); bitOffset++) {
    if (!path.hasNode(Nodes.at(bitOffset))) {
      variableBitOffsets.push_back(bitOffset);
    }
  }

  return variableBitOffsets;
}

std::list<uint32_t>
CGConditionVectorVariationsBuilder::buildVariantsRecursively(
    uint32_t conditionVector, const std::vector<uint8_t> &variableBitOffsets,
    uint8_t bitToVaryIdx) {
  // if there is no more bit to vary, return assembled condition vector
  if (bitToVaryIdx == variableBitOffsets.size())
    return {conditionVector};

  uint8_t bitToVaryOffset = variableBitOffsets.at(bitToVaryIdx);
  assert(bitToVaryOffset < sizeof(uint32_t) * 8);

  uint32_t vectorTrueBit = 1u << bitToVaryOffset;
  assert((conditionVector & ~vectorTrueBit) == conditionVector);

  std::list<uint32_t> falseVariations = buildVariantsRecursively(
      conditionVector, variableBitOffsets, bitToVaryIdx + 1);

  std::list<uint32_t> trueVariations = buildVariantsRecursively(
      conditionVector | vectorTrueBit, variableBitOffsets, bitToVaryIdx + 1);

  return concatLists(std::move(falseVariations), std::move(trueVariations));
}
