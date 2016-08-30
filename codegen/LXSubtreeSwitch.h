#pragma once

#include <memory>

#include "codegen/CGBase.h"

class CompilerSession;

class LXSubtreeSwitch : public CGBase {
public:
  LXSubtreeSwitch(llvm::LLVMContext &ctx, uint8_t levels)
      : CGBase(ctx), Levels(levels) {}

  uint8_t getJointSubtreeDepth() const override { return Levels; }

  std::vector<CGNodeInfo>
  emitSubtreeEvaluation(CGNodeInfo subtreeRoot,
                        const CompilerSession &session) override;

private:
  uint8_t Levels;
};
