#pragma once

#include <llvm/IR/LLVMContext.h>

#include "codegen/CodeGenerator.h"
#include "data/DecisionSubtreeRef.h"

class L1IfThenElse : public CodeGenerator {
public:
  L1IfThenElse() = default;
  uint8_t getJointSubtreeDepth() const override { return 1; }

  std::vector<CGNodeInfo> emitEvaluation(const CompilerSession &session,
                                         CGNodeInfo nodeInfo) override;

private:
  std::vector<CGNodeInfo> emitConditionalBranch(const CompilerSession &session,
                                                CGNodeInfo nodeInfo,
                                                DecisionSubtreeRef subtree);

  std::vector<CGNodeInfo> emitSingleChildForward(CGNodeInfo nodeInfo,
                                                 DecisionSubtreeRef subtree);

  llvm::Value *emitLoadFeatureValue(const CompilerSession &session,
                                    DecisionTreeNode node);

  llvm::BasicBlock *makeIfThenElseBB(llvm::LLVMContext &ctx, CGNodeInfo nodeInfo,
                                     std::string suffix);
};
