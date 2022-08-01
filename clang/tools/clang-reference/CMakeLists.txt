
set(LLVM_LINK_COMPONENTS
  Option
  Support
  )

add_clang_tool(clang-reference
  FindRef.cpp
  )

clang_target_link_libraries(clang-reference
PRIVATE
  clangAST
  clangBasic
  clangFrontend
  clangIndex
  clangRewrite
  clangTooling
)