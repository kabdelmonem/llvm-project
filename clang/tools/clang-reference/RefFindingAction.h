#pragma once

#include "RefFinder.h"

#include "clang/Frontend/FrontendAction.h"
#include "clang/Frontend/CompilerInstance.h"

#include <memory>

namespace clang {
class ASTConsumer;
}

class RefFindingAction : public clang::ASTFrontendAction {
public:
  std::unique_ptr<clang::ASTConsumer>
  CreateASTConsumer(clang::CompilerInstance &CI, clang::StringRef) final {
    return std::unique_ptr<clang::ASTConsumer>(
        new RefFinder(CI.getSourceManager()));
  }
};

