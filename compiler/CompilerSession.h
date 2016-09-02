#pragma once

#include <map>
#include <memory>
#include <string>

#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Value.h>
#include <llvm/Target/TargetMachine.h>

#include "data/DecisionTree.h"

class CodeGenerator;
class CodeGeneratorSelector;
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

  CompilerSession(DecisionTreeCompiler *compiler,
                  llvm::TargetMachine *targetMachine, std::string name);
  ~CompilerSession();

  mutable llvm::IRBuilder<> Builder;

  DecisionTree Tree;
  std::unique_ptr<llvm::Module> Module = nullptr;

  llvm::Type *NodeIdxTy;
  llvm::Type *DataSetFeatureValueTy;

  llvm::Value *InputDataSetPtr;
  llvm::Value *OutputNodeIdxPtr;

  std::shared_ptr<CodeGeneratorSelector> CodegenSelector;
  CodeGenerator *selectCodeGenerator(uint8_t remainingLevels) const;
};
