#pragma once

#include <memory>
#include <string>

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>

class DecisionTreeCompiler {
public:
  llvm::LLVMContext Ctx;

  std::unique_ptr<llvm::Module> makeModule(std::string name);
};
