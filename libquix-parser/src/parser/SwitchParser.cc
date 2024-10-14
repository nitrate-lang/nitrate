////////////////////////////////////////////////////////////////////////////////
///                                                                          ///
///  ░▒▓██████▓▒░░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░ ░▒▓██████▓▒░  ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░        ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓██████▓▒░░▒▓█▓▒░      ░▒▓█▓▒░        ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░        ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ///
///  ░▒▓██████▓▒░ ░▒▓██████▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░ ░▒▓██████▓▒░  ///
///    ░▒▓█▓▒░                                                               ///
///     ░▒▓██▓▒░                                                             ///
///                                                                          ///
///   * QUIX LANG COMPILER - The official compiler for the Quix language.    ///
///   * Copyright (C) 2024 Wesley C. Jones                                   ///
///                                                                          ///
///   The QUIX Compiler Suite is free software; you can redistribute it or   ///
///   modify it under the terms of the GNU Lesser General Public             ///
///   License as published by the Free Software Foundation; either           ///
///   version 2.1 of the License, or (at your option) any later version.     ///
///                                                                          ///
///   The QUIX Compiler Suite is distributed in the hope that it will be     ///
///   useful, but WITHOUT ANY WARRANTY; without even the implied warranty of ///
///   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      ///
///   Lesser General Public License for more details.                        ///
///                                                                          ///
///   You should have received a copy of the GNU Lesser General Public       ///
///   License along with the QUIX Compiler Suite; if not, see                ///
///   <https://www.gnu.org/licenses/>.                                       ///
///                                                                          ///
////////////////////////////////////////////////////////////////////////////////

#include <parser/Parse.h>

using namespace qparse::parser;
using namespace qparse::diag;

bool qparse::parser::parse_switch(qparse_t &job, qlex_t *rd, Stmt **node) {
  Expr *cond = nullptr;
  if (!parse_expr(job, rd, {qlex_tok_t(qPunc, qPuncLCur)}, &cond)) {
    syntax(qlex_peek(rd), "Expected switch condition");
  }

  SwitchCases cases;
  Block *default_case = nullptr;

  qlex_tok_t tok = qlex_next(rd);
  if (!tok.is<qPuncLCur>()) {
    syntax(tok, "Expected '{' after switch condition");
  }

  while ((tok = qlex_peek(rd)).ty != qEofF) {
    if (tok.is<qPuncRCur>()) {
      break;
    }

    if (tok.is<qKDefault>()) {
      qlex_next(rd);
      if (default_case) {
        syntax(tok, "Multiple default cases in switch statement");
      }

      tok = qlex_next(rd);
      if (!tok.is<qPuncColn>()) {
        syntax(tok, "Expected ':' after 'default' keyword");
      }

      if (!parse(job, rd, &default_case)) {
        syntax(tok, "Expected block after 'default' keyword");
      }

      continue;
    }

    if (!tok.is<qKCase>()) {
      syntax(tok, "Expected 'case' or 'default' keyword in switch statement");
    }
    qlex_next(rd);

    Expr *case_expr = nullptr;
    if (!parse_expr(job, rd, {qlex_tok_t(qPunc, qPuncColn)}, &case_expr)) {
      syntax(qlex_peek(rd), "Expected case expression");
    }

    tok = qlex_next(rd);
    if (!tok.is<qPuncColn>()) {
      syntax(tok, "Expected ':' after case expression");
    }

    Block *case_block = nullptr;
    if (!parse(job, rd, &case_block)) {
      syntax(tok, "Expected block after case expression");
    }

    CaseStmt *case_stmt = CaseStmt::get(case_expr, case_block);
    case_stmt->set_end_pos(case_block->get_end_pos());
    cases.push_back(case_stmt);
  }

  tok = qlex_next(rd);
  if (!tok.is<qPuncRCur>()) {
    syntax(tok, "Expected '}' after switch statement");
  }

  *node = SwitchStmt::get(cond, cases, default_case);
  (*node)->set_end_pos(tok.end);

  return true;
}
