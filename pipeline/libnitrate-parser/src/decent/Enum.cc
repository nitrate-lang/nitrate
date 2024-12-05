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

#include <nitrate-parser/Node.h>

#include <decent/Recurse.hh>

using namespace qparse;
using namespace qparse;

static bool recurse_enum_field(qparse_t &S, qlex_t &rd, EnumDefItems &fields) {
  qlex_tok_t tok = next();
  if (!tok.is(qName)) {
    diagnostic << tok << "Enum field must be named by an identifier";
    return false;
  }

  EnumItem item;

  item.first = tok.as_string(&rd);

  tok = peek();
  if (tok.is<qOpSet>()) {
    next();
    Expr *expr = recurse_expr(
        S, rd, {qlex_tok_t(qPunc, qPuncComa), qlex_tok_t(qPunc, qPuncRCur)});

    item.second = expr;
    item.second->set_pos(expr->get_pos());

    tok = peek();
  }

  fields.push_back(item);

  if (tok.is<qPuncComa>()) {
    next();
    return true;
  }

  if (!tok.is<qPuncRCur>()) {
    diagnostic << tok << "Expected a comma or a closing curly brace";
    return false;
  }

  return true;
}

qparse::Stmt *qparse::recurse_enum(qparse_t &S, qlex_t &rd) {
  qlex_tok_t tok = next();
  if (!tok.is(qName)) {
    diagnostic << tok << "Enum definition must be named by an identifier";
    return mock_stmt(QAST_NODE_ENUM);
  }

  std::string name = tok.as_string(&rd);

  tok = peek();
  Type *type = nullptr;
  if (tok.is<qPuncColn>()) {
    next();
    type = recurse_type(S, rd);
  }

  tok = next();
  if (!tok.is<qPuncLCur>()) {
    diagnostic << tok << "Expected a '{' to start the enum definition";
    return mock_stmt(QAST_NODE_ENUM);
  }

  EnumDefItems fields;

  while (true) {
    tok = peek();
    if (tok.is<qPuncRCur>()) {
      next();
      break;
    }

    if (!recurse_enum_field(S, rd, fields)) {
      return mock_stmt(QAST_NODE_ENUM);
    }
  }

  auto R = EnumDef::get(name, type, fields);
  R->set_end_pos(tok.end);

  return R;
}
