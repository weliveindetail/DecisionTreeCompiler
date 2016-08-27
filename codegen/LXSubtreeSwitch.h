#pragma once

#include "codegen/CGBase.h"
#include "codegen/CGL1IfThenElse.h"

class CompilerSession;

class LXSubtreeSwitch : public CGBase {
public:
  LXSubtreeSwitch(llvm::LLVMContext &ctx) : CGBase(ctx) {}

  ~LXSubtreeSwitch() override{};

  uint8_t getOptimalJointEvaluationDepth() const override { return 2; };

  CGBase *getFallbackCG() override;

  std::vector<CGNodeInfo>
  emitSubtreeEvaluation(const CompilerSession &session,
                        CGNodeInfo subtreeRoot) override {
    return {};
  }

private:
  std::unique_ptr<CGL1IfThenElse> FallbackCGL1 = nullptr;
};

inline CGBase *LXSubtreeSwitch::getFallbackCG() {
  if (!FallbackCGL1)
    FallbackCGL1 = std::make_unique<CGL1IfThenElse>(Ctx);

  return FallbackCGL1.get();
};
