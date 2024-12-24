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

string Parser::recurse_enum_name() {
  if (auto tok = next_if(Name)) {
    return tok->as_string();
  } else {
    return "";
  }
}

NullableFlowPtr<parse::Type> Parser::recurse_enum_type() {
  if (next_if(PuncColn)) {
    return recurse_type();
  } else {
    return std::nullopt;
  }
}

NullableFlowPtr<Expr> Parser::recurse_enum_item_value() {
  if (next_if(OpSet)) {
    return recurse_expr(

        {Token(Punc, PuncSemi), Token(Punc, PuncComa), Token(Punc, PuncRCur)});
  } else {
    return std::nullopt;
  }
}

std::optional<EnumItem> Parser::recurse_enum_item() {
  if (auto name = next_if(Name)) {
    auto value = recurse_enum_item_value();

    return EnumItem(name->as_string(), value);
  } else {
    diagnostic << current() << "Enum field is missing a name.";
  }

  return std::nullopt;
}

std::optional<EnumDefItems> Parser::recurse_enum_items() {
  EnumDefItems items;

  if (next_if(PuncSemi)) {
    return items;
  }

  if (next_if(PuncLCur)) {
    while (true) {
      if (next_if(EofF)) {
        diagnostic << current()
                   << "Unexpected EOF encountered while parsing enum fields.";
        break;
      }

      if (next_if(PuncRCur)) {
        return items;
      }

      if (auto item = recurse_enum_item()) {
        items.push_back(item.value());
      } else {
        break;
      }

      next_if(PuncComa) || next_if(PuncSemi);
    }
  } else if (next_if(OpArrow)) {
    if (auto item = recurse_enum_item()) {
      items.push_back(item.value());
      return items;
    }
  }

  return std::nullopt;
}

FlowPtr<Stmt> Parser::recurse_enum() {
  auto name = recurse_enum_name();
  auto type = recurse_enum_type();

  if (auto items = recurse_enum_items()) {
    return make<EnumDef>(name, type, std::move(items.value()))();
  } else {
    return mock_stmt(QAST_ENUM);
  }
}
