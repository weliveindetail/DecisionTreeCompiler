#pragma once

#include <atomic>
#include <memory>
#include <string>

#include <llvm/IR/Function.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Value.h>

#include "codegen/utility/CGNodeInfo.h"
#include "data/DecisionTree.h"

class CGBase;
class CompilerSession;

enum class CodeGeneratorType {
  L1IfThenElse,
  LXSubtreeSwitch,
  L3SubtreeSwitchAVX
};

struct CompileResult {
  DecisionTree Tree;
  std::unique_ptr<llvm::Module> Module;
  std::string EvaluatorFunctionName;
  bool Success;
};

class DecisionTreeCompiler {
public:
  llvm::LLVMContext Ctx;

  std::unique_ptr<llvm::Module> makeModule(std::string name);
  std::unique_ptr<CGBase> makeCodeGenerator(CodeGeneratorType type);

  CompileResult compile(CodeGeneratorType codegenType, DecisionTree tree);

private:
  std::vector<CGNodeInfo> compileSubtrees(CompilerSession &session,
                                          CGNodeInfo rootNode);

  void connectSubtreeEndpoints(const CompilerSession &session,
                               std::vector<CGNodeInfo> evaluatorEndPoints);

  CGNodeInfo makeEvalRoot(CompilerSession &session, std::string functionName);

  llvm::FunctionType *getEvalFunctionTy(const CompilerSession &session);

  llvm::Function *emitEvalFunctionDecl(std::string name,
                                       llvm::FunctionType *signature,
                                       llvm::Module *module);

  llvm::Value *allocOutputVal(const CompilerSession &session);

private:
  struct AutoSetUpTearDownLLVM {
    AutoSetUpTearDownLLVM();
    ~AutoSetUpTearDownLLVM();

  private:
    static std::atomic<int> instances;
  };

  AutoSetUpTearDownLLVM SetupTearDownHelper;
};
