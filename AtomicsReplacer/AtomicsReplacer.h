#ifndef ATOMICS_REPLACER_CLANG_PLUGIN_H
#define ATOMICS_REPLACER_CLANG_PLUGIN_H

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

using namespace clang;
using namespace ast_matchers;
using namespace llvm;


//-----------------------------------------------------------------------------
// ASTFinder callback
//-----------------------------------------------------------------------------
class CodeRefactorMatcher : public clang::ast_matchers::MatchFinder::MatchCallback {
public:
	explicit CodeRefactorMatcher(
		ASTContext& Context,
		clang::Rewriter &RewriterForCodeRefactor,
		std::string ClassNameToReplace,
		std::string ClassNameToInsert
	);
	
	void onEndOfTranslationUnit() override;
	void run(const clang::ast_matchers::MatchFinder::MatchResult &Result) override;
	std::string GetArgumentsFromTemplateType(const TemplateSpecializationType *TST);

private:
	ASTContext& Context;
	clang::Rewriter CodeRefactorRewriter;
	std::string ClassNameToReplace;
	std::string ClassNameToInsert;

	std::string getSourceRangeAsString(const SourceRange& SR) const;
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
	);

	void HandleTranslationUnit(clang::ASTContext &Ctx) override;

private:
	clang::ast_matchers::MatchFinder Finder;
	CodeRefactorMatcher CodeRefactorHandler;

	std::string ClassNameToReplace;
	std::string ClassNameToInsert;
};


#endif