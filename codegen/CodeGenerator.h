#pragma once

#include <cstdint>
#include <vector>

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>

#include "codegen/utility/CGNodeInfo.h"
#include "compiler/CompilerSession.h"

class CodeGenerator {
public:
  CodeGenerator() = delete;
  CodeGenerator(CodeGenerator &&) = delete;
  CodeGenerator(const CodeGenerator &) = delete;
  CodeGenerator &operator=(CodeGenerator &&) = delete;
  CodeGenerator &operator=(const CodeGenerator &) = delete;

  CodeGenerator(const CompilerSession &session)
    : Ctx(session.Builder.getContext()), Session(session) {}
  virtual ~CodeGenerator() {}

  virtual uint8_t getJointSubtreeDepth() const = 0;

  virtual std::vector<CGNodeInfo>
  emitSubtreeEvaluation(CGNodeInfo subtreeRoot) = 0;

protected:
  llvm::LLVMContext &Ctx;
  const CompilerSession &Session;
};
