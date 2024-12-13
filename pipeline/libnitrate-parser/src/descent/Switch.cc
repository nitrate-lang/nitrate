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
#include <nitrate-parser/AST.hh>

using namespace npar;

static Stmt *recurse_switch_case_body(npar_t &S, qlex_t &rd) {
  if (next_if(qOpArrow)) {
    return recurse_block(S, rd, false, true);
  } else {
    return recurse_block(S, rd, true, false);
  }
}

static std::pair<CaseStmt *, bool> recurse_switch_case(npar_t &S, qlex_t &rd) {
  let cond = recurse_expr(
      S, rd, {qlex_tok_t(qOper, qOpArrow), qlex_tok_t(qPunc, qPuncLCur)});

  let body = recurse_switch_case_body(S, rd);

  let is_default_case =
      cond->is(QAST_IDENT) && cond->as<Ident>()->get_name() == "_";

  if (is_default_case) {
    return {make<CaseStmt>(nullptr, body), true};
  } else {
    return {make<CaseStmt>(cond, body), false};
  }
}

static std::optional<std::pair<SwitchCases, std::optional<CaseStmt *>>>
recurse_switch_body(npar_t &S, qlex_t &rd) {
  SwitchCases cases;
  std::optional<CaseStmt *> default_;

  while (true) {
    if (peek().is(qEofF)) {
      diagnostic << current() << "Unexpected EOF in switch statement.";
      break;
    }

    if (next_if(qPuncRCur)) {
      return {{cases, default_}};
    }

    let[field, is_default] = recurse_switch_case(S, rd);

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

npar::Stmt *npar::recurse_switch(npar_t &S, qlex_t &rd) {
  let switch_cond = recurse_expr(S, rd, {qlex_tok_t(qPunc, qPuncLCur)});

  if (next_if(qPuncLCur)) {
    if (auto body_opt = recurse_switch_body(S, rd)) {
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
