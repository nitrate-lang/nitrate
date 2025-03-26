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

#include <nitrate/code.h>

#include <atomic>
#include <nitrate-core/Init.hh>
#include <nitrate-core/Macro.hh>
#include <nitrate-lexer/Init.hh>
#include <nitrate-parser/Init.hh>
#include <nitrate-seq/Init.hh>

static std::atomic<size_t> NitLibRefCount = 0;

auto NitLibInit() -> bool {
  if (NitLibRefCount++ > 1) {
    return true;
  }

  if (!ncc::CoreLibrary.InitRC()) {
    return false;
  }

  if (!ncc::lex::LexerLibrary.InitRC()) {
    return false;
  }

  if (!ncc::seq::SeqLibrary.InitRC()) {
    return false;
  }

  if (!ncc::parse::ParseLibrary.InitRC()) {
    return false;
  }

  return true;
}

void NitDeinit() {
  if (--NitLibRefCount > 0) {
    return;
  }

  ncc::parse::ParseLibrary.DeinitRC();
  ncc::seq::SeqLibrary.DeinitRC();
  ncc::lex::LexerLibrary.DeinitRC();
  ncc::CoreLibrary.DeinitRC();
}
