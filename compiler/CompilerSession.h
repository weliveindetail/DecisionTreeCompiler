#pragma once

#include <memory>
#include <string>

#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Value.h>

#include "data/DecisionTree.h"

class CGBase;
class DecisionTreeCompiler;

struct CompilerSession final {
  CompilerSession() = delete;
  CompilerSession(CompilerSession &&) = delete;
  CompilerSession(const CompilerSession &) = delete;
  CompilerSession &operator=(CompilerSession &&) = delete;
  CompilerSession &operator=(const CompilerSession &) = delete;

  CompilerSession(DecisionTreeCompiler *compiler, DecisionTree tree,
                  std::unique_ptr<CGBase> preferredCodegen, std::string name);

  mutable llvm::IRBuilder<> Builder;

  DecisionTree Tree;
  std::unique_ptr<llvm::Module> Module;
  std::unique_ptr<CGBase> PreferredCodegen;

  llvm::Type *NodeIdxTy;
  llvm::Type *DataSetFeatureValueTy;

  llvm::Value *InputDataSetPtr;
  llvm::Value *OutputNodeIdxPtr;

  CGBase *selectCodeGenerator(uint8_t remainingLevels) const;
};
