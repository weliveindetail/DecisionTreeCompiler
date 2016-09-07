#include "codegen/CodeGeneratorSelector.h"

#include "codegen/L1IfThenElse.h"
#include "codegen/LXSubtreeSwitch.h"
#include "codegen/L3SubtreeSwitchAVX.h"

CodeGenerator *DefaultSelector::select(const CompilerSession &session,
                                       int remainingLevels) {
  static L1IfThenElse DefaultL1IfThenElse;
  static LXSubtreeSwitch L2SubtreeSwitchForLeafSwitchTables(2);
  static L3SubtreeSwitchAVX L3SubtreeSwitchForLeafSwitchTables;

  if (remainingLevels == 3 /* && has AVX support */)
    return &L3SubtreeSwitchForLeafSwitchTables;

  if (remainingLevels == 2)
    return &L2SubtreeSwitchForLeafSwitchTables;

  return &DefaultL1IfThenElse;
}
