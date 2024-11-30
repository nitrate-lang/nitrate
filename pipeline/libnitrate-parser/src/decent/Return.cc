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

  Expr *expr = nullptr;
  if (!recurse_expr(S, rd, {qlex_tok_t(qPunc, qPuncSemi)}, &expr) || !expr) {
    syntax(tok, "Expected an expression in the return statement.");
    return mock_stmt(QAST_NODE_RETURN);
  }

  tok = next();

  if (!tok.is<qPuncSemi>()) {
    syntax(tok, "Expected a semicolon after the return statement.");
    return mock_stmt(QAST_NODE_RETURN);
  }

  auto R = ReturnStmt::get(expr);
  R->set_end_pos(tok.end);

  return R;
}

Stmt *qparse::recurse_retif(qparse_t &S, qlex_t &rd) {
  Expr *condition = nullptr;
  if (!recurse_expr(S, rd, {qlex_tok_t(qPunc, qPuncComa)}, &condition)) {
    syntax(peek(), "Expected a condition in the return-if statement.");
    return mock_stmt(QAST_NODE_RETIF);
  }

  qlex_tok_t tok = next();
  if (!tok.is<qPuncComa>()) {
    syntax(tok, "Expected a comma after the return-if expression.");
    return mock_stmt(QAST_NODE_RETIF);
  }

  Expr *return_expr = nullptr;
  if (!recurse_expr(S, rd, {qlex_tok_t(qPunc, qPuncSemi)}, &return_expr)) {
    syntax(tok, "Expected a return expression after the comma.");
    return mock_stmt(QAST_NODE_RETIF);
  }

  tok = next();
  if (!tok.is<qPuncSemi>()) {
    syntax(tok, "Expected a semicolon after the return-if expression.");
    return mock_stmt(QAST_NODE_RETIF);
  }

  auto R = ReturnIfStmt::get(condition, return_expr);
  R->set_end_pos(tok.end);

  return R;
}
