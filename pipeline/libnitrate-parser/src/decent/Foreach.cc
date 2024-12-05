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

qparse::Stmt *qparse::recurse_foreach(qparse_t &S, qlex_t &rd) {
  qlex_tok_t tok = next();
  bool has_parens = false;

  if (tok.is<qPuncLPar>()) {
    has_parens = true;
    tok = next();
  }

  if (!tok.is(qName)) {
    diagnostic << tok
               << "Expected identifier as index variable in foreach statement";
    return mock_stmt(QAST_NODE_FOREACH);
  }
  std::string first_ident = tok.as_string(&rd), second_ident;

  tok = next();

  if (tok.is<qPuncComa>()) {
    tok = next();
    if (!tok.is(qName)) {
      diagnostic
          << tok
          << "Expected identifier as value variable in foreach statement";
      return mock_stmt(QAST_NODE_FOREACH);
    }

    second_ident = tok.as_string(&rd);

    tok = next();
  } else {
    second_ident = "_";
  }

  if (!tok.is<qOpIn>()) {
    diagnostic << tok
               << "Expected 'in' after value variable in foreach statement";
    return mock_stmt(QAST_NODE_FOREACH);
  }

  Expr *expr = nullptr;
  if (has_parens) {
    expr = recurse_expr(S, rd, {qlex_tok_t(qPunc, qPuncRPar)});
    tok = next();
    if (!tok.is<qPuncRPar>()) {
      diagnostic << tok << "Expected ')' after expression in foreach statement";
      return mock_stmt(QAST_NODE_FOREACH);
    }
  } else {
    expr = recurse_expr(
        S, rd, {qlex_tok_t(qPunc, qPuncLCur), qlex_tok_t(qOper, qOpArrow)});
  }

  tok = peek();

  Stmt *block = nullptr;
  if (tok.is<qOpArrow>()) {
    next();
    block = recurse_block(S, rd, false, true);
  } else {
    block = recurse_block(S, rd);
  }

  auto R = ForeachStmt::get(first_ident, second_ident, expr, block);
  R->set_end_pos(block->get_end_pos());
  return R;
}
