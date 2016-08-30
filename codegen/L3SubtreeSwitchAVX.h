#pragma once

#include <memory>

#include <llvm/IR/Instructions.h>

#include "codegen/CGBase.h"
#include "codegen/utility/CGEvaluationPath.h"

class CompilerSession;

class L3SubtreeSwitchAVX : public CGBase {
  constexpr static uint8_t Levels = 3;

public:
  L3SubtreeSwitchAVX(llvm::LLVMContext &ctx) : CGBase(ctx) {}

  uint8_t getJointSubtreeDepth() const override { return Levels; };

  std::vector<CGNodeInfo>
  emitSubtreeEvaluation(CGNodeInfo subtreeRoot,
                        const CompilerSession &session) override;

private:
  std::vector<CGNodeInfo>
  emitSwitchTargets(const std::vector<CGEvaluationPath> &evaluationPaths,
                    llvm::Function *ownerFunction, llvm::BasicBlock *returnBB);

  uint32_t emitSwitchCaseLabels(llvm::SwitchInst *switchInst,
                                llvm::Type *switchCondTy,
                                CGNodeInfo targetNodeInfo,
                                std::vector<uint32_t> pathCaseValues);

  llvm::BasicBlock *makeSwitchBB(CGNodeInfo subtreeRoot, std::string suffix);
};
