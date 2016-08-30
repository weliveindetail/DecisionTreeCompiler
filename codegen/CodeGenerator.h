#pragma once

#include <cstdint>
#include <vector>

#include <llvm/IR/LLVMContext.h>

#include "codegen/utility/CGNodeInfo.h"

class CompilerSession;

class CodeGenerator {
public:
  CodeGenerator() = delete;
  CodeGenerator(CodeGenerator &&) = delete;
  CodeGenerator(const CodeGenerator &) = delete;
  CodeGenerator &operator=(CodeGenerator &&) = delete;
  CodeGenerator &operator=(const CodeGenerator &) = delete;

  CodeGenerator(llvm::LLVMContext &ctx) : Ctx(ctx) {}
  virtual ~CodeGenerator() {}

  virtual uint8_t getJointSubtreeDepth() const = 0;

  virtual std::vector<CGNodeInfo>
  emitSubtreeEvaluation(CGNodeInfo subtreeRoot,
                        const CompilerSession &session) = 0;

protected:
  llvm::LLVMContext &Ctx;
};
