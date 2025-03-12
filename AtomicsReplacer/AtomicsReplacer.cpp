//==============================================================================
// FILE:
//    AtomicsReplacer.cpp
//
// DESCRIPTION:
//    Substitutes all input files std::atomic<T> usages with myAtomic<T> with
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

// //-----------------------------------------------------------------------------
// // RecursiveASTVisitor
// //-----------------------------------------------------------------------------
// class AtomicsReplacerVisitor : public RecursiveASTVisitor<AtomicsReplacerVisitor> {
// public:
//   // explicit AtomicsReplacerVisitor(ASTContext *Context) : Context(Context) {}
//   explicit AtomicsReplacerVisitor(Rewriter& R) : MyRewriterRef(R) {}

//   // bool VisitCXXRecordDecl(CXXRecordDecl *Decl) {
//   //   FullSourceLoc FullLocation = Context->getFullLoc(Decl->getBeginLoc());

//   //   // Basic sanity checking
//   //   if (!FullLocation.isValid())
//   //     return true;

//   //   // There are 2 types of source locations: in a file or a macro expansion. The
//   //   // latter contains the spelling location and the expansion location (both are
//   //   // file locations), but only the latter is needed here (i.e. where the macro
//   //   // is expanded). File locations are just that - file locations.
//   //   if (FullLocation.isMacroID())
//   //     FullLocation = FullLocation.getExpansionLoc();

//   //   SourceManager &SrcMgr = Context->getSourceManager();
//   //   OptionalFileEntryRef Entry = SrcMgr.getFileEntryRefForID(SrcMgr.getFileID(FullLocation));
    
//   //   llvm::outs() << "(atomic-replacer)  entry: " << Entry->getName()
//   //               << "\n";

//   //   return true;
//   // }

//   bool VisitCallExpr(CallExpr *CE) {
//     FunctionDecl *Callee = CE->getDirectCallee();
//     if (Callee && Callee->getNameAsString() == "foo") {
//       // Replace the function name in the call expression
//       llvm::outs() << "Found foo() call, must replace\n";
//       SourceLocation Start = CE->getBeginLoc();
//       SourceLocation End = Start.getLocWithOffset(strlen("foo") - 1);
//       MyRewriterRef.ReplaceText(SourceRange(Start, End), "bar");
//     }
//     return true;
//   }

// private:
//   // ASTContext *Context;
//   Rewriter& MyRewriterRef;
// };

// //-----------------------------------------------------------------------------
// // ASTConsumer
// //-----------------------------------------------------------------------------
// class AtomicsReplacerASTConsumer : public ASTConsumer {
// public:
//   // explicit AtomicsReplacerASTConsumer(ASTContext *Ctx) : Visitor(Ctx) {}
//   explicit AtomicsReplacerASTConsumer(Rewriter &R) : Visitor(R) {}

//   void HandleTranslationUnit(ASTContext &Ctx) override {
//     Visitor.TraverseDecl(Ctx.getTranslationUnitDecl());
//     llvm::outs() << "(atomic-replacer)  hello from ast consumer\n";
//   }

//   void Initialize(ASTContext &Context) override {
//     Ctx = &Context;
//   }

//   // bool HandleTopLevelDecl(DeclGroupRef DG) override {
//   //   for (auto D : DG) {
//   //     if (FunctionDecl *FD = dyn_cast<FunctionDecl>(D)) {
//   //       if (FD->getNameAsString() == "foo") {
//   //         llvm::outs() << "Found foo() func decl\n";
//   //         // IdentifierInfo& NewName = Ctx->Idents.get("new_foo");
//   //         // DeclarationName NewDeclName(&NewName);
//   //         // FD->setDeclName(NewDeclName);
//   //       }
//   //     }
//   //   }
//   //   return true;
//   // }

// private:
//   AtomicsReplacerVisitor Visitor;
//   ASTContext* Ctx;
// };

// //-----------------------------------------------------------------------------
// // FrontendAction for AtomicsReplacer
// //-----------------------------------------------------------------------------
// class ReplacerClassPlugin : public PluginASTAction {
// public:
//   std::unique_ptr<ASTConsumer> CreateASTConsumer(
//     CompilerInstance &Compiler,
//     llvm::StringRef InFile
//   ) override {
//     llvm::outs() << "CreateASTConsumer(): file=" << InFile << "\n";
//     MyRewriter.setSourceMgr(Compiler.getSourceManager(), Compiler.getLangOpts());
//     return std::make_unique<AtomicsReplacerASTConsumer>(MyRewriter);
//     // return std::unique_ptr<ASTConsumer>(
//     //   std::make_unique<AtomicsReplacerASTConsumer>(&Compiler.getASTContext())
//     // );
//   }

