#pragma once

#include <memory>
#include <string>
#include <system_error>

#include <llvm/Bitcode/ReaderWriter.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/Path.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/Transforms/IPO/PassManagerBuilder.h>
#include <llvm/IR/LegacyPassManager.h>

#include "codegen/CodeGeneratorSelector.h"
#include "codegen/L1IfThenElse.h"
#include "codegen/LXSubtreeSwitch.h"
#include "codegen/L3SubtreeSwitchAVX.h"
#include "compiler/DecisionTreeCompiler.h"
#include "data/DecisionTree.h"
#include "driver/utility/AutoSetUpTearDownLLVM.h"

class StaticDriver {
public:
  StaticDriver() : Compiler(LLVM.getTargetMachine()) {}

  void run() {
    if (!llvm::sys::fs::is_regular_file(InputFileName)) {
      llvm::errs() << "Cannot read decision tree from file ";
      llvm::errs() << InputFileName << "\n";
      llvm::errs() << "Aborting\n";
      return;
    }

    // todo: read input file
    DecisionTreeFactory treeFactory;
    DecisionTree decisionTree = treeFactory.makePerfectDistinctUniformTree(3);

    if (isOutputFileSpecified()) {
      llvm::outs() << "Compiling decision tree from file ";
      llvm::outs() << InputFileName << "..\n";

      if (Debug) {
        llvm::outs() << "Debug output enabled\n";
      }
    }

    if (DefaultCodegen && FallbackCodegen) {
      Compiler.setCodegenSelector(makeLambdaSelector(
        [this](const CompilerSession &session, int remainingLevels) {
          if (remainingLevels >= DefaultCodegen->getJointSubtreeDepth())
            return DefaultCodegen.get();
          else
            return FallbackCodegen.get();
        }));
    } else {
      Compiler.setCodegenSelector(std::make_shared<DefaultSelector>());
    }

    CompileResult result =
        Compiler.compile(std::move(decisionTree));

    // todo: respect actual levels when optimizing
    if (OptimizationLevel > 0) {
      runStandardOptimizations(result.Module.get());
    }

    if (isOutputFileSpecified()) {
      int FD;
      std::string uniqueName;

      std::string fileName = getOutputFileName();
      if (std::error_code EC = openOutputFile(fileName, FD, uniqueName)) {
        llvm::errs() << "Cannot open output file ";
        llvm::errs() << fileName << " for writing\n";
        llvm::errs() << "Aborting\n";
        return;
      }

      llvm::outs() << "Writing compiled module as ";
      llvm::outs() << (WriteAsBitcode ? "bitcode" : "human-readable") << " IR ";
      llvm::outs() << "to file " << uniqueName << "\n";

      result.Module->setModuleIdentifier(uniqueName);

      constexpr bool autoClose = true;
      llvm::raw_fd_ostream outfile(FD, autoClose);
      writeModuleToStream(outfile, result.Module.get());
    }
    else {
      writeModuleToStream(llvm::outs(), result.Module.get());
    }
  }

  void enableDebug() { Debug = true; }
  void setOutputFormatText() { WriteAsBitcode = false; }
  void setOptimizerLevel(int level) { OptimizationLevel = level; }

  void setCodeGeneratorL1IfThenElse() {
    DefaultCodegen = std::make_unique<L1IfThenElse>();
    FallbackCodegen = std::make_unique<L1IfThenElse>();
  }

  void setCodeGeneratorL2SubtreeSwitch() {
    DefaultCodegen = std::make_unique<LXSubtreeSwitch>(2);
    FallbackCodegen = std::make_unique<L1IfThenElse>();
  }

  void setCodeGeneratorL3SubtreeSwitchAVX() {
    DefaultCodegen = std::make_unique<L3SubtreeSwitchAVX>();
    FallbackCodegen = std::make_unique<L1IfThenElse>();
  }

  void setOutputFileName(std::string fileName) {
    OutputFileName = std::move(fileName);
  }

  void setInputFileName(std::string fileName) {
    InputFileName = std::move(fileName);
  }

  bool isConfigurationComplete() const { return !InputFileName.empty(); }
  bool isOutputFileSpecified() const { return !OutputFileName.empty(); }

private:
  AutoSetUpTearDownLLVM LLVM;
  DecisionTreeCompiler Compiler;

  std::unique_ptr<CodeGenerator> DefaultCodegen;
  std::unique_ptr<CodeGenerator> FallbackCodegen;
  std::string InputFileName;
  std::string OutputFileName;
  int OptimizationLevel = 0;
  bool WriteAsBitcode = true;
  bool Debug = false;

  void runStandardOptimizations(llvm::Module *module) {
    llvm::PassManagerBuilder PMBuilder;
    PMBuilder.BBVectorize = true;
    PMBuilder.SLPVectorize = true;
    PMBuilder.VerifyInput = true;
    PMBuilder.VerifyOutput = true;

    llvm::legacy::FunctionPassManager perFunctionPasses(module);
    PMBuilder.populateFunctionPassManager(perFunctionPasses);

    perFunctionPasses.doInitialization();

    for (llvm::Function &function : *module)
      perFunctionPasses.run(function);

    perFunctionPasses.doFinalization();

    llvm::legacy::PassManager perModulePasses;
    PMBuilder.populateModulePassManager(perModulePasses);
    perModulePasses.run(*module);
  }

  std::error_code openOutputFile(std::string fileName, int &resultFD,
                                 std::string &resultName) {
    using namespace llvm::sys;
    llvm::StringRef dir = path::parent_path(fileName);

    if (!dir.empty())
      if (auto EC = fs::create_directories(dir))
        return EC;

    int FD;
    fs::OpenFlags flags = WriteAsBitcode ? fs::F_RW : fs::F_Text;

    if (auto EC = fs::openFileForWrite(fileName, FD, flags))
      return EC;

    resultFD = FD;
    resultName = std::move(fileName);
    return std::error_code();
  }

  void writeModuleToStream(llvm::raw_ostream &out, llvm::Module *module) {
    if (WriteAsBitcode)
      llvm::WriteBitcodeToFile(module, out);
    else
      out << *module;
  }

  std::string getOutputFileName() const {
    if (OutputFileName.empty()) {
      std::string fileName = ensureOutputFileExt(InputFileName);
      if (llvm::sys::fs::exists(fileName))
        return deduplicateFileName(std::move(fileName));
    }

    return ensureOutputFileExt(OutputFileName);
  }

  std::string ensureOutputFileExt(std::string fileName) const {
    std::string ext = WriteAsBitcode ? ".bc" : ".ll";

    // correct extension?
    if (fileName.substr(fileName.size() - ext.size()) == ext)
      return fileName;

    // no extension?
    size_t lastDot = fileName.find_last_of('.');
    if (lastDot == std::string::npos)
      return fileName + ext;

    // wrong extension
    return fileName.substr(0, lastDot) + ext;
  }

  std::string deduplicateFileName(std::string fileName) const {
    // add deduplication counter before dot
    size_t lastDot = fileName.find_last_of('.');
    std::string front, back;

    if (lastDot == std::string::npos) {
      front = fileName + " (";
      back = ")";
    } else {
      front = fileName.substr(0, lastDot) + " (";
      back = ")" + fileName.substr(lastDot);
    }

    for (int c = 0; llvm::sys::fs::exists(fileName); c++) {
      fileName = front + std::to_string(c) + back;
    }

    return fileName;
  }
};
