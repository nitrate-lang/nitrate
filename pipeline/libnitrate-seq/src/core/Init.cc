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

#include <core/EC.hh>
#include <nitrate-core/Init.hh>
#include <nitrate-core/Logger.hh>
#include <nitrate-core/Macro.hh>
#include <nitrate-lexer/Init.hh>
#include <nitrate-seq/Init.hh>

using namespace ncc::seq;

NCC_EXPORT ncc::LibraryRC<SeqLibrarySetup> ncc::seq::SeqLibrary;

NCC_EXPORT auto SeqLibrarySetup::Init() -> bool {
  Log << ec::SeqLog << Debug << "libnitrate-seq initializing...";

  if (!ncc::CoreLibrary.InitRC()) {
    Log << ec::SeqLog << "libnitrate-seq init failed: libnitrate-core failed to initialize";
    return false;
  }

  if (!ncc::lex::LexerLibrary.InitRC()) {
    Log << ec::SeqLog << "libnitrate-seq init failed: libnitrate-lexer failed to initialize";
    return false;
  }

  Log << ec::SeqLog << Debug << "libnitrate-seq initialized";

  return true;
}

NCC_EXPORT void SeqLibrarySetup::Deinit() {
  Log << ec::SeqLog << Debug << "libnitrate-seq deinitializing...";

  ncc::lex::LexerLibrary.DeinitRC();
  ncc::CoreLibrary.DeinitRC();

  Log << ec::SeqLog << Debug << "libnitrate-seq deinitialized";
}

NCC_EXPORT auto SeqLibrarySetup::GetVersionId() -> std::string_view { return __TARGET_VERSION; }

NCC_EXPORT auto SeqLibrarySetup::GetSemVersion() -> std::array<uint32_t, 3> {
  return {__TARGET_MAJOR_VERSION, __TARGET_MINOR_VERSION, __TARGET_PATCH_VERSION};
}
