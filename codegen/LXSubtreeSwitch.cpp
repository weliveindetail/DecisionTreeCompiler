#include "codegen/LXSubtreeSwitch.h"

#include <llvm/IR/GlobalVariable.h>

#include "codegen/utility/CGConditionVectorEmitter.h"
#include "codegen/utility/CGConditionVectorVariationsBuilder.h"
#include "codegen/utility/CGEvaluationPathsBuilder.h"
#include "compiler/CompilerSession.h"
#include "data/DecisionSubtreeRef.h"

using namespace llvm;

std::vector<CGNodeInfo>
LXSubtreeSwitch::emitEvaluation(const CompilerSession &session,
                                CGNodeInfo subtreeRoot) {
  DecisionSubtreeRef subtreeRef =
      session.Tree.getSubtreeRef(subtreeRoot.Index, Levels);

  Value *conditionVector = emitConditionVector(session, subtreeRef, subtreeRoot);
  LLVMContext &ctx = session.Builder.getContext();

  auto *returnBB = makeSwitchBB(ctx, subtreeRoot, "return");
  auto *defaultBB = makeSwitchBB(ctx, subtreeRoot, "default");

  auto expectedCaseLabels = PowerOf2<uint32_t>(subtreeRef.getNodeCount());

  session.Builder.SetInsertPoint(subtreeRoot.EvalBlock);
  SwitchInst *switchInst = session.Builder.CreateSwitch(
      conditionVector, defaultBB, expectedCaseLabels);

  CGEvaluationPathsBuilder pathBuilder(subtreeRef);
  std::vector<CGEvaluationPath> evaluationPaths = pathBuilder.run();

  std::vector<CGNodeInfo> continuationNodes = emitSwitchTargets(
      ctx, evaluationPaths, subtreeRoot.OwnerFunction, returnBB);

  assert(continuationNodes.size() == evaluationPaths.size());

  uint32_t emittedCaseLabels = 0;
  CGConditionVectorVariationsBuilder variantsBuilder(subtreeRef);

  for (size_t i = 0; i < continuationNodes.size(); i++) {
    std::vector<uint32_t> pathCaseValues =
        variantsBuilder.run(std::move(evaluationPaths[i]));

    emittedCaseLabels +=
        emitSwitchCaseLabels(ctx, switchInst, conditionVector->getType(),
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

Value *LXSubtreeSwitch::emitLeafEvaluation(const CompilerSession &session,
                                           CGNodeInfo subtreeRoot) {
  DecisionSubtreeRef subtreeRef =
      session.Tree.getSubtreeRef(subtreeRoot.Index, Levels);

  CGEvaluationPathsBuilder pathBuilder(subtreeRef);
  std::vector<CGEvaluationPath> evaluationPaths = pathBuilder.run();

  auto expectedSwitchCases = PowerOf2<uint32_t>(subtreeRef.getNodeCount());

  std::vector<uint64_t> data = collectSwitchTableData(
      subtreeRef, std::move(evaluationPaths));

  assert(data.size() == expectedSwitchCases);
  Constant *switchTable = emitSwitchTable(session, std::move(data));

  session.Builder.SetInsertPoint(subtreeRoot.EvalBlock);
  Value *conditionVector = emitConditionVector(session, subtreeRef, subtreeRoot);

  Constant *ptrDeref = ConstantInt::get(conditionVector->getType(), 0);
  Value *returnValPtr = session.Builder.CreateGEP(switchTable,
                                                  {ptrDeref, conditionVector});

  return session.Builder.CreateLoad(returnValPtr);
}

std::vector<CGNodeInfo> LXSubtreeSwitch::emitSwitchTargets(
    LLVMContext &ctx, const std::vector<CGEvaluationPath> &evaluationPaths,
    Function *ownerFunction, BasicBlock *returnBB) {
  std::vector<CGNodeInfo> continuationNodes;

  // create a continuation node-info for each path endpoint
  for (const CGEvaluationPath &path : evaluationPaths) {
    uint64_t idx = path.getDestNode().getIdx();
    std::string label = "n" + std::to_string(idx);
    BasicBlock *BB = BasicBlock::Create(ctx, label, ownerFunction);

    continuationNodes.emplace_back(idx, ownerFunction, BB, returnBB);
  }

  return continuationNodes;
}

uint32_t LXSubtreeSwitch::emitSwitchCaseLabels(
    LLVMContext &ctx, SwitchInst *switchInst, Type *switchCondTy,
    CGNodeInfo targetNodeInfo, std::vector<uint32_t> pathCaseValues) {
  IntegerType *switchCondIntTy =
      IntegerType::get(ctx, switchCondTy->getIntegerBitWidth());

  for (uint32_t variant : pathCaseValues) {
    ConstantInt *caseVal = ConstantInt::get(switchCondIntTy, variant);
    switchInst->addCase(caseVal, targetNodeInfo.EvalBlock);
  }

  return pathCaseValues.size();
}

Constant *LXSubtreeSwitch::emitSwitchTable(const CompilerSession &session,
                                           std::vector<uint64_t> data) {
  LLVMContext &ctx = session.Builder.getContext();

  Type *tableTy = ArrayType::get(session.NodeIdxTy, data.size());
  Constant *init = ConstantDataArray::get(ctx, std::move(data));

  return new GlobalVariable(*session.Module.get(),
                            tableTy, true,
                            GlobalVariable::PrivateLinkage,
                            init, "switchTable");
}

std::vector<uint64_t> LXSubtreeSwitch::collectSwitchTableData(
    DecisionSubtreeRef subtreeRef,
    std::vector<CGEvaluationPath> evaluationPaths) {
  size_t tableSize = PowerOf2<size_t>(subtreeRef.getNodeCount());

  std::vector<uint64_t> data(tableSize, 0);
  CGConditionVectorVariationsBuilder variantsBuilder(subtreeRef);

  for (CGEvaluationPath path : evaluationPaths) {
    uint64_t destNodeIdx = path.getDestNode().getIdx();
    std::vector<uint32_t> caseValues = variantsBuilder.run(std::move(path));

    for (uint32_t val : caseValues) {
      assert(data.at(val) == 0);
      data[val] = destNodeIdx;
    }
  }

  assert(data.size() == tableSize);
  return data;
}

BasicBlock *LXSubtreeSwitch::makeSwitchBB(LLVMContext &ctx,
                                          CGNodeInfo subtreeRoot,
                                          std::string suffix) {
  auto lbl = "switch" + std::to_string(subtreeRoot.Index) + "_" + suffix;
  return BasicBlock::Create(ctx, std::move(lbl), subtreeRoot.OwnerFunction);
}
