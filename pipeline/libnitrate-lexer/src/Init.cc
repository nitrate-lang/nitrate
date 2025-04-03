////////////////////////////////////////////////////////////////////////////////
///                                                                          ///
///     .-----------------.    .----------------.     .----------------.     ///
///    | .--------------. |   | .--------------. |   | .--------------. |    ///
///    | | ____  _____  | |   | |     ____     | |   | |    ______    | |    ///
///    | ||_   _|_   _| | |   | |   .'    `.   | |   | |   / ____ `.  | |    ///
///    | |  |   \ | |   | |   | |  /  .--.  \  | |   | |   `'  __) |  | |    ///
///    | |  | |\ \| |   | |   | |  | |    | |  | |   | |   _  |__ '.  | |    ///
///    | | _| |_\   |_  | |   | |  \  `--'  /  | |   | |  | \____) |  | |    ///
///    | ||_____|\____| | |   | |   `.____.'   | |   | |   \______.'  | |    ///
///    | |              | |   | |              | |   | |              | |    ///
///    | '--------------' |   | '--------------' |   | '--------------' |    ///
///     '----------------'     '----------------'     '----------------'     ///
///                                                                          ///
///   * NITRATE TOOLCHAIN - The official toolchain for the Nitrate language. ///
///   * Copyright (C) 2024 Wesley C. Jones                                   ///
///                                                                          ///
///   The Nitrate Toolchain is free software; you can redistribute it or     ///
///   modify it under the terms of the GNU Lesser General Public             ///
///   License as published by the Free Software Foundation; either           ///
///   version 2.1 of the License, or (at your option) any later version.     ///
///                                                                          ///
///   The Nitrate Toolcain is distributed in the hope that it will be        ///
///   useful, but WITHOUT ANY WARRANTY; without even the implied warranty of ///
///   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      ///
///   Lesser General Public License for more details.                        ///
///                                                                          ///
///   You should have received a copy of the GNU Lesser General Public       ///
///   License along with the Nitrate Toolchain; if not, see                  ///
///   <https://www.gnu.org/licenses/>.                                       ///
///                                                                          ///
////////////////////////////////////////////////////////////////////////////////

#include <nitrate-core/Init.hh>
#include <nitrate-core/Logger.hh>
#include <nitrate-core/Macro.hh>
#include <nitrate-lexer/Init.hh>

#include "EC.hh"

using namespace ncc::lex;

NCC_EXPORT ncc::LibraryRC<LexerLibrarySetup> ncc::lex::LexerLibrary;

NCC_EXPORT auto LexerLibrarySetup::Init() -> bool {
  Log << Runtime << Trace << "libnitrate-lexer initializing...";

  if (!ncc::CoreLibrary.InitRC()) [[unlikely]] {
    Log << Runtime << "libnitrate-lexer failed init: libnitrate-core failed to initialize";
    return false;
  }

  Log << Runtime << Trace << "libnitrate-lexer initialized";

  return true;
}

NCC_EXPORT void LexerLibrarySetup::Deinit() {
  Log << Runtime << Trace << "libnitrate-lexer deinitializing...";

  ncc::CoreLibrary.DeinitRC();

  Log << Runtime << Trace << "libnitrate-lexer deinitialized";
}

NCC_EXPORT auto LexerLibrarySetup::GetSemVersion() -> std::array<uint32_t, 3> {
  return {__TARGET_MAJOR_VERSION, __TARGET_MINOR_VERSION, __TARGET_PATCH_VERSION};
}

NCC_EXPORT auto LexerLibrarySetup::BuildId() -> ncc::BuildId {
  return {__TARGET_COMMIT_HASH, __TARGET_COMMIT_DATE, __TARGET_COMMIT_BRANCH};
}

std::string ncc::lex::Formatter(std::string_view msg, Sev sev) {
  if (sev == Raw) {
    return std::string(msg);
  }

  if (sev <= ncc::Debug) {
    return "\x1b[37;1m[\x1b[0m\x1b[34;1mLexer\x1b[0m\x1b[37;1m]: debug:\x1b[0m " + std::string(msg);
  }

  return "\x1b[37;1m[\x1b[0m\x1b[34;1mLexer\x1b[0m\x1b[37;1m]:\x1b[0m " + std::string(msg);
}
