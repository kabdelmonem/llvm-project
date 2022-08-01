#pragma once

#include "RefVisitor.h"

#include "clang/AST/ASTConsumer.h"
#include "clang/AST/ASTContext.h"
#include "clang/Basic/SourceManager.h"
#include <iostream>

bool parseheaders=false;

class RefFinder : public clang::ASTConsumer {
  clang::SourceManager &SourceManager;
  ASTVisitor Visitor;
public:
  RefFinder(clang::SourceManager &SM) : SourceManager(SM), Visitor(SM) {}

  void HandleTranslationUnit(clang::ASTContext &Context) final {

    if (parseheaders){
      //Visitor.TraverseDecl(Context.getTranslationUnitDecl());
      Visitor.TraverseAST(Context);

    }
   else{
     /*
      auto Decls = Context.getTranslationUnitDecl()->decls();
      for (auto &Decl : Decls) {
        const auto& FileID = SourceManager.getFileID(Decl->getLocation());
        if (FileID != SourceManager.getMainFileID())
          continue;
        Visitor.TraverseDecl(Decl);

      //llvm::outs()<<"Now_the_declaration_references_begin"<<"\n";
      
      }*/
      Visitor.TraverseAST(Context);
    }
  }
};
