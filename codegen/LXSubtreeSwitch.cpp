#include "codegen/LXSubtreeSwitch.h"
#include "codegen/L1IfThenElse.h"

CGBase *LXSubtreeSwitch::getFallbackCG() {
  if (!FallbackCGL1)
    FallbackCGL1 = std::make_unique<L1IfThenElse>(Ctx);

  return FallbackCGL1.get();
}

std::vector<CGNodeInfo>
LXSubtreeSwitch::emitSubtreeEvaluation(CGNodeInfo subtreeRoot,
                                       const CompilerSession &session) {
  return {};
}
