set(LLVM_LINK_COMPONENTS
  Option
  Support
  )

add_clang_tool(clang-playground
  FindDecl.cpp
  )

clang_target_link_libraries(clang-playground
PRIVATE
  clangAST
  clangBasic
  clangFrontend
  clangIndex
  clangRewrite
  clangTooling
)
