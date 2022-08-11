set(LLVM_LINK_COMPONENTS
  Support
  )

add_clang_tool(clang-macros
  PPTrace.cpp
  PPCallbacksTracker.cpp
  )

clang_target_link_libraries(clang-macros
  PRIVATE
  clangAST
  clangBasic
  clangFrontend
  clangLex
  clangSerialization
  clangTooling
  )
