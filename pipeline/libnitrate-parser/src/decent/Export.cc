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

/// TODO: Cleanup this code; it's a mess from refactoring.

#include <decent/Recurse.hh>

npar::Stmt *npar::recurse_pub(npar_t &S, qlex_t &rd) {
  qlex_tok_t tok = peek();

  std::string_view abiName;

  if (tok.is(qText)) {
    next();

    abiName = tok.as_string(&rd);

    tok = peek();
  }

  if (tok.is<qPuncLCur>()) {
    Stmt *block = recurse_block(S, rd, true);

    auto R = ExportDecl::get(block, abiName);
    R->set_end_pos(block->get_end_pos());
    return R;
  }

  Stmt *block = recurse_block(S, rd, false, true);

  auto R = ExportDecl::get(block, abiName);
  R->set_end_pos(block->get_end_pos());
  return R;
}

npar::Stmt *npar::recurse_sec(npar_t &S, qlex_t &rd) {
  qlex_tok_t tok = peek();

  std::string_view abiName;

  if (tok.is(qText)) {
    next();

    abiName = tok.as_string(&rd);

    tok = peek();
  }

  if (tok.is<qPuncLCur>()) {
    Stmt *block = recurse_block(S, rd, true);

    block->set_end_pos(block->get_end_pos());
    return block;
  }

  Stmt *block = recurse_block(S, rd, false, true);

  block->set_end_pos(block->get_end_pos());
  return block;
}

npar::Stmt *npar::recurse_pro(npar_t &S, qlex_t &rd) {
  qlex_tok_t tok = peek();

  std::string_view abiName;

  if (tok.is(qText)) {
    next();

    abiName = tok.as_string(&rd);

    tok = peek();
  }

  if (tok.is<qPuncLCur>()) {
    Stmt *block = recurse_block(S, rd, true);

    block->set_end_pos(block->get_end_pos());
    return block;
  }

  Stmt *block = recurse_block(S, rd, false, true);

  block->set_end_pos(block->get_end_pos());

  return block;
}
