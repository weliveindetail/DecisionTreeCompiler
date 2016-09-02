#include "DecisionTreeCompiler.h"

#include <llvm/ADT/StringExtras.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/Host.h>

#include "codegen/CodeGenerator.h"
#include "codegen/CodeGeneratorSelector.h"
#include "compiler/CompilerSession.h"

using namespace llvm;

DecisionTreeCompiler::DecisionTreeCompiler(TargetMachine *target)
  : Target(target), CodegenSelector(nullptr) {
  llvm::sys::getHostCPUFeatures(CpuFeatures);
}

void DecisionTreeCompiler::setCodegenSelector(
      std::shared_ptr<CodeGeneratorSelector> codegenSelector) {
  CodegenSelector = codegenSelector;
  CodegenSelector->AvxSupport = CpuFeatures["avx"];
}

CompileResult DecisionTreeCompiler::compile(DecisionTree tree) {
  if (CodegenSelector == nullptr)
    setCodegenSelector(std::make_shared<DefaultSelector>());

  CompilerSession session(this, Target, "sessionName");
  session.CodegenSelector = CodegenSelector;
  session.Tree = std::move(tree);

  CGNodeInfo root = makeEvalRoot("EvaluatorFunction", session);

  session.Builder.SetInsertPoint(root.EvalBlock);
  session.OutputNodeIdxPtr = allocOutputVal(session);
  session.InputDataSetPtr = &*root.OwnerFunction->arg_begin();

  std::vector<CGNodeInfo> leafNodes = compileSubtrees(root, session);
  connectSubtreeEndpoints(std::move(leafNodes), session);

  session.Builder.SetInsertPoint(root.ContinuationBlock);
  session.Builder.CreateRet(
      session.Builder.CreateLoad(session.OutputNodeIdxPtr));

  CompileResult result;
  result.Tree = std::move(session.Tree);
  result.Module = std::move(session.Module);
  result.EvaluatorFunctionName = root.OwnerFunction->getName();
  result.Success = verifyFunction(*root.OwnerFunction);

  return result;
}

CGNodeInfo DecisionTreeCompiler::makeEvalRoot(std::string functionName,
                                              const CompilerSession &session) {
  CGNodeInfo root;
  root.Index = session.Tree.getRootNodeIdx();

  Module *module = session.Module.get();
  FunctionType *ty = getEvalFunctionTy(session);
  Function *fn = emitEvalFunctionDecl(functionName, ty, module);

  root.OwnerFunction = fn;
  root.EvalBlock = BasicBlock::Create(Ctx, "entry", fn);
  root.ContinuationBlock = BasicBlock::Create(Ctx, "exit", fn);

  return root;
}

FunctionType *
DecisionTreeCompiler::getEvalFunctionTy(const CompilerSession &session) {
  Type *returnTy = session.NodeIdxTy;
  Type *argTy = session.DataSetFeatureValueTy->getPointerTo();
  return FunctionType::get(returnTy, {argTy}, false);
}

Function *DecisionTreeCompiler::emitEvalFunctionDecl(std::string name,
                                                     FunctionType *signature,
                                                     Module *module) {
  Function *evalFn =
      Function::Create(signature, Function::ExternalLinkage, name, module);

  evalFn->setAttributes(collectEvalFunctionAttribs());
  evalFn->setName(name);
  return evalFn;
}

AttributeSet DecisionTreeCompiler::collectEvalFunctionAttribs() {
  std::vector<std::string> features;
  for (const StringMapEntry<bool> &feature : CpuFeatures) {
    if (feature.getValue())
      features.emplace_back("+" + feature.getKey().str());
  }

  AttributeSet attributeSet;
  if (features.empty())
    return attributeSet;

  std::sort(features.begin(), features.end());
  return attributeSet.addAttribute(
      Ctx, AttributeSet::FunctionIndex, "target-features",
      join(features.begin(), features.end(), ","));
}

Value *DecisionTreeCompiler::allocOutputVal(const CompilerSession &session) {
  Value *ptr =
      session.Builder.CreateAlloca(session.NodeIdxTy, nullptr, "result");

  Constant *initVal = ConstantInt::get(session.NodeIdxTy, 0);
  session.Builder.CreateStore(initVal, ptr);
  return ptr;
}

std::vector<CGNodeInfo>
DecisionTreeCompiler::compileSubtrees(CGNodeInfo rootNode,
                                      const CompilerSession &session) {
  std::vector<CGNodeInfo> nodesNextLevel = {rootNode};
  uint8_t remainingLevels = session.Tree.getNumLevels();

  while (remainingLevels > 0) {
    CodeGenerator *codegen = session.selectCodeGenerator(remainingLevels);
    std::vector<CGNodeInfo> roots = std::move(nodesNextLevel);
    nodesNextLevel.clear();

    for (CGNodeInfo node : roots) {
      std::vector<CGNodeInfo> continuationNodes =
          codegen->emitEvaluation(session, node);

      std::move(continuationNodes.begin(), continuationNodes.end(),
                std::back_inserter(nodesNextLevel));
    }

    remainingLevels -= codegen->getJointSubtreeDepth();
  }

  return nodesNextLevel;
}

void DecisionTreeCompiler::connectSubtreeEndpoints(
    std::vector<CGNodeInfo> evaluatorEndPoints,
    const CompilerSession &session) {
  for (CGNodeInfo node : evaluatorEndPoints) {
    session.Builder.SetInsertPoint(node.EvalBlock);

    Constant *nodeIdxVal = ConstantInt::get(session.NodeIdxTy, node.Index);
    session.Builder.CreateStore(nodeIdxVal, session.OutputNodeIdxPtr);
    session.Builder.CreateBr(node.ContinuationBlock);
  }
}