//   bool ParseArgs(
//     const CompilerInstance &CI,
//     const std::vector<std::string> &args
//   ) override {
//     return true;
//   }

//   bool BeginSourceFileAction(CompilerInstance &CI) override {
//     llvm::outs() << "BeginSourceFileAction\n";
//     return true;
//   }

//   void EndSourceFileAction() override {
//     llvm::outs() << "EndSourceFileAction\n";
//     // Output the rewritten source code
//     MyRewriter.getEditBuffer(MyRewriter.getSourceMgr().getMainFileID()).write(llvm::outs());
//   }

//   bool shouldEraseOutputFiles() override {
//     return false;
//   }

//   // PluginASTAction::ActionType getActionType() override {
//   //   return ActionType::AddAfterMainAction;
//   // }

// private:
//   Rewriter MyRewriter;
// };

//-----------------------------------------------------------------------------
// Registration
//-----------------------------------------------------------------------------
// static FrontendPluginRegistry::Add<ReplacerClassPlugin> 
//   X(/*Name=*/"atomics-replacer", /*Description=*/"Plugin which replaces std::atomic<T> classes with custom definitions");


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
    // Handle variable declarations of OtherAtomic type
    // if (const auto *varDecl = Result.Nodes.getNodeAs<clang::VarDecl>("AtomicVarDecl")) {      
    //   // Get the class name and its length from the record declaration
    //   // const auto *RecordDecl = Result.Nodes.getNodeAs<clang::RecordDecl>("AtomicClass");
    //   // if (!RecordDecl)
    //   //   return;

    //   clang::TypeLoc TypeLoc = varDecl->getTypeSourceInfo()->getTypeLoc();
    //   std::string TypeSourceString = getSourceRangeAsString(TypeLoc.getSourceRange());

    //   llvm::outs() << "Actual type as string matched: '"
    //                << TypeSourceString << "' "
    //                << varDecl->getType().getAsString() << " "
    //                << "replace: " << ClassNameToReplace << "\n";

    //   CodeRefactorRewriter.ReplaceText(TypeLoc.getBeginLoc(), TypeSourceString.length(), ClassNameToInsert);
    // }

    
    llvm::outs() << "Matched something\n";
    if (const auto *templateTypeLoc = Result.Nodes.getNodeAs<TypeLoc>("TemplateTypeLoc")) {
      // QualifiedTypeLoc actualTypeLoc = templateTypeLoc->getAs<QualifiedTypeLoc>();
      // if (actualTypeLoc) {
      //   llvm::outs() << "Qualified template loc!" << getSourceRangeAsString(actualTypeLoc.getSourceRange()) << "\n";
      // }

      // switch (templateTypeLoc->getTypeLocClass()) {
      //   case clang::TypeLoc::Qualified: {
      //     QualifiedTypeLoc actualTypeLoc = templateTypeLoc->getAs<QualifiedTypeLoc>();
      //     llvm::outs() << "Qualified template loc!" << getSourceRangeAsString(actualTypeLoc.getSourceRange()) << "\n";
      //     break;
      //   }
      //   case clang::TypeLoc::TemplateSpecialization: {
      //     llvm::outs() << "TemplateSpecialization\n";
      //     break;
      //   }
      //   default: {
      //     llvm::outs() << "None of two\n";
      //   }
      // }

      const auto* templType = templateTypeLoc->getType()->getAs<TemplateSpecializationType>();
      if (!templType) {
        return;
      }
      
      // if (const ClassTemplateSpecializationDecl *CTSD =
      //     dyn_cast_or_null<ClassTemplateSpecializationDecl>(templType->getAsCXXRecordDecl())) {
      //     // Get the fully qualified name, including namespaces
      //     std::string FullyQualifiedName = GetFullyQualifiedName(CTSD);

      //     // Print the fully qualified name
      //     llvm::outs() << "Fully Qualified Name: " << FullyQualifiedName << "\n";
      // }

      std::string templateArgs;
      llvm::raw_string_ostream os(templateArgs);
      printTemplateArgumentList(os, templType->template_arguments(), Context.getPrintingPolicy());

      llvm::outs() << "Template: '" << getSourceRangeAsString(templateTypeLoc->getSourceRange()) << "'\n";
      llvm::outs() << "Template args: " << templateArgs << "\n";
      
      //CodeRefactorRewriter.ReplaceText(templateTypeLoc->getSourceRange(), ClassNameToInsert + templateArgs);
    }

    if (const auto* fqTemplateTypeLoc = Result.Nodes.getNodeAs<ElaboratedTypeLoc>("TemplateFQTypeLoc")) {      
      const auto* templType = fqTemplateTypeLoc->getType()->getAs<TemplateSpecializationType>();
      if (!templType) {
        return;
      }

      std::string templateArgs;
      llvm::raw_string_ostream os(templateArgs);
      printTemplateArgumentList(os, templType->template_arguments(), Context.getPrintingPolicy());

      llvm::outs() << "FQ Template: '" << getSourceRangeAsString(fqTemplateTypeLoc->getSourceRange()) << "'\n";
      llvm::outs() << "FQ Template args: " << templateArgs << "\n";

      CodeRefactorRewriter.ReplaceText(fqTemplateTypeLoc->getSourceRange(), ClassNameToInsert + templateArgs);
    }

    // const MemberExpr *MemberAccess =
    //     Result.Nodes.getNodeAs<clang::MemberExpr>("MemberAccess");
  
    // if (MemberAccess) {
    //   SourceRange CallExprSrcRange = MemberAccess->getMemberLoc();
    //   CodeRefactorRewriter.ReplaceText(CallExprSrcRange, NewName);
    // }
  
    // const NamedDecl *MemberDecl =
    //     Result.Nodes.getNodeAs<clang::NamedDecl>("MemberDecl");
  
    // if (MemberDecl) {
    //   SourceRange MemberDeclSrcRange = MemberDecl->getLocation();
    //   CodeRefactorRewriter.ReplaceText(
    //       CharSourceRange::getTokenRange(MemberDeclSrcRange), NewName);
    // }
  }

  std::string GetFullyQualifiedName(const Decl *D) {
    PrintingPolicy Policy(Context.getLangOpts());
    Policy.SuppressScope = false; // Ensure namespace qualifiers are included

    std::string QualName;
    llvm::raw_string_ostream OS(QualName);
    D->print(OS, Policy);
    return QualName;
  }

