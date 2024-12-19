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

using namespace ncc::lex;
using namespace ncc::parse;

Stmt *Parser::recurse_switch_case_body() {
  if (next_if(qOpArrow)) {
    return recurse_block(false, true, SafetyMode::Unknown);
  } else {
    return recurse_block(true, false, SafetyMode::Unknown);
  }
}

std::pair<CaseStmt *, bool> Parser::recurse_switch_case() {
  let cond = recurse_expr({Token(qOper, qOpArrow), Token(qPunc, qPuncLCur)});

  let body = recurse_switch_case_body();

  let is_default_case =
      cond->is(QAST_IDENT) && cond->as<Ident>()->get_name() == "_";

  if (is_default_case) {
    return {make<CaseStmt>(nullptr, body), true};
  } else {
    return {make<CaseStmt>(cond, body), false};
  }
}

std::optional<std::pair<SwitchCases, std::optional<CaseStmt *>>>
Parser::recurse_switch_body() {
  SwitchCases cases;
  std::optional<CaseStmt *> default_;

  while (true) {
    if (next_if(qEofF)) {
      diagnostic << current() << "Unexpected EOF in switch statement.";
      break;
    }

    if (next_if(qPuncRCur)) {
      return {{cases, default_}};
    }

    let[field, is_default] = recurse_switch_case();

    if (is_default) {
      if (default_) {
        diagnostic << current() << "Duplicate default case in switch.";
      }

      default_ = field;
    } else {
      cases.push_back(field);
    }
  }

  return std::nullopt;
}

Stmt *Parser::recurse_switch() {
  let switch_cond = recurse_expr({Token(qPunc, qPuncLCur)});

  if (next_if(qPuncLCur)) {
    if (auto body_opt = recurse_switch_body()) {
      let[switch_cases, switch_default_opt] = body_opt.value();

      return make<SwitchStmt>(switch_cond, std::move(switch_cases),
                              switch_default_opt.value_or(nullptr));
    } else {
      diagnostic << current() << "Switch statement body is malformed.";
    }
  } else {
    diagnostic << current() << "Expected '{' after switch condition.";
  }

  return mock_stmt(QAST_SWITCH);
}
