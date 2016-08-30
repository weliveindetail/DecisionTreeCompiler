#pragma once

#include <memory>

#include "codegen/CodeGenerator.h"

class CompilerSession;

class LXSubtreeSwitch : public CodeGenerator {
public:
  LXSubtreeSwitch(llvm::LLVMContext &ctx, uint8_t levels)
      : CodeGenerator(ctx), Levels(levels) {}

  uint8_t getJointSubtreeDepth() const override { return Levels; }

  std::vector<CGNodeInfo>
  emitSubtreeEvaluation(CGNodeInfo subtreeRoot,
                        const CompilerSession &session) override;

private:
  uint8_t Levels;
};
