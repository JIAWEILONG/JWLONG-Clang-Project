#include <stdio.h>
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
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
using namespace std;


// CommonOptionsParser declares HelpMessage with a description of the common
// command-line options related to the compilation database and input files.
// It's nice to have this help message in all tools.
static cl::extrahelp CommonHelp(CommonOptionsParser::HelpMessage);

// A help message for this specific tool can be added afterwards.
static cl::extrahelp MoreHelp("\nMore help text...");

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

DeclarationMatcher vMatcher = 
  varDecl(isDefinition()).bind("varDecl");

class VarPrinter : public MatchFinder::MatchCallback {
public:
  virtual void run(const MatchFinder::MatchResult &Result) {    
    if (const VarDecl *D = Result.Nodes.getNodeAs<clang::VarDecl>("varDecl"))
      // D->dumpColor();
      printf("VariableName:%s\tType:%s\n",
        D->getDeclName().getAsString().c_str(),
        D->getType().getAsString().c_str());
  }
};



class VarDeclASTVisitor : public RecursiveASTVisitor<VarDeclASTVisitor>{
public:
  VarDeclASTVisitor(ASTContext *context, SourceLocation st) : Context(context), ST(st){}
   bool VisitVarDecl(VarDecl *V)
    { 
      std::string i;
      i = "\nprintf(\"ORBS: %d\\n\",";
      i += V->getNameAsString();
      i += ");\n";
      Rewriter Rewrite;
      Rewrite.setSourceMgr(Context->getSourceManager(),Context->getLangOpts());
      Rewrite.InsertTextAfter(ST,i);
      Rewrite.getEditBuffer(Rewrite.getSourceMgr().getMainFileID()).write(errs());

      unsigned position = Context->getSourceManager().getFileOffset(ST);
      unsigned lineNum = Context->getSourceManager().getLineNumber(Context->getSourceManager().getFileID(ST), position);
      string varName = V->getDeclName().getAsString().c_str();

      string orig_path = Context->getSourceManager().getBufferName(ST);       
      unsigned pos = orig_path.find(".");
      string orig_filename = orig_path.substr(0, pos);

      std::stringstream ss;
      ss << orig_filename << "_" << varName << "_" << lineNum << ".c";

      string str(ss.str());
      const char* cstr = str.c_str();
      ofstream outFile(cstr);

      const RewriteBuffer *RewriteBuf = Rewrite.getRewriteBufferFor(Rewrite.getSourceMgr().getMainFileID());
      outFile << std::string(RewriteBuf->begin(), RewriteBuf->end());
      outFile.close();

      return true;
    }
private:
  ASTContext *Context;
  SourceLocation ST;
};

class DeclStmtASTVisitor : public RecursiveASTVisitor<DeclStmtASTVisitor>{
public:
  DeclStmtASTVisitor(ASTContext *context) : Context(context){}
   bool VisitDeclStmt(DeclStmt *V)
   {
      if(Context->getSourceManager().isInMainFile(V->getStartLoc()))
      {
        SourceLocation st = V->getLocEnd().getLocWithOffset(1);
        VarDeclASTVisitor VDVisitor(Context, st);
        VDVisitor.TraverseStmt((Stmt*)V);
        return true;
      }
      else
        return false; 
    }
private:
  ASTContext *Context;
};

class DeclConsumer : public clang::ASTConsumer{
public:
    DeclConsumer(ASTContext *context) :visitor(context){};
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
  // CommonOptionsParser OptionsParser(argc, argv);
  // ClangTool Tool(OptionsParser.getCompilations(),
  //                OptionsParser.getSourcePathList());
  // FunctionPrinter Printer1;
  // VarPrinter Printer2;
  // MatchFinder Finder;
  // Finder.addMatcher(fMatcher, &Printer1);
  // Finder.addMatcher(vMatcher, &Printer2);
  // return (Tool.run(newFrontendActionFactory(&Finder)));

  CommonOptionsParser op(argc, argv);
  ClangTool Tool(op.getCompilations(), op.getSourcePathList());
  int result = Tool.run(newFrontendActionFactory<InstrumentAction>());

  // Rewrite.getEditBuffer(Rewrite.getSourceMgr().getMainFileID()).write(errs());
  
  return result;
}
