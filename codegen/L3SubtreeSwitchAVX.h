#pragma once

#include "codegen/LXSubtreeSwitch.h"
#include "codegen/utility/CGConditionVectorEmitter.h"

class L3SubtreeSwitchAVX : public LXSubtreeSwitch {
  constexpr static uint8_t Levels = 3;

public:
  L3SubtreeSwitchAVX() : LXSubtreeSwitch(Levels) {}

  llvm::Value *emitConditionVector(const CompilerSession &session,
                                   DecisionSubtreeRef subtree,
                                   CGNodeInfo rootNodeInfo) override {
    CGConditionVectorEmitterAVX emitter(session, subtree);
    return emitter.run(rootNodeInfo);
  }
};
