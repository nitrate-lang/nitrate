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

#include <descent/Recurse.hh>

using namespace npar;

constexpr static std::string_view recurse_enum_name(qlex_t &rd) {
  if (let tok = next_if(qName)) {
    return tok->as_string(&rd);
  } else {
    return "";
  }
}

constexpr static std::optional<Type *> recurse_enum_type(npar_t &S,
                                                         qlex_t &rd) {
  if (next_if(qPuncColn)) {
    return recurse_type(S, rd);
  } else {
    return std::nullopt;
  }
}

constexpr static std::optional<Expr *> recurse_enum_item_value(npar_t &S,
                                                               qlex_t &rd) {
  if (next_if(qOpSet)) {
    return recurse_expr(
        S, rd,
        {qlex_tok_t(qPunc, qPuncSemi), qlex_tok_t(qPunc, qPuncComa),
         qlex_tok_t(qPunc, qPuncRCur)});
  } else {
    return std::nullopt;
  }
}

constexpr static std::optional<EnumItem> recurse_enum_item(npar_t &S,
                                                           qlex_t &rd) {
  if (let name = next_if(qName)) {
    let value = recurse_enum_item_value(S, rd);

    return EnumItem(name->as_string(&rd), value.value_or(nullptr));
  } else {
    diagnostic << current() << "Enum field is missing a name.";
  }

  return std::nullopt;
}

constexpr static std::optional<EnumDefItems> recurse_enum_items(npar_t &S,
                                                                qlex_t &rd) {
  EnumDefItems items;

  if (next_if(qPuncSemi)) {
    return items;
  }

  if (next_if(qPuncLCur)) {
    while (true) {
      if (peek().is(qEofF)) {
        diagnostic << current()
                   << "Unexpected EOF encountered while parsing enum fields.";
        break;
      }

      if (next_if(qPuncRCur)) {
        return items;
      }

      if (let item = recurse_enum_item(S, rd)) {
        items.push_back(item.value());
      } else {
        break;
      }

      next_if(qPuncComa) || next_if(qPuncSemi);
    }
  } else if (next_if(qOpArrow)) {
    if (let item = recurse_enum_item(S, rd)) {
      items.push_back(item.value());
      return items;
    }
  }

  return std::nullopt;
}

npar::Stmt *npar::recurse_enum(npar_t &S, qlex_t &rd) {
  let name = recurse_enum_name(rd);
  let type = recurse_enum_type(S, rd);

  if (let items = recurse_enum_items(S, rd)) {
    return EnumDef::get(name, type.value_or(nullptr), std::move(items.value()));
  }

  return mock_stmt(QAST_NODE_ENUM);
}
