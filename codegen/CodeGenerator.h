#pragma once

#include <cstdint>
#include <vector>

#include "codegen/utility/CGNodeInfo.h"

class CompilerSession;

class CodeGenerator {
public:
  CodeGenerator() = default;
  virtual ~CodeGenerator() = default;

  // don't copy or move polymorphic class instances
  CodeGenerator(CodeGenerator &&) = delete;
  CodeGenerator(const CodeGenerator &) = delete;
  CodeGenerator &operator=(CodeGenerator &&) = delete;
  CodeGenerator &operator=(const CodeGenerator &) = delete;

  virtual uint8_t getJointSubtreeDepth() const = 0;

  virtual std::vector<CGNodeInfo>
  emitEvaluation(const CompilerSession &session, CGNodeInfo nodeInfo) = 0;

  virtual bool canEmitLeafEvaluation() const { return false; }
  virtual llvm::Value *emitLeafEvaluation(const CompilerSession &session,
                                          CGNodeInfo nodeInfo) {
    llvm_unreachable("Override this if canEmitLeafEvaluation returns true");
  }
};
