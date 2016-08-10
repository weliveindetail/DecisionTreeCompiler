#include "codegen/CGL3NestedSwitchesAVX.h"

using namespace llvm;

std::vector<CGNodeInfo> CGL3NestedSwitchesAVX::emitSubtreeEvaluation(
    CGSubtreeInfo subtreeInfo, Value *dataSetPtr) {
  auto& Ctx = getDriver()->Ctx;
  auto& Builder = getDriver()->Builder;

  Type *nodeIdxTy = Type::getInt64Ty(Ctx);
  auto numNodes = TreeNodes<uint8_t>(Levels);

  Value *conditionVector =
      emitComputeConditionVector(dataSetPtr, subtreeInfo.Root.Index, Levels);

  std::string returnBBLabel =
      "switch" + std::to_string(subtreeInfo.Root.Index) + "_return";
  auto *returnBB = BasicBlock::Create(Ctx, returnBBLabel, subtreeInfo.OwnerFunction);

  std::string defaultBBLabel =
      "switch" + std::to_string(subtreeInfo.Root.Index) + "_default";
  auto *defaultBB = BasicBlock::Create(Ctx, defaultBBLabel, subtreeInfo.OwnerFunction);

  std::string evalResultLabel =
      "switch_" + std::to_string(subtreeInfo.Root.Index) + "_value";
  //Value *evalResult = Builder.CreateAlloca(nodeIdxTy, nullptr, evalResultLabel);
  //Builder.CreateStore(ConstantInt::get(nodeIdxTy, 0), evalResult);

  auto *switchInst = Builder.CreateSwitch(conditionVector, defaultBB,
                                          PowerOf2<uint32_t>(numNodes - 1));

  DecisionSubtreeRef subtreeRef =
      getDriver()->DecisionTreeData.getSubtreeRef(subtreeInfo.Root.Index, Levels);

  std::vector<DecisionTreeEvaluationPath> evaluationPaths =
      getDriver()->buildSubtreeEvaluationPaths(subtreeRef);

  std::vector<CGNodeInfo> continuationNodes;

  for (auto path : evaluationPaths) {
    BasicBlock *BB = emitSubtreeSwitchTarget(
        subtreeRef, path, subtreeInfo.OwnerFunction, dataSetPtr, returnBB); // evalResult ?

    continuationNodes.emplace_back(path.getTargetIdx(), BB);
  }

  for (size_t i = 0; i < evaluationPaths.size(); i++) {
    emitSubtreeSwitchCaseLabels(subtreeInfo.OwnerBB, switchInst,
                                continuationNodes[i].EvalBlock, numNodes,
                                std::move(evaluationPaths[i]));
  }

  defaultBB->moveAfter(continuationNodes.back().EvalBlock);
  returnBB->moveAfter(defaultBB);

  Builder.SetInsertPoint(defaultBB);
  Builder.CreateBr(returnBB);

  return continuationNodes;
}

Value *CGL3NestedSwitchesAVX::emitComputeConditionVector(
    Value *dataSetPtr, uint64_t subtreeRootIdx, uint8_t subtreeLevels) {
  return nullptr;
}

BasicBlock *CGL3NestedSwitchesAVX::emitSubtreeSwitchTarget(
    DecisionSubtreeRef subtreeRef, DecisionTreeEvaluationPath path,
    Function *ownerFunction, Value *dataSetPtr, BasicBlock *returnBB) {
  return nullptr;
}

void CGL3NestedSwitchesAVX::emitSubtreeSwitchCaseLabels(
    BasicBlock *ownerBB, SwitchInst *switchInst, BasicBlock *continuationBB,
    uint8_t numNodes, DecisionTreeEvaluationPath pathInfo) {

}
