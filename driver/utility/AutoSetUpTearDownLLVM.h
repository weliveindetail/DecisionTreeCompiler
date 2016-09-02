#pragma once

#include <atomic>
#include <llvm/Target/TargetMachine.h>

struct AutoSetUpTearDownLLVM {
  AutoSetUpTearDownLLVM();

  llvm::TargetMachine *getTargetMachine() {
    assert(targetMachine != nullptr);
    return targetMachine;
  }

private:
  struct StaticShutDownHelper {
    ~StaticShutDownHelper();
  };

  static std::atomic_flag initialized;
  static StaticShutDownHelper StaticShutdown;

  static llvm::TargetMachine *targetMachine;
};
