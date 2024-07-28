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

bool qparse::parser::parse_return(qparse_t &job, qlex_t *rd, Stmt **node) {
  qlex_tok_t tok = qlex_peek(rd);

  if (tok.is<qPuncSemi>()) {
    qlex_next(rd);
    *node = ReturnStmt::get();
    return true;
  }

  Expr *expr = nullptr;
  if (!parse_expr(job, rd, {qlex_tok_t(qPunc, qPuncSemi)}, &expr)) return false;
  *node = ReturnStmt::get(expr);

  tok = qlex_next(rd);

  if (!tok.is<qPuncSemi>()) {
    /// TODO: Write the ERROR message
    return false;
  }

  return true;
}

bool qparse::parser::parse_retif(qparse_t &job, qlex_t *rd, Stmt **node) {
  qlex_tok_t tok;

  Expr *return_expr = nullptr;
  if (!parse_expr(job, rd, {qlex_tok_t(qPunc, qPuncComa)}, &return_expr)) return false;

  tok = qlex_next(rd);
  if (!tok.is<qPuncComa>()) {
    /// TODO: Write the ERROR message
    return false;
  }

  Expr *condition = nullptr;
  if (!parse_expr(job, rd, {qlex_tok_t(qPunc, qPuncSemi)}, &condition)) return false;

  tok = qlex_next(rd);
  if (!tok.is<qPuncSemi>()) {
    /// TODO: Write the ERROR message
    return false;
  }
  *node = ReturnIfStmt::get(condition, return_expr);

  return true;
}

bool qparse::parser::parse_retz(qparse_t &job, qlex_t *rd, Stmt **node) {
  qlex_tok_t tok;

  Expr *return_expr = nullptr;
  if (!parse_expr(job, rd, {qlex_tok_t(qPunc, qPuncComa)}, &return_expr)) return false;

  tok = qlex_next(rd);
  if (!tok.is<qPuncComa>()) {
    /// TODO: Write the ERROR message
    return false;
  }

  Expr *condition = nullptr;
  if (!parse_expr(job, rd, {qlex_tok_t(qPunc, qPuncSemi)}, &condition)) return false;

  tok = qlex_next(rd);
  if (!tok.is<qPuncSemi>()) {
    /// TODO: Write the ERROR message
    return false;
  }
  *node = RetZStmt::get(condition, return_expr);

  return true;
}

bool qparse::parser::parse_retv(qparse_t &job, qlex_t *rd, Stmt **node) {
  qlex_tok_t tok;

  Expr *cond = nullptr;
  if (!parse_expr(job, rd, {qlex_tok_t(qPunc, qPuncSemi)}, &cond)) return false;

  tok = qlex_next(rd);
  if (!tok.is<qPuncSemi>()) {
    /// TODO: Write the ERROR message
    return false;
  }

  *node = RetVStmt::get(cond);

  return true;
}