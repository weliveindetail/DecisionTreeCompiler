#pragma once

#include <memory>

#include "codegen/CGBase.h"

class CompilerSession;
class L1IfThenElse;

class LXSubtreeSwitch : public CGBase {
public:
  LXSubtreeSwitch(llvm::LLVMContext &ctx) : CGBase(ctx) {}

  CGBase *getFallbackCG() override;
  uint8_t getOptimalJointEvaluationDepth() const override { return 2; }

  std::vector<CGNodeInfo>
  emitSubtreeEvaluation(CGNodeInfo subtreeRoot,
                        const CompilerSession &session) override;

private:
  std::unique_ptr<L1IfThenElse> FallbackCGL1 = nullptr;
};
