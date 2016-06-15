#pragma once

#include <random>

constexpr int64_t PowerOf2(const int exp) {
  return (int64_t)1 << exp;
}

constexpr int64_t TreeNodes(const int depth) {
  return PowerOf2(depth) - 1;
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
