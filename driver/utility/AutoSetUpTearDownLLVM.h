#pragma once

#include <mutex>
#include <llvm/Target/TargetMachine.h>

struct AutoSetUpTearDownLLVM {
  AutoSetUpTearDownLLVM();
  llvm::TargetMachine *getTargetMachine() const { return TM; }

private:
  struct StaticShutDownHelper {
    ~StaticShutDownHelper();
  };

  llvm::TargetMachine *TM;
  static std::mutex NoRaceInGlobalInit;
  static StaticShutDownHelper StaticShutdown;
};
