#pragma once

#include <random>

constexpr unsigned long TreeSize(const unsigned long depth) {
  return (1 << depth) - 1;
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
