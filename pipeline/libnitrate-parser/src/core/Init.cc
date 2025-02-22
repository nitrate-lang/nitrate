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

#include <google/protobuf/stubs/common.h>

#include <core/SyntaxDiagnostics.hh>
#include <memory>
#include <nitrate-core/Init.hh>
#include <nitrate-core/Logger.hh>
#include <nitrate-core/Macro.hh>
#include <nitrate-lexer/Init.hh>
#include <nitrate-parser/ASTBase.hh>
#include <nitrate-parser/ASTData.hh>
#include <nitrate-parser/Init.hh>

using namespace ncc::parse;

NCC_EXPORT ncc::LibraryRC<ParseLibrarySetup> ncc::parse::ParseLibrary;

NCC_EC_EX(ec::ParseEG, Runtime, ec::Formatter);

NCC_EXPORT auto ParseLibrarySetup::Init() -> bool {
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  Log << Runtime << Debug << "libnitrate-parser: initializing...";

  if (!ncc::CoreLibrary.InitRC()) {
    Log << Runtime << "libnitrate-parser init failed: libnitrate-core failed to initialize";
    return false;
  }

  if (!ncc::lex::LexerLibrary.InitRC()) {
    Log << Runtime << "libnitrate-parser init failed: libnitrate-lexer failed to initialize";
    return false;
  }

  Log << Runtime << Debug << "libnitrate-parser: initialized";

  return true;
}

NCC_EXPORT void ParseLibrarySetup::Deinit() {
  Log << Runtime << Debug << "libnitrate-parser: deinitializing...";

  ExtensionDataStore.Reset();
  parse::MainAllocator = std::make_unique<DynamicArena>();

  ncc::lex::LexerLibrary.DeinitRC();
  ncc::CoreLibrary.DeinitRC();

  Log << Runtime << Debug << "libnitrate-parser: deinitialized";
}

NCC_EXPORT auto ParseLibrarySetup::GetVersionId() -> std::string_view { return __TARGET_VERSION; }
