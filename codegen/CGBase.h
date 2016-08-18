#pragma once

#include <cstdint>
#include <memory>
#include <unordered_map>

#include <llvm/IR/Value.h>
#include <vector>

#include "resolver/Driver.h"

struct CGNodeInfo {
  CGNodeInfo(uint64_t index, llvm::BasicBlock *evalBB,
             llvm::BasicBlock *continuationBB)
      : Index(index), EvalBlock(evalBB), ContinuationBlock(continuationBB) {};

  uint64_t Index;
  llvm::BasicBlock *EvalBlock;
  llvm::BasicBlock *ContinuationBlock;
};

struct CGSubtreeInfo {
  CGNodeInfo Root;
  llvm::Function *OwnerFunction;
  llvm::BasicBlock *OwnerBB;
};

class CGBase {
public:
  CGBase(DecisionTreeCompiler *driver)
      : Driver(*driver), Ctx(driver->Ctx), Builder(driver->Builder) {}

  virtual ~CGBase() {}

  virtual uint8_t getOptimalJointEvaluationDepth() const = 0;

  virtual CGBase &getFallbackCG() = 0;

  virtual std::vector<CGNodeInfo> emitSubtreeEvaluation(
      CGSubtreeInfo subtree, llvm::Value *dataSetPtr) = 0;

protected:
  DecisionTreeCompiler &Driver;

  llvm::LLVMContext &Ctx;
  llvm::IRBuilder<> &Builder;
};
