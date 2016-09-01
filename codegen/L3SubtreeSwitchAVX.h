#pragma once

#include "codegen/LXSubtreeSwitch.h"
#include "codegen/utility/CGConditionVectorEmitter.h"

class L3SubtreeSwitchAVX : public LXSubtreeSwitch {
  constexpr static uint8_t Levels = 3;

public:
  L3SubtreeSwitchAVX(const CompilerSession &session)
      : LXSubtreeSwitch(session, Levels) {}

  llvm::Value *emitConditionVector(DecisionSubtreeRef subtree,
                                   CGNodeInfo rootNodeInfo) override {
    CGConditionVectorEmitterAVX emitter(Session, subtree);
    return emitter.run(rootNodeInfo);
  }
};
