#pragma once

#include <array>
#include <cstdint>
#include <vector>

#include <llvm/IR/Constant.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>

#include "codegen/utility/CGNodeInfo.h"

#include "data/DecisionSubtreeRef.h"
#include "data/DecisionTreeNode.h"

class CompilerSession;

// -----------------------------------------------------------------------------

class CGConditionVectorEmitter {
public:
  CGConditionVectorEmitter(const CompilerSession &session);
  virtual ~CGConditionVectorEmitter() {}

  virtual llvm::Value *run(CGNodeInfo subtreeRoot) = 0;

protected:
  const CompilerSession &Session;

  // redundant frequently used refs
  llvm::LLVMContext &Ctx;
  llvm::IRBuilder<> &Builder;

  llvm::Type *FloatTy = llvm::Type::getFloatTy(Ctx);

  llvm::Value *emitLoadFeatureValue(DecisionTreeNode node);
};

// -----------------------------------------------------------------------------

class CGConditionVectorEmitterX86 : public CGConditionVectorEmitter {
public:
  CGConditionVectorEmitterX86(const CompilerSession &session,
                              DecisionSubtreeRef subtree);

  llvm::Value *run(CGNodeInfo subtreeRoot);

private:
  DecisionSubtreeRef Subtree;
  std::vector<DecisionTreeNode> Nodes;
};

// -----------------------------------------------------------------------------

class CGConditionVectorEmitterAVX : public CGConditionVectorEmitter {
public:
  CGConditionVectorEmitterAVX(const CompilerSession &session,
                              DecisionSubtreeRef subtree);

  llvm::Value *run(CGNodeInfo subtreeRoot);

private:
  constexpr static uint8_t AvxPackSize = 8;
  constexpr static unsigned AvxAlignment = 32;

  DecisionSubtreeRef Subtree;
  std::array<DecisionTreeNode, AvxPackSize - 1> Nodes;

  llvm::Type *Int8Ty = llvm::Type::getInt8Ty(Ctx);
  llvm::Type *Int32Ty = llvm::Type::getInt32Ty(Ctx);

  llvm::Constant *AvxPackSizeVal = llvm::ConstantInt::get(Int8Ty, AvxPackSize);

  llvm::Value *emitCollectDataSetValues();
  llvm::Value *emitDefineTreeNodeValues();
  llvm::Value *emitDefineBitShiftMaskValues();

  llvm::Value *emitComputeCompareAvx(llvm::Value *lhs, llvm::Value *rhs);
  llvm::Value *emitComputeBitShiftsAvx(llvm::Value *avxPackedCmpResults);
  llvm::Value *emitComputeHorizontalOrAvx(llvm::Value *avxPackedInts);
};
