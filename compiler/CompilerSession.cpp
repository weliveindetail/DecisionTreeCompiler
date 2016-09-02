#include "CompilerSession.h"

#include "codegen/CodeGeneratorSelector.h"
#include "compiler/DecisionTreeCompiler.h"

using namespace llvm;

CompilerSession::CompilerSession(DecisionTreeCompiler *compiler,
                                 TargetMachine *targetMachine,
                                 std::string name)
    : Builder(compiler->Ctx),
      NodeIdxTy(Type::getInt64Ty(compiler->Ctx)),
      DataSetFeatureValueTy(Type::getFloatTy(compiler->Ctx)) {
  Module = std::make_unique<llvm::Module>("file:" + name, compiler->Ctx);
  Module->setDataLayout(targetMachine->createDataLayout());
}

CompilerSession::~CompilerSession() = default;

CodeGenerator *
CompilerSession::selectCodeGenerator(uint8_t remainingLevels) const {
  return CodegenSelector->select(*this, remainingLevels);
}
