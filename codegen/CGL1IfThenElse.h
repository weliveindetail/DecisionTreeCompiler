#pragma once

#include "CGBase.h"

class DecisionTreeCompiler;

class CGL1IfThenElse : public CGBase {
public:
  CGL1IfThenElse(DecisionTreeCompiler *driver)
    : CGBase(driver) {}

  ~CGL1IfThenElse() override {};

  uint8_t getOptimalJointEvaluationDepth() const override { return 1; };

  CGBase &getFallbackCG() override { return *this; };

  std::vector<CGNodeInfo> emitSubtreeEvaluation(
      CGSubtreeInfo subtree, llvm::Value *dataSetPtr) override { return {}; }
};
