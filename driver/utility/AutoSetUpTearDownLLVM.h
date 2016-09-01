#pragma once

#include <atomic>
#include <llvm/Target/TargetMachine.h>

struct AutoSetUpTearDownLLVM {
  AutoSetUpTearDownLLVM();
  ~AutoSetUpTearDownLLVM();

  llvm::TargetMachine *getTargetMachine() {
    assert(targetMachine != nullptr);
    return targetMachine;
  }

private:
  static std::atomic<int> instances;
  static llvm::TargetMachine *targetMachine;
};
