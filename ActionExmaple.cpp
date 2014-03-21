#include <stdio.h>
#include <string>
#include <sstream>

// Declares clang::SyntaxOnlyAction.
#include "clang/Frontend/FrontendActions.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"


// Declares llvm::cl::extrahelp.
#include "llvm/Support/CommandLine.h"

#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchersMacros.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Rewrite/Frontend/Rewriters.h"
#include "clang/Basic/SourceManager.h"

#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/Parse/ParseAST.h"

#include "clang/Basic/TargetOptions.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/DeclBase.h"
#include "clang/Lex/Lexer.h"


using namespace clang::tooling;
using namespace llvm;

using namespace clang;
using namespace clang::ast_matchers;

class DeclStmtASTVisitor : public RecursiveASTVisitor<DeclStmtASTVisitor>{
public:
  DeclStmtASTVisitor(ASTContext *context) : Context(context){}

    bool VisitDeclStmt(DeclStmt *V)
    {
      // handle DeclStmt
    }

private:
  ASTContext *Context;    
};

class DeclConsumer : public clang::ASTConsumer
{
public:
    DeclConsumer(ASTContext *context)
        :visitor(context){};

    virtual bool HandleTopLevelDecl(DeclGroupRef DR) 
    {
      for (DeclGroupRef::iterator b = DR.begin(), e = DR.end(); b != e; ++b)
        visitor.TraverseDecl(*b);
      return true;
    }

private:
    DeclStmtASTVisitor visitor;
};

class InstrumentAction : public clang::ASTFrontendAction {
public:
  virtual clang::ASTConsumer *CreateASTConsumer(clang::CompilerInstance &Compiler, llvm::StringRef InFile) {
    DeclConsumer *consumer = new DeclConsumer(&Compiler.getASTContext());
    return consumer;
  }
};


int main(int argc, const char **argv) {
  // parse the command-line args passed to your code
  CommonOptionsParser op(argc, argv);
  // create a new Clang Tool instance (a LibTooling environment)
  ClangTool Tool(op.getCompilations(), op.getSourcePathList());
  Tool.run(newFrontendActionFactory<InstrumentAction>());
}
