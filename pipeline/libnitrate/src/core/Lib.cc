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

#include <nitrate-emit/Lib.h>
#include <nitrate-ir/Lib.h>
#include <nitrate-seq/Lib.h>
#include <nitrate/code.h>

#include <atomic>
#include <nitrate-core/Init.hh>
#include <nitrate-core/Macro.hh>
#include <nitrate-lexer/Init.hh>
#include <nitrate-parser/Init.hh>

static std::atomic<size_t> nit_lib_ref_count = 0;

bool nit_lib_init() {
  if (nit_lib_ref_count++ > 1) {
    return true;
  }

  if (!ncc::CoreLibrary.InitRC()) {
    return false;
  }

  if (!ncc::lex::LexerLibrary.InitRC()) {
    return false;
  }

  if (!qprep_lib_init()) {
    return false;
  }

  if (!ncc::parse::ParseLibrary.InitRC()) {
    return false;
  }

  if (!nr_lib_init()) {
    return false;
  }

  if (!qcode_lib_init()) {
    return false;
  }

  return true;
}

void nit_deinit() {
  if (--nit_lib_ref_count > 0) {
    return;
  }

  qcode_lib_deinit();
  nr_lib_deinit();
  ncc::parse::ParseLibrary.DeinitRC();
  qprep_lib_deinit();
  ncc::lex::LexerLibrary.DeinitRC();
  ncc::CoreLibrary.DeinitRC();
}
