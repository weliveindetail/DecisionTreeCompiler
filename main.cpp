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

    printf("Building decision tree with depth %lu..\n", TreeDepth_);
    auto tree = makeDecisionTree<TreeDepth_, DataSetFeatures_>();

    printf("Benchmarking: %d runs with %lu features..\n",
           repetitions, DataSetFeatures_);

    float totalRuntime = 0;

    for (int i = 0; i < repetitions; i++) {
        auto dataSet = makeRandomDataSet<DataSetFeatures_>();
        auto start = high_resolution_clock::now();

        unsigned long leafIdx =
            computeLeafNodeIdxForDataSet<TreeDepth_, DataSetFeatures_>(
                    tree, dataSet);

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
        int repetitions = 1000;
        constexpr auto treeDepth = 25; // ~1GB
        constexpr auto dataSetFeatures = 100;
        runBenchmark<treeDepth, dataSetFeatures>(repetitions);
    }

    return 0;
}
