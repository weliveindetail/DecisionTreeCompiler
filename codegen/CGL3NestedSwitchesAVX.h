#pragma once

#include "codegen/CGBase.h"
#include "codegen/CGL2NestedSwitches.h"
#include "codegen/CGConditionVectorEmitter.h"

class CGL3NestedSwitchesAVX : public CGBase {
  constexpr static uint8_t Levels = 3;

public:
  ~CGL3NestedSwitchesAVX() override {};

  uint8_t getOptimalJointEvaluationDepth() const override { return Levels; };

  CGBase &getFallbackCG() override { return FallbackCGL2; };

  std::vector<CGNodeInfo> emitSubtreeEvaluation(
      CGSubtreeInfo subtreeInfo, llvm::Value *dataSetPtr) override;

private:
  CGL2NestedSwitches FallbackCGL2;

  llvm::BasicBlock *emitSubtreeSwitchTarget(
      DecisionSubtreeRef subtreeRef, DecisionTreeEvaluationPath path,
      llvm::Function *ownerFunction, llvm::Value *dataSetPtr,
      llvm::BasicBlock *returnBB);

  void emitSubtreeSwitchCaseLabels(
      llvm::BasicBlock *ownerBB, llvm::SwitchInst *switchInst,
      llvm::BasicBlock *continuationBB, uint8_t numNodes,
      DecisionTreeEvaluationPath pathInfo);

};
