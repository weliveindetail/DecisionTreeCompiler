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
        Utils.h
        codegen/CodeGenerator.h
        codegen/CodeGeneratorSelector.h
        codegen/CodeGeneratorSelector.cpp
        codegen/L1IfThenElse.h
        codegen/L1IfThenElse.cpp
        codegen/L3SubtreeSwitchAVX.h
        codegen/L3SubtreeSwitchAVX.cpp
        codegen/LXSubtreeSwitch.h
        codegen/LXSubtreeSwitch.cpp
        codegen/utility/CGConditionVectorEmitter.h
        codegen/utility/CGConditionVectorEmitter.cpp
        codegen/utility/CGConditionVectorVariationsBuilder.h
        codegen/utility/CGConditionVectorVariationsBuilder.cpp
        codegen/utility/CGEvaluationPath.h
        codegen/utility/CGEvaluationPathsBuilder.h
        codegen/utility/CGEvaluationPathsBuilder.cpp
        codegen/utility/CGNodeInfo.h
        compiler/CompilerSession.h
        compiler/CompilerSession.cpp
        compiler/DecisionTreeCompiler.h
        compiler/DecisionTreeCompiler.cpp
        compiler/SimpleOrcJit.h
        compiler/SimpleOrcJit.cpp
        data/DataSetFactory.h
        data/DecisionSubtreeRef.h
        data/DecisionTree.h
        data/DecisionTree.cpp
        data/DecisionTreeNode.h
        data/DecisionTreeNode.cpp
        driver/JitDriver.h
        driver/utility/AutoSetUpTearDownLLVM.h
        driver/utility/AutoSetUpTearDownLLVM.cpp
        driver/utility/Interpreter.h)

# libEvalTreeJit
add_library(EvalTreeJit ${SOURCE_FILES})
target_include_directories(EvalTreeJit            PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
                                                         ${LLVM_INCLUDE_DIRS}
                                                         3rdparty/json/src)

target_link_libraries     (EvalTreeJit            PUBLIC ${LLVM_LIBS})

# static driver
add_executable(EvalTreeJit_Static main_static.cpp driver/StaticDriver.h)
target_include_directories(EvalTreeJit_Static     PRIVATE EvalTreeJit)
target_link_libraries     (EvalTreeJit_Static     PRIVATE EvalTreeJit)

# benchmark
add_executable(EvalTreeJit_Benchmark main_benchmark.cpp)
target_include_directories(EvalTreeJit_Benchmark  PRIVATE EvalTreeJit benchmark)
target_link_libraries     (EvalTreeJit_Benchmark  PRIVATE EvalTreeJit benchmark)

# unit tests
set(TEST_FILES
    test/TestCGConditionVectorVariationsBuilder.h
    test/TestCGEvaluationPath.h
    test/TestCGEvaluationPathsBuilder.h
    test/TestSingleCodegenL1.h
    test/TestSingleCodegenL2.h
    test/TestSingleCodegenL3.h
    test/TestSingleCodegenL4.h
    test/TestSingleCodegenL6.h
    test/TestDecisionTree.h)

add_executable(EvalTreeJit_Test main_test.cpp ${TEST_FILES})
target_include_directories(EvalTreeJit_Test       PRIVATE EvalTreeJit googletest)
target_link_libraries     (EvalTreeJit_Test       PRIVATE EvalTreeJit gtest)
