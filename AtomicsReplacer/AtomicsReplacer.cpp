//==============================================================================
// FILE:
//    AtomicsReplacer.cpp
//
// DESCRIPTION:
//    Substitutes all input files std::atomic<T> usages with MyAtomic<T> with
//    the same API.
//
// USAGE:
// clang++ -Xclang -load -Xclang ./build/lib/libAtomicsReplacer.so -Xclang -add-plugin -Xclang atomics-replacer ./AtomicsReplacer/test-project/main.cpp 
//
// License: The Unlicense
//==============================================================================

#include "clang/AST/ASTConsumer.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/Rewrite/Frontend/FixItRewriter.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/AST/Expr.h"
#include "clang/AST/ExprCXX.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/Tooling/Refactoring/Rename/RenamingAction.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/raw_ostream.h"

using namespace clang;
using namespace ast_matchers;
using namespace llvm;

//-----------------------------------------------------------------------------
// ASTFinder callback
//-----------------------------------------------------------------------------
class CodeRefactorMatcher
    : public clang::ast_matchers::MatchFinder::MatchCallback {
public:
  explicit CodeRefactorMatcher(
    ASTContext& Context,
    clang::Rewriter &RewriterForCodeRefactor,
    std::string ClassNameToReplace,
    std::string ClassNameToInsert)
      : Context(Context),
        CodeRefactorRewriter(RewriterForCodeRefactor),
        ClassNameToReplace(ClassNameToReplace),
        ClassNameToInsert(ClassNameToInsert) {}
  
  void onEndOfTranslationUnit() override {
    // Output to stdout
    CodeRefactorRewriter
      .getEditBuffer(CodeRefactorRewriter.getSourceMgr().getMainFileID())
      .write(llvm::outs());
  }

  void run(const clang::ast_matchers::MatchFinder::MatchResult &Result) override {
    if (const auto* ETL = Result.Nodes.getNodeAs<ElaboratedTypeLoc>("ElaboratedTypeLoc")) {      
      const auto* TemplType = ETL->getType()->getAs<TemplateSpecializationType>();
      if (!TemplType) {
        return;
      }

      CodeRefactorRewriter.ReplaceText(ETL->getSourceRange(), ClassNameToInsert + GetArgumentsFromTemplateType(TemplType));
    }

    if (const auto* QTL = Result.Nodes.getNodeAs<QualifiedTypeLoc>("QualifiedTypeLoc")) {
      const auto* TemplType = QTL->getType()->getAs<TemplateSpecializationType>();
      if (!TemplType) {
        return;
      }

      CodeRefactorRewriter.ReplaceText(QTL->getSourceRange(), ClassNameToInsert + GetArgumentsFromTemplateType(TemplType));
    }
  }

  std::string GetArgumentsFromTemplateType(const TemplateSpecializationType *TST) {
    std::string args;
    llvm::raw_string_ostream os(args);
    printTemplateArgumentList(os, TST->template_arguments(), Context.getPrintingPolicy());
    return args;
  }

private:
  ASTContext& Context;
  clang::Rewriter CodeRefactorRewriter;
  std::string ClassNameToReplace;
  std::string ClassNameToInsert;

  // Util function for debugging purposes
  std::string getSourceRangeAsString(const SourceRange& SR) const {
    auto& sm = CodeRefactorRewriter.getSourceMgr();
    auto& langOpts = CodeRefactorRewriter.getLangOpts();

    clang::SourceLocation start = SR.getBegin();
    clang::SourceLocation end = SR.getEnd();
    end = clang::Lexer::getLocForEndOfToken(end, 0, sm, langOpts);

    bool isInvalid = false;
    const char *startData = sm.getCharacterData(start, &isInvalid);
    
    if (isInvalid) {
      return "<invalid begin>";
      isInvalid = false;
    }

    const char *endData = sm.getCharacterData(end, &isInvalid);

    if (isInvalid) {
      return "<invalid end>";
      isInvalid = false;
    }
    size_t length = endData - startData;

    return std::string(startData, length);
  }
};

//-----------------------------------------------------------------------------
// ASTConsumer
//-----------------------------------------------------------------------------
class CodeRefactorASTConsumer : public clang::ASTConsumer {
public:
  CodeRefactorASTConsumer(
    ASTContext& Context,
    clang::Rewriter &R,
    std::string ClassNameToReplace,
    std::string ClassNameToInsert
  ): CodeRefactorHandler(Context, R, ClassNameToReplace, ClassNameToInsert),
     ClassNameToReplace(ClassNameToReplace),
     ClassNameToInsert(ClassNameToInsert) {
    // Does not support matching the parameters of the functions
    const auto MatcherForFQTemplateTypes = elaboratedTypeLoc(
      hasNamedTypeLoc(
        loc(
          templateSpecializationType(
            hasDeclaration(
              classTemplateSpecializationDecl(
                hasName(ClassNameToReplace)
              )
            )
          )
        )
      )
    );

    // Uses previous matcher inside, but returns a wrapping QualifiedTypeLoc node
    // which is used in the function parameters
    const auto MatcherForFQTemplateParams = qualifiedTypeLoc(
      hasUnqualifiedLoc(
        MatcherForFQTemplateTypes
      )
    );

    Finder.addMatcher(MatcherForFQTemplateTypes.bind("ElaboratedTypeLoc"), &CodeRefactorHandler);
    Finder.addMatcher(MatcherForFQTemplateParams.bind("QualifiedTypeLoc"), &CodeRefactorHandler);
  }

  void HandleTranslationUnit(clang::ASTContext &Ctx) override {
    Finder.matchAST(Ctx);
  }

private:
  clang::ast_matchers::MatchFinder Finder;
  CodeRefactorMatcher CodeRefactorHandler;

  std::string ClassNameToReplace;
  std::string ClassNameToInsert;
};


//-----------------------------------------------------------------------------
// FrontendAction
//-----------------------------------------------------------------------------
class CodeRefactorAddPluginAction : public PluginASTAction {
public:
  bool ParseArgs(const CompilerInstance &CI, const std::vector<std::string> &Args) override {
    return true;
  }

  // Returns our ASTConsumer per translation unit.
  std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI,
                                                  StringRef file) override {
    RewriterForCodeRefactor.setSourceMgr(CI.getSourceManager(),
                                          CI.getLangOpts());
    return std::make_unique<CodeRefactorASTConsumer>(
        CI.getASTContext(), RewriterForCodeRefactor, ClassNameToReplace, ClassNameToInsert);
  }

private:
  Rewriter RewriterForCodeRefactor;
  std::string ClassNameToReplace = "::custom::OtherAtomic";
  std::string ClassNameToInsert = "MyAtomic";
};

  
static FrontendPluginRegistry::Add<CodeRefactorAddPluginAction> X("atomics-replacer", "Replace all foo calls with bar calls");