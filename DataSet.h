#pragma once

#include "DecisionTree.h"

using DataSet_t = std::vector<float>;

std::vector<DataSet_t> makeRandomDataSets(size_t dataSets, size_t features, const DecisionTree_t& tree);
