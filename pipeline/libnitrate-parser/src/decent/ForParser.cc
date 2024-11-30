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

/// TODO: Source location

#include <decent/Parse.h>

using namespace qparse;
using namespace qparse::parser;
using namespace qparse::diag;

bool qparse::parser::parse_for(qparse_t &job, qlex_t *rd, Stmt **node) {
  Expr *x0 = nullptr, *x1 = nullptr, *x2 = nullptr;

  qlex_tok_t tok = qlex_peek(rd);
  if (tok.is<qPuncLPar>()) {
    tok = qlex_next(rd);
    tok = qlex_peek(rd);

    if (tok.is<qKLet>()) {
      qlex_next(rd);
      std::vector<Stmt *> let_node;
      if (!parse_let(job, rd, let_node)) {
        syntax(tok, "Failed to parse let statement in for loop");
      }

      if (let_node.size() != 1) {
        syntax(tok, "Expected let statement to have exactly one declaration");
      } else {
        x0 = StmtExpr::get(let_node[0]);
      }
    } else {
      if (!parse_expr(job, rd, {qlex_tok_t(qPunc, qPuncSemi)}, &x0)) {
        syntax(tok, "Failed to parse for loop initializer");
      }

      tok = qlex_next(rd);
      if (!tok.is<qPuncSemi>()) {
        syntax(tok, "Expected ';' after for loop initializer");
      }
    }

    if (!parse_expr(job, rd, {qlex_tok_t(qPunc, qPuncSemi)}, &x1)) {
      syntax(tok, "Failed to parse for loop condition");
    }

    tok = qlex_next(rd);
    if (!tok.is<qPuncSemi>()) {
      syntax(tok, "Expected ';' after for loop condition");
    }

    if (!parse_expr(job, rd, {qlex_tok_t(qPunc, qPuncRPar)}, &x2)) {
      syntax(tok, "Failed to parse for loop increment");
      return false;
    }
    tok = qlex_next(rd);
    if (!tok.is<qPuncRPar>()) {
      syntax(tok, "Expected ')' after for loop increment");
      return false;
    }

    Block *then_block = nullptr;

    if (qlex_peek(rd).is<qOpArrow>()) {
      tok = qlex_next(rd);
      if (!parse(job, rd, &then_block, false, true)) {
        syntax(tok, "Expected single statement after '=>' in for loop");
      }
    } else {
      if (!parse(job, rd, &then_block, true)) {
        syntax(tok, "Failed to parse block in for loop");
      }
    }

    *node = ForStmt::get(x0, x1, x2, then_block);

    return true;
  } else {
    tok = qlex_peek(rd);

    if (tok.is<qKLet>()) {
      qlex_next(rd);
      std::vector<Stmt *> let_node;
      if (!parse_let(job, rd, let_node)) {
        syntax(tok, "Failed to parse let statement in for loop");
      }

      if (let_node.size() != 1) {
        syntax(tok, "Expected let statement to have exactly one declaration");
      } else {
        x0 = StmtExpr::get(let_node[0]);
      }
    } else {
      if (!parse_expr(job, rd, {qlex_tok_t(qPunc, qPuncSemi)}, &x0)) {
        return false;
      }

      tok = qlex_next(rd);
      if (!tok.is<qPuncSemi>()) {
        syntax(tok, "Expected ';' after for loop initializer");
      }
    }

    if (!parse_expr(job, rd, {qlex_tok_t(qPunc, qPuncSemi)}, &x1)) return false;

    tok = qlex_next(rd);
    if (!tok.is<qPuncSemi>()) {
      syntax(tok, "Expected ';' after for loop condition");
    }

    if (!parse_expr(job, rd,
                    {qlex_tok_t(qPunc, qPuncLCur), qlex_tok_t(qOper, qOpArrow)},
                    &x2))
      return false;

    Block *then_block = nullptr;

    if (qlex_peek(rd).is<qOpArrow>()) {
      tok = qlex_next(rd);
      if (!parse(job, rd, &then_block, false, true)) return false;
    } else {
      if (!parse(job, rd, &then_block, true)) return false;
    }

    *node = ForStmt::get(x0, x1, x2, then_block);

    return true;
  }
}
