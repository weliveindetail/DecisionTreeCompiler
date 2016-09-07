#include "codegen/CodeGeneratorSelector.h"

#include "codegen/L1IfThenElse.h"
#include "codegen/LXSubtreeSwitch.h"
#include "codegen/L3SubtreeSwitchAVX.h"

CodeGenerator *DefaultSelector::select(const CompilerSession &session,
                                       int remainingLevels) {
  static L1IfThenElse L1IfThenElse;
  static LXSubtreeSwitch L2SubtreeSwitch(2);
  static L3SubtreeSwitchAVX L3SubtreeSwitchAVX;

  if (remainingLevels > 2 && AvxSupport)
    return &L3SubtreeSwitchAVX;

  if (remainingLevels > 1)
    return &L2SubtreeSwitch;

  if (remainingLevels == 1)
    return &L1IfThenElse;

  assert(false);
  return nullptr;
}
