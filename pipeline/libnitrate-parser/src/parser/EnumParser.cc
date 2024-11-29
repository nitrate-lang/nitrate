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

#include <parser/Parse.h>

using namespace qparse;
using namespace qparse::parser;
using namespace qparse::diag;

static bool parse_enum_field(qparse_t &job, qlex_t *rd, EnumDefItems &fields) {
  qlex_tok_t tok = qlex_next(rd);
  if (!tok.is(qName)) {
    syntax(tok, "Enum field must be named by an identifier");
    return false;
  }

  EnumItem item;

  item.first = tok.as_string(rd);

  tok = qlex_peek(rd);
  if (tok.is<qOpSet>()) {
    qlex_next(rd);
    Expr *expr = nullptr;
    if (!parse_expr(
            job, rd,
            {qlex_tok_t(qPunc, qPuncComa), qlex_tok_t(qPunc, qPuncRCur)},
            &expr) ||
        !expr) {
      syntax(tok, "Expected an expression after '='");
      return false;
    }

    item.second = expr;
    item.second->set_start_pos(expr->get_start_pos());
    item.second->set_end_pos(expr->get_end_pos());

    tok = qlex_peek(rd);
  }

  fields.push_back(item);

  if (tok.is<qPuncComa>()) {
    qlex_next(rd);
    return true;
  }

  if (!tok.is<qPuncRCur>()) {
    syntax(tok, "Expected a comma or a closing curly brace");
    return false;
  }

  return true;
}

bool qparse::parser::parse_enum(qparse_t &job, qlex_t *rd, Stmt **node) {
  qlex_tok_t tok = qlex_next(rd);
  if (!tok.is(qName)) {
    syntax(tok, "Enum definition must be named by an identifier");
    return false;
  }

  std::string name = tok.as_string(rd);

  tok = qlex_peek(rd);
  Type *type = nullptr;
  if (tok.is<qPuncColn>()) {
    qlex_next(rd);
    if (!parse_type(job, rd, &type)) {
      return false;
    }
  }

  tok = qlex_next(rd);
  if (!tok.is<qPuncLCur>()) {
    syntax(tok, "Expected a '{' to start the enum definition");
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

  *node = EnumDef::get(name, type, fields);
  (*node)->set_end_pos(tok.end);

  return true;
}