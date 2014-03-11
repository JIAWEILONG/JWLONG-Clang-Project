#include <stdio.h>
// Declares clang::SyntaxOnlyAction.
#include "clang/Frontend/FrontendActions.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
// Declares llvm::cl::extrahelp.
#include "llvm/Support/CommandLine.h"

#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchersMacros.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang::tooling;
using namespace llvm;

using namespace clang;
using namespace clang::ast_matchers;

// CommonOptionsParser declares HelpMessage with a description of the common
// command-line options related to the compilation database and input files.
// It's nice to have this help message in all tools.
static cl::extrahelp CommonHelp(CommonOptionsParser::HelpMessage);

// A help message for this specific tool can be added afterwards.
static cl::extrahelp MoreHelp("\nMore help text...");


DeclarationMatcher vMatcher = 
  varDecl(isDefinition()).bind("varDecl");

class VarPrinter : public MatchFinder::MatchCallback {
public:
  virtual void run(const MatchFinder::MatchResult &Result) {    
    if (const VarDecl *v = Result.Nodes.getNodeAs<clang::VarDecl>("varDecl"))
      // D->dumpColor();
      printf("VariableName:%s\tType:%s\n",
        v->getDeclName().getAsString().c_str(),
        v->getType().getAsString().c_str());

  }
};


DeclarationMatcher fMatcher = 
  functionDecl(isDefinition()).bind("funcDecl");

class FunctionPrinter : public MatchFinder::MatchCallback {
public:
	virtual void run(const MatchFinder::MatchResult &Result) {		
    if (const FunctionDecl *FD = Result.Nodes.getNodeAs<clang::FunctionDecl>("funcDecl"))
      // FD->dumpColor();
      printf("FunctionName:%s\tType:%s\n",
        FD->getNameInfo().getAsString().c_str(), 
        FD->getResultType().getAsString().c_str());
	}
};


int main(int argc, const char **argv) {
  CommonOptionsParser OptionsParser(argc, argv);
  
  ClangTool Tool(OptionsParser.getCompilations(),
                 OptionsParser.getSourcePathList());

  FunctionPrinter Printer1;
  VarPrinter Printer2;
  MatchFinder Finder;
  Finder.addMatcher(fMatcher, &Printer1);
  Finder.addMatcher(vMatcher, &Printer2);
  
  return (Tool.run(newFrontendActionFactory(&Finder)));
}
