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

#include <nitrate-parser/Node.h>

#include <decent/Recurse.hh>

npar::Stmt *npar::recurse_typedef(npar_t &S, qlex_t &rd) {
  qlex_tok_t tok = next();
  if (!tok.is(qName)) {
    diagnostic << tok << "Expected name in typedef declaration";
    return mock_stmt(QAST_NODE_TYPEDEF);
  }

  auto name = tok.as_string(&rd);

  tok = next();
  if (!tok.is<qOpSet>()) {
    diagnostic << tok << "Expected '=' in typedef declaration";
    return mock_stmt(QAST_NODE_TYPEDEF);
  }

  Type *type = recurse_type(S, rd);

  tok = next();
  if (!tok.is<qPuncSemi>()) {
    diagnostic << tok << "Expected ';' in typedef declaration";
    return mock_stmt(QAST_NODE_TYPEDEF);
  }

  auto R = TypedefDecl::get(name, type);
  R->set_end_pos(tok.end);
  return R;
}
