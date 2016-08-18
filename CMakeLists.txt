cmake_minimum_required(VERSION 3.4)
project(EvalTreeJit)

add_subdirectory(benchmark)

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
        main.cpp
        CompiledResolver.h
        CompiledResolver.cpp
        DataSet.h
        DataSet.cpp
        DecisionTree.h
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
        codegen/CGEvaluationPathsBuilder.h
        codegen/CGEvaluationPathsBuilder.cpp
        resolver/Driver.h
        resolver/Driver.cpp)

add_executable(EvalTreeJit ${SOURCE_FILES})

target_include_directories(EvalTreeJit
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${LLVM_INCLUDE_DIRS} benchmark)

target_link_libraries(EvalTreeJit
        PUBLIC ${LLVM_LIBS} benchmark)
