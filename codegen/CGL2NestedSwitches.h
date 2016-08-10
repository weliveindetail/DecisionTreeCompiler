#pragma once

#include "codegen/CGBase.h"
#include "codegen/CGL1IfThenElse.h"

class CGL2NestedSwitches : public CGBase {
public:
  ~CGL2NestedSwitches() override {};

  uint8_t getOptimalJointEvaluationDepth() const override { return 2; };

  CGBase &getFallbackCG() override { return FallbackCGL1; };

  std::vector<CGNodeInfo> emitSubtreeEvaluation(
      CGSubtreeInfo subtree, llvm::Value *dataSetPtr) override { return {}; }

private:
  CGL1IfThenElse FallbackCGL1;

};
