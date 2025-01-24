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

auto Parser::PImpl::RecurseEnumType() -> NullableFlowPtr<parse::Type> {
  if (NextIf(PuncColn)) {
    return RecurseType();
  }

  return std::nullopt;
}

auto Parser::PImpl::RecurseEnumItem() -> std::optional<EnumItem> {
  auto member_name = RecurseName();
  if (member_name->empty()) {
    Log << SyntaxError << current() << "Enum member name cannot be empty.";
    return std::nullopt;
  }

  if (NextIf(OpSet)) {
    auto member_value = RecurseExpr({
        Token(Punc, PuncSemi),
        Token(Punc, PuncComa),
        Token(Punc, PuncRCur),
    });

    return EnumItem(member_name, member_value);
  }

  return EnumItem(member_name, std::nullopt);
}

auto Parser::PImpl::RecurseEnumItems() -> std::optional<EnumItems> {
  EnumItems items;

  if (NextIf(PuncSemi)) {
    return items;
  }

  if (NextIf(PuncLCur)) {
    while (true) {
      if (NextIf(EofF)) [[unlikely]] {
        Log << SyntaxError << current()
            << "Unexpected EOF encountered while parsing enum fields.";
        break;
      }

      if (NextIf(PuncRCur)) {
        return items;
      }

      if (auto enum_member = RecurseEnumItem()) {
        items.push_back(enum_member.value());
      } else {
        Log << SyntaxError << current() << "Failed to parse enum field.";
      }

      NextIf(PuncComa) || NextIf(PuncSemi);
    }
  } else if (NextIf(OpArrow)) {
    if (auto item = RecurseEnumItem()) {
      items.push_back(item.value());
      return items;
    }
  }

  return std::nullopt;
}

auto Parser::PImpl::RecurseEnum() -> FlowPtr<Stmt> {
  auto name = RecurseName();
  auto type = RecurseEnumType();

  if (auto items = RecurseEnumItems()) {
    return CreateNode<Enum>(name, type, items.value())();
  }

  return MockStmt(QAST_ENUM);
}
