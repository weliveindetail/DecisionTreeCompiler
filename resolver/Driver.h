#pragma once

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>

#include "LegacyDecisionTree.h"
#include "resolver/DecisionTree.h"

class DecisionTreeCompiler {
public:
  llvm::LLVMContext Ctx;
  llvm::IRBuilder<> Builder;
  std::unique_ptr<llvm::Module> TheModule;
  //std::unique_ptr<SimpleOrcJit> TheCompiler;

  const DecisionTree_t &DecisionTreeLegacyData;
  const DecisionTree &DecisionTreeData;
};
