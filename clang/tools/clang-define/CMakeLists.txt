set(LLVM_LINK_COMPONENTS
  Option
  Support
  )

add_clang_tool(clang-define
  FindDecl.cpp
  )

clang_target_link_libraries(clang-define
PRIVATE
  clangAST
  clangBasic
  clangFrontend
  clangIndex
  clangRewrite
  clangTooling
)
