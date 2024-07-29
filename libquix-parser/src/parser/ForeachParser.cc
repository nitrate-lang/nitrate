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

bool qparse::parser::parse_foreach(qparse_t &job, qlex_t *rd, Stmt **node) {
  qlex_tok_t tok = qlex_next(rd);
  bool has_parens = false;

  if (tok.is<qPuncLPar>()) {
    has_parens = true;
    tok = qlex_next(rd);
  }

  if (!tok.is(qName)) {
    /// TODO: Write the ERROR message
    return false;
  }
  std::string idx_ident = tok.as_string(rd);

  tok = qlex_next(rd);
  if (!tok.is<qPuncComa>()) {
    /// TODO: Write the ERROR message
    return false;
  }

  tok = qlex_next(rd);
  if (!tok.is(qName)) {
    /// TODO: Write the ERROR message
    return false;
  }

  std::string val_ident = tok.as_string(rd);

  tok = qlex_next(rd);
  if (!tok.is<qOpIn>()) {
    /// TODO: Write the ERROR message
    return false;
  }

  Expr *expr = nullptr;
  if (has_parens) {
    if (!parse_expr(job, rd, {qlex_tok_t(qPunc, qPuncRPar)}, &expr)) {
      /// TODO: Write the ERROR message
      return false;
    }
    tok = qlex_next(rd);
    if (!tok.is<qPuncRPar>()) {
      /// TODO: Write the ERROR message
      return false;
    }
  } else {
    if (!parse_expr(job, rd, {qlex_tok_t(qPunc, qPuncLCur), qlex_tok_t(qOper, qOpArrow)}, &expr)) {
      /// TODO: Write the ERROR message
      return false;
    }
  }

  tok = qlex_peek(rd);

  Block *block = nullptr;
  if (tok.is<qOpArrow>()) {
    qlex_next(rd);
    if (!parse(job, rd, &block, false, true)) {
      /// TODO: Write the ERROR message
      return false;
    }
  } else {
    if (!parse(job, rd, &block)) {
      /// TODO: Write the ERROR message
      return false;
    }
  }

  *node = ForeachStmt::get(idx_ident, val_ident, expr, block);

  return true;
}
