#include "RefFindingAction.h"
#include "RefFinder.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include<string.h>
#include <ostream>

static llvm::cl::extrahelp CommonHelp(clang::tooling::CommonOptionsParser::HelpMessage);
llvm::cl::OptionCategory FindRefCategory("find-ref options");

#define FIND_REF_VERSION "0.0.1"

static char FindRefUsage[] = "find-ref <source file>";

int main(int argc, const char **argv) {
  int i;
  int pos=0;
  for (i=0;i<argc;i++){
    if(strcmp(argv[i], "-h") == 0){
      pos=i;
      //llvm::outs() << "found -h :"<< argv[i] << "parseheaders befor"<<parseheaders;
      parseheaders=true;
      argc=argc-1;
           
    }
  }
  if(pos>0){
    for (i = pos - 1; i < argc; i++)  
      { argv[i] = argv[i+1];  }  
  }
  //llvm::outs() <<  "parseheaders after"<<parseheaders<<"\n";
  
  llvm::cl::SetVersionPrinter([](llvm::raw_ostream &OS) { OS << "find-ref version: " << FIND_REF_VERSION << "\n"; });
  clang::tooling::CommonOptionsParser option(argc, argv, FindRefCategory, FindRefUsage);

  auto files = option.getSourcePathList();
  clang::tooling::ClangTool tool(option.getCompilations(), files);

  return tool.run(clang::tooling::newFrontendActionFactory<RefFindingAction>().get());
}

