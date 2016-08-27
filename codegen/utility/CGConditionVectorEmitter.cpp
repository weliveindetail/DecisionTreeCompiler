#include "CGConditionVectorEmitter.h"
#include "compiler/CompilerSession.h"

#include <algorithm>

using namespace llvm;

CGConditionVectorEmitterBase::CGConditionVectorEmitterBase(
    const CompilerSession &session)
    : Session(session), Ctx(session.Builder.getContext()),
      Builder(session.Builder) {}

CGConditionVectorEmitterAVX::CGConditionVectorEmitterAVX(
    const CompilerSession &session, DecisionSubtreeRef subtree)
    : CGConditionVectorEmitterBase(session), Subtree(std::move(subtree)) {
  assert(Subtree.getNodeCount() == AvxPackSize - 1);
  collectSubtreeNodes();
}

void CGConditionVectorEmitterAVX::collectSubtreeNodes() {
  size_t idx = 0;
  for (DecisionTreeNode node : Subtree.collectNodes()) {
    Nodes[idx++] = std::move(node);
  }
}

Value *CGConditionVectorEmitterAVX::run(CGNodeInfo subtreeRoot) {
  Builder.SetInsertPoint(subtreeRoot.EvalBlock);

  Value *dataSetValues = emitCollectDataSetValues();
  Value *treeNodeValues = emitDefineTreeNodeValues();

  Value *avxCmpResults = emitComputeCompareAvx(dataSetValues, treeNodeValues);
  Value *avxBitShiftResults = emitComputeBitShiftsAvx(avxCmpResults);
  Value *conditionVectorVal = emitComputeHorizontalOrAvx(avxBitShiftResults);

  unsigned significantBits = Subtree.getNodeCount() + 1; // +1 = sign bit
  return Builder.CreateTrunc(conditionVectorVal,
                             Type::getIntNTy(Ctx, significantBits));
}

Value *CGConditionVectorEmitterAVX::emitCollectDataSetValues() {
  Value *featureValues = Builder.Insert(
      new AllocaInst(FloatTy, AvxPackSizeVal, AvxAlignment), "featureValues");

  uint8_t bitOffset = 0;
  for (DecisionTreeNode node : Nodes) {
    Builder.CreateStore(emitLoadFeatureValue(std::move(node)),
                        Builder.CreateConstGEP1_32(featureValues, bitOffset++));
  }

  return featureValues;
}

Value *
CGConditionVectorEmitterAVX::emitLoadFeatureValue(DecisionTreeNode node) {
  llvm::Value *dataSetFeaturePtr = Builder.CreateConstGEP1_32(
      Session.InputDataSetPtr, node.DataSetFeatureIdx);

  return Builder.CreateLoad(dataSetFeaturePtr);
}

Value *CGConditionVectorEmitterAVX::emitDefineTreeNodeValues() {
  Value *compareValues = Builder.Insert(
      new AllocaInst(FloatTy, AvxPackSizeVal, AvxAlignment), "compareValues");

  uint8_t bitOffset = 0;
  for (DecisionTreeNode node : Nodes) {
    Builder.CreateStore(ConstantFP::get(FloatTy, node.Bias),
                        Builder.CreateConstGEP1_32(compareValues, bitOffset++));
  }

  return compareValues;
}

Value *CGConditionVectorEmitterAVX::emitDefineBitShiftMaskValues() {
  Value *bitShiftValues = Builder.Insert(
      new AllocaInst(Int32Ty, AvxPackSizeVal, AvxAlignment), "bitShiftValues");

  uint8_t numNodes = AvxPackSize - 1;
  for (uint8_t i = 0; i < numNodes; i++) {
    Builder.CreateStore(
        ConstantInt::get(Int32Ty, PowerOf2(i)),
        Builder.CreateConstGEP1_32(bitShiftValues, numNodes - i - 1));
  }

  // AND with 0 in unused last item
  Builder.CreateStore(ConstantInt::get(Int32Ty, 0),
                      Builder.CreateConstGEP1_32(bitShiftValues, numNodes));

  return bitShiftValues;
}

Value *CGConditionVectorEmitterAVX::emitComputeCompareAvx(Value *lhs,
                                                          Value *rhs) {
  Type *avxCmpOpTy = Type::getInt8Ty(Ctx);
  Type *avx8FloatsTy = VectorType::get(FloatTy, AvxPackSize);

  Value *lhsAvxPtr = Builder.CreateBitCast(lhs, avx8FloatsTy->getPointerTo());
  Value *rhsAvxPtr = Builder.CreateBitCast(rhs, avx8FloatsTy->getPointerTo());

  Function *avxCmpFn = Intrinsic::getDeclaration(Session.Module.get(),
                                                 Intrinsic::x86_avx_cmp_ps_256);

  ArrayRef<Value *> avxCmpArgs{
      Builder.CreateAlignedLoad(lhsAvxPtr, AvxAlignment),
      Builder.CreateAlignedLoad(rhsAvxPtr, AvxAlignment),
      ConstantInt::get(avxCmpOpTy, 14) // _CMP_GT_OS
  };

  return Builder.CreateCall(avxCmpFn, avxCmpArgs);
}

Value *CGConditionVectorEmitterAVX::emitComputeBitShiftsAvx(
    Value *avxPackedCmpResults) {
  Type *avx8IntsTy = VectorType::get(Int32Ty, AvxPackSize);
  Value *bitShiftValues = emitDefineBitShiftMaskValues();

  Value *avxBitShiftIntsPtr =
      Builder.CreateBitCast(bitShiftValues, avx8IntsTy->getPointerTo());

  Value *avxBitShiftInts =
      Builder.CreateAlignedLoad(avxBitShiftIntsPtr, AvxAlignment);

  Value *avxCmpResInts = Builder.CreateBitCast(avxPackedCmpResults, avx8IntsTy);
  return Builder.CreateAnd(avxCmpResInts, avxBitShiftInts);
}

Value *
CGConditionVectorEmitterAVX::emitComputeHorizontalOrAvx(Value *avxPackedInts) {
  Type *avx8IntsTy = VectorType::get(Int32Ty, AvxPackSize);
  Type *avx4IntsTy = VectorType::get(Int32Ty, AvxPackSize / 2);

  Value *no8Ints = UndefValue::get(avx8IntsTy);
  Value *no4Ints = UndefValue::get(avx4IntsTy);

  Value *avxOr_04_15_26_37 = Builder.CreateOr(
      Builder.CreateShuffleVector(avxPackedInts, no8Ints, {0, 1, 2, 3}),
      Builder.CreateShuffleVector(avxPackedInts, no8Ints, {4, 5, 6, 7}));

  Value *avxOr_0145_15_2367_37 = Builder.CreateOr(
      Builder.CreateShuffleVector(avxOr_04_15_26_37, no4Ints, {1, 1, 3, 3}),
      avxOr_04_15_26_37);

  return Builder.CreateOr(
      Builder.CreateExtractElement(avxOr_0145_15_2367_37, 0ull),
      Builder.CreateExtractElement(avxOr_0145_15_2367_37, 2ull));
}
