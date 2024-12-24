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

NullableFlowPtr<Stmt> Parser::recurse_for_init_expr() {
  if (next_if(PuncSemi)) {
    return std::nullopt;
  }

  return recurse_block(false, true, SafetyMode::Unknown);
}

NullableFlowPtr<Expr> Parser::recurse_for_cond_expr() {
  if (next_if(PuncSemi)) {
    return std::nullopt;
  }

  auto cond_expr = recurse_expr({Token(Punc, PuncSemi)});

  if (!next_if(PuncSemi)) {
    diagnostic << current() << "Expected semicolon after condition expression";
  }

  return cond_expr;
}

NullableFlowPtr<Expr> Parser::recurse_for_step_expr(bool has_paren) {
  if (has_paren) {
    if (peek().is<PuncRPar>()) {
      return std::nullopt;
    } else {
      return recurse_expr({Token(Punc, PuncRPar)});
    }
  } else {
    if (peek().is<OpArrow>() || peek().is<PuncLCur>()) {
      return std::nullopt;
    } else {
      return recurse_expr({Token(Punc, PuncLCur), Token(Oper, OpArrow)});
    }
  }
}

FlowPtr<Stmt> Parser::recurse_for_body() {
  if (next_if(OpArrow)) {
    return recurse_block(false, true, SafetyMode::Unknown);
  } else {
    return recurse_block(true, false, SafetyMode::Unknown);
  }
}

FlowPtr<Stmt> Parser::recurse_for() {
  bool has_paren = next_if(PuncLPar).has_value();

  auto init = recurse_for_init_expr();
  auto cond = recurse_for_cond_expr();
  auto step = recurse_for_step_expr(has_paren);

  if (has_paren) {
    if (!next_if(PuncRPar)) {
      diagnostic << current()
                 << "Expected closing parenthesis in for statement";
    }
  }

  auto body = recurse_for_body();

  return make<ForStmt>(init, cond, step, body)();
}
