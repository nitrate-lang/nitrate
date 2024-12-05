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

qparse::Stmt *qparse::recurse_switch(qparse_t &S, qlex_t &rd) {
  Expr *cond = recurse_expr(S, rd, {qlex_tok_t(qPunc, qPuncLCur)});

  SwitchCases cases;
  Stmt *default_case = nullptr;

  qlex_tok_t tok = next();
  if (!tok.is<qPuncLCur>()) {
    diagnostic << tok << "Expected '{' after switch condition";
    return mock_stmt(QAST_NODE_SWITCH);
  }

  while ((tok = peek()).ty != qEofF) {
    if (tok.is<qPuncRCur>()) {
      break;
    }

    if (tok.is<qKDefault>()) {
      next();
      if (default_case) {
        diagnostic << tok << "Multiple default cases in switch statement";
        return mock_stmt(QAST_NODE_SWITCH);
      }

      tok = next();
      if (!tok.is<qPuncColn>()) {
        diagnostic << tok << "Expected ':' after 'default' keyword";
        return mock_stmt(QAST_NODE_SWITCH);
      }

      default_case = recurse(S, rd);

      continue;
    }

    if (!tok.is<qKCase>()) {
      diagnostic << tok
                 << "Expected 'case' or 'default' keyword in switch statement";
      return mock_stmt(QAST_NODE_SWITCH);
    }
    next();

    Expr *case_expr = recurse_expr(S, rd, {qlex_tok_t(qPunc, qPuncColn)});

    tok = next();
    if (!tok.is<qPuncColn>()) {
      diagnostic << tok << "Expected ':' after case expression";
      return mock_stmt(QAST_NODE_SWITCH);
    }

    Stmt *case_block = recurse(S, rd);

    CaseStmt *case_stmt = CaseStmt::get(case_expr, case_block);
    case_stmt->set_end_pos(case_block->get_end_pos());
    cases.push_back(case_stmt);
  }

  tok = next();
  if (!tok.is<qPuncRCur>()) {
    diagnostic << tok << "Expected '}' after switch statement";
    return mock_stmt(QAST_NODE_SWITCH);
  }

  auto R = SwitchStmt::get(cond, cases, default_case);
  R->set_end_pos(tok.end);
  return R;
}
