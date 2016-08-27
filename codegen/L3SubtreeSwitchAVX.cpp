#include "codegen/L3SubtreeSwitchAVX.h"

#include "codegen/CGConditionVectorEmitter.h"
#include "codegen/CGConditionVectorVariationsBuilder.h"
#include "codegen/CGEvaluationPathsBuilder.h"
#include "codegen/LXSubtreeSwitch.h"
#include "resolver/CompilerSession.h"

using namespace llvm;

CGBase *L3SubtreeSwitchAVX::getFallbackCG() {
  if (!FallbackCGL2)
    FallbackCGL2 = std::make_unique<LXSubtreeSwitch>(Ctx);

  return FallbackCGL2.get();
}

std::vector<CGNodeInfo> L3SubtreeSwitchAVX::emitSubtreeEvaluation(
    const CompilerSession &session, CGNodeInfo subtreeRoot) {
  DecisionSubtreeRef subtreeRef =
      session.Tree.getSubtreeRef(subtreeRoot.Index, Levels);

  CGConditionVectorEmitterAVX conditionVectorEmitter(session, subtreeRef);
  Value *conditionVector = conditionVectorEmitter.run(subtreeRoot);

  auto *returnBB = makeSwitchBB(subtreeRoot, "return");
  auto *defaultBB = makeSwitchBB(subtreeRoot, "default");

  auto expectedCaseLabels = PowerOf2<uint32_t>(subtreeRef.getNodeCount());

  session.Builder.SetInsertPoint(subtreeRoot.EvalBlock);
  SwitchInst *switchInst = session.Builder.CreateSwitch(
      conditionVector, defaultBB, expectedCaseLabels);

  CGEvaluationPathsBuilder pathBuilder(subtreeRef);
  std::vector<CGEvaluationPath> evaluationPaths = pathBuilder.run();

  const std::vector<CGNodeInfo> continuationNodes =
      emitSwitchTargets(subtreeRef, evaluationPaths,
                        subtreeRoot.OwnerFunction, returnBB);

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
  session.Builder.SetInsertPoint(defaultBB);
  session.Builder.CreateUnreachable();
  session.Builder.CreateBr(returnBB);

  returnBB->moveAfter(defaultBB);
  session.Builder.SetInsertPoint(returnBB);
  session.Builder.CreateBr(subtreeRoot.ContinuationBlock);

  return continuationNodes;
}

std::vector<CGNodeInfo> L3SubtreeSwitchAVX::emitSwitchTargets(
    DecisionSubtreeRef subtreeRef,
    const std::vector<CGEvaluationPath> &evaluationPaths,
    Function *ownerFunction, BasicBlock *returnBB) {
  std::vector<CGNodeInfo> continuationNodes;

  for (const CGEvaluationPath &path : evaluationPaths) {
    uint64_t idx = path.getContinuationNode().NodeIdx;
    std::string label = "n" + std::to_string(idx);
    BasicBlock *BB = BasicBlock::Create(Ctx, label, ownerFunction);

    continuationNodes.emplace_back(idx, ownerFunction, BB, returnBB);
  }

  return continuationNodes;
}

uint32_t L3SubtreeSwitchAVX::emitSwitchCaseLabels(
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

BasicBlock *L3SubtreeSwitchAVX::makeSwitchBB(
    CGNodeInfo subtreeRoot, std::string suffix) {
  auto l = "switch" + std::to_string(subtreeRoot.Index) + "_" + suffix;
  return BasicBlock::Create(Ctx, std::move(l), subtreeRoot.OwnerFunction);
}
