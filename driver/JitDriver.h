#pragma once

#include "compiler/DecisionTreeCompiler.h"
#include "compiler/SimpleOrcJit.h"
#include "data/DecisionTree.h"
#include "driver/utility/AutoSetUpTearDownLLVM.h"

class CodeGeneratorSelector;

struct JitCompileResult {
  using Evaluator_f = uint64_t(float*);

  JitCompileResult(CompileResult frontendResult, Evaluator_f *evalFunction)
      : Tree(std::move(frontendResult.Tree)), EvaluatorFunction(evalFunction) {}

  DecisionTree Tree;
  Evaluator_f *EvaluatorFunction;
};

class JitDriver {
public:
  JitDriver() : LLVM(),
                DecisionTreeFrontend(LLVM.getTargetMachine()),
                JitBackend(LLVM.getTargetMachine()) {}

  void setCodegenSelector(std::shared_ptr<CodeGeneratorSelector> codegenSel) {
    DecisionTreeFrontend.setCodegenSelector(std::move(codegenSel));
  }

  JitCompileResult run(DecisionTree decisionTree) {
    CompileResult frontendResult =
        DecisionTreeFrontend.compile(std::move(decisionTree));

    std::string entryFnName = frontendResult.EvaluatorFunctionName;
    assert(frontendResult.Module->getFunction(entryFnName) != nullptr);

    JitBackend.submitModule(std::move(frontendResult.Module));

    return JitCompileResult(
        std::move(frontendResult),
        JitBackend.getFnPtr<JitCompileResult::Evaluator_f>(entryFnName));
  }

private:
  AutoSetUpTearDownLLVM LLVM;
  DecisionTreeCompiler DecisionTreeFrontend;
  SimpleOrcJit JitBackend;
};
