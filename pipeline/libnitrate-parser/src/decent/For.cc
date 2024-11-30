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

#include <nitrate-parser/Node.h>

#include <decent/Recurse.hh>

qparse::Stmt *qparse::recurse_for(qparse_t &S, qlex_t &rd) {
  Expr *x0 = nullptr, *x1 = nullptr, *x2 = nullptr;

  qlex_tok_t tok = peek();
  if (tok.is<qPuncLPar>()) {
    tok = next();
    tok = peek();

    if (tok.is<qKLet>()) {
      next();
      std::vector<Stmt *> let_node;
      if (!recurse_let(S, rd, let_node)) {
        syntax(tok, "Failed to parse let statement in for loop");
        return mock_stmt(QAST_NODE_FOR);
      }

      if (let_node.size() != 1) {
        syntax(tok, "Expected let statement to have exactly one declaration");
        return mock_stmt(QAST_NODE_FOR);

      } else {
        x0 = StmtExpr::get(let_node[0]);
      }
    } else {
      if (!recurse_expr(S, rd, {qlex_tok_t(qPunc, qPuncSemi)}, &x0)) {
        syntax(tok, "Failed to parse for loop initializer");
        return mock_stmt(QAST_NODE_FOR);
      }

      tok = next();
      if (!tok.is<qPuncSemi>()) {
        syntax(tok, "Expected ';' after for loop initializer");
        return mock_stmt(QAST_NODE_FOR);
      }
    }

    if (!recurse_expr(S, rd, {qlex_tok_t(qPunc, qPuncSemi)}, &x1)) {
      syntax(tok, "Failed to parse for loop condition");
      return mock_stmt(QAST_NODE_FOR);
    }

    tok = next();
    if (!tok.is<qPuncSemi>()) {
      syntax(tok, "Expected ';' after for loop condition");
      return mock_stmt(QAST_NODE_FOR);
    }

    if (!recurse_expr(S, rd, {qlex_tok_t(qPunc, qPuncRPar)}, &x2)) {
      syntax(tok, "Failed to parse for loop increment");
      return mock_stmt(QAST_NODE_FOR);
    }
    tok = next();
    if (!tok.is<qPuncRPar>()) {
      syntax(tok, "Expected ')' after for loop increment");
      return mock_stmt(QAST_NODE_FOR);
    }

    Stmt *then_block = nullptr;

    if (peek().is<qOpArrow>()) {
      tok = next();
      then_block = recurse(S, rd, false, true);
    } else {
      then_block = recurse(S, rd, true);
    }

    return ForStmt::get(x0, x1, x2, then_block);
  } else {
    tok = peek();

    if (tok.is<qKLet>()) {
      next();
      std::vector<Stmt *> let_node;
      if (!recurse_let(S, rd, let_node)) {
        syntax(tok, "Failed to parse let statement in for loop");
        return mock_stmt(QAST_NODE_FOR);
      }

      if (let_node.size() != 1) {
        syntax(tok, "Expected let statement to have exactly one declaration");
        return mock_stmt(QAST_NODE_FOR);
      } else {
        x0 = StmtExpr::get(let_node[0]);
      }
    } else {
      if (!recurse_expr(S, rd, {qlex_tok_t(qPunc, qPuncSemi)}, &x0)) {
        return mock_stmt(QAST_NODE_FOR);
      }

      tok = next();
      if (!tok.is<qPuncSemi>()) {
        syntax(tok, "Expected ';' after for loop initializer");
        return mock_stmt(QAST_NODE_FOR);
      }
    }

    if (!recurse_expr(S, rd, {qlex_tok_t(qPunc, qPuncSemi)}, &x1)) {
      return mock_stmt(QAST_NODE_FOR);
    };

    tok = next();
    if (!tok.is<qPuncSemi>()) {
      syntax(tok, "Expected ';' after for loop condition");
      return mock_stmt(QAST_NODE_FOR);
    }

    if (!recurse_expr(
            S, rd, {qlex_tok_t(qPunc, qPuncLCur), qlex_tok_t(qOper, qOpArrow)},
            &x2)) {
      return mock_stmt(QAST_NODE_FOR);
    }

    Stmt *then_block = nullptr;

    if (peek().is<qOpArrow>()) {
      tok = next();
      then_block = recurse(S, rd, false, true);
    } else {
      then_block = recurse(S, rd, true);
    }

    return ForStmt::get(x0, x1, x2, then_block);
  }
}
