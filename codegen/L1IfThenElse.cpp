#include "codegen/L1IfThenElse.h"

using namespace llvm;

std::vector<CGNodeInfo>
L1IfThenElse::emitEvaluation(CGNodeInfo nodeInfo) {
  DecisionSubtreeRef subtree = Session.Tree.getSubtreeRef(nodeInfo.Index, 2);
  switch (subtree.getNodeCount()) {
    case 1:
      assert(subtree.Root.isLeaf());
      return {std::move(nodeInfo)};

    case 2:
      return emitSingleChildForward(std::move(nodeInfo), std::move(subtree));

    case 3:
      return emitConditionalBranch(std::move(nodeInfo), std::move(subtree));
  }

  llvm_unreachable("2-level subtree must have 1, 2 or 3 nodes");
}

std::vector<CGNodeInfo>
L1IfThenElse::emitSingleChildForward(CGNodeInfo nodeInfo,
                                     DecisionSubtreeRef subtree) {
  assert(subtree.Root.hasLeftChild() != subtree.Root.hasRightChild());

  NodeEvaluation onliestEval = subtree.Root.hasLeftChild()
                             ? NodeEvaluation::ContinueZeroLeft
                             : NodeEvaluation::ContinueOneRight;

  DecisionTreeNode onliestChild =
      subtree.Root.getChildFor(onliestEval, subtree);

  return {CGNodeInfo(onliestChild.getIdx(), nodeInfo.OwnerFunction,
                     nodeInfo.EvalBlock, nodeInfo.ContinuationBlock)};
}

std::vector<CGNodeInfo>
L1IfThenElse::emitConditionalBranch(CGNodeInfo nodeInfo,
                                    DecisionSubtreeRef subtree) {
  std::vector<CGNodeInfo> continuationNodes;

  LLVMContext &ctx = Session.Builder.getContext();

  auto *leftChildBB = makeIfThenElseBB(nodeInfo, "left");
  auto *rightChildBB = makeIfThenElseBB(nodeInfo, "right");
  auto *mergeBB = makeIfThenElseBB(nodeInfo, "merge");

  Session.Builder.SetInsertPoint(nodeInfo.EvalBlock);

  Value *featureVal = emitLoadFeatureValue(subtree.Root);
  Type *floatTy = Type::getFloatTy(ctx);
  Constant *biasVal = ConstantFP::get(floatTy, subtree.Root.getFeatureBias());
  Value *cmpResult = Session.Builder.CreateFCmpOGT(featureVal, biasVal);

  Session.Builder.CreateCondBr(cmpResult, rightChildBB, leftChildBB);

  Session.Builder.SetInsertPoint(mergeBB);
  Session.Builder.CreateBr(nodeInfo.ContinuationBlock);

  DecisionTreeNode leftChild =
      subtree.Root.getChildFor(NodeEvaluation::ContinueZeroLeft, subtree);

  continuationNodes.emplace_back(leftChild.getIdx(), nodeInfo.OwnerFunction,
                                 leftChildBB, mergeBB);

  DecisionTreeNode rightChild =
      subtree.Root.getChildFor(NodeEvaluation::ContinueOneRight, subtree);

  continuationNodes.emplace_back(rightChild.getIdx(), nodeInfo.OwnerFunction,
                                 rightChildBB, mergeBB);

  return continuationNodes;
}

Value *
L1IfThenElse::emitLoadFeatureValue(DecisionTreeNode node) {
  Value *dataSetFeaturePtr = Session.Builder.CreateConstGEP1_32(
      Session.InputDataSetPtr, node.getFeatureIdx());

  return Session.Builder.CreateLoad(dataSetFeaturePtr);
}

BasicBlock *L1IfThenElse::makeIfThenElseBB(CGNodeInfo nodeInfo,
                                           std::string suffix) {
  std::string name = "n" + std::to_string(nodeInfo.Index) + suffix;
  return BasicBlock::Create(Ctx, name, nodeInfo.OwnerFunction);
}