private:
  ASTContext& Context;
  clang::Rewriter CodeRefactorRewriter;
  std::string ClassNameToReplace;
  std::string ClassNameToInsert;

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
    // const auto MatcherForAtomicVarDecl = varDecl(
    //   hasType(
    //     recordDecl(hasName(ClassNameToReplace)) // .bind("AtomicClass")
    //   ),
    //   unless(hasType(autoType()))
    // ).bind("AtomicVarDecl");

    // Finder.addMatcher(MatcherForAtomicVarDecl, &CodeRefactorHandler);
    
    // specifiesNamespace

    /*
    
    match elaboratedTypeLoc(loc(templateSpecializationType(hasDeclaration(classTemplateSpecializationDecl(hasName("OtherAtomic"))))))
    
    */

    const auto MatcherForTemplateTypes = typeLoc(
      loc(
        templateSpecializationType(
          hasDeclaration(
            classTemplateSpecializationDecl(
              hasName(ClassNameToReplace)
            )
          )
        )
      )
    ).bind("TemplateTypeLoc");

    const auto MatcherForFQTemplateTypes = elaboratedTypeLoc(
      hasNamedTypeLoc(
        loc(
          templateSpecializationType(
            hasDeclaration(
              classTemplateSpecializationDecl(
                hasName("custom::OtherAtomic")
              )
            )
          )
        )
      )
    ).bind("TemplateFQTypeLoc");

    Finder.addMatcher(MatcherForTemplateTypes, &CodeRefactorHandler);
    Finder.addMatcher(MatcherForFQTemplateTypes, &CodeRefactorHandler);
    
    // Match class type references in declarations
    // const auto MatcherForTypeReferences = typeLoc(
    //   hasType(hasDeclaration(recordDecl(hasName(ClassNameToReplace))))
    // ).bind("AtomicTypeLoc");

    // const auto MatcherForMemberAccess = cxxMemberCallExpr(
    //   callee(memberExpr(member(hasName(OldName))).bind("MemberAccess")),
    //   thisPointerType(cxxRecordDecl(isSameOrDerivedFrom(hasName(ClassName)))));

    // Finder.addMatcher(MatcherForMemberAccess, &CodeRefactorHandler);

    // const auto MatcherForMemberDecl = cxxRecordDecl(
    //     allOf(isSameOrDerivedFrom(hasName(ClassName)),
    //           hasMethod(decl(namedDecl(hasName(OldName))).bind("MemberDecl"))));

    // Finder.addMatcher(MatcherForMemberDecl, &CodeRefactorHandler);
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