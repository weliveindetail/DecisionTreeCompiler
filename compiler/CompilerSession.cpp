#include "CompilerSession.h"

#include "codegen/CGBase.h"
#include "DecisionTreeCompiler.h"

using namespace llvm;

CompilerSession::CompilerSession(DecisionTreeCompiler *compiler,
                                 DecisionTree tree,
                                 std::unique_ptr<CGBase> preferredCodegen,
                                 std::string name)
  : Builder(compiler->Ctx)
  , Tree(std::move(tree))
  , Module(compiler->makeModule(std::move(name)))
  , PreferredCodegen(std::move(preferredCodegen))
  , NodeIdxTy(Type::getInt64Ty(compiler->Ctx))
  , DataSetFeatureValueTy(Type::getFloatTy(compiler->Ctx)) {}

CGBase *CompilerSession::selectCodeGenerator(uint8_t remainingLevels) const {
  assert(remainingLevels > 0);
  CGBase *codegen = PreferredCodegen.get();

  while (codegen->getOptimalJointEvaluationDepth() > remainingLevels) {
    codegen = codegen->getFallbackCG();
  }

  return codegen;
}
