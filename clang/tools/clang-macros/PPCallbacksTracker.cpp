//===--- PPCallbacksTracker.cpp - Preprocessor tracker -*--*---------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// Implementations for preprocessor tracking.
///
/// See the header for details.
///
//===----------------------------------------------------------------------===//

#include "PPCallbacksTracker.h"
#include "clang/Basic/FileManager.h"
#include "clang/Lex/MacroArgs.h"
#include "llvm/Support/raw_ostream.h"

namespace clang {
namespace pp_trace {

// Get a "file:line:column" source location string.
static std::string getSourceLocationString(Preprocessor &PP,
                                           SourceLocation Loc) {
  if (Loc.isInvalid())
    return std::string("(none)");

  if (Loc.isFileID()) {
    PresumedLoc PLoc = PP.getSourceManager().getPresumedLoc(Loc);

    if (PLoc.isInvalid()) {
      return std::string("(invalid)");
    }

    std::string Str;
    llvm::raw_string_ostream SS(Str);

    // The macro expansion and spelling pos is identical for file locs.
    SS << "\"" << PLoc.getFilename() << ':' << PLoc.getLine() << ':'
       << PLoc.getColumn() << "\"";

    std::string Result = SS.str();

    // YAML treats backslash as escape, so use forward slashes.
    std::replace(Result.begin(), Result.end(), '\\', '/');

    return Result;
  }

  return std::string("(nonfile)");
}

// Enum string tables.


// MacroDirective::Kind strings.
static const char *const MacroDirectiveKindStrings[] = {
  "MD_Define","MD_Undefine", "MD_Visibility"
};


// PPCallbacksTracker functions.

PPCallbacksTracker::PPCallbacksTracker(const FilterType &Filters,
                                       std::vector<CallbackCall> &CallbackCalls,
                                       Preprocessor &PP)
    : CallbackCalls(CallbackCalls), Filters(Filters), PP(PP) {}

PPCallbacksTracker::~PPCallbacksTracker() {}

// Callback functions.

// Called by Preprocessor::HandleMacroExpandedIdentifier when a
// macro invocation is found.
void PPCallbacksTracker::MacroExpands(const Token &MacroNameTok,
                                      const MacroDefinition &MacroDefinition,
                                      SourceRange Range,
                                      const MacroArgs *Args) {
  beginCallback("MacroExpands");
  appendArgument("MacroNameTok", MacroNameTok);
  appendArgument("MacroDefinition", MacroDefinition);
  appendArgument("Range", Range);
  appendArgument("Args", Args);
}

// Hook called whenever a macro definition is seen.
void PPCallbacksTracker::MacroDefined(const Token &MacroNameTok,
                                      const MacroDirective *MacroDirective
                                      ) {
  beginCallback("MacroDefined");
  appendArgument("", MacroDirective->getMacroInfo()->getDefinitionLoc() );
  appendArgument("MacroNameTok", MacroNameTok);

  for (Token Tok : MacroDirective-> getMacroInfo()->tokens() ){
   
    if (strcmp(Tok.getName (), "identifier") == 0){
      appendArgument("", Tok );
    }
    
  }
}

// Hook called whenever a macro #undef is seen.
void PPCallbacksTracker::MacroUndefined(const Token &MacroNameTok,
                                        const MacroDefinition &MacroDefinition,
                                        const MacroDirective *Undef) {
  beginCallback("MacroUndefined");
  appendArgument("MacroNameTok", MacroNameTok);
  appendArgument("MacroDefinition", MacroDefinition);
}

// Hook called whenever the 'defined' operator is seen.
void PPCallbacksTracker::Defined(const Token &MacroNameTok,
                                 const MacroDefinition &MacroDefinition,
                                 SourceRange Range) {
  beginCallback("Defined");
  appendArgument("MacroNameTok", MacroNameTok);
  appendArgument("MacroDefinition", MacroDefinition);
  appendArgument("Range", Range);
}


// Helper functions.

// Start a new callback.
void PPCallbacksTracker::beginCallback(const char *Name) {
  auto R = CallbackIsEnabled.try_emplace(Name, false);
  if (R.second) {
    llvm::StringRef N(Name);
    for (const std::pair<llvm::GlobPattern, bool> &Filter : Filters)
      if (Filter.first.match(N))
        R.first->second = Filter.second;
  }
  DisableTrace = !R.first->second;
  if (DisableTrace)
    return;
  CallbackCalls.push_back(CallbackCall(Name));
}

// Append a bool argument to the top trace item.
void PPCallbacksTracker::appendArgument(const char *Name, bool Value) {
  appendArgument(Name, (Value ? "true" : "false"));
}

// Append an int argument to the top trace item.
void PPCallbacksTracker::appendArgument(const char *Name, int Value) {
  std::string Str;
  llvm::raw_string_ostream SS(Str);
  SS << Value;
  appendArgument(Name, SS.str());
}

// Append a string argument to the top trace item.
void PPCallbacksTracker::appendArgument(const char *Name, const char *Value) {
  if (DisableTrace)
    return;
  CallbackCalls.back().Arguments.push_back(Argument{Name, Value});
}

// Append a string object argument to the top trace item.
void PPCallbacksTracker::appendArgument(const char *Name,
                                        llvm::StringRef Value) {
  appendArgument(Name, Value.str());
}

// Append a string object argument to the top trace item.
void PPCallbacksTracker::appendArgument(const char *Name,
                                        const std::string &Value) {
  appendArgument(Name, Value.c_str());
}

// Append a token argument to the top trace item.
void PPCallbacksTracker::appendArgument(const char *Name, const Token &Value) {
  appendArgument(Name, PP.getSpelling(Value));
}

// Append an enum argument to the top trace item.
void PPCallbacksTracker::appendArgument(const char *Name, int Value,
                                        const char *const Strings[]) {
  appendArgument(Name, Strings[Value]);
}

// Append a FileID argument to the top trace item.
void PPCallbacksTracker::appendArgument(const char *Name, FileID Value) {
  if (Value.isInvalid()) {
    appendArgument(Name, "(invalid)");
    return;
  }
  const FileEntry *FileEntry = PP.getSourceManager().getFileEntryForID(Value);
  if (!FileEntry) {
    appendArgument(Name, "(getFileEntryForID failed)");
    return;
  }
  appendFilePathArgument(Name, FileEntry->getName());
}

// Append a FileEntry argument to the top trace item.
void PPCallbacksTracker::appendArgument(const char *Name,
                                        Optional<FileEntryRef> Value) {
  if (!Value) {
    appendArgument(Name, "(null)");
    return;
  }
  appendArgument(Name, *Value);
}

void PPCallbacksTracker::appendArgument(const char *Name, FileEntryRef Value) {
  appendFilePathArgument(Name, Value.getName());
}

// Append a SourceLocation argument to the top trace item.
void PPCallbacksTracker::appendArgument(const char *Name,
                                        SourceLocation Value) {
  if (Value.isInvalid()) {
    appendArgument(Name, "(invalid)");
    return;
  }
  appendArgument(Name, getSourceLocationString(PP, Value).c_str());
}

// Append a SourceRange argument to the top trace item.
void PPCallbacksTracker::appendArgument(const char *Name, SourceRange Value) {
  if (DisableTrace)
    return;
  if (Value.isInvalid()) {
    appendArgument(Name, "(invalid)");
    return;
  }
  std::string Str;
  llvm::raw_string_ostream SS(Str);
  SS << "[" << getSourceLocationString(PP, Value.getBegin()) << ", "
     << getSourceLocationString(PP, Value.getEnd()) << "]";
  appendArgument(Name, SS.str());
}

// Append a CharSourceRange argument to the top trace item.
void PPCallbacksTracker::appendArgument(const char *Name,
                                        CharSourceRange Value) {
  if (Value.isInvalid()) {
    appendArgument(Name, "(invalid)");
    return;
  }
  appendArgument(Name, getSourceString(Value).str().c_str());
}

// Append a SourceLocation argument to the top trace item.
void PPCallbacksTracker::appendArgument(const char *Name, ModuleIdPath Value) {
  if (DisableTrace)
    return;
  std::string Str;
  llvm::raw_string_ostream SS(Str);
  SS << "[";
  for (int I = 0, E = Value.size(); I != E; ++I) {
    if (I)
      SS << ", ";
    SS << "{"
       << "Name: " << Value[I].first->getName() << ", "
       << "Loc: " << getSourceLocationString(PP, Value[I].second) << "}";
  }
  SS << "]";
  appendArgument(Name, SS.str());
}

// Append an IdentifierInfo argument to the top trace item.
void PPCallbacksTracker::appendArgument(const char *Name,
                                        const IdentifierInfo *Value) {
  if (!Value) {
    appendArgument(Name, "(null)");
    return;
  }
  appendArgument(Name, Value->getName().str().c_str());
}

// Append a MacroDirective argument to the top trace item.
void PPCallbacksTracker::appendArgument(const char *Name,
                                        const MacroDirective *Value) {
  if (!Value) {
    appendArgument(Name, "(null)");
    return;
  }
  appendArgument(Name, MacroDirectiveKindStrings[Value->getKind()]);
}

// Append a MacroDefinition argument to the top trace item.
void PPCallbacksTracker::appendArgument(const char *Name,
                                        const MacroDefinition &Value) {
  std::string Str;
  llvm::raw_string_ostream SS(Str);
  SS << "[";
  bool Any = false;
  if (Value.getLocalDirective()) {
    SS << "(local)";
    Any = true;
  }
  for (auto *MM : Value.getModuleMacros()) {
    if (Any) SS << ", ";
    SS << MM->getOwningModule()->getFullModuleName();
  }
  SS << "]";
  appendArgument(Name, SS.str());
}

// Append a MacroArgs argument to the top trace item.
void PPCallbacksTracker::appendArgument(const char *Name,
                                        const MacroArgs *Value) {
  if (!Value) {
    appendArgument(Name, "(null)");
    return;
  }
  std::string Str;
  llvm::raw_string_ostream SS(Str);
  SS << "[";

  // Each argument is is a series of contiguous Tokens, terminated by a eof.
  // Go through each argument printing tokens until we reach eof.
  for (unsigned I = 0; I < Value->getNumMacroArguments(); ++I) {
    const Token *Current = Value->getUnexpArgument(I);
    if (I)
      SS << ", ";
    bool First = true;
    while (Current->isNot(tok::eof)) {
      if (!First)
        SS << " ";
      // We need to be careful here because the arguments might not be legal in
      // YAML, so we use the token name for anything but identifiers and
      // numeric literals.
      if (Current->isAnyIdentifier() || Current->is(tok::numeric_constant)) {
        SS << PP.getSpelling(*Current);
      } else {
        SS << "<" << Current->getName() << ">";
      }
      ++Current;
      First = false;
    }
  }
  SS << "]";
  appendArgument(Name, SS.str());
}

// Append a Module argument to the top trace item.
void PPCallbacksTracker::appendArgument(const char *Name, const Module *Value) {
  if (!Value) {
    appendArgument(Name, "(null)");
    return;
  }
  appendArgument(Name, Value->Name.c_str());
}

// Append a double-quoted argument to the top trace item.
void PPCallbacksTracker::appendQuotedArgument(const char *Name,
                                              const std::string &Value) {
  std::string Str;
  llvm::raw_string_ostream SS(Str);
  SS << "\"" << Value << "\"";
  appendArgument(Name, SS.str());
}

// Append a double-quoted file path argument to the top trace item.
void PPCallbacksTracker::appendFilePathArgument(const char *Name,
                                                llvm::StringRef Value) {
  std::string Path(Value);
  // YAML treats backslash as escape, so use forward slashes.
  std::replace(Path.begin(), Path.end(), '\\', '/');
  appendQuotedArgument(Name, Path);
}

// Get the raw source string of the range.
llvm::StringRef PPCallbacksTracker::getSourceString(CharSourceRange Range) {
  const char *B = PP.getSourceManager().getCharacterData(Range.getBegin());
  const char *E = PP.getSourceManager().getCharacterData(Range.getEnd());
  return llvm::StringRef(B, E - B);
}

} // namespace pp_trace
} // namespace clang
