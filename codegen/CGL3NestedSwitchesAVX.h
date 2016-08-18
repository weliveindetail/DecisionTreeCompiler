#pragma once

#include "codegen/CGBase.h"
#include "codegen/CGL2NestedSwitches.h"

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
      const std::vector<DecisionTreeEvaluationPath> &evaluationPaths,
      llvm::Function *ownerFunction, llvm::BasicBlock *returnBB);

  uint32_t emitSwitchCaseLabels(
      llvm::SwitchInst *switchInst, llvm::Type *switchCondTy,
      CGNodeInfo targetNodeInfo, std::vector<uint32_t> pathCaseValues);

  llvm::BasicBlock *makeSwitchBB(CGSubtreeInfo subtreeInfo, std::string suffix);

  /*template<
      class Collection1_t, class Collection2_t,
      class ItemsEvaluate_f, class Accumulate_t>
  static Accumulate_t accumulateSimultaneously(
      Collection1_t collection1,  Collection2_t collection2,
      Accumulate_t init, ItemsEvaluate_f evaluate) {
    Accumulate_t result = init;
    auto it1 = collection1.begin();
    auto it2 = collection2.begin();

    for (; it1 != collection1.end() && it2 != collection2.end(); ++it1, ++it2)
      result += result, evaluate(it1, it2);

    assert(it1 == collection1.end());
    assert(it2 == collection2.end());
    return result;
  };*/
};
