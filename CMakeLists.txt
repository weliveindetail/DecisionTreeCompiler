cmake_minimum_required(VERSION 3.4)
project(EvalTreeJit)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

set(SOURCE_FILES main.cpp Utils.h DecisionTree.h RegularResolver.h)
add_executable(EvalTreeJit ${SOURCE_FILES})