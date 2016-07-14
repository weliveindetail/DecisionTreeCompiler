#pragma once

#include <llvm/ExecutionEngine/Orc/JITSymbol.h>
#include <llvm/ExecutionEngine/RTDyldMemoryManager.h>
#include <llvm/ExecutionEngine/SectionMemoryManager.h>
#include <llvm/Support/DynamicLibrary.h>

template <class OrcLayer_t>
class DefaultLinkingResolver : public llvm::RuntimeDyld::SymbolResolver {
public:
  DefaultLinkingResolver(OrcLayer_t &orcLayer) : OrcLayer(orcLayer) {}

  llvm::RuntimeDyld::SymbolInfo findSymbol(const std::string &name) final {
    if (auto Sym = findMangledSymbol(name))
      return llvm::RuntimeDyld::SymbolInfo(Sym.getAddress(), Sym.getFlags());
    else
      return llvm::RuntimeDyld::SymbolInfo(nullptr);
  }

  llvm::RuntimeDyld::SymbolInfo
  findSymbolInLogicalDylib(const std::string &Name) final {
    assert(false);
    return llvm::RuntimeDyld::SymbolInfo(nullptr);
  }

private:
  OrcLayer_t &OrcLayer;

  llvm::orc::JITSymbol findMangledSymbol(const std::string &name) {
    // local symbols first
    if (auto symbol = OrcLayer.findSymbol(name, false))
      return symbol;

    // host process symbols otherwise
    if (auto addr = llvm::RTDyldMemoryManager::getSymbolAddressInProcess(name))
      return llvm::orc::JITSymbol(addr, llvm::JITSymbolFlags::Exported);

    return nullptr;
  }
};

template <class OrcLayer_t>
auto makeDefaultLinkingResolver(OrcLayer_t &orcLayer) {
  return std::make_unique<DefaultLinkingResolver<OrcLayer_t>>(orcLayer);
}

auto makeDefaultMemoryManager() {
  return std::make_unique<llvm::SectionMemoryManager>();
}

template <typename T> static auto singletonSet(T t) {
  std::vector<T> vec;
  vec.push_back(std::move(t));
  return vec;
}
