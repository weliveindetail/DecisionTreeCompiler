#pragma once

#include "codegen/CGBase.h"

class CompilerSession;

class L1IfThenElse : public CGBase {
public:
  L1IfThenElse(llvm::LLVMContext &ctx) : CGBase(ctx) {}

  uint8_t getJointSubtreeDepth() const override { return 1; }

  std::vector<CGNodeInfo>
  emitSubtreeEvaluation(CGNodeInfo subtreeRoot,
                        const CompilerSession &session) override {
    return {};
  }
};
