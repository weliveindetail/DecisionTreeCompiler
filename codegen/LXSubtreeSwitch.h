#pragma once

#include "codegen/CGBase.h"
#include "codegen/CGL1IfThenElse.h"

class CompilerSession;

class LXSubtreeSwitch : public CGBase {
public:
  LXSubtreeSwitch(llvm::LLVMContext &ctx) : CGBase(ctx) {}
  ~LXSubtreeSwitch() override{}

  CGBase *getFallbackCG() override;
  uint8_t getOptimalJointEvaluationDepth() const override { return 2; }

  std::vector<CGNodeInfo>
  emitSubtreeEvaluation(CGNodeInfo subtreeRoot,
                        const CompilerSession &session) override {
    return {};
  }

private:
  std::unique_ptr<CGL1IfThenElse> FallbackCGL1 = nullptr;
};

inline CGBase *LXSubtreeSwitch::getFallbackCG() {
  if (!FallbackCGL1)
    FallbackCGL1 = std::make_unique<CGL1IfThenElse>(Ctx);

  return FallbackCGL1.get();
}
