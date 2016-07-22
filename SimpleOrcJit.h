#pragma once

#include <unistd.h>

#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/Orc/CompileUtils.h>
#include <llvm/ExecutionEngine/Orc/IRCompileLayer.h>
#include <llvm/ExecutionEngine/Orc/IRTransformLayer.h>
#include <llvm/ExecutionEngine/Orc/ObjectLinkingLayer.h>
#include <llvm/IR/Mangler.h>
#include <llvm/Support/DynamicLibrary.h>
#include <llvm/Transforms/Scalar.h>

#include <llvm/Transforms/IPO/PassManagerBuilder.h>

#include "OrcJitUtils.h"
#include "SimpleObjectCache.h"

using llvm::orc::ObjectLinkingLayer;
using llvm::orc::IRCompileLayer;
using llvm::orc::IRTransformLayer;

class SimpleOrcJit {
  using ModulePtr_t = std::unique_ptr<llvm::Module>;
  using OptimizeFunction_t = std::function<ModulePtr_t(ModulePtr_t)>;

  using ObjectLayer_t = ObjectLinkingLayer<>;
  using CompileLayer_t = IRCompileLayer<ObjectLayer_t>;
  using OptimizeLayer_t = IRTransformLayer<CompileLayer_t, OptimizeFunction_t>;

  using ModuleHandle_t = OptimizeLayer_t::ModuleSetHandleT;

public:
  SimpleOrcJit(llvm::TargetMachine &targetMachine,
               std::unique_ptr<SimpleObjectCache> cache)
      : ObjectLayer(),
        CompileLayer(ObjectLayer, llvm::orc::SimpleCompiler(targetMachine)),
        OptimizeLayer(
            CompileLayer,
            [this](ModulePtr_t M) { return optimizeModule(std::move(M)); }),
        DataLayout(targetMachine.createDataLayout()),
        ObjCache(std::move(cache)) {
    CompileLayer.setObjectCache(ObjCache.get());
    llvm::sys::DynamicLibrary::LoadLibraryPermanently(nullptr);
  }

  ModuleHandle_t submitModule(ModulePtr_t module) {
    ModuleHandle_t handle = OptimizeLayer.addModuleSet(
        singletonSet(std::move(module)), makeDefaultMemoryManager(),
        makeDefaultLinkingResolver(OptimizeLayer));

    OptimizeLayer.emitAndFinalize(handle);
    return handle;
  }

  ModuleHandle_t loadModuleFromCache(ModulePtr_t module) {
    ModuleHandle_t handle = CompileLayer.addModuleSet(
        singletonSet(std::move(module)), makeDefaultMemoryManager(),
        makeDefaultLinkingResolver(CompileLayer));

    CompileLayer.emitAndFinalize(handle);
    return handle;
  }

  template<typename Evaluator_f>
  Evaluator_f *getFnPtr(std::string unmangledName) {
    auto jitSymbol = CompileLayer.findSymbol(mangle(unmangledName), false);
    auto functionAddr = jitSymbol.getAddress();

    assert(functionAddr != 0);
    return (Evaluator_f *)functionAddr;
  }

private:
  ObjectLayer_t ObjectLayer;
  CompileLayer_t CompileLayer;
  OptimizeLayer_t OptimizeLayer;
  llvm::DataLayout DataLayout;
  std::unique_ptr<SimpleObjectCache> ObjCache;

  ModulePtr_t optimizeModule(ModulePtr_t module) {
    llvm::PassManagerBuilder PMBuilder;
    PMBuilder.BBVectorize = true;
    PMBuilder.SLPVectorize = true;
    PMBuilder.VerifyInput = true;
    PMBuilder.VerifyOutput = true;

    llvm::legacy::FunctionPassManager perFunctionPasses(module.get());
    PMBuilder.populateFunctionPassManager(perFunctionPasses);

    perFunctionPasses.doInitialization();

    for (llvm::Function &function : *module)
      perFunctionPasses.run(function);

    perFunctionPasses.doFinalization();

    llvm::legacy::PassManager perModulePasses;
    PMBuilder.populateModulePassManager(perModulePasses);
    perModulePasses.run(*module);

   #ifndef NDEBUG
    llvm::outs() << "\n\nOptimized the code:\n\n";
    llvm::outs() << *module.get() << "\n\n";
   #endif

    return module;
  }

  std::string mangle(std::string name) {
    std::string mangledName;
    {
      llvm::raw_string_ostream ostream(mangledName);
      llvm::Mangler::getNameWithPrefix(ostream, std::move(name), DataLayout);
    }
    return mangledName;
  }
};
