#pragma once

#include <unistd.h>

#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/Orc/CompileUtils.h>
#include <llvm/ExecutionEngine/Orc/IRCompileLayer.h>
#include <llvm/ExecutionEngine/Orc/IRTransformLayer.h>
#include <llvm/ExecutionEngine/Orc/LambdaResolver.h>
#include <llvm/ExecutionEngine/Orc/ObjectLinkingLayer.h>
#include <llvm/ExecutionEngine/RTDyldMemoryManager.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Mangler.h>
#include <llvm/Support/DynamicLibrary.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Transforms/Scalar.h>

#include "SimpleObjectCache.h"

class SimpleOrcJit {
  using ObjectLayer_t = llvm::orc::ObjectLinkingLayer<>;
  using CompileLayer_t = llvm::orc::IRCompileLayer<ObjectLayer_t>;

  using OptimizeFunction_t = std::function<std::unique_ptr<llvm::Module>(
      std::unique_ptr<llvm::Module>)>;
  using OptimizeLayer_t =
      llvm::orc::IRTransformLayer<CompileLayer_t, OptimizeFunction_t>;

  using ModuleHandle_t = OptimizeLayer_t::ModuleSetHandleT;

public:
  SimpleOrcJit(llvm::TargetMachine &targetMachine,
               std::unique_ptr<SimpleObjectCache> cache)
      : ObjectLayer(),
        CompileLayer(ObjectLayer, llvm::orc::SimpleCompiler(targetMachine)),
        OptimizeLayer(CompileLayer,
                      [this](std::unique_ptr<llvm::Module> M) {
                        return optimizeModule(std::move(M));
                      }),
        DataLayout(targetMachine.createDataLayout()),
        ObjCache(std::move(cache)) {
    CompileLayer.setObjectCache(ObjCache.get());
    llvm::sys::DynamicLibrary::LoadLibraryPermanently(nullptr);
  }

  ModuleHandle_t submitModule(std::unique_ptr<llvm::Module> module) {
    auto lambdaResolver = llvm::orc::createLambdaResolver(
        [&](const std::string &name) {
          if (auto Sym = findMangledSymbol(name))
            return llvm::RuntimeDyld::SymbolInfo(Sym.getAddress(),
                                                 Sym.getFlags());

          return llvm::RuntimeDyld::SymbolInfo(nullptr);
        },
        [](const std::string &S) { return nullptr; });

    llvm::Module *modulePtr = module.get();

    // hand over ownership
    ModuleHandle_t handle = OptimizeLayer.addModuleSet(
        singletonSet(std::move(module)),
        std::make_unique<llvm::SectionMemoryManager>(),
        std::move(lambdaResolver));

    // auto &modvec = ModuleSets[handle];
    // modvec.push_back(modulePtr);

    return handle;
  }

  bool isModuleCached(std::string moduleId) {
    std::string cacheFile;
    if (!ObjCache->getCacheFilename(moduleId, cacheFile))
      return false;

    int FD;
    std::error_code EC = llvm::sys::fs::openFileForRead(cacheFile, FD);
    if (EC)
      return false;

    close(FD);
    return true;
  }

  using Evaluator_f = int64_t(const float *);
  Evaluator_f *getEvaluatorFnPtr(std::string unmangledName) {
    auto jitSymbol = CompileLayer.findSymbol(mangle(unmangledName), false);
    auto functionAddr = jitSymbol.getAddress();

    // printf("Symbol %s resolved to %p\n", unmangledName.c_str(),
    // (void*)functionAddr);
    assert(functionAddr != 0);

    return (Evaluator_f *)functionAddr;
  }

private:
  ObjectLayer_t ObjectLayer;
  CompileLayer_t CompileLayer;
  OptimizeLayer_t OptimizeLayer;
  llvm::DataLayout DataLayout;
  std::unique_ptr<SimpleObjectCache> ObjCache;
  std::map<ModuleHandle_t, std::vector<llvm::Module *>> ModuleSets;

  std::unique_ptr<llvm::Module>
  optimizeModule(std::unique_ptr<llvm::Module> M) {
    auto FPM = llvm::make_unique<llvm::legacy::FunctionPassManager>(M.get());

    FPM->add(llvm::createInstructionCombiningPass());
    FPM->add(llvm::createReassociatePass());
    FPM->add(llvm::createGVNPass());
    FPM->add(llvm::createCFGSimplificationPass());
    FPM->doInitialization();

    // run on all functions in the module
    for (auto &F : *M)
      FPM->run(F);

    return M;
  }

  llvm::orc::JITSymbol findMangledSymbol(const std::string &name) {
    // local symbols first
    if (auto symbol = CompileLayer.findSymbol(name, false))
      return symbol;

    // host process symbols otherwise
    if (auto SymAddr =
            llvm::RTDyldMemoryManager::getSymbolAddressInProcess(name))
      return llvm::orc::JITSymbol(SymAddr, llvm::JITSymbolFlags::Exported);

    return nullptr;
  }

  std::string mangle(std::string name) {
    std::string mangledName;
    {
      llvm::raw_string_ostream ostream(mangledName);
      llvm::Mangler::getNameWithPrefix(ostream, std::move(name), DataLayout);
    }
    return mangledName;
  }

  template <typename T> static std::vector<T> singletonSet(T t) {
    std::vector<T> vec;
    vec.push_back(std::move(t));
    return vec;
  }
};
