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

#include <decent/Recurse.hh>

using namespace qparse;

bool qparse::recurse_pub(qparse_t &S, qlex_t &rd, Stmt **node) {
  qlex_tok_t tok = peek();

  String abiName;

  if (tok.is(qText)) {
    next();

    abiName = tok.as_string(&rd);
    std::transform(abiName.begin(), abiName.end(), abiName.begin(), ::tolower);

    tok = peek();
  }

  if (tok.is<qPuncLCur>()) {
    Block *block = nullptr;
    if (!recurse(S, rd, &block, true)) {
      return false;
    }

    *node = ExportDecl::get(block, abiName);
    (*node)->set_end_pos(block->get_end_pos());
    return true;
  }

  Block *block = nullptr;
  if (!recurse(S, rd, &block, false, true)) {
    syntax(tok, "Expected block or statement list after 'pub'");
    return false;
  }

  *node = ExportDecl::get(block, abiName);
  (*node)->set_end_pos(block->get_end_pos());
  return true;
}

bool qparse::recurse_sec(qparse_t &S, qlex_t &rd, Stmt **node) {
  qlex_tok_t tok = peek();

  String abiName;

  if (tok.is(qText)) {
    next();

    abiName = tok.as_string(&rd);
    std::transform(abiName.begin(), abiName.end(), abiName.begin(), ::tolower);

    tok = peek();
  }

  if (tok.is<qPuncLCur>()) {
    Block *block = nullptr;
    if (!recurse(S, rd, &block, true)) return false;

    *node = block;
    (*node)->set_end_pos(block->get_end_pos());
    return true;
  }

  Block *block = nullptr;
  if (!recurse(S, rd, &block, false, true)) {
    syntax(tok, "Expected block or statement list after 'sec'");
    return false;
  }

  *node = block;
  (*node)->set_end_pos(block->get_end_pos());
  return true;
}

bool qparse::recurse_pro(qparse_t &S, qlex_t &rd, Stmt **node) {
  qlex_tok_t tok = peek();

  String abiName;

  if (tok.is(qText)) {
    next();

    abiName = tok.as_string(&rd);
    std::transform(abiName.begin(), abiName.end(), abiName.begin(), ::tolower);

    tok = peek();
  }

  if (tok.is<qPuncLCur>()) {
    Block *block = nullptr;
    if (!recurse(S, rd, &block, true)) return false;

    *node = block;
    (*node)->set_end_pos(block->get_end_pos());
    return true;
  }

  Block *block = nullptr;
  if (!recurse(S, rd, &block, false, true)) {
    syntax(tok, "Expected block or statement list after 'pro'");
    return false;
  }

  *node = block;
  (*node)->set_end_pos(block->get_end_pos());

  return true;
}
