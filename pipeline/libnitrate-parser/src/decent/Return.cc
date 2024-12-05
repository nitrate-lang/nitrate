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

#include "nitrate-parser/Node.h"

using namespace qparse;

Stmt *qparse::recurse_return(qparse_t &S, qlex_t &rd) {
  qlex_tok_t tok = peek();

  if (tok.is<qPuncSemi>()) {
    next();
    auto R = ReturnStmt::get();
    R->set_end_pos(tok.end);
    return R;
  }

  Expr *expr = recurse_expr(S, rd, {qlex_tok_t(qPunc, qPuncSemi)});

  tok = next();

  if (!tok.is<qPuncSemi>()) {
    diagnostic << tok << "Expected a semicolon after the return statement.";
    return mock_stmt(QAST_NODE_RETURN);
  }

  auto R = ReturnStmt::get(expr);
  R->set_end_pos(tok.end);

  return R;
}

Stmt *qparse::recurse_retif(qparse_t &S, qlex_t &rd) {
  Expr *condition = recurse_expr(S, rd, {qlex_tok_t(qPunc, qPuncComa)});

  qlex_tok_t tok = next();
  if (!tok.is<qPuncComa>()) {
    diagnostic << tok << "Expected a comma after the return-if expression.";
    return mock_stmt(QAST_NODE_RETIF);
  }

  Expr *return_expr = recurse_expr(S, rd, {qlex_tok_t(qPunc, qPuncSemi)});

  tok = next();
  if (!tok.is<qPuncSemi>()) {
    diagnostic << tok << "Expected a semicolon after the return-if expression.";
    return mock_stmt(QAST_NODE_RETIF);
  }

  auto R = ReturnIfStmt::get(condition, return_expr);
  R->set_end_pos(tok.end);

  return R;
}
