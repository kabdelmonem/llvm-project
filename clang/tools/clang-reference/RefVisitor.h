#pragma once

#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Basic/SourceManager.h"
#include "llvm/Support/raw_ostream.h"

#include <string>
#include <sstream>

class ASTVisitor : public clang::RecursiveASTVisitor<ASTVisitor> {
  clang::SourceManager &SourceManager;

public:
  ASTVisitor(clang::SourceManager &SourceManager)
      : SourceManager(SourceManager) {}

  bool VisitDeclRefExpr(clang::DeclRefExpr *Declref) {

    //llvm::outs() << "Found " << NamedDecl->getDeclKindName ()<<" " <<NamedDecl->getQualifiedNameAsString() << " at "<< getDeclLocation(NamedDecl->getBeginLoc()) << "\n";
    //if (strcmp(getDeclLocation(Declref->getFoundDecl()->getBeginLoc()).c_str() , getDeclLocation(Declref->getBeginLoc()).c_str() ) ) {
    llvm::outs()<<"declaration_reference"<< ">"<<Declref->getFoundDecl()->getQualifiedNameAsString()<< ">"<< getDeclLocation(Declref->getFoundDecl()->getBeginLoc())<< ">"<< getDeclLocation(Declref->getBeginLoc()) <<"\n";
    //}            
    return true;
  }

private:
  std::string getDeclLocation(clang::SourceLocation Loc) const {
    std::ostringstream OSS;
    OSS << SourceManager.getFilename(Loc).str() << ":"
        << SourceManager.getSpellingLineNumber(Loc) << ":"
        << SourceManager.getSpellingColumnNumber(Loc);
    return OSS.str();
  }
};

