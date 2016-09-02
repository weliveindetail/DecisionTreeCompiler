#pragma once

#include <cstdint>
#include <map>
#include <memory>

#include "codegen/L1IfThenElse.h"
#include "codegen/LXSubtreeSwitch.h"
#include "codegen/L3SubtreeSwitchAVX.h"

class CodeGenerator;
class CompilerSession;

class CodeGeneratorSelector {
public:
  virtual ~CodeGeneratorSelector() = default;
  virtual CodeGenerator *select(const CompilerSession &session, int remainingLevels) = 0;

  bool AvxSupport = false;
};

class L1IfThenElse;
class LXSubtreeSwitch;
class L3SubtreeSwitchAVX;

class DefaultSelector : public CodeGeneratorSelector {
public:
  ~DefaultSelector() override;
  CodeGenerator *select(const CompilerSession &session, int remainingLevels) override;

private:
  mutable std::unique_ptr<L1IfThenElse> CachedGenL1IfThenElse;
  mutable std::unique_ptr<L3SubtreeSwitchAVX> CachedGenL3SubtreeSwitchAVX;
  mutable std::map<int, std::unique_ptr<LXSubtreeSwitch>> CachedGensLXSubtreeSwitch;
};

template <class LambdaSelect_f>
class LambdaSelector : public CodeGeneratorSelector {
public:
  LambdaSelector(LambdaSelect_f func) : LambdaSelect(func) {}

  CodeGenerator *select(const CompilerSession &session, int remainingLevels) override {
    return LambdaSelect(session, remainingLevels);
  }

private:
  LambdaSelect_f LambdaSelect;
};

template <class LambdaSelect_f>
std::shared_ptr<LambdaSelector<LambdaSelect_f>>
makeLambdaSelector(LambdaSelect_f func) {
  return std::make_shared<LambdaSelector<LambdaSelect_f>>(func);
}
