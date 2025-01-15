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

FlowPtr<Stmt> Parser::RecurseSwitchCaseBody() {
  if (!next_if(OpArrow)) {
    Log << SyntaxError << current() << "Expected '=>' in switch case.";
  }

  if (peek().is<PuncLCur>()) {
    return RecurseBlock(true, false, SafetyMode::Unknown);
  } else {
    return RecurseBlock(false, true, SafetyMode::Unknown);
  }
}

std::pair<FlowPtr<Stmt>, bool> Parser::RecurseSwitchCase() {
  auto cond = RecurseExpr({
      Token(Oper, OpArrow),
      Token(Punc, PuncLCur),
  });
  auto body = RecurseSwitchCaseBody();

  auto is_the_default_case =
      cond->is(QAST_IDENT) && cond->as<Ident>()->GetName() == "_";

  if (is_the_default_case) {
    return {body, true};
  } else {
    return {make<CaseStmt>(cond, body)(), false};
  }
}

std::optional<std::pair<SwitchCases, NullableFlowPtr<Stmt>>>
Parser::RecurseSwitchBody() {
  SwitchCases cases;
  NullableFlowPtr<Stmt> default_case;

  while (true) {
    if (next_if(EofF)) [[unlikely]] {
      Log << SyntaxError << current() << "Unexpected EOF in switch statement.";
      break;
    }

    if (next_if(PuncRCur)) {
      return {{cases, default_case}};
    }

    auto [stmt, is_default] = RecurseSwitchCase();
    if (is_default) {
      if (!default_case) [[likely]] {
        default_case = stmt;
      } else {
        Log << SyntaxError << current() << "Duplicate default case in switch.";
      }
    } else {
      cases.push_back(stmt.as<CaseStmt>());
    }

    next_if(PuncComa) || next_if(PuncSemi);
  }

  return std::nullopt;
}

FlowPtr<Stmt> Parser::RecurseSwitch() {
  auto switch_cond = RecurseExpr({
      Token(Punc, PuncLCur),
  });

  if (next_if(PuncLCur)) [[likely]] {
    if (auto switch_body = RecurseSwitchBody()) [[likely]] {
      auto [switch_cases, switch_default] = switch_body.value();

      return make<SwitchStmt>(switch_cond, switch_cases, switch_default)();
    } else {
      Log << SyntaxError << current() << "Switch statement body is malformed.";
    }
  } else {
    Log << SyntaxError << current() << "Expected '{' after switch condition.";
  }

  return MockStmt(QAST_SWITCH);
}
