#pragma once

#include "LegacyDecisionTree.h"

using DataSet_t = std::vector<float>;

std::vector<DataSet_t> makeRandomDataSets(uint32_t dataSets, uint32_t features, const DecisionTree_t& tree);
