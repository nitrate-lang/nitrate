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

/// TODO: Source location

#include <nitrate-parser/Node.h>

#include <descent/Recurse.hh>

npar::Stmt *npar::recurse_for(npar_t &S, qlex_t &rd) {
  Expr *x0 = nullptr, *x1 = nullptr, *x2 = nullptr;

  qlex_tok_t tok = peek();
  if (tok.is<qPuncLPar>()) {
    tok = next();
    tok = peek();

    if (tok.is<qKLet>()) {
      next();
      std::vector<Stmt *> let_node = recurse_variable(S, rd, VarDeclType::Let);

      if (let_node.size() != 1) {
        diagnostic << tok
                   << "Expected let statement to have exactly one declaration";
        return mock_stmt(QAST_NODE_FOR);

      } else {
        x0 = StmtExpr::get(let_node[0]);
      }
    } else {
      x0 = recurse_expr(S, rd, {qlex_tok_t(qPunc, qPuncSemi)});

      tok = next();
      if (!tok.is<qPuncSemi>()) {
        diagnostic << tok << "Expected ';' after for loop initializer";
        return mock_stmt(QAST_NODE_FOR);
      }
    }

    x1 = recurse_expr(S, rd, {qlex_tok_t(qPunc, qPuncSemi)});

    tok = next();
    if (!tok.is<qPuncSemi>()) {
      diagnostic << tok << "Expected ';' after for loop condition";
      return mock_stmt(QAST_NODE_FOR);
    }

    x2 = recurse_expr(S, rd, {qlex_tok_t(qPunc, qPuncRPar)});
    tok = next();
    if (!tok.is<qPuncRPar>()) {
      diagnostic << tok << "Expected ')' after for loop increment";
      return mock_stmt(QAST_NODE_FOR);
    }

    Stmt *then_block = nullptr;

    if (peek().is<qOpArrow>()) {
      tok = next();
      then_block = recurse_block(S, rd, false, true);
    } else {
      then_block = recurse_block(S, rd, true);
    }

    return ForStmt::get(x0, x1, x2, then_block);
  } else {
    tok = peek();

    if (tok.is<qKLet>()) {
      next();
      std::vector<Stmt *> let_node = recurse_variable(S, rd, VarDeclType::Let);

      if (let_node.size() != 1) {
        diagnostic << tok
                   << "Expected let statement to have exactly one declaration";
        return mock_stmt(QAST_NODE_FOR);
      } else {
        x0 = StmtExpr::get(let_node[0]);
      }
    } else {
      x0 = recurse_expr(S, rd, {qlex_tok_t(qPunc, qPuncSemi)});

      tok = next();
      if (!tok.is<qPuncSemi>()) {
        diagnostic << tok << "Expected ';' after for loop initializer";
        return mock_stmt(QAST_NODE_FOR);
      }
    }

    x1 = recurse_expr(S, rd, {qlex_tok_t(qPunc, qPuncSemi)});

    tok = next();
    if (!tok.is<qPuncSemi>()) {
      diagnostic << tok << "Expected ';' after for loop condition";
      return mock_stmt(QAST_NODE_FOR);
    }

    x2 = recurse_expr(
        S, rd, {qlex_tok_t(qPunc, qPuncLCur), qlex_tok_t(qOper, qOpArrow)});

    Stmt *then_block = nullptr;

    if (peek().is<qOpArrow>()) {
      tok = next();
      then_block = recurse_block(S, rd, false, true);
    } else {
      then_block = recurse_block(S, rd, true);
    }

    return ForStmt::get(x0, x1, x2, then_block);
  }
}
