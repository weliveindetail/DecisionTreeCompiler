#include "resolver/Driver.h"

#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/Support/ManagedStatic.h>
#include <llvm/Support/TargetSelect.h>

#include "codegen/CGL1IfThenElse.h"
#include "codegen/CGL2NestedSwitches.h"
#include "codegen/CGL3NestedSwitchesAVX.h"

#include "resolver/CompilerSession.h"

using namespace llvm;

CompileResult DecisionTreeCompiler::compile(
    CodeGeneratorType codegenType, DecisionTree tree) {
  std::unique_ptr<CGBase> codegen = makeCodeGenerator(codegenType);
  CompilerSession session(this, std::move(tree), std::move(codegen), "sessionName");

  CGNodeInfo root = makeEvalRoot(session, "EvaluatorFunction");

  session.Builder.SetInsertPoint(root.EvalBlock);
  session.OutputNodeIdxPtr = allocOutputVal(session);
  session.InputDataSetPtr = &*root.OwnerFunction->arg_begin();

  // todo: invoke codegen, finalize

  CompileResult result;
  result.Tree = std::move(session.Tree);
  result.Module = std::move(session.Module);
  result.EvaluatorFunctionName = root.OwnerFunction->getName();

  return result;
}

std::unique_ptr<CGBase> DecisionTreeCompiler::makeCodeGenerator(
    CodeGeneratorType type) {
  switch (type) {
    case CodeGeneratorType::L1IfThenElse:
      return std::make_unique<CGL1IfThenElse>(Ctx);

    case CodeGeneratorType::LXSubtreeSwitch:
      return std::make_unique<CGL2NestedSwitches>(Ctx);

    case CodeGeneratorType::L3SubtreeSwitchAVX:
      return std::make_unique<CGL3NestedSwitchesAVX>(Ctx);
  }
}

CGNodeInfo DecisionTreeCompiler::makeEvalRoot(CompilerSession& session,
                                              std::string functionName) {
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

FunctionType *DecisionTreeCompiler::getEvalFunctionTy(
    const CompilerSession &session) {
  Type *returnTy = session.NodeIdxTy;
  Type *argTy = session.DataSetFeatureValueTy->getPointerTo();
  return FunctionType::get(returnTy, {argTy}, false);
}

Function *DecisionTreeCompiler::emitEvalFunctionDecl(
    std::string name, FunctionType *signature, Module *module) {
  Function *evalFn = Function::Create(
      signature, Function::ExternalLinkage, name, module);

  AttributeSet attributeSet;
  evalFn->setAttributes(attributeSet.addAttribute(
      Ctx, AttributeSet::FunctionIndex, "target-features", "+avx"));

  evalFn->setName(name);
  return evalFn;
}

Value *DecisionTreeCompiler::allocOutputVal(const CompilerSession &session) {
  Value *ptr = session.Builder.CreateAlloca(session.NodeIdxTy,
                                            nullptr, "result");

  Constant *initVal = ConstantInt::get(session.NodeIdxTy, 0);
  session.Builder.CreateStore(initVal, ptr);
  return ptr;
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
std::atomic<int> DecisionTreeCompiler::AutoSetUpTearDownLLVM::instances {0};
