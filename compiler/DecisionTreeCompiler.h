#pragma once

#include <atomic>
#include <memory>
#include <string>

#include <llvm/ADT/StringMap.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Value.h>

#include "codegen/utility/CGNodeInfo.h"
#include "data/DecisionTree.h"

class CGBase;
class CompilerSession;

struct CompileResult {
  DecisionTree Tree;
  std::unique_ptr<llvm::Module> Module;
  std::string EvaluatorFunctionName;
  bool Success;
};

class DecisionTreeCompiler {
public:
  llvm::LLVMContext Ctx;

  DecisionTreeCompiler();
  CompileResult compile(DecisionTree tree);

private:
  std::vector<CGNodeInfo> compileSubtrees(CGNodeInfo rootNode,
                                          const CompilerSession &session);

  void connectSubtreeEndpoints(std::vector<CGNodeInfo> evaluatorEndPoints,
                               const CompilerSession &session);

  CGNodeInfo makeEvalRoot(std::string functionName,
                          const CompilerSession &session);

  llvm::FunctionType *getEvalFunctionTy(const CompilerSession &session);

  llvm::Function *emitEvalFunctionDecl(std::string name,
                                       llvm::FunctionType *signature,
                                       llvm::Module *module);

  llvm::Value *allocOutputVal(const CompilerSession &session);

private:
  llvm::StringMap<bool> CpuFeatures;

  struct AutoSetUpTearDownLLVM {
    AutoSetUpTearDownLLVM();
    ~AutoSetUpTearDownLLVM();

  private:
    static std::atomic<int> instances;
  };

  AutoSetUpTearDownLLVM SetupTearDownHelper;
};
