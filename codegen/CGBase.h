#pragma once

#include <cstdint>
#include <memory>
#include <unordered_map>

#include <llvm/IR/Value.h>
#include <vector>

#include "codegen/CGNodeInfo.h"
#include "resolver/Driver.h"

class CGBase {
public:
  CGBase(DecisionTreeCompiler *driver)
      : Driver(*driver), Ctx(driver->Ctx), Builder(driver->Builder) {}

  virtual ~CGBase() {}

  virtual uint8_t getOptimalJointEvaluationDepth() const = 0;

  virtual CGBase &getFallbackCG() = 0;

  virtual std::vector<CGNodeInfo> emitSubtreeEvaluation(
      CGNodeInfo subtreeRoot, llvm::Value *dataSetPtr) = 0;

protected:
  DecisionTreeCompiler &Driver;

  llvm::LLVMContext &Ctx;
  llvm::IRBuilder<> &Builder;
};
