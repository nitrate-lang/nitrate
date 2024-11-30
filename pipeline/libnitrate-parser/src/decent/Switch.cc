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

bool qparse::recurse_switch(qparse_t &S, qlex_t &rd, Stmt **node) {
  Expr *cond = nullptr;
  if (!recurse_expr(S, rd, {qlex_tok_t(qPunc, qPuncLCur)}, &cond)) {
    syntax(peek(), "Expected switch condition");
  }

  SwitchCases cases;
  Stmt *default_case = nullptr;

  qlex_tok_t tok = next();
  if (!tok.is<qPuncLCur>()) {
    syntax(tok, "Expected '{' after switch condition");
  }

  while ((tok = peek()).ty != qEofF) {
    if (tok.is<qPuncRCur>()) {
      break;
    }

    if (tok.is<qKDefault>()) {
      next();
      if (default_case) {
        syntax(tok, "Multiple default cases in switch statement");
      }

      tok = next();
      if (!tok.is<qPuncColn>()) {
        syntax(tok, "Expected ':' after 'default' keyword");
      }

      default_case = recurse(S, rd);

      continue;
    }

    if (!tok.is<qKCase>()) {
      syntax(tok, "Expected 'case' or 'default' keyword in switch statement");
    }
    next();

    Expr *case_expr = nullptr;
    if (!recurse_expr(S, rd, {qlex_tok_t(qPunc, qPuncColn)}, &case_expr)) {
      syntax(peek(), "Expected case expression");
    }

    tok = next();
    if (!tok.is<qPuncColn>()) {
      syntax(tok, "Expected ':' after case expression");
    }

    Stmt *case_block = recurse(S, rd);

    CaseStmt *case_stmt = CaseStmt::get(case_expr, case_block);
    case_stmt->set_end_pos(case_block->get_end_pos());
    cases.push_back(case_stmt);
  }

  tok = next();
  if (!tok.is<qPuncRCur>()) {
    syntax(tok, "Expected '}' after switch statement");
  }

  *node = SwitchStmt::get(cond, cases, default_case);
  (*node)->set_end_pos(tok.end);

  return true;
}
