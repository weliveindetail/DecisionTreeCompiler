#include "resolver/Driver.h"

#include <llvm/ExecutionEngine/ExecutionEngine.h>

using namespace llvm;

std::unique_ptr<llvm::Module>
DecisionTreeCompiler::makeModule(std::string name) {
  auto M = std::make_unique<llvm::Module>("file:" + name, Ctx);
  M->setDataLayout(EngineBuilder().selectTarget()->createDataLayout());

  return M;
}
