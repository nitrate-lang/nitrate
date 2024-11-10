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

bool qparse::parser::parse_form(qparse_t &job, qlex_t *rd, Stmt **node) {
  qlex_tok_t tok = qlex_next(rd);

  if (!tok.is<qPuncLPar>()) {
    syntax(tok, "Expected '(' in form statement");
  }

  Expr *maxjobs = nullptr;
  if (!parse_expr(job, rd, {qlex_tok_t(qPunc, qPuncRPar)}, &maxjobs) || !maxjobs) {
    syntax(tok, "Failed to parse parallelism expression in form statement");
  }

  tok = qlex_next(rd);
  if (!tok.is<qPuncRPar>()) {
    syntax(tok, "Expected ')' in form statement");
  }

  tok = qlex_next(rd);
  if (!tok.is<qPuncLPar>()) {
    syntax(tok, "Expected '(' in form statement");
  }

  tok = qlex_next(rd);
  if (!tok.is(qName)) {
    syntax(tok, "Expected identifier as index variable in form statement");
  }
  std::string idx_ident = tok.as_string(rd);

  tok = qlex_next(rd);
  if (!tok.is<qPuncComa>()) {
    syntax(tok, "Expected ',' after index variable in form statement");
  }

  tok = qlex_next(rd);
  if (!tok.is(qName)) {
    syntax(tok, "Expected identifier as value variable in form statement");
  }
  std::string val_ident = tok.as_string(rd);

  tok = qlex_next(rd);
  if (!tok.is<qOpIn>()) {
    syntax(tok, "Expected 'in' after value variable in form statement");
  }

  Expr *expr = nullptr;
  if (!parse_expr(job, rd, {qlex_tok_t(qPunc, qPuncRPar)}, &expr) || !expr) {
    syntax(tok, "Failed to parse expression in form statement");
  }
  tok = qlex_next(rd);
  if (!tok.is<qPuncRPar>()) {
    syntax(tok, "Expected ')' in form statement");
  }

  tok = qlex_peek(rd);

  Block *block = nullptr;
  if (tok.is<qOpArrow>()) {
    qlex_next(rd);
    if (!parse(job, rd, &block, false, true)) {
      syntax(tok, "Failed to parse single statement in form statement");
    }
  } else {
    if (!parse(job, rd, &block)) {
      syntax(tok, "Failed to parse block in form statement");
    }
  }

  *node = FormStmt::get(idx_ident, val_ident, expr, maxjobs, block);
  (*node)->set_end_pos(block->get_end_pos());

  return true;
}
