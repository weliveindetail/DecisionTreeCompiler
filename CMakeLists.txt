cmake_minimum_required(VERSION 3.4)

project(EvalTreeJit)
enable_testing()

add_subdirectory(3rdparty/benchmark)
add_subdirectory(3rdparty/googletest)
add_subdirectory(3rdparty/json)

find_package(LLVM)

llvm_map_components_to_libnames(LLVM_LIBS
        core
        ipo
        orcjit
        transformutils
        x86asmparser
        x86codegen)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -stdlib=libc++ -fno-rtti")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__STDC_CONSTANT_MACROS -D__STDC_LIMIT_MACROS")

set(SOURCE_FILES
        CompiledResolver.h
        CompiledResolver.cpp
        DataSet.h
        DataSet.cpp
        LegacyDecisionTree.h
        OrcJitUtils.h
        RegularResolver.h
        SimpleOrcJit.h
        SimpleObjectCache.h
        Utils.h
        codegen/CGBase.h
        codegen/CGL1IfThenElse.h
        codegen/CGL2NestedSwitches.h
        codegen/CGL3NestedSwitchesAVX.h
        codegen/CGL3NestedSwitchesAVX.cpp
        codegen/CGConditionVectorEmitter.h
        codegen/CGConditionVectorEmitter.cpp
        codegen/CGConditionVectorVariationsBuilder.h
        codegen/CGConditionVectorVariationsBuilder.cpp
        codegen/CGEvaluationPath.h
        codegen/CGEvaluationPathsBuilder.h
        codegen/CGEvaluationPathsBuilder.cpp
        codegen/CGNodeInfo.h
        resolver/DecisionTreeCompiler.h
        resolver/DecisionTreeCompiler.cpp
        resolver/DecisionTree.h
        resolver/DecisionTree.cpp
        resolver/CompilerSession.h
        resolver/CompilerSession.cpp
        test/TestCGConditionVectorVariationsBuilder.h
        test/TestCGEvaluationPath.h
        test/TestCGEvaluationPathsBuilder.h
        test/TestDecisionTree.h)

# libEvalTreeJit project
add_library(EvalTreeJit ${SOURCE_FILES})
target_include_directories(EvalTreeJit
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${LLVM_INCLUDE_DIRS} 3rdparty/json/src)
target_link_libraries(EvalTreeJit
        PUBLIC ${LLVM_LIBS})

# static driver for libEvalTreeJit
add_executable(EvalTreeJit_Static main_static.cpp driver/StaticDriver.h)
target_include_directories(EvalTreeJit_Static
        PRIVATE EvalTreeJit)
target_link_libraries(EvalTreeJit_Static
        PRIVATE EvalTreeJit)

# benchmarking libEvalTreeJit
add_executable(EvalTreeJit_Benchmark main_benchmark.cpp)
target_include_directories(EvalTreeJit_Benchmark
        PRIVATE EvalTreeJit benchmark)
target_link_libraries(EvalTreeJit_Benchmark
        PRIVATE EvalTreeJit benchmark)

# testing libEvalTreeJit
add_executable(EvalTreeJit_Test main_test.cpp)
target_include_directories(EvalTreeJit_Test
        PRIVATE EvalTreeJit googletest)
target_link_libraries(EvalTreeJit_Test
        PRIVATE EvalTreeJit gtest)
