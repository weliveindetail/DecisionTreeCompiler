#include "codegen/L3SubtreeSwitchAVX.h"

#include "codegen/utility/CGConditionVectorEmitter.h"
#include "codegen/utility/CGConditionVectorVariationsBuilder.h"
#include "codegen/utility/CGEvaluationPathsBuilder.h"
#include "compiler/CompilerSession.h"

using namespace llvm;

std::vector<CGNodeInfo>
L3SubtreeSwitchAVX::emitSubtreeEvaluation(CGNodeInfo subtreeRoot,
                                          const CompilerSession &session) {
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

  std::vector<CGNodeInfo> continuationNodes = emitSwitchTargets(
      evaluationPaths, subtreeRoot.OwnerFunction, returnBB);

  assert(continuationNodes.size() == evaluationPaths.size());

  uint32_t emittedCaseLabels = 0;
  CGConditionVectorVariationsBuilder variantsBuilder(subtreeRef);

  for (size_t i = 0; i < continuationNodes.size(); i++) {
    std::vector<uint32_t> pathCaseValues =
        variantsBuilder.run(std::move(evaluationPaths[i]));

    emittedCaseLabels +=
        emitSwitchCaseLabels(switchInst, conditionVector->getType(),
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
    const std::vector<CGEvaluationPath> &evaluationPaths,
    Function *ownerFunction, BasicBlock *returnBB) {
  std::vector<CGNodeInfo> continuationNodes;

  // create a continuation node-info for each path endpoint
  for (const CGEvaluationPath &path : evaluationPaths) {
    uint64_t idx = path.getDestNode().getIdx();
    std::string label = "n" + std::to_string(idx);
    BasicBlock *BB = BasicBlock::Create(Ctx, label, ownerFunction);

    continuationNodes.emplace_back(idx, ownerFunction, BB, returnBB);
  }

  return continuationNodes;
}

uint32_t L3SubtreeSwitchAVX::emitSwitchCaseLabels(
    SwitchInst *switchInst, Type *switchCondTy, CGNodeInfo targetNodeInfo,
    std::vector<uint32_t> pathCaseValues) {
  IntegerType *switchCondIntTy =
      IntegerType::get(Ctx, switchCondTy->getIntegerBitWidth());

  for (uint32_t variant : pathCaseValues) {
    ConstantInt *caseVal = ConstantInt::get(switchCondIntTy, variant);
    switchInst->addCase(caseVal, targetNodeInfo.EvalBlock);
  }

  return pathCaseValues.size();
}

BasicBlock *L3SubtreeSwitchAVX::makeSwitchBB(CGNodeInfo subtreeRoot,
                                             std::string suffix) {
  auto lbl = "switch" + std::to_string(subtreeRoot.Index) + "_" + suffix;
  return BasicBlock::Create(Ctx, std::move(lbl), subtreeRoot.OwnerFunction);
}
