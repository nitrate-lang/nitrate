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

using namespace qparse;
using namespace qparse::parser;

bool qparse::parser::parse_for(qparse_t &job, qlex_t *rd, Stmt **node) {
  Stmt *x0 = nullptr;
  Expr *x1 = nullptr, *x2 = nullptr;

  qlex_tok_t tok = qlex_peek(rd);
  if (tok.is<qPuncLPar>()) {
    tok = qlex_next(rd);
    tok = qlex_peek(rd);

    if (tok.is<qKLet>()) {
      qlex_next(rd);
      StmtListItems let_node;
      if (!parse_let(job, rd, let_node)) {
        /// TODO: Write the ERROR message
        return false;
      }

      x0 = StmtList::get(let_node);
    } else {
      Expr *x0_tmp = nullptr;
      if (!parse_expr(job, rd, {qlex_tok_t(qPunc, qPuncSemi)}, &x0_tmp)) {
        return false;
      }
      x0 = ExprStmt::get(x0_tmp);

      tok = qlex_next(rd);
      if (!tok.is<qPuncSemi>()) {
        /// TODO: Write the ERROR message
        return false;
      }
    }

    if (!parse_expr(job, rd, {qlex_tok_t(qPunc, qPuncSemi)}, &x1)) return false;

    tok = qlex_next(rd);
    if (!tok.is<qPuncSemi>()) {
      /// TODO: Write the ERROR message
      return false;
    }

    if (!parse_expr(job, rd, {qlex_tok_t(qPunc, qPuncRPar)}, &x2)) return false;

    tok = qlex_next(rd);
    if (!tok.is<qPuncRPar>()) {
      /// TODO: Write the ERROR message
      return false;
    }

    Block *then_block = nullptr;

    if (qlex_peek(rd).is<qOpArrow>()) {
      tok = qlex_next(rd);
      if (!parse(job, rd, &then_block, false, true)) return false;
    } else {
      if (!parse(job, rd, &then_block, true)) return false;
    }

    *node = ForStmt::get(x0, x1, x2, then_block);

    return true;
  } else {
    tok = qlex_peek(rd);

    if (tok.is<qKLet>()) {
      qlex_next(rd);
      StmtListItems let_node;
      if (!parse_let(job, rd, let_node)) {
        /// TODO: Write the ERROR message
        return false;
      }

      x0 = StmtList::get(let_node);
    } else {
      Expr *x0_tmp = nullptr;
      if (!parse_expr(job, rd, {qlex_tok_t(qPunc, qPuncSemi)}, &x0_tmp)) {
        return false;
      }
      x0 = ExprStmt::get(x0_tmp);

      tok = qlex_next(rd);
      if (!tok.is<qPuncSemi>()) {
        /// TODO: Write the ERROR message
        return false;
      }
    }

    if (!parse_expr(job, rd, {qlex_tok_t(qPunc, qPuncSemi)}, &x1)) return false;

    tok = qlex_next(rd);
    if (!tok.is<qPuncSemi>()) {
      /// TODO: Write the ERROR message
      return false;
    }

    if (!parse_expr(job, rd, {qlex_tok_t(qPunc, qPuncLCur), qlex_tok_t(qOper, qOpArrow)}, &x2))
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
