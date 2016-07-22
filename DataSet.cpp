#include "DataSet.h"
#include "RegularResolver.h"
#include "Utils.h"

std::vector<DataSet_t> makeRandomDataSets(uint32_t dataSets, uint32_t features, const DecisionTree_t& tree) {
  std::vector<DataSet_t> dataSetCollection(dataSets);
  RegularResolver resolver;

  auto makeRandomDataSet = [&]() {
      DataSet_t dataSet(features + 1);
      std::generate(dataSet.begin(), dataSet.end() - 1, makeRandomFloat);
      dataSet.back() = resolver.run(tree, dataSet); // extra slot for expected result index
      return dataSet;
  };

  std::generate(dataSetCollection.begin(), dataSetCollection.end(), makeRandomDataSet);
  return dataSetCollection;
}
