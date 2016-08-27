#pragma once

#include <llvm/IR/Instructions.h>

#include "codegen/CGBase.h"
#include "codegen/utility/CGEvaluationPath.h"

class CompilerSession;
class LXSubtreeSwitch;

class L3SubtreeSwitchAVX : public CGBase {
  constexpr static uint8_t Levels = 3;

public:
  L3SubtreeSwitchAVX(llvm::LLVMContext &ctx) : CGBase(ctx) {}
  ~L3SubtreeSwitchAVX() override{};

  CGBase *getFallbackCG() override;
  uint8_t getOptimalJointEvaluationDepth() const override { return Levels; };

  std::vector<CGNodeInfo>
  emitSubtreeEvaluation(const CompilerSession &session,
                        CGNodeInfo subtreeRoot) override;

private:
  std::unique_ptr<LXSubtreeSwitch> FallbackCGL2 = nullptr;

  std::vector<CGNodeInfo>
  emitSwitchTargets(const std::vector<CGEvaluationPath> &evaluationPaths,
                    llvm::Function *ownerFunction, llvm::BasicBlock *returnBB);

  uint32_t emitSwitchCaseLabels(llvm::SwitchInst *switchInst,
                                llvm::Type *switchCondTy,
                                CGNodeInfo targetNodeInfo,
                                std::vector<uint32_t> pathCaseValues);

  llvm::BasicBlock *makeSwitchBB(CGNodeInfo subtreeRoot, std::string suffix);
};
