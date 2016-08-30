#pragma once

#include "codegen/CodeGenerator.h"

class CompilerSession;

class L1IfThenElse : public CodeGenerator {
public:
  L1IfThenElse(llvm::LLVMContext &ctx) : CodeGenerator(ctx) {}

  uint8_t getJointSubtreeDepth() const override { return 1; }

  std::vector<CGNodeInfo>
  emitSubtreeEvaluation(CGNodeInfo subtreeRoot,
                        const CompilerSession &session) override {
    return {};
  }
};
