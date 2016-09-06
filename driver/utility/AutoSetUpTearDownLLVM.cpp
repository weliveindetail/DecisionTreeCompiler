#include "driver/utility/AutoSetUpTearDownLLVM.h"

#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/Support/ManagedStatic.h>
#include <llvm/Support/TargetSelect.h>

using namespace llvm;

AutoSetUpTearDownLLVM::AutoSetUpTearDownLLVM() {
  std::lock_guard<std::mutex> lock(NoRaceInGlobalInit);

  InitializeNativeTarget();
  InitializeNativeTargetAsmPrinter();
  InitializeNativeTargetAsmParser();

  TM = EngineBuilder().selectTarget();
}

AutoSetUpTearDownLLVM::StaticShutDownHelper::~StaticShutDownHelper() {
  llvm_shutdown();
}

std::mutex AutoSetUpTearDownLLVM::NoRaceInGlobalInit;
AutoSetUpTearDownLLVM::StaticShutDownHelper
    AutoSetUpTearDownLLVM::StaticShutdown;
