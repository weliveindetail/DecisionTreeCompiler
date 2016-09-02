#include "codegen/CodeGeneratorSelector.h"

// in order to delete forward declared cached generators
DefaultSelector::~DefaultSelector() {
  CachedGenL1IfThenElse = nullptr;
}

CodeGenerator *DefaultSelector::select(const CompilerSession &session,
                                       int remainingLevels) {
  if (remainingLevels > 2 && AvxSupport) {
    if (!CachedGenL3SubtreeSwitchAVX)
      CachedGenL3SubtreeSwitchAVX = std::make_unique<L3SubtreeSwitchAVX>();

    return CachedGenL3SubtreeSwitchAVX.get();
  }

  if (remainingLevels > 1) {
    int jointSubtreeLevels = 2;
    auto it = CachedGensLXSubtreeSwitch.find(jointSubtreeLevels);

    if (it == CachedGensLXSubtreeSwitch.end()) {
      CachedGensLXSubtreeSwitch[jointSubtreeLevels] =
          std::make_unique<LXSubtreeSwitch>(jointSubtreeLevels);
    }

    return CachedGensLXSubtreeSwitch.at(jointSubtreeLevels).get();
  }

  if (remainingLevels == 1) {
    if (!CachedGenL1IfThenElse)
      CachedGenL1IfThenElse = std::make_unique<L1IfThenElse>();

    return CachedGenL1IfThenElse.get();
  }

  assert(false);
  return nullptr;
}
