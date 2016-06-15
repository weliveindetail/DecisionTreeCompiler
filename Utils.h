#pragma once

#include <random>

constexpr int64_t TreeSize(const int depth) {
  return ((int64_t)1 << depth) - 1;
}

float makeRandomFloat() {
  static std::random_device rd;
  static std::default_random_engine engine(rd());
  static std::uniform_real_distribution<float> dist(0, 1);
  return dist(engine);
}

template <int Min_, int Max_> int makeRandomInt() {
  static std::random_device rd;
  static std::default_random_engine engine(rd());
  static std::uniform_int_distribution<int> dist(Min_, Max_);
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
