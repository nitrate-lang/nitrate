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

static bool parse_enum_field(qparse_t &job, qlex_t *rd, EnumDefItems &fields) {
  qlex_tok_t tok = qlex_next(rd);
  if (!tok.is(qName)) {
    /// TODO: Write the ERROR message
    return false;
  }

  EnumItem item;

  item.first = tok.as_string(rd);

  tok = qlex_peek(rd);
  if (tok.is<qOpSet>()) {
    qlex_next(rd);
    Expr *expr = nullptr;
    if (!parse_expr(job, rd, {qlex_tok_t(qPunc, qPuncComa)}, &expr)) {
      /// TODO: Write the ERROR message
      return false;
    }

    item.second = ConstExpr::get(expr);

    tok = qlex_peek(rd);
  }

  fields.push_back(item);

  if (tok.is<qPuncComa>()) {
    qlex_next(rd);
    return true;
  }

  if (!tok.is<qPuncRCur>()) {
    /// TODO: Write the ERROR message
    return false;
  }

  return true;
}

bool qparse::parser::parse_enum(qparse_t &job, qlex_t *rd, Stmt **node) {
  qlex_tok_t tok = qlex_next(rd);
  if (!tok.is(qName)) {
    /// TODO: Write the ERROR message
    return false;
  }

  std::string name = tok.as_string(rd);

  tok = qlex_peek(rd);
  Type *type = nullptr;
  if (tok.is<qPuncColn>()) {
    qlex_next(rd);
    if (!parse_type(job, rd, &type)) return false;
  }

  tok = qlex_next(rd);
  if (!tok.is<qPuncLCur>()) {
    /// TODO: Write the ERROR message
    return false;
  }

  EnumDefItems fields;

  while (true) {
    tok = qlex_peek(rd);
    if (tok.is<qPuncRCur>()) {
      qlex_next(rd);
      break;
    }

    if (!parse_enum_field(job, rd, fields)) {
      return false;
    }
  }

  *node = EnumDef::get(name, EnumTy::get(name, type), fields);
  return true;
}