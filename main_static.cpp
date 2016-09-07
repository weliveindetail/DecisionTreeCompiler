#include <getopt.h>
#include <stdlib.h>

#include <string>

#include <llvm/Support/raw_ostream.h>

#include "driver/StaticDriver.h"

// EvalTreeJit_Static -h
// EvalTreeJit_Static [-d] [-S] [-o outputFile] tree1.json

void printHelp(llvm::raw_ostream &out) {
  out << "Usage: dtg [OPTIONS] INPUT\n";
  out << "Read decision tree file given as INPUT and compile ";
  out << "an evaluator function for it in LLVM IR\n";
  out << "\n";
  out << "OPTIONS:\n";
  out << "  -h             Print help message\n";
  out << "  -d             Enable debug output\n";
  out << "  -S             Write output IR as human-readable text\n";
  out << "  -o FILE_NAME   Write output to FILE_NAME (defaults to stdout)\n";
  out << "\n";
  out << "Example usage:\n";
  out << "  dtg -S -d -o module.ll module.json\n";
}

void printIgnoredInput(llvm::raw_ostream &out, std::string input) {
  out << "Ignored non-option input " << input << "\n";
}

void printIgnoredOption(llvm::raw_ostream &out, char opt) {
  out << "Ignored option -" << opt << "\n";
}

void printIgnoredOption(llvm::raw_ostream &out, char opt, std::string arg) {
  out << "Ignored option -" << opt << " with argument " << arg << "\n";
}

void printInvalidArgument(llvm::raw_ostream &out, std::string option,
                          std::string arg) {
  out << "Invalid argument for option " << option << ": " << arg << "\n";
}

bool isValidArgument(std::string arg) {
  assert(!arg.empty());
  return (arg.at(0) != '-');
}

int main(int argc, char **argv) {
  StaticDriver driver;

  int c;
  opterr = 0;
  while ((c = getopt(argc, argv, "hdSo:")) != -1) {
    switch (c) {
      case 'h':
        printHelp(llvm::outs());
        exit(EXIT_SUCCESS);
      case 'd':
        driver.enableDebug();
        break;
      case 'S':
        driver.setOutputFormatText();
        break;
      case 'o':
        if (isValidArgument(optarg)) {
          driver.setOutputFileName(optarg);
          break;
        }
        else {
          printInvalidArgument(llvm::errs(), "-o", optarg);
          exit(EXIT_FAILURE);
        }
      case '?':
        if (optarg)
          printIgnoredOption(llvm::errs(), optopt, optarg);
        else
          printIgnoredOption(llvm::errs(), optopt);
        break;
      default:
        exit(EXIT_FAILURE);
    }
  }

  if (optind >= argc) {
    llvm::errs() << "Missing INPUT source file\n\n";
    printHelp(llvm::errs());
    exit(EXIT_FAILURE);
  }

  driver.setInputFileName(argv[optind++]);

  for (int index = optind; index < argc; index++)
    printIgnoredInput(llvm::errs(), argv[index]);

  if (!driver.isConfigurationComplete()) {
    llvm::errs() << "Missing required argument\n\n";
    printHelp(llvm::errs());
    exit(EXIT_FAILURE);
  }

  driver.run();
  return EXIT_SUCCESS;
}
