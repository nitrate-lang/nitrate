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

#include <parser/Parse.h>

using namespace qparse::parser;
using namespace qparse::diag;

bool qparse::parser::parse_return(qparse_t &job, qlex_t *rd, Stmt **node) {
  qlex_tok_t tok = qlex_peek(rd);

  if (tok.is<qPuncSemi>()) {
    qlex_next(rd);
    *node = ReturnStmt::get();
    (*node)->set_end_pos(tok.end);
    return true;
  }

  Expr *expr = nullptr;
  if (!parse_expr(job, rd, {qlex_tok_t(qPunc, qPuncSemi)}, &expr) || !expr) {
    syntax(tok, "Expected an expression in the return statement.");
  }

  *node = ReturnStmt::get(expr);

  tok = qlex_next(rd);

  if (!tok.is<qPuncSemi>()) {
    syntax(tok, "Expected a semicolon after the return statement.");
  }

  (*node)->set_end_pos(tok.end);

  return true;
}

bool qparse::parser::parse_retif(qparse_t &job, qlex_t *rd, Stmt **node) {
  qlex_tok_t tok;

  Expr *condition = nullptr;
  if (!parse_expr(job, rd, {qlex_tok_t(qPunc, qPuncComa)}, &condition)) {
    syntax(tok, "Expected a condition in the return-if statement.");
  }

  tok = qlex_next(rd);
  if (!tok.is<qPuncComa>()) {
    syntax(tok, "Expected a comma after the return-if expression.");
  }

  Expr *return_expr = nullptr;
  if (!parse_expr(job, rd, {qlex_tok_t(qPunc, qPuncSemi)}, &return_expr)) {
    syntax(tok, "Expected a return expression after the comma.");
  }

  tok = qlex_next(rd);
  if (!tok.is<qPuncSemi>()) {
    syntax(tok, "Expected a semicolon after the return-if expression.");
  }
  *node = ReturnIfStmt::get(condition, return_expr);
  (*node)->set_end_pos(tok.end);

  return true;
}

bool qparse::parser::parse_retz(qparse_t &job, qlex_t *rd, Stmt **node) {
  qlex_tok_t tok;

  Expr *condition = nullptr;
  if (!parse_expr(job, rd, {qlex_tok_t(qPunc, qPuncComa)}, &condition) || !condition) {
    syntax(tok, "Expected a condition in the return-zero statement.");
  }

  tok = qlex_next(rd);
  if (!tok.is<qPuncComa>()) {
    syntax(tok, "Expected a comma after the return-zero expression.");
  }

  Expr *return_expr = nullptr;
  if (!parse_expr(job, rd, {qlex_tok_t(qPunc, qPuncSemi)}, &return_expr) || !return_expr) {
    syntax(tok, "Expected a return expression after the comma.");
  }

  tok = qlex_next(rd);
  if (!tok.is<qPuncSemi>()) {
    syntax(tok, "Expected a semicolon after the return-zero expression.");
  }
  *node = RetZStmt::get(condition, return_expr);
  (*node)->set_end_pos(tok.end);

  return true;
}

bool qparse::parser::parse_retv(qparse_t &job, qlex_t *rd, Stmt **node) {
  qlex_tok_t tok;

  Expr *cond = nullptr;
  if (!parse_expr(job, rd, {qlex_tok_t(qPunc, qPuncSemi)}, &cond) || !cond) {
    syntax(tok, "Expected a condition in the return-void statement.");
  }

  tok = qlex_next(rd);
  if (!tok.is<qPuncSemi>()) {
    syntax(tok, "Expected a semicolon after the return-void expression.");
  }

  *node = RetVStmt::get(cond);
  (*node)->set_end_pos(tok.end);

  return true;
}