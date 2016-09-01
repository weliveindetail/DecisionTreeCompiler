#pragma once

#include "codegen/CodeGenerator.h"
#include "data/DecisionSubtreeRef.h"

class L1IfThenElse : public CodeGenerator {
public:
  L1IfThenElse(const CompilerSession &session) : CodeGenerator(session) {}

  uint8_t getJointSubtreeDepth() const override { return 1; }

  std::vector<CGNodeInfo> emitEvaluation(CGNodeInfo nodeInfo) override;

private:
  std::vector<CGNodeInfo> emitConditionalBranch(CGNodeInfo nodeInfo,
                                                DecisionSubtreeRef subtree);

  std::vector<CGNodeInfo> emitSingleChildForward(CGNodeInfo nodeInfo,
                                                 DecisionSubtreeRef subtree);

  llvm::Value *emitLoadFeatureValue(DecisionTreeNode node);
  llvm::BasicBlock *makeIfThenElseBB(CGNodeInfo nodeInfo, std::string suffix);
};
