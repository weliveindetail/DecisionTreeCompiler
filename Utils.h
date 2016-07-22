#pragma once

#include <random>
#include <sstream>
#include <string>

#include <unistd.h>

#include <llvm/Support/FileSystem.h>
#include <llvm/Support/raw_ostream.h>

template<typename T>
constexpr bool isPowerOf2(const T n) {
  return n == 1 || (n & (n-1)) == 0;
}

template<typename Res_t=uint64_t, typename Exp_t>
constexpr Res_t PowerOf2(const Exp_t exp) {
  assert(exp >= 0 && exp < sizeof(Res_t) * 8);
  return (Res_t)1 << exp;
}

template<typename Res_t=uint64_t>
constexpr Res_t TreeNodes(const int depth) {
  return PowerOf2(depth) - 1;
}

constexpr uint8_t Log2 (uint64_t value)
{
  constexpr uint8_t tab64[64] = {
      63,  0, 58,  1, 59, 47, 53,  2,
      60, 39, 48, 27, 54, 33, 42,  3,
      61, 51, 37, 40, 49, 18, 28, 20,
      55, 30, 34, 11, 43, 14, 22,  4,
      62, 57, 46, 52, 38, 26, 32, 41,
      50, 36, 17, 19, 29, 10, 13, 21,
      56, 45, 25, 31, 35, 16,  9, 12,
      44, 24, 15,  8, 23,  7,  6,  5
  };

  value |= value >> 1;
  value |= value >> 2;
  value |= value >> 4;
  value |= value >> 8;
  value |= value >> 16;
  value |= value >> 32;

  uint64_t x = (value - (value >> 1)) * 0x07EDD5E59A4E28C2;
  return tab64[x >> 58];
}

static bool isFileInCache(std::string fileName) {
  int FD;
  std::error_code EC = llvm::sys::fs::openFileForRead(fileName, FD);
  if (EC)
    return false;

  close(FD);
  return true;
}

static std::string makeTreeFileName(int treeDepth, int dataSetFeatures) {
  std::ostringstream osstr;

  osstr << "cache/";
  osstr << "_td" << treeDepth;
  osstr << "_dsf" << dataSetFeatures;
  osstr << ".t";

  return osstr.str();
}

static std::string makeObjFileName(int treeDepth, int dataSetFeatures,
                                   int compiledFunctionDepth,
                                   int compiledFunctionSwitchDepth) {
  std::ostringstream osstr;

  osstr << "cache/";
  osstr << "_td" << treeDepth;
  osstr << "_dsf" << dataSetFeatures;
  osstr << "_cfd" << compiledFunctionDepth;
  osstr << "_cfsd" << compiledFunctionSwitchDepth;
  osstr << ".o";

  return osstr.str();
}

static float makeRandomFloat() {
  static std::random_device rd;
  static std::default_random_engine engine(rd());
  static std::uniform_real_distribution<float> dist(0, 1);
  return dist(engine);
}

template<typename T>
static T makeRandomInt(T min, T max) {
  static std::random_device rd;
  static std::default_random_engine engine(rd());
  std::uniform_int_distribution<T> dist(min, max);
  return dist(engine);
}
