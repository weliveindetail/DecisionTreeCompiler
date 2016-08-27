#pragma once

#include "codegen/CGBase.h"

class CompilerSession;

class CGL1IfThenElse : public CGBase {
public:
  CGL1IfThenElse(llvm::LLVMContext &ctx) : CGBase(ctx) {}
  ~CGL1IfThenElse() override{};

  CGBase *getFallbackCG() override { return this; }
  uint8_t getOptimalJointEvaluationDepth() const override { return 1; }

  std::vector<CGNodeInfo>
  emitSubtreeEvaluation(const CompilerSession &session,
                        CGNodeInfo subtreeRoot) override {
    return {};
  }
};
