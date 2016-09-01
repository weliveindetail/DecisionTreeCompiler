#pragma once

#include <map>
#include <memory>
#include <string>

#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Value.h>

#include "data/DecisionTree.h"

class CodeGenerator;
class DecisionTreeCompiler;

class L1IfThenElse;
class LXSubtreeSwitch;
class L3SubtreeSwitchAVX;

struct CompilerSession final {
  CompilerSession() = delete;
  CompilerSession(CompilerSession &&) = delete;
  CompilerSession(const CompilerSession &) = delete;
  CompilerSession &operator=(CompilerSession &&) = delete;
  CompilerSession &operator=(const CompilerSession &) = delete;

  CompilerSession(DecisionTreeCompiler *compiler, std::string name);
  ~CompilerSession();

  mutable llvm::IRBuilder<> Builder;

  DecisionTree Tree;
  std::unique_ptr<llvm::Module> Module = nullptr;

  llvm::Type *NodeIdxTy;
  llvm::Type *DataSetFeatureValueTy;

  llvm::Value *InputDataSetPtr;
  llvm::Value *OutputNodeIdxPtr;

  // relevant target-specific features
  bool AvxSupport = false;

  CodeGenerator *selectCodeGenerator(uint8_t remainingLevels) const;

private:
  mutable std::unique_ptr<L1IfThenElse> CachedGenL1IfThenElse;
  mutable std::unique_ptr<L3SubtreeSwitchAVX> CachedGenL3SubtreeSwitchAVX;
  mutable std::map<int, std::unique_ptr<LXSubtreeSwitch>> CachedGensLXSubtreeSwitch;
};
