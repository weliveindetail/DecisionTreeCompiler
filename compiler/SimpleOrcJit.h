#pragma once

#include <functional>
#include <memory>
#include <string>

#include <llvm/ExecutionEngine/Orc/IRCompileLayer.h>
#include <llvm/ExecutionEngine/Orc/IRTransformLayer.h>
#include <llvm/ExecutionEngine/Orc/JITSymbol.h>
#include <llvm/ExecutionEngine/Orc/ObjectLinkingLayer.h>
#include <llvm/IR/DataLayout.h>
#include <llvm/IR/Module.h>
#include <llvm/Target/TargetMachine.h>

using llvm::orc::IRCompileLayer;
using llvm::orc::IRTransformLayer;
using llvm::orc::ObjectLinkingLayer;

class SimpleOrcJit {
  using ModulePtr_t = std::unique_ptr<llvm::Module>;
  using Optimize_f = std::function<ModulePtr_t(ModulePtr_t)>;

  using ObjectLayer_t = ObjectLinkingLayer<>;
  using CompileLayer_t = IRCompileLayer<ObjectLayer_t>;
  using OptimizeLayer_t = IRTransformLayer<CompileLayer_t, Optimize_f>;

  using ModuleHandle_t = OptimizeLayer_t::ModuleSetHandleT;

public:
  SimpleOrcJit(llvm::TargetMachine *targetMachine);
  ModuleHandle_t submitModule(ModulePtr_t module);

  template<typename Evaluator_f>
  Evaluator_f *getFnPtr(std::string unmangledName) {
    return (Evaluator_f*)getFnAddress(std::move(unmangledName));
  }

private:
  ObjectLayer_t ObjectLayer;
  CompileLayer_t CompileLayer;
  OptimizeLayer_t OptimizeLayer;
  llvm::DataLayout TargetDataLayout;

  ModulePtr_t optimizeModule(ModulePtr_t module);
  llvm::orc::TargetAddress getFnAddress(std::string unmangledName);
  std::string mangle(std::string name);
};
