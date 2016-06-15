#include <vector>
#include <cassert>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <sstream>
#include <thread>

#include "CompiledResolver.h"
#include "RegularResolver.h"
#include "Utils.h"

void llvm::ObjectCache::anchor() {}

template <int DataSetFeatures_>
std::vector<float> makeRandomDataSet() {
  std::vector<float> dataSet(DataSetFeatures_);

  for (int i = 0; i < DataSetFeatures_; i++)
    dataSet[i] = makeRandomFloat(); // range [0, 1)

  return dataSet;
};

template <int DataSetFeatures_>
std::tuple<int64_t, std::chrono::nanoseconds>
runBenchmarkEvalRegular(const DecisionTree &tree,
                        const std::vector<float> &dataSet) {
  using namespace std::chrono;
  auto start = high_resolution_clock::now();

  int64_t leafIdx =
      computeLeafNodeIdxForDataSet<DataSetFeatures_>(tree, dataSet);

  auto end = high_resolution_clock::now();
  return std::make_tuple(leafIdx, duration_cast<nanoseconds>(end - start));
};

template <int DataSetFeatures_>
std::tuple<int64_t, std::chrono::nanoseconds>
runBenchmarkEvalCompiled(const DecisionTree &tree,
                         const std::vector<float> &dataSet) {
  using namespace std::chrono;
  auto start = high_resolution_clock::now();

  int64_t leafIdx =
      computeLeafNodeIdxForDataSetCompiled<DataSetFeatures_>(tree, dataSet);

  auto end = high_resolution_clock::now();
  return std::make_tuple(leafIdx, duration_cast<nanoseconds>(end - start));
};

std::string makeTreeFileName(int treeDepth, int dataSetFeatures) {
  std::ostringstream osstr;
  osstr << "cache/_td" << treeDepth << "_dsf" << dataSetFeatures << ".t";

  return osstr.str();
}

std::string makeObjFileName(int treeDepth, int dataSetFeatures,
                            int compiledFunctionDepth) {
  std::ostringstream osstr;
  osstr << "cache/_td" << treeDepth << "_dsf" << dataSetFeatures << "_cfd"
        << compiledFunctionDepth << ".o";

  return osstr.str();
}

template <int TreeDepth_, int DataSetFeatures_>
void runBenchmark(int repetitions, int compiledFunctionDepth) {
  initializeLLVM();

  DecisionTree tree;

  int64_t actualEvaluators;
  int64_t expectedEvaluators =
      getNumCompiledEvaluators<TreeDepth_>(compiledFunctionDepth);

  std::string cachedTreeFile = makeTreeFileName(TreeDepth_, DataSetFeatures_);
  bool isTreeFileCached = isFileInCache(cachedTreeFile);

  if (isTreeFileCached) {
    printf("Loading decision tree with depth %d from file %s\n", TreeDepth_,
           cachedTreeFile.c_str());
    tree = loadDecisionTree(std::move(cachedTreeFile));
  } else {
    printf("Building decision tree with depth %d and cache it in file %s\n",
           TreeDepth_, cachedTreeFile.c_str());
    tree = makeDecisionTree<TreeDepth_, DataSetFeatures_>(
        std::move(cachedTreeFile));
  }

  std::string cachedObjFile =
      makeObjFileName(TreeDepth_, DataSetFeatures_, compiledFunctionDepth);
  bool isObjFileCached = isFileInCache(cachedObjFile);
  setupModule("file:" + cachedObjFile);

  if (isTreeFileCached && isObjFileCached) {
    printf("Loading %lld evaluators for %lu nodes from file %s",
           expectedEvaluators, tree.size(), cachedObjFile.c_str());
    actualEvaluators = loadEvaluators<TreeDepth_>(tree, compiledFunctionDepth);
  } else {
    printf("Generating %lld evaluators for %lu nodes and cache it in file %s",
           expectedEvaluators, tree.size(), cachedObjFile.c_str());
    actualEvaluators =
        compileEvaluators<TreeDepth_>(tree, compiledFunctionDepth);
  }

  assert(expectedEvaluators == actualEvaluators);

  {
    printf("\n\nBenchmarking: %d runs with %d features\n", repetitions,
           DataSetFeatures_);

    int64_t resultRegular;
    int64_t resultCompiled;

    std::chrono::nanoseconds runtimeRegular;
    std::chrono::nanoseconds runtimeCompiled;

    float totalRuntimeRegular = 0;
    float totalRuntimeCompiled = 0;

    for (int i = 0; i < repetitions; i++) {
      auto dataSet = makeRandomDataSet<DataSetFeatures_>();

      std::tie(resultRegular, runtimeRegular) =
          runBenchmarkEvalRegular<DataSetFeatures_>(tree, dataSet);

      std::tie(resultCompiled, runtimeCompiled) =
          runBenchmarkEvalCompiled<DataSetFeatures_>(tree, dataSet);

      assert(resultRegular == resultCompiled);

      totalRuntimeRegular += runtimeRegular.count();
      totalRuntimeCompiled += runtimeCompiled.count();
    }

    float averageRuntimeRegular = totalRuntimeRegular / repetitions;
    printf("Average evaluation time regular: %fns\n", averageRuntimeRegular);

    float averageRuntimeCompiled = totalRuntimeCompiled / repetitions;
    printf("Average evaluation time compiled: %fns\n", averageRuntimeCompiled);
  }

  shutdownLLVM();
}

int main() {
  int repetitions = 1000;
  int compiledFunctionDepth = 10;
  constexpr int treeDepth = 10; // depth 25 ~ 1GB data in memory
  constexpr int dataSetFeatures = 100;

  runBenchmark<treeDepth, dataSetFeatures>(repetitions, compiledFunctionDepth);

  return 0;
}
