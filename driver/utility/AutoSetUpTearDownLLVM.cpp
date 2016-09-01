#include "driver/utility/AutoSetUpTearDownLLVM.h"

#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/Support/ManagedStatic.h>
#include <llvm/Support/TargetSelect.h>

using namespace llvm;

AutoSetUpTearDownLLVM::AutoSetUpTearDownLLVM() {
  int existingInstancesBeforeConstruction = instances.fetch_add(1);
  if (existingInstancesBeforeConstruction == 0) {
    InitializeNativeTarget();
    InitializeNativeTargetAsmPrinter();
    InitializeNativeTargetAsmParser();

    assert(targetMachine == nullptr);
    targetMachine = EngineBuilder().selectTarget();
  }
}

AutoSetUpTearDownLLVM::~AutoSetUpTearDownLLVM() {
  int existingInstancesBeforeDestruction = instances.fetch_sub(1);
  if (existingInstancesBeforeDestruction == 1) {
    llvm_shutdown();
  }
}

// static init
std::atomic<int> AutoSetUpTearDownLLVM::instances{0};
TargetMachine *AutoSetUpTearDownLLVM::targetMachine{nullptr};
