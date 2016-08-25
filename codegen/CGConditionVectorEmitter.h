#pragma once

#include <vector>
#include <array>
#include <cstdint>
#include <memory>
#include <unordered_map>

#include <llvm/IR/Value.h>

#include "codegen/CGNodeInfo.h"
#include "resolver/DecisionTree.h"
#include "resolver/Driver.h"

class CGConditionVectorEmitterBase {
public:
  CGConditionVectorEmitterBase(DecisionTreeCompiler *driver)
      : Driver(*driver), Ctx(driver->Ctx), Builder(driver->Builder) {}

  virtual ~CGConditionVectorEmitterBase () {}

  virtual llvm::Value *run(llvm::Value *dataSetPtr) = 0;

protected:
  DecisionTreeCompiler &Driver;

  llvm::LLVMContext &Ctx;
  llvm::IRBuilder<> &Builder;
};

class CGConditionVectorEmitterAVX : public CGConditionVectorEmitterBase {
public:
  CGConditionVectorEmitterAVX(DecisionTreeCompiler *driver,
                              DecisionSubtreeRef subtree);

  llvm::Value *run(llvm::Value *dataSetPtr) override;

private:
  constexpr static uint8_t AvxPackSize = 8;

  DecisionSubtreeRef Subtree;
  std::array<DecisionTreeNode*, AvxPackSize - 1> Nodes;

  llvm::Type *Int8Ty = llvm::Type::getInt8Ty(Ctx);
  llvm::Type *Int32Ty = llvm::Type::getInt32Ty(Ctx);
  llvm::Type *FloatTy = llvm::Type::getFloatTy(Ctx);

  llvm::Constant *AvxPackSizeVal = llvm::ConstantInt::get(Int8Ty, AvxPackSize);

  llvm::Value *emitLoadFeatureValue(DecisionTreeNode *node, llvm::Value *dataSetPtr);

  llvm::Value *emitCollectDataSetValues(llvm::Value *dataSetPtr);
  llvm::Value *emitDefineTreeNodeValues();
  llvm::Value *emitDefineBitShiftMaskValues();

  llvm::Value *emitComputeCompareAvx(llvm::Value *lhs, llvm::Value *rhs);
  llvm::Value *emitComputeBitShiftsAvx(llvm::Value *avxPackedCmpResults);
  llvm::Value *emitComputeHorizontalOrAvx(llvm::Value *avxPackedInts);

  void collectSubtreeNodes();
};
