#include "DecisionTreeCompiler.h"

#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/ManagedStatic.h>
#include <llvm/Support/TargetSelect.h>

#include "codegen/CGL1IfThenElse.h"
#include "codegen/L3SubtreeSwitchAVX.h"
#include "codegen/LXSubtreeSwitch.h"

#include "CompilerSession.h"

using namespace llvm;

CompileResult DecisionTreeCompiler::compile(CodeGeneratorType codegenType,
                                            DecisionTree tree) {
  std::unique_ptr<CGBase> codegen = makeCodeGenerator(codegenType);
  CompilerSession session(this, std::move(tree), std::move(codegen),
                          "sessionName");

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

std::unique_ptr<CGBase>
DecisionTreeCompiler::makeCodeGenerator(CodeGeneratorType type) {
  switch (type) {
  case CodeGeneratorType::L1IfThenElse:
    return std::make_unique<CGL1IfThenElse>(Ctx);

  case CodeGeneratorType::LXSubtreeSwitch:
    return std::make_unique<LXSubtreeSwitch>(Ctx);

  case CodeGeneratorType::L3SubtreeSwitchAVX:
    return std::make_unique<L3SubtreeSwitchAVX>(Ctx);
  }
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

  AttributeSet attributeSet;
  evalFn->setAttributes(attributeSet.addAttribute(
      Ctx, AttributeSet::FunctionIndex, "target-features", "+avx"));

  evalFn->setName(name);
  return evalFn;
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
    CGBase *codegen = session.selectCodeGenerator(remainingLevels);
    std::vector<CGNodeInfo> roots = std::move(nodesNextLevel);
    nodesNextLevel.clear();

    for (CGNodeInfo node : roots) {
      std::vector<CGNodeInfo> continuationNodes =
          codegen->emitSubtreeEvaluation(node, session);

      std::move(continuationNodes.begin(), continuationNodes.end(),
                std::back_inserter(nodesNextLevel));
    }

    remainingLevels -= codegen->getOptimalJointEvaluationDepth();
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

std::unique_ptr<llvm::Module>
DecisionTreeCompiler::makeModule(std::string name) {
  auto M = std::make_unique<llvm::Module>("file:" + name, Ctx);
  M->setDataLayout(EngineBuilder().selectTarget()->createDataLayout());

  return M;
}

DecisionTreeCompiler::AutoSetUpTearDownLLVM::AutoSetUpTearDownLLVM() {
  int existingInstancesBeforeConstruction = instances.fetch_add(1);
  if (existingInstancesBeforeConstruction == 0) {
    InitializeNativeTarget();
    InitializeNativeTargetAsmPrinter();
    InitializeNativeTargetAsmParser();
  }
}

DecisionTreeCompiler::AutoSetUpTearDownLLVM::~AutoSetUpTearDownLLVM() {
  int existingInstancesBeforeDestruction = instances.fetch_sub(1);
  if (existingInstancesBeforeDestruction == 1) {
    llvm_shutdown();
  }
}

// static inti
std::atomic<int> DecisionTreeCompiler::AutoSetUpTearDownLLVM::instances{0};
