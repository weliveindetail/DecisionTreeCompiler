#pragma once

#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Function.h>

struct CGNodeInfo {
  CGNodeInfo() = default;
  CGNodeInfo(CGNodeInfo &&) = default;
  CGNodeInfo(const CGNodeInfo &) = default;
  CGNodeInfo &operator=(CGNodeInfo &&) = default;
  CGNodeInfo &operator=(const CGNodeInfo &) = default;

  CGNodeInfo(uint64_t index, llvm::Function *ownerFunction,
             llvm::BasicBlock *evalBB, llvm::BasicBlock *continuationBB)
      : Index(index), OwnerFunction(ownerFunction), EvalBlock(evalBB),
        ContinuationBlock(continuationBB){}

  uint64_t Index;
  llvm::Function *OwnerFunction;
  llvm::BasicBlock *EvalBlock;
  llvm::BasicBlock *ContinuationBlock;
};
