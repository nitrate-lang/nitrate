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

#include <descent/Recurse.hh>

using namespace ncc;
using namespace ncc::lex;
using namespace ncc::parse;

std::string_view Parser::recurse_enum_name() {
  if (let tok = next_if(qName)) {
    return tok->as_string();
  } else {
    return "";
  }
}

std::optional<FlowPtr<Type> > Parser::recurse_enum_type() {
  if (next_if(qPuncColn)) {
    return recurse_type();
  } else {
    return std::nullopt;
  }
}

std::optional<FlowPtr<Expr> > Parser::recurse_enum_item_value() {
  if (next_if(qOpSet)) {
    return recurse_expr(

        {Token(qPunc, qPuncSemi), Token(qPunc, qPuncComa),
         Token(qPunc, qPuncRCur)});
  } else {
    return std::nullopt;
  }
}

std::optional<EnumItem> Parser::recurse_enum_item() {
  if (let name = next_if(qName)) {
    let value = recurse_enum_item_value();

    return EnumItem(SaveString(name->as_string()), value);
  } else {
    diagnostic << current() << "Enum field is missing a name.";
  }

  return std::nullopt;
}

std::optional<EnumDefItems> Parser::recurse_enum_items() {
  EnumDefItems items;

  if (next_if(qPuncSemi)) {
    return items;
  }

  if (next_if(qPuncLCur)) {
    while (true) {
      if (next_if(qEofF)) {
        diagnostic << current()
                   << "Unexpected EOF encountered while parsing enum fields.";
        break;
      }

      if (next_if(qPuncRCur)) {
        return items;
      }

      if (let item = recurse_enum_item()) {
        items.push_back(item.value());
      } else {
        break;
      }

      next_if(qPuncComa) || next_if(qPuncSemi);
    }
  } else if (next_if(qOpArrow)) {
    if (let item = recurse_enum_item()) {
      items.push_back(item.value());
      return items;
    }
  }

  return std::nullopt;
}

FlowPtr<Stmt> Parser::recurse_enum() {
  let name = recurse_enum_name();
  let type = recurse_enum_type();

  if (let items = recurse_enum_items()) {
    return make<EnumDef>(SaveString(name), type, std::move(items.value()))();
  }

  return mock_stmt(QAST_ENUM);
}
