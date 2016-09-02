#pragma once

#include <vector>

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Type.h>

#include "codegen/CodeGenerator.h"
#include "codegen/utility/CGConditionVectorEmitter.h"
#include "codegen/utility/CGEvaluationPath.h"

class LXSubtreeSwitch : public CodeGenerator {
public:
  LXSubtreeSwitch(uint8_t levels) : Levels(levels) {}
  uint8_t getJointSubtreeDepth() const override { return Levels; }

  std::vector<CGNodeInfo>
  emitEvaluation(const CompilerSession &session, CGNodeInfo subtreeRoot) override;

protected:
  virtual llvm::Value *emitConditionVector(const CompilerSession &session,
                                           DecisionSubtreeRef subtree,
                                           CGNodeInfo rootNodeInfo) {
    CGConditionVectorEmitterX86 emitter(session, subtree);
    return emitter.run(rootNodeInfo);
  }

  llvm::BasicBlock *makeSwitchBB(llvm::LLVMContext &ctx, CGNodeInfo subtreeRoot,
                                 std::string suffix);

  std::vector<CGNodeInfo>
  emitSwitchTargets(llvm::LLVMContext &ctx,
                    const std::vector<CGEvaluationPath> &evaluationPaths,
                    llvm::Function *ownerFunction, llvm::BasicBlock *returnBB);

  uint32_t emitSwitchCaseLabels(llvm::LLVMContext &ctx,
                                llvm::SwitchInst *switchInst,
                                llvm::Type *switchCondTy,
                                CGNodeInfo targetNodeInfo,
                                std::vector<uint32_t> pathCaseValues);
private:
  uint8_t Levels;
};
