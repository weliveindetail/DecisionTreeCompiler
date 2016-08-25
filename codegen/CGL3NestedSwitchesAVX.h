#pragma once

#include <llvm/IR/Instructions.h>

#include "codegen/CGBase.h"
#include "codegen/CGEvaluationPath.h"

class CompilerSession;
class CGL2NestedSwitches;

class CGL3NestedSwitchesAVX : public CGBase {
  constexpr static uint8_t Levels = 3;

public:
  CGL3NestedSwitchesAVX(llvm::LLVMContext &ctx) : CGBase(ctx) {}

  ~CGL3NestedSwitchesAVX() override {};

  uint8_t getOptimalJointEvaluationDepth() const override { return Levels; };

  CGBase *getFallbackCG() override;

  std::vector<CGNodeInfo> emitSubtreeEvaluation(
      const CompilerSession &session, CGNodeInfo subtreeRoot) override;

private:
  std::unique_ptr<CGL2NestedSwitches> FallbackCGL2 = nullptr;

  std::vector<CGNodeInfo> emitSwitchTargets(
      DecisionSubtreeRef subtreeRef,
      const std::vector<CGEvaluationPath> &evaluationPaths,
      llvm::Function *ownerFunction, llvm::BasicBlock *returnBB);

  uint32_t emitSwitchCaseLabels(
      llvm::SwitchInst *switchInst, llvm::Type *switchCondTy,
      CGNodeInfo targetNodeInfo, std::vector<uint32_t> pathCaseValues);

  llvm::BasicBlock *makeSwitchBB(CGNodeInfo subtreeRoot, std::string suffix);

};
