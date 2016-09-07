#pragma once

#include <atomic>
#include <memory>
#include <string>

#include <llvm/ADT/StringMap.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Value.h>
#include <llvm/Target/TargetMachine.h>

#include "codegen/utility/CGNodeInfo.h"
#include "data/DecisionTree.h"

class CodeGenerator;
class CodeGeneratorSelector;
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

  DecisionTreeCompiler(llvm::TargetMachine *target);

  void setCodegenSelector(
      std::shared_ptr<CodeGeneratorSelector> codegenSelector);

  CompileResult compile(DecisionTree tree);

private:
  std::vector<CGNodeInfo> compileSubtrees(CGNodeInfo rootNode,
                                          const CompilerSession &session);

  std::vector<CGNodeInfo> compileNestedSubtrees(CodeGenerator *codegen,
                                                std::vector<CGNodeInfo> roots,
                                                const CompilerSession &session);

  void compileLeafSubtrees(CodeGenerator *codegen,
                           std::vector<CGNodeInfo> roots,
                           const CompilerSession &session);

  void connectSubtreeEndpoints(std::vector<CGNodeInfo> evaluatorEndPoints,
                               const CompilerSession &session);

  CGNodeInfo makeEvalRoot(std::string functionName,
                          const CompilerSession &session);

  llvm::Function *emitEvalFunctionDecl(std::string name,
                                       llvm::FunctionType *signature,
                                       llvm::Module *module);

  llvm::FunctionType *getEvalFunctionTy(const CompilerSession &session);
  llvm::AttributeSet collectEvalFunctionAttribs();

  llvm::Value *allocOutputVal(const CompilerSession &session);

private:
  llvm::TargetMachine *Target;
  llvm::StringMap<bool> CpuFeatures;
  std::shared_ptr<CodeGeneratorSelector> CodegenSelector;
};
