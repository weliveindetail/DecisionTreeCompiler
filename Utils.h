#pragma once

#include <random>

constexpr bool isPowerOf2(const int n) {
  return n == 1 || (n & (n-1)) == 0;
}

constexpr int64_t PowerOf2(const int exp) {
  return (int64_t)1 << exp;
}

constexpr int64_t TreeNodes(const int depth) {
  return PowerOf2(depth) - 1;
}

constexpr int Log2 (uint64_t value)
{
  constexpr int tab64[64] = {
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

float makeRandomFloat() {
  static std::random_device rd;
  static std::default_random_engine engine(rd());
  static std::uniform_real_distribution<float> dist(0, 1);
  return dist(engine);
}

int makeRandomInt(int min, int max) {
  static std::random_device rd;
  static std::default_random_engine engine(rd());
  std::uniform_int_distribution<int> dist(min, max);
  return dist(engine);
}

bool isFileInCache(std::string fileName) {
  int FD;
  std::error_code EC = llvm::sys::fs::openFileForRead(fileName, FD);
  if (EC)
    return false;

  close(FD);
  return true;
}
