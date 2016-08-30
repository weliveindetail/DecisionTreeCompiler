#pragma once

#include <cstdint>
#include <vector>

#include <llvm/IR/LLVMContext.h>

#include "codegen/utility/CGNodeInfo.h"

class CompilerSession;

class CGBase {
public:
  CGBase() = delete;
  CGBase(CGBase &&) = delete;
  CGBase(const CGBase &) = delete;
  CGBase &operator=(CGBase &&) = delete;
  CGBase &operator=(const CGBase &) = delete;

  CGBase(llvm::LLVMContext &ctx) : Ctx(ctx) {}
  virtual ~CGBase() {}

  virtual uint8_t getJointSubtreeDepth() const = 0;

  virtual std::vector<CGNodeInfo>
  emitSubtreeEvaluation(CGNodeInfo subtreeRoot,
                        const CompilerSession &session) = 0;

protected:
  llvm::LLVMContext &Ctx;
};
