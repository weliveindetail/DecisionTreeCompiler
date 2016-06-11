#include <array>
#include <chrono>
#include <cstdio>
#include <cassert>
#include <thread>

#include "Utils.h"
#include "RegularResolver.h"

template<unsigned long DataSetFeatures_>
std::array<float, DataSetFeatures_> makeRandomDataSet() {
    std::array<float, DataSetFeatures_> dataSet;

    for (int i = 0; i < DataSetFeatures_; i++)
        dataSet[i] = makeRandomFloat(); // range [0, 1)

    return dataSet;
};

template<unsigned long TreeDepth_, unsigned long DataSetFeatures_>
void runBenchmark(int repetitions) {
    using namespace std::chrono;

    printf("Benchmarking: %d runs with tree depth %lu and %lu features..\n",
           repetitions, TreeDepth_, DataSetFeatures_);

    float totalRuntime = 0;
    auto tree = makeDecisionTree<TreeDepth_, DataSetFeatures_>();

    for (int i = 0; i < repetitions; i++) {
        auto dataSet = makeRandomDataSet<DataSetFeatures_>();
        auto start = high_resolution_clock::now();

        int leafIdx = computeLeafNodeIdxForDataSet<TreeDepth_, DataSetFeatures_>(tree, dataSet);
        (void)leafIdx; // unused

        auto end = high_resolution_clock::now();
        auto dur = duration_cast<nanoseconds>(end - start);

        totalRuntime += dur.count();
        //std::this_thread::sleep_for(nanoseconds(1000));
    }

    float averageRuntime = totalRuntime / repetitions;
    printf("Average evaluation time: %fns\n\n", averageRuntime);
}

int main() {
    {
        int repetitions = 100;
        constexpr auto treeDepth = 10;
        constexpr auto dataSetFeatures = 10;
        runBenchmark<treeDepth, dataSetFeatures>(repetitions);
    }
    {
        int repetitions = 100;
        constexpr auto treeDepth = 18; // maximum for array-based approach
        constexpr auto dataSetFeatures = 10;
        runBenchmark<treeDepth, dataSetFeatures>(repetitions);
    }
    {
        int repetitions = 10;
        constexpr auto treeDepth = 18;
        constexpr auto dataSetFeatures = 1000;
        runBenchmark<treeDepth, dataSetFeatures>(repetitions);
    }
    {
        int repetitions = 1000;
        constexpr auto treeDepth = 18;
        constexpr auto dataSetFeatures = 1000;
        runBenchmark<treeDepth, dataSetFeatures>(repetitions);
    }
    {
        int repetitions = 100000; // performance loss due to weird cache effects!?
        constexpr auto treeDepth = 18;
        constexpr auto dataSetFeatures = 1000;
        runBenchmark<treeDepth, dataSetFeatures>(repetitions);
    }

    return 0;
}
