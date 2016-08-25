#include "resolver/Driver.h"

#include <llvm/ExecutionEngine/ExecutionEngine.h>

#include "codegen/CGL1IfThenElse.h"
#include "codegen/CGL2NestedSwitches.h"
#include "codegen/CGL3NestedSwitchesAVX.h"

#include "resolver/CompilerSession.h"

using namespace llvm;

CompileResult DecisionTreeCompiler::compile(
    CodeGeneratorType codegenType, DecisionTree tree) {
  std::unique_ptr<CGBase> codegen = makeCodeGenerator(codegenType);
  CompilerSession session(this, std::move(tree), std::move(codegen), "sessionName");

  // todo: prepare, invoke codegen, finalize

  CompileResult result;
  result.Tree = std::move(session.Tree);
  result.Module = std::move(session.Module);
  result.EvaluatorFunctionName = "";

  return result;
}

std::unique_ptr<CGBase> DecisionTreeCompiler::makeCodeGenerator(
    CodeGeneratorType type) {
  switch (type) {
    case CodeGeneratorType::L1IfThenElse:
      return std::make_unique<CGL1IfThenElse>(Ctx);

    case CodeGeneratorType::LXSubtreeSwitch:
      return std::make_unique<CGL2NestedSwitches>(Ctx);

    case CodeGeneratorType::L3SubtreeSwitchAVX:
      return std::make_unique<CGL3NestedSwitchesAVX>(Ctx);
  }
}

std::unique_ptr<llvm::Module>
DecisionTreeCompiler::makeModule(std::string name) {
  auto M = std::make_unique<llvm::Module>("file:" + name, Ctx);
  M->setDataLayout(EngineBuilder().selectTarget()->createDataLayout());

  return M;
}
