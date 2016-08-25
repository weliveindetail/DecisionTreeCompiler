#pragma once

#include <memory>
#include <string>
#include <system_error>

#include <llvm/Support/FileSystem.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/Path.h>
#include <llvm/Support/raw_ostream.h>

#include "resolver/DecisionTree.h"
#include "resolver/Driver.h"

class StaticDriver {
public:
  void run() {
    if (!llvm::sys::fs::is_regular_file(InputFileName)) {
      llvm::errs() << "Cannot read decision tree from file ";
      llvm::errs() << InputFileName << "\n";
      llvm::errs() << "Aborting\n";
      return;
    }

    // todo: read input file
    DecisionTreeFactory treeFactory;
    DecisionTree decisionTree = treeFactory.makeRandomRegular(3, 100);

    llvm::outs() << "Compiling decision tree from file ";
    llvm::outs() << InputFileName << "..\n";

    if (Debug) {
      llvm::outs() << "Debug output enabled\n";
    }

    CompileResult result =
        Compiler.compile(CodeGenType, std::move(decisionTree));

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

    // todo: set name for module and write to disk
  }

  bool isConfigurationComplete() const {
    return !InputFileName.empty();
  }

  void enableDebug() {
    Debug = true;
  }

  void setOutputFormatText() {
    WriteAsBitcode = false;
  }

  void setOutputFileName(std::string fileName) {
    OutputFileName = std::move(fileName);
  }

  void setInputFileName(std::string fileName) {
    InputFileName = std::move(fileName);
  }

  void setCodeGenerator(CodeGeneratorType codegenType) {
    CodeGenType = codegenType;
  }

private:
  DecisionTreeCompiler Compiler;
  CodeGeneratorType CodeGenType;

  std::string InputFileName;
  std::string OutputFileName;
  bool WriteAsBitcode = true;
  bool Debug = false;

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
    }
    else {
      front = fileName.substr(0, lastDot) + " (";
      back = ")" + fileName.substr(lastDot);
    }

    for (int c = 0; llvm::sys::fs::exists(fileName); c++) {
      fileName = front + std::to_string(c) + back;
    }

    return fileName;
  }
};
