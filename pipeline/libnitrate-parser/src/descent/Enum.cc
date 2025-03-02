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

auto GeneralParser::PImpl::RecurseEnumType() -> NullableFlowPtr<parse::Type> {
  if (NextIf<PuncColn>()) {
    return RecurseType();
  }

  return std::nullopt;
}

auto GeneralParser::PImpl::RecurseEnumItem() -> std::optional<std::pair<string, NullableFlowPtr<Expr>>> {
  if (auto member_name = RecurseName()) {
    if (NextIf<OpSet>()) {
      auto member_value = RecurseExpr({
          Token(Punc, PuncSemi),
          Token(Punc, PuncComa),
          Token(Punc, PuncRCur),
      });

      return std::pair<string, NullableFlowPtr<Expr>>(member_name, member_value);
    }

    return std::pair<string, NullableFlowPtr<Expr>>(member_name, std::nullopt);
  }

  Log << SyntaxError << Current() << "Enum member name cannot be empty.";

  return std::nullopt;
}

auto GeneralParser::PImpl::RecurseEnumItems() -> std::optional<std::vector<std::pair<string, NullableFlowPtr<Expr>>>> {
  std::vector<std::pair<string, NullableFlowPtr<Expr>>> items;

  if (NextIf<PuncSemi>()) {
    return items;
  }

  if (NextIf<PuncLCur>()) {
    while (true) {
      if (Current().Is(EofF)) [[unlikely]] {
        Log << SyntaxError << Current() << "Unexpected EOF encountered while parsing enum fields.";
        break;
      }

      if (NextIf<PuncRCur>()) {
        return items;
      }

      if (auto enum_member = RecurseEnumItem()) {
        items.push_back(enum_member.value());
      } else {
        Log << SyntaxError << Current() << "Failed to parse enum field.";
      }

      NextIf<PuncComa>() || NextIf<PuncSemi>();
    }
  } else if (NextIf<OpArrow>()) {
    if (auto item = RecurseEnumItem()) {
      items.push_back(item.value());
      return items;
    }
  }

  return std::nullopt;
}

auto GeneralParser::PImpl::RecurseEnum() -> FlowPtr<Expr> {
  auto name = RecurseName();
  auto type = RecurseEnumType();

  if (auto items = RecurseEnumItems()) {
    return m_fac.CreateEnum(name, items.value(), type);
  }

  return m_fac.CreateMockInstance<Enum>();
}
