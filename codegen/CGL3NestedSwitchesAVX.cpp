#include "codegen/CGL3NestedSwitchesAVX.h"
#include "codegen/CGConditionVectorEmitter.h"
#include "codegen/CGConditionVectorVariationsBuilder.h"
#include "codegen/CGEvaluationPathsBuilder.h"

using namespace llvm;

std::vector<CGNodeInfo> CGL3NestedSwitchesAVX::emitSubtreeEvaluation(
    CGSubtreeInfo subtreeInfo, Value *dataSetPtr) {
  DecisionSubtreeRef subtreeRef =
      Driver.DecisionTreeData.getSubtreeRef(subtreeInfo.Root.Index, Levels);

  CGConditionVectorEmitterAVX conditionVectorEmitter(&Driver, subtreeRef);
  Value *conditionVector = conditionVectorEmitter(dataSetPtr);

  auto *returnBB = makeSwitchBB(subtreeInfo, "return");
  auto *defaultBB = makeSwitchBB(subtreeInfo, "default");

  auto expectedCaseLabels = PowerOf2<uint32_t>(subtreeRef.getNodeCount() - 1);

  Builder.SetInsertPoint(subtreeInfo.OwnerBB);
  SwitchInst *switchInst = Builder.CreateSwitch(
      conditionVector, defaultBB, expectedCaseLabels);

  CGEvaluationPathsBuilder pathBuilder(subtreeRef);
  std::vector<CGEvaluationPath> evaluationPaths = pathBuilder.run();

  const std::vector<CGNodeInfo> continuationNodes =
      emitSwitchTargets(subtreeRef, evaluationPaths,
                        subtreeInfo.OwnerFunction, returnBB);

  uint32_t emittedCaseLabels = 0;
  CGConditionVectorVariationsBuilder variantsBuilder(subtreeRef);

  for (size_t i = 0; i < continuationNodes.size(); i++) {
    std::vector<uint32_t> pathCaseValues =
        variantsBuilder.run(std::move(evaluationPaths[i]));

    emittedCaseLabels += emitSwitchCaseLabels(
        switchInst, conditionVector->getType(),
        continuationNodes[i], std::move(pathCaseValues));
  }

  assert(emittedCaseLabels == expectedCaseLabels);

  defaultBB->moveAfter(continuationNodes.back().EvalBlock);
  returnBB->moveAfter(defaultBB);

  Builder.SetInsertPoint(defaultBB);
  Builder.CreateBr(returnBB);

  return continuationNodes;
}

std::vector<CGNodeInfo> CGL3NestedSwitchesAVX::emitSwitchTargets(
    DecisionSubtreeRef subtreeRef,
    const std::vector<CGEvaluationPath> &evaluationPaths,
    Function *ownerFunction, BasicBlock *returnBB) {
  std::vector<CGNodeInfo> continuationNodes;

  for (const CGEvaluationPath &path : evaluationPaths) {
    uint64_t idx = path.getContinuationNode().NodeIdx;
    std::string label = "n" + std::to_string(idx);
    BasicBlock *BB = BasicBlock::Create(Ctx, label, ownerFunction);

    continuationNodes.emplace_back(idx, BB, returnBB);
  }

  return continuationNodes;
}

uint32_t CGL3NestedSwitchesAVX::emitSwitchCaseLabels(
    SwitchInst *switchInst, Type *switchCondTy,
    CGNodeInfo targetNodeInfo, std::vector<uint32_t> pathCaseValues) {
  IntegerType *switchCondIntTy =
      IntegerType::get(Ctx, switchCondTy->getIntegerBitWidth());

  for (uint32_t variant : pathCaseValues) {
    ConstantInt *caseVal = ConstantInt::get(switchCondIntTy, variant);
    switchInst->addCase(caseVal, targetNodeInfo.EvalBlock);
  }

  return pathCaseValues.size();
}

BasicBlock *CGL3NestedSwitchesAVX::makeSwitchBB(
    CGSubtreeInfo subtreeInfo, std::string suffix) {
  auto l = "switch" + std::to_string(subtreeInfo.Root.Index) + "_" + suffix;
  return BasicBlock::Create(Ctx, std::move(l), subtreeInfo.OwnerFunction);
}
