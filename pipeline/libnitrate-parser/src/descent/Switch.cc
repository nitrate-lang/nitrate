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

auto GeneralParser::Context::RecurseSwitchCaseBody() -> FlowPtr<Expr> {
  if (!NextIf<OpArrow>()) {
    Log << ParserSignal << Current() << "Expected '=>' in switch case.";
  }

  if (Peek().Is<PuncLCur>()) {
    return RecurseBlock(true, false, BlockMode::Unknown);
  }

  return RecurseBlock(false, true, BlockMode::Unknown);
}

auto GeneralParser::Context::RecurseSwitchCase() -> std::pair<FlowPtr<Expr>, bool> {
  auto cond = RecurseExpr({
      Token(Oper, OpArrow),
      Token(Punc, PuncLCur),
  });
  auto body = RecurseSwitchCaseBody();

  auto is_the_default_case = cond->Is(AST_eIDENT) && cond->As<Identifier>()->GetName() == "_";

  if (is_the_default_case) {
    return {body, true};
  }

  return {m_fac.CreateCase(cond, body), false};
}

auto GeneralParser::Context::RecurseSwitchBody()
    -> std::optional<std::pair<std::vector<FlowPtr<Case>>, NullableFlowPtr<Expr>>> {
  std::vector<FlowPtr<Case>> cases;
  NullableFlowPtr<Expr> default_case;

  while (true) {
    if (m_rd.IsEof()) [[unlikely]] {
      Log << ParserSignal << Current() << "Unexpected EOF in switch statement.";
      break;
    }

    if (NextIf<PuncRCur>()) {
      return {{cases, default_case}};
    }

    auto [stmt, is_default] = RecurseSwitchCase();
    if (is_default) {
      if (!default_case) [[likely]] {
        default_case = stmt;
      } else {
        Log << ParserSignal << Current() << "Duplicate default case in switch.";
      }
    } else {
      cases.push_back(stmt.As<Case>());
    }

    NextIf<PuncComa>() || NextIf<PuncSemi>();
  }

  return std::nullopt;
}

auto GeneralParser::Context::RecurseSwitch() -> FlowPtr<Expr> {
  auto switch_cond = RecurseExpr({
      Token(Punc, PuncLCur),
  });

  if (NextIf<PuncLCur>()) [[likely]] {
    if (auto switch_body = RecurseSwitchBody()) [[likely]] {
      auto [switch_cases, switch_default] = switch_body.value();

      return m_fac.CreateSwitch(switch_cond, switch_default, switch_cases);
    } else {
      Log << ParserSignal << Current() << "Switch statement body is malformed.";
    }
  } else {
    Log << ParserSignal << Current() << "Expected '{' after switch condition.";
  }

  return m_fac.CreateMockInstance<Switch>();
}
