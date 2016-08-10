#pragma once

#include <cstdint>
#include <memory>
#include <unordered_map>

#include <llvm/IR/Value.h>
#include <vector>

#include "resolver/Driver.h"

struct CGNodeInfo {
  CGNodeInfo(uint64_t index, llvm::BasicBlock *basicBlock)
      : Index(index), EvalBlock(basicBlock) {};

  uint64_t Index;
  llvm::BasicBlock *EvalBlock;
};

struct CGSubtreeInfo {
  CGNodeInfo Root;
  llvm::Function *OwnerFunction;
  llvm::BasicBlock *OwnerBB;
};

class CGBase {
public:
  virtual ~CGBase() {}

  virtual uint8_t getOptimalJointEvaluationDepth() const = 0;

  virtual CGBase &getFallbackCG() = 0;

  virtual std::vector<CGNodeInfo> emitSubtreeEvaluation(
      CGSubtreeInfo subtree, llvm::Value *dataSetPtr) = 0;

  void setDriver(DecisionTreeCompiler *driver) {
    assert(!driver);
    Driver = driver;
  }

  DecisionTreeCompiler *getDriver() {
    assert(Driver);
    return Driver;
  }

private:
  DecisionTreeCompiler *Driver = nullptr;

};
