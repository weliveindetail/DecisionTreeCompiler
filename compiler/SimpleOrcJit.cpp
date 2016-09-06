#include "compiler/SimpleOrcJit.h"

#include <llvm/ExecutionEngine/RuntimeDyld.h>
#include <llvm/ExecutionEngine/RTDyldMemoryManager.h>
#include <llvm/ExecutionEngine/SectionMemoryManager.h>
#include <llvm/ExecutionEngine/Orc/CompileUtils.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Mangler.h>
#include <llvm/Support/DynamicLibrary.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/Transforms/IPO/PassManagerBuilder.h>

using namespace llvm;

SimpleOrcJit::SimpleOrcJit(TargetMachine *targetMachine)
  : ObjectLayer(),
    CompileLayer(ObjectLayer, orc::SimpleCompiler(*targetMachine)),
    OptimizeLayer(
        CompileLayer,
        [this](ModulePtr_t M) { return optimizeModule(std::move(M)); }),
    TargetDataLayout(targetMachine->createDataLayout()) {
  sys::DynamicLibrary::LoadLibraryPermanently(nullptr);
}

orc::TargetAddress SimpleOrcJit::getFnAddress(std::string unmangledName) {
  auto jitSymbol = CompileLayer.findSymbol(mangle(unmangledName), false);
  assert(jitSymbol.getAddress() != 0);
  return jitSymbol.getAddress();
}

orc::TargetAddress SimpleOrcJit::getFnAddressIn(ModuleHandle_t module,
                                                std::string unmangledName) {
  auto jitSymbol = CompileLayer.findSymbolIn(module, mangle(unmangledName), false);
  assert(jitSymbol.getAddress() != 0);
  return jitSymbol.getAddress();
}

auto SimpleOrcJit::mangle(std::string name) -> std::string {
  std::string mangledName;
  {
    raw_string_ostream ostream(mangledName);
    Mangler::getNameWithPrefix(ostream, std::move(name), TargetDataLayout);
  }
  return mangledName;
}

auto SimpleOrcJit::optimizeModule(ModulePtr_t module) -> ModulePtr_t {
  PassManagerBuilder PMBuilder;
  PMBuilder.BBVectorize = true;
  PMBuilder.SLPVectorize = true;
  PMBuilder.VerifyInput = true;
  PMBuilder.VerifyOutput = true;

  legacy::FunctionPassManager perFunctionPasses(module.get());
  PMBuilder.populateFunctionPassManager(perFunctionPasses);

  /*
  llvm::outs() << "\nModule raw IR code:\n";
  llvm::outs() << *module.get();
  //*/

  perFunctionPasses.doInitialization();

  for (Function &function : *module)
    perFunctionPasses.run(function);

  perFunctionPasses.doFinalization();

  legacy::PassManager perModulePasses;
  PMBuilder.populateModulePassManager(perModulePasses);
  perModulePasses.run(*module);

  /*
  llvm::outs() << "\nModule optimized IR code:\n";
  llvm::outs() << *module.get();
  //*/

  return module;
}

namespace {
  template <class OrcLayer_t>
  class DefaultLinkingResolver : public RuntimeDyld::SymbolResolver {
  public:
    DefaultLinkingResolver(OrcLayer_t &orcLayer) : OrcLayer(orcLayer) {}

    RuntimeDyld::SymbolInfo findSymbol(const std::string &name) override {
      if (auto Sym = findMangledSymbol(name))
        return RuntimeDyld::SymbolInfo(Sym.getAddress(), Sym.getFlags());
      else
        return RuntimeDyld::SymbolInfo(nullptr);
    }

    RuntimeDyld::SymbolInfo
    findSymbolInLogicalDylib(const std::string &Name) override {
      assert(false);
      return RuntimeDyld::SymbolInfo(nullptr);
    }

  private:
    OrcLayer_t &OrcLayer;

    orc::JITSymbol findMangledSymbol(const std::string &name) {
      // local symbols first
      if (auto symbol = OrcLayer.findSymbol(name, false))
        return symbol;

      // host process symbols otherwise
      if (auto addr = RTDyldMemoryManager::getSymbolAddressInProcess(name))
        return orc::JITSymbol(addr, JITSymbolFlags::Exported);

      return nullptr;
    }
  };
}

template <class OrcLayer_t>
auto makeLinkingResolver(OrcLayer_t &orcLayer) {
  return std::make_unique<DefaultLinkingResolver<OrcLayer_t>>(orcLayer);
}

auto makeMemoryManager() {
  return std::make_unique<SectionMemoryManager>();
}

template <class T> auto makeModuleSet(T t) {
  std::vector<T> vec;
  vec.push_back(std::move(t));
  return vec;
}

auto SimpleOrcJit::submitModule(ModulePtr_t module) -> ModuleHandle_t {
  ModuleHandle_t handle = OptimizeLayer.addModuleSet(
      makeModuleSet(std::move(module)), makeMemoryManager(),
      makeLinkingResolver(OptimizeLayer));

  OptimizeLayer.emitAndFinalize(handle);
  return handle;
}
