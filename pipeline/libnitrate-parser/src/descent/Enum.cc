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
#include <nitrate-parser/ASTStmt.hh>

using namespace ncc;
using namespace ncc::lex;
using namespace ncc::parse;

static auto RecurseEnumField(GeneralParser::Context& m) -> std::pair<string, NullableFlowPtr<Expr>> {
  auto member_name = m.RecurseName();
  if (!member_name) {
    Log << ParserSignal << m.Current() << "Enum member name cannot be empty.";
    return {"", std::nullopt};
  }

  if (m.NextIf<OpSet>()) {
    auto member_value = m.RecurseExpr({
        Token(Punc, PuncSemi),
        Token(Punc, PuncComa),
        Token(Punc, PuncRCur),
    });

    return {member_name, member_value};
  }

  return {member_name, std::nullopt};
}

static auto RecurseEnumFields(GeneralParser::Context& m) -> std::vector<std::pair<string, NullableFlowPtr<Expr>>> {
  std::vector<std::pair<string, NullableFlowPtr<Expr>>> items;

  if (m.NextIf<PuncSemi>()) {
    return items;
  }

  if (!m.NextIf<PuncLCur>()) {
    Log << ParserSignal << m.Current() << "Expected '{' to start enum fields.";
    return items;
  }

  while (true) {
    if (m.IsEof()) [[unlikely]] {
      Log << ParserSignal << m.Current() << "Unexpected EOF encountered while parsing enum fields.";
      return items;
    }

    if (m.NextIf<PuncRCur>()) {
      return items;
    }

    items.push_back(RecurseEnumField(m));

    if (m.NextIf<PuncComa>()) {
      continue;
    }

    if (m.NextIf<PuncSemi>()) {
      continue;
    }

    Log << ParserSignal << m.Next() << "Expected ',' or ';' after enum field.";
  }
}

auto GeneralParser::Context::RecurseEnum() -> FlowPtr<Expr> {
  auto name = RecurseName();
  auto type = NextIf<PuncColn>() ? NullableFlowPtr<parse::Type>(RecurseType()) : std::nullopt;
  auto items = RecurseEnumFields(m);

  return CreateEnum(name, items, type);
}
