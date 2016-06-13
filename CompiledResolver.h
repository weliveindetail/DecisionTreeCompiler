#pragma once

#include <unordered_map>
#include <string>

#include "llvm/ExecutionEngine/GenericValue.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"

#include "SimpleOrcJit.h"
#include "DecisionTree.h"

llvm::LLVMContext Ctx;
llvm::IRBuilder<> Builder(Ctx);
std::unique_ptr<llvm::Module> TheModule;
std::unique_ptr<SimpleOrcJit> TheCompiler;

using compiledNodeEvaluator_f = unsigned long(const float*);
using compiledNodeEvaluatorsMap_t =
    std::unordered_map<unsigned long, compiledNodeEvaluator_f*>;

compiledNodeEvaluatorsMap_t compiledNodeEvaluators;

std::unique_ptr<llvm::Module> setupModule()
{
    auto module = std::make_unique<llvm::Module>("test", Ctx);

    auto* targetMachine = llvm::EngineBuilder().selectTarget();
    module->setDataLayout(targetMachine->createDataLayout());

    return module;
}

std::unique_ptr<SimpleOrcJit> setupCompiler()
{
    auto* targetMachine = llvm::EngineBuilder().selectTarget();
    return std::make_unique<SimpleOrcJit>(*targetMachine);
}

void initializeLLVM() {
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();
    TheCompiler = setupCompiler();
    TheModule = setupModule();
}

void shutdownLLVM() {
    TheModule.reset();
    TheCompiler.reset();
    llvm::llvm_shutdown();
}

void compileEvaluators(const DecisionTree& tree) {
    using namespace llvm;

    std::string nameStub = "nodeEvaluator_";

    // emit an evaluator function per node
    for (const auto& entry : tree) {
        unsigned long idx = entry.first;
        const TreeNode& node = entry.second;

        // declare function
        auto name = nameStub + std::to_string(idx);
        auto returnTy = Type::getInt64Ty(Ctx);
        auto argTy = Type::getFloatTy(Ctx)->getPointerTo();
        auto signature = FunctionType::get(returnTy, { argTy }, false);
        auto linkage = Function::ExternalLinkage;

        Function *evalFn = Function::Create(signature, linkage, name, TheModule.get());
        evalFn->setName(name);

        // todo: emit function definition

        llvm::verifyFunction(*evalFn);
    }

    // submit for jit compilation
    TheCompiler->submitModule(std::move(TheModule));

    // collect evaluators
    for (const auto& entry : tree)
        compiledNodeEvaluators[entry.first] =
            TheCompiler->getEvaluatorFnPtr(nameStub + std::to_string(entry.first));
}

template<unsigned long DataSetFeatures_>
unsigned long computeLeafNodeIdxForDataSetCompiled(
        const DecisionTree& tree,
        const std::array<float, DataSetFeatures_>& dataSet) {
    unsigned long treeNodeIdx = 0;

    while (!tree.at(treeNodeIdx).isLeaf()) {
        auto compiledEvaluator = compiledNodeEvaluators[treeNodeIdx];
        treeNodeIdx = compiledEvaluator(dataSet.data());
    }

    return treeNodeIdx;
}