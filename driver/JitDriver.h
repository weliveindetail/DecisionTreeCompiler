#pragma once

#include "compiler/DecisionTreeCompiler.h"
#include "compiler/SimpleOrcJit.h"
#include "data/DecisionTree.h"
#include "driver/utility/AutoSetUpTearDownLLVM.h"

class CodeGeneratorSelector;

class JitDriver {
public:
  JitDriver() : LLVM(),
                DecisionTreeFrontend(LLVM.getTargetMachine()),
                JitBackend(LLVM.getTargetMachine()) {}

  void setCodegenSelector(std::shared_ptr<CodeGeneratorSelector> codegenSel) {
    DecisionTreeFrontend.setCodegenSelector(std::move(codegenSel));
  }

  using Evaluator_f = uint64_t(float*);
  Evaluator_f *run(DecisionTree decisionTree) {
    CompileResult result =
        DecisionTreeFrontend.compile(std::move(decisionTree));

    JitBackend.submitModule(std::move(result.Module));
    return JitBackend.getFnPtr<Evaluator_f>(result.EvaluatorFunctionName);
  }

private:
  AutoSetUpTearDownLLVM LLVM;
  DecisionTreeCompiler DecisionTreeFrontend;
  SimpleOrcJit JitBackend;
};
