#pragma once

#include <cstdint>
#include <map>
#include <memory>

class CodeGenerator;
class CompilerSession;

class CodeGeneratorSelector {
public:
  virtual ~CodeGeneratorSelector() = default;
  virtual CodeGenerator *select(const CompilerSession &session, int remainingLevels) = 0;

  bool AvxSupport = false;
};

class DefaultSelector : public CodeGeneratorSelector {
public:
  CodeGenerator *select(const CompilerSession &session, int remainingLevels) override;
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
