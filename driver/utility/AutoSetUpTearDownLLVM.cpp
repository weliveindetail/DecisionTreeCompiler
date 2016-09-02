#include "driver/utility/AutoSetUpTearDownLLVM.h"

#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/Support/ManagedStatic.h>
#include <llvm/Support/TargetSelect.h>

using namespace llvm;

// static init
std::atomic_flag AutoSetUpTearDownLLVM::initialized = ATOMIC_FLAG_INIT;
TargetMachine *AutoSetUpTearDownLLVM::targetMachine = nullptr;

AutoSetUpTearDownLLVM::StaticShutDownHelper
    AutoSetUpTearDownLLVM::StaticShutdown;

AutoSetUpTearDownLLVM::AutoSetUpTearDownLLVM() {
  if (!initialized.test_and_set(std::memory_order_acquire)) {
    InitializeNativeTarget();
    InitializeNativeTargetAsmPrinter();
    InitializeNativeTargetAsmParser();

    assert(targetMachine == nullptr);
    targetMachine = EngineBuilder().selectTarget();
  }
}

AutoSetUpTearDownLLVM::StaticShutDownHelper::~StaticShutDownHelper() {
  if (initialized.test_and_set(std::memory_order_release)) {
    llvm_shutdown();
  }

  initialized.clear(std::memory_order_release); // for completeness
}
