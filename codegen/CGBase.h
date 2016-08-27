#pragma once

#include <cstdint>
#include <vector>

#include <llvm/IR/LLVMContext.h>

#include "codegen/utility/CGNodeInfo.h"

class CompilerSession;

class CGBase {
public:
  CGBase(llvm::LLVMContext &ctx) : Ctx(ctx) {}

  virtual ~CGBase() {}

  virtual uint8_t getOptimalJointEvaluationDepth() const = 0;

  virtual CGBase *getFallbackCG() = 0;

  virtual std::vector<CGNodeInfo>
  emitSubtreeEvaluation(const CompilerSession &session,
                        CGNodeInfo subtreeRoot) = 0;

protected:
  llvm::LLVMContext &Ctx;
};
