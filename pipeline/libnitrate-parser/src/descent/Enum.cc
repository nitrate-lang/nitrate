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

string Parser::RecurseEnumName() {
  if (auto tok = next_if(Name)) {
    return tok->as_string();
  } else {
    return "";
  }
}

NullableFlowPtr<parse::Type> Parser::RecurseEnumType() {
  if (next_if(PuncColn)) {
    return RecurseType();
  } else {
    return std::nullopt;
  }
}

NullableFlowPtr<Expr> Parser::RecurseEnumItemValue() {
  if (next_if(OpSet)) {
    return RecurseExpr({
        Token(Punc, PuncSemi),
        Token(Punc, PuncComa),
        Token(Punc, PuncRCur),
    });
  } else {
    return std::nullopt;
  }
}

std::optional<EnumItem> Parser::RecurseEnumItem() {
  if (auto member_name = next_if(Name)) [[likely]] {
    auto member_value = RecurseEnumItemValue();

    return EnumItem(member_name->as_string(), member_value);
  } else {
    Log << SyntaxError << next() << "Enum field is missing a name.";
    return std::nullopt;
  }
}

std::optional<EnumDefItems> Parser::RecurseEnumItems() {
  EnumDefItems items;

  if (next_if(PuncSemi)) {
    return items;
  }

  if (next_if(PuncLCur)) {
    while (true) {
      if (next_if(EofF)) [[unlikely]] {
        Log << SyntaxError << current()
            << "Unexpected EOF encountered while parsing enum fields.";
        break;
      }

      if (next_if(PuncRCur)) {
        return items;
      }

      if (auto enum_member = RecurseEnumItem()) {
        items.push_back(enum_member.value());
      } else {
        Log << SyntaxError << current() << "Failed to parse enum field.";
      }

      next_if(PuncComa) || next_if(PuncSemi);
    }
  } else if (next_if(OpArrow)) {
    if (auto item = RecurseEnumItem()) {
      items.push_back(item.value());
      return items;
    }
  }

  return std::nullopt;
}

FlowPtr<Stmt> Parser::RecurseEnum() {
  auto name = RecurseEnumName();
  auto type = RecurseEnumType();

  if (auto items = RecurseEnumItems()) {
    return make<EnumDef>(name, type, items.value())();
  } else {
    return MockStmt(QAST_ENUM);
  }
}
