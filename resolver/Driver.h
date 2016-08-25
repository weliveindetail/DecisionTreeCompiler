#pragma once

#include <memory>
#include <string>

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>

#include "resolver/DecisionTree.h"

class CGBase;

enum class CodeGeneratorType {
  L1IfThenElse,
  LXSubtreeSwitch,
  L3SubtreeSwitchAVX
};

struct CompileResult {
  DecisionTree Tree;
  std::unique_ptr<llvm::Module> Module;
  std::string EvaluatorFunctionName;
};

class DecisionTreeCompiler {
public:
  llvm::LLVMContext Ctx;

  std::unique_ptr<llvm::Module> makeModule(std::string name);
  std::unique_ptr<CGBase> makeCodeGenerator(CodeGeneratorType type);

  CompileResult compile(CodeGeneratorType codegenType, DecisionTree tree);
};
