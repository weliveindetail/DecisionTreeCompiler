#include "CompilerSession.h"

#include <llvm/ExecutionEngine/ExecutionEngine.h>

#include "codegen/L1IfThenElse.h"
#include "codegen/LXSubtreeSwitch.h"
#include "codegen/L3SubtreeSwitchAVX.h"

#include "compiler/DecisionTreeCompiler.h"

using namespace llvm;

CompilerSession::CompilerSession(DecisionTreeCompiler *compiler,
                                 std::string name)
    : Builder(compiler->Ctx),
      NodeIdxTy(Type::getInt64Ty(compiler->Ctx)),
      DataSetFeatureValueTy(Type::getFloatTy(compiler->Ctx)) {
  Module = std::make_unique<llvm::Module>("file:" + name, compiler->Ctx);
  Module->setDataLayout(EngineBuilder().selectTarget()->createDataLayout());
}

CompilerSession::~CompilerSession() = default;

CodeGenerator *CompilerSession::selectCodeGenerator(uint8_t remainingLevels) const {
  if (remainingLevels > 2 && AvxSupport) {
    if (!CachedGenL3SubtreeSwitchAVX)
      CachedGenL3SubtreeSwitchAVX =
          std::make_unique<L3SubtreeSwitchAVX>(*this);

    return CachedGenL3SubtreeSwitchAVX.get();
  }

  if (remainingLevels > 1) {
    int jointSubtreeLevels = 2;
    auto it = CachedGensLXSubtreeSwitch.find(jointSubtreeLevels);

    if (it == CachedGensLXSubtreeSwitch.end()) {
      CachedGensLXSubtreeSwitch[jointSubtreeLevels] =
          std::make_unique<LXSubtreeSwitch>(*this, jointSubtreeLevels);
    }

    return CachedGensLXSubtreeSwitch.at(jointSubtreeLevels).get();
  }

  if (remainingLevels == 1) {
    if (!CachedGenL1IfThenElse)
      CachedGenL1IfThenElse =
          std::make_unique<L1IfThenElse>(*this);

    return CachedGenL1IfThenElse.get();
  }

  assert(false);
  return nullptr;
}
