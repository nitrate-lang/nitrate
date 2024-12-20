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

std::optional<Stmt *> Parser::recurse_for_init_expr() {
  if (next_if(qPuncSemi)) {
    return std::nullopt;
  }

  return recurse_block(false, true, SafetyMode::Unknown);
}

std::optional<Expr *> Parser::recurse_for_cond_expr() {
  if (next_if(qPuncSemi)) {
    return std::nullopt;
  }

  let cond_expr = recurse_expr({Token(qPunc, qPuncSemi)});

  if (!next_if(qPuncSemi)) {
    diagnostic << current() << "Expected semicolon after condition expression";
  }

  return cond_expr;
}

std::optional<Expr *> Parser::recurse_for_step_expr(bool has_paren) {
  if (has_paren) {
    if (peek().is<qPuncRPar>()) {
      return std::nullopt;
    } else {
      return recurse_expr({Token(qPunc, qPuncRPar)});
    }
  } else {
    if (peek().is<qOpArrow>() || peek().is<qPuncLCur>()) {
      return std::nullopt;
    } else {
      return recurse_expr({Token(qPunc, qPuncLCur), Token(qOper, qOpArrow)});
    }
  }
}

Stmt *Parser::recurse_for_body() {
  if (next_if(qOpArrow)) {
    return recurse_block(false, true, SafetyMode::Unknown);
  } else {
    return recurse_block(true, false, SafetyMode::Unknown);
  }
}

Stmt *Parser::recurse_for() {
  bool has_paren = next_if(qPuncLPar).has_value();

  let init = recurse_for_init_expr();
  let cond = recurse_for_cond_expr();
  let step = recurse_for_step_expr(has_paren);

  if (has_paren) {
    if (!next_if(qPuncRPar)) {
      diagnostic << current()
                 << "Expected closing parenthesis in for statement";
    }
  }

  let body = recurse_for_body();

  return make<ForStmt>(init, cond, step, body);
}
