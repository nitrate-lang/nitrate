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

#include <decent/Recurse.hh>

using namespace qparse;
using namespace qparse;

static bool recurse_enum_field(qparse_t &S, qlex_t *rd, EnumDefItems &fields) {
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
    if (!recurse_expr(
            S, rd, {qlex_tok_t(qPunc, qPuncComa), qlex_tok_t(qPunc, qPuncRCur)},
            &expr) ||
        !expr) {
      syntax(tok, "Expected an expression after '='");
      return false;
    }

    item.second = expr;
    item.second->set_pos(expr->get_pos());

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

bool qparse::recurse_enum(qparse_t &S, qlex_t *rd, Stmt **node) {
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
    if (!recurse_type(S, rd, &type)) {
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

    if (!recurse_enum_field(S, rd, fields)) {
      return false;
    }
  }

  *node = EnumDef::get(name, type, fields);
  (*node)->set_end_pos(tok.end);

  return true;
}