#include <cassert>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <sstream>
#include <thread>
#include <vector>

#include "CompiledResolver.h"
#include "RegularResolver.h"
#include "Utils.h"

void llvm::ObjectCache::anchor() {}

std::vector<float> makeRandomDataSet(int features) {
  std::vector<float> dataSet((size_t)features);

  for (int i = 0; i < features; i++)
    dataSet[i] = makeRandomFloat(); // range [0, 1)

  return dataSet;
};

std::tuple<int64_t, std::chrono::nanoseconds>
runBenchmarkEvalRegular(const DecisionTree &tree,
                        const std::vector<float> &dataSet) {
  using namespace std::chrono;
  auto start = high_resolution_clock::now();

  int64_t leafIdx = computeLeafNodeIdxForDataSet(tree, dataSet);

  auto end = high_resolution_clock::now();
  return std::make_tuple(leafIdx, duration_cast<nanoseconds>(end - start));
};

std::tuple<int64_t, std::chrono::nanoseconds>
runBenchmarkEvalCompiled(const DecisionTree &tree,
                         const std::vector<float> &dataSet) {
  using namespace std::chrono;
  auto start = high_resolution_clock::now();

  int64_t leafIdx = computeLeafNodeIdxForDataSetCompiled(tree, dataSet);

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

void runBenchmark(int repetitions, int treeDepth, int dataSetFeatures,
                  int compiledFunctionDepth, int compiledFunctionSwitchDepth) {
  initializeLLVM();

  DecisionTree tree;

  int64_t actualEvaluators;
  int64_t expectedEvaluators =
      getNumCompiledEvaluators(treeDepth, compiledFunctionDepth);

  std::string cachedTreeFile = makeTreeFileName(treeDepth, dataSetFeatures);
  bool isTreeFileCached = isFileInCache(cachedTreeFile);

  if (isTreeFileCached) {
    printf("Loading decision tree with depth %d from file %s\n", treeDepth,
           cachedTreeFile.c_str());
    tree = loadDecisionTree(treeDepth, std::move(cachedTreeFile));
  } else {
    printf("Building decision tree with depth %d and cache it in file %s\n",
           treeDepth, cachedTreeFile.c_str());
    tree =
        makeDecisionTree(treeDepth, dataSetFeatures, std::move(cachedTreeFile));
  }

  std::string cachedObjFile =
      makeObjFileName(treeDepth, dataSetFeatures, compiledFunctionDepth);
  bool isObjFileCached = isFileInCache(cachedObjFile);
  setupModule("file:" + cachedObjFile);

  if (isTreeFileCached && isObjFileCached) {
    printf("Loading %lld evaluators for %lu nodes from file %s",
           expectedEvaluators, tree.size(), cachedObjFile.c_str());
    actualEvaluators = loadEvaluators(tree, treeDepth, compiledFunctionDepth);
  } else {
    printf("Generating %lld evaluators for %lu nodes and cache it in file %s",
           expectedEvaluators, tree.size(), cachedObjFile.c_str());
    actualEvaluators = compileEvaluators(tree, treeDepth, compiledFunctionDepth,
                                         compiledFunctionSwitchDepth);
  }

  assert(expectedEvaluators == actualEvaluators);

  {
    printf("\n\nBenchmarking: %d runs with %d features\n", repetitions,
           dataSetFeatures);

    int64_t resultRegular;
    int64_t resultCompiled;

    std::chrono::nanoseconds runtimeRegular;
    std::chrono::nanoseconds runtimeCompiled;

    float totalRuntimeRegular = 0;
    float totalRuntimeCompiled = 0;

    for (int i = 0; i < repetitions; i++) {
      auto dataSet = makeRandomDataSet(dataSetFeatures);

      std::tie(resultRegular, runtimeRegular) =
          runBenchmarkEvalRegular(tree, dataSet);

      std::tie(resultCompiled, runtimeCompiled) =
          runBenchmarkEvalCompiled(tree, dataSet);

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
  int dataSetFeatures = 100;

  int treeDepth = 8; // depth 25 ~ 1GB data in memory
  int compiledFunctionDepth = 4;
  int compiledFunctionSwitchDepth = 2;

  runBenchmark(repetitions, treeDepth, dataSetFeatures, compiledFunctionDepth,
               compiledFunctionSwitchDepth);

  return 0;
}
