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
#include <nitrate-parser/ASTExpr.hh>
#include <nitrate-parser/ASTStmt.hh>

using namespace ncc;
using namespace ncc::lex;
using namespace ncc::parse;

static auto RecurseSwitchCaseBody(GeneralParser::Context& m) -> FlowPtr<Expr> {
  if (!m.NextIf<OpArrow>()) {
    Log << ParserSignal << m.Current() << "Expected '=>' in switch case.";
  }

  if (m.Peek().Is<PuncLCur>()) {
    return m.RecurseBlock(true, false, BlockMode::Unknown);
  }

  return m.RecurseBlock(false, true, BlockMode::Unknown);
}

static auto RecurseSwitchCase(GeneralParser::Context& m) -> std::pair<FlowPtr<Expr>, bool> {
  auto cond = m.RecurseExpr({
      Token(Oper, OpArrow),
      Token(Punc, PuncLCur),
  });
  auto body = RecurseSwitchCaseBody(m);

  auto is_the_default_case = cond->Is(AST_eIDENT) && cond->As<Identifier>()->GetName() == "_";

  if (is_the_default_case) {
    return {body, true};
  }

  return {m.CreateCase(cond, body), false};
}

static auto RecurseSwitchBody(GeneralParser::Context& m)
    -> std::optional<std::pair<std::vector<FlowPtr<Case>>, NullableFlowPtr<Expr>>> {
  std::vector<FlowPtr<Case>> cases;
  NullableFlowPtr<Expr> default_case;

  while (true) {
    if (m.IsEof()) [[unlikely]] {
      Log << ParserSignal << m.Current() << "Unexpected EOF in switch statement.";
      break;
    }

    if (m.NextIf<PuncRCur>()) {
      return {{cases, default_case}};
    }

    auto [stmt, is_default] = RecurseSwitchCase(m);
    if (is_default) {
      if (!default_case) [[likely]] {
        default_case = stmt;
      } else {
        Log << ParserSignal << m.Current() << "Duplicate default case in switch.";
      }
    } else {
      cases.push_back(stmt.As<Case>());
    }

    m.NextIf<PuncComa>() || m.NextIf<PuncSemi>();
  }

  return std::nullopt;
}

auto GeneralParser::Context::RecurseSwitch() -> FlowPtr<Expr> {
  auto switch_cond = RecurseExpr({
      Token(Punc, PuncLCur),
  });

  if (NextIf<PuncLCur>()) [[likely]] {
    if (auto switch_body = RecurseSwitchBody(*this)) [[likely]] {
      auto [switch_cases, switch_default] = switch_body.value();

      return CreateSwitch(switch_cond, switch_default, switch_cases);
    } else {
      Log << ParserSignal << Current() << "Switch statement body is malformed.";
    }
  } else {
    Log << ParserSignal << Current() << "Expected '{' after switch condition.";
  }

  return CreateMockInstance<Switch>();
}
