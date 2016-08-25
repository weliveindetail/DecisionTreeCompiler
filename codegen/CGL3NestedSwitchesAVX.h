#pragma once

#include "codegen/CGBase.h"
#include "codegen/CGNodeInfo.h"
#include "codegen/CGL2NestedSwitches.h"
#include "codegen/CGEvaluationPath.h"

class DecisionTreeCompiler;

class CGL3NestedSwitchesAVX : public CGBase {
  constexpr static uint8_t Levels = 3;

public:
  CGL3NestedSwitchesAVX(DecisionTreeCompiler *driver)
      : CGBase(driver), FallbackCGL2(driver) {}

  ~CGL3NestedSwitchesAVX() override {};

  uint8_t getOptimalJointEvaluationDepth() const override { return Levels; };

  CGBase &getFallbackCG() override { return FallbackCGL2; };

  std::vector<CGNodeInfo> emitSubtreeEvaluation(
      CGSubtreeInfo subtreeInfo, llvm::Value *dataSetPtr) override;

private:
  CGL2NestedSwitches FallbackCGL2;

  std::vector<CGNodeInfo> emitSwitchTargets(
      DecisionSubtreeRef subtreeRef,
      const std::vector<CGEvaluationPath> &evaluationPaths,
      llvm::Function *ownerFunction, llvm::BasicBlock *returnBB);

  uint32_t emitSwitchCaseLabels(
      llvm::SwitchInst *switchInst, llvm::Type *switchCondTy,
      CGNodeInfo targetNodeInfo, std::vector<uint32_t> pathCaseValues);

  llvm::BasicBlock *makeSwitchBB(CGSubtreeInfo subtreeInfo, std::string suffix);

};
