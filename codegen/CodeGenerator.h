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
};
