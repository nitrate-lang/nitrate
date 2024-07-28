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

#define __QUIX_IMPL__

#include "LibMacro.h"
#include "parser/Parse.h"

using namespace qparse::parser;

bool qparse::parser::parse_switch(qparse_t &job, qlex_t *rd, Stmt **node) {
  Expr *cond = nullptr;
  if (!parse_expr(job, rd, {qlex_tok_t(qPunc, qPuncLCur)}, &cond)) {
    return false;
  }

  SwitchCases cases;
  Block *default_case = nullptr;

  qlex_tok_t tok = qlex_next(rd);
  if (!tok.is<qPuncLCur>()) {
    /// TODO: Write the ERROR message
    return false;
  }

  while (true) {
    tok = qlex_peek(rd);
    if (tok.is<qPuncRCur>()) {
      break;
    }

    if (tok.is<qKDefault>()) {
      qlex_next(rd);
      if (default_case) {
        /// TODO: Write the ERROR message
        return false;
      }

      tok = qlex_next(rd);
      if (!tok.is<qPuncColn>()) {
        /// TODO: Write the ERROR message
        return false;
      }

      if (!parse(job, rd, &default_case)) {
        return false;
      }

      continue;
    }

    if (!tok.is<qKCase>()) {
      /// TODO: Write the ERROR message
      return false;
    }
    qlex_next(rd);

    Expr *case_expr = nullptr;
    if (!parse_expr(job, rd, {qlex_tok_t(qPunc, qPuncColn)}, &case_expr)) {
      return false;
    }

    tok = qlex_next(rd);
    if (!tok.is<qPuncColn>()) {
      /// TODO: Write the ERROR message
      return false;
    }

    Block *case_block = nullptr;
    if (!parse(job, rd, &case_block)) {
      return false;
    }

    cases.push_back(CaseStmt::get(case_expr, case_block));
  }

  tok = qlex_next(rd);
  if (!tok.is<qPuncRCur>()) {
    /// TODO: Write the ERROR message
    return false;
  }

  *node = SwitchStmt::get(cond, cases, default_case);

  return true;
}
