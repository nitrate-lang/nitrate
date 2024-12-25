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

FlowPtr<Stmt> Parser::recurse_switch_case_body() {
  if (next_if(OpArrow)) {
    return recurse_block(false, true, SafetyMode::Unknown);
  } else {
    return recurse_block(true, false, SafetyMode::Unknown);
  }
}

std::variant<FlowPtr<CaseStmt>, FlowPtr<Stmt>> Parser::recurse_switch_case() {
  auto cond = recurse_expr({
      Token(Oper, OpArrow),
      Token(Punc, PuncLCur),
  });
  auto body = recurse_switch_case_body();

  auto is_the_default_case =
      cond->is(QAST_IDENT) && cond->as<Ident>()->get_name() == "_";

  if (is_the_default_case) {
    return body;
  } else {
    return make<CaseStmt>(cond, body)();
  }
}

std::optional<std::pair<SwitchCases, NullableFlowPtr<Stmt>>>
Parser::recurse_switch_body() {
  SwitchCases cases;
  NullableFlowPtr<Stmt> default_case;

  while (true) {
    if (next_if(EofF)) [[unlikely]] {
      diagnostic << current() << "Unexpected EOF in switch statement.";
      break;
    }

    if (next_if(PuncRCur)) {
      return {{cases, default_case}};
    }

    auto case_or_default = recurse_switch_case();
    if (std::holds_alternative<FlowPtr<Stmt>>(case_or_default)) {
      if (!default_case) [[likely]] {
        default_case = std::get<FlowPtr<Stmt>>(case_or_default);
      } else {
        diagnostic << current() << "Duplicate default case in switch.";
      }
    } else {
      auto case_stmt = std::get<FlowPtr<CaseStmt>>(case_or_default);
      cases.push_back(case_stmt);
    }
  }

  return std::nullopt;
}

FlowPtr<Stmt> Parser::recurse_switch() {
  auto switch_cond = recurse_expr({
      Token(Punc, PuncLCur),
  });

  if (next_if(PuncLCur)) [[likely]] {
    if (auto switch_body = recurse_switch_body()) [[likely]] {
      auto [switch_cases, switch_default] = switch_body.value();

      return make<SwitchStmt>(switch_cond, switch_cases, switch_default)();
    } else {
      diagnostic << current() << "Switch statement body is malformed.";
    }
  } else {
    diagnostic << current() << "Expected '{' after switch condition.";
  }

  return mock_stmt(QAST_SWITCH);
}
