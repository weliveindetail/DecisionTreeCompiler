#pragma once

#include "codegen/CodeGenerator.h"

class L1IfThenElse : public CodeGenerator {
public:
  L1IfThenElse(const CompilerSession &session) : CodeGenerator(session) {}

  uint8_t getJointSubtreeDepth() const override { return 1; }

  std::vector<CGNodeInfo>
  emitEvaluation(CGNodeInfo node) override {
    return {};
  }
};
