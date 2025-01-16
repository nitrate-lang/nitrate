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

std::optional<std::pair<string, string>> Parser::PImpl::RecurseForeachNames() {
  if (auto name_a = next_if(Name)) [[likely]] {
    if (next_if(PuncComa)) {
      if (auto name_b = next_if(Name)) [[likely]] {
        return std::make_pair(name_a->GetString(), name_b->GetString());
      } else {
        Log << SyntaxError << current()
            << "Expected identifier in foreach statement";
      }
    } else {
      return std::make_pair("", name_a->GetString());
    }
  } else {
    Log << SyntaxError << current()
        << "Expected identifier in foreach statement";
  }

  return std::nullopt;
}

FlowPtr<Expr> Parser::PImpl::RecurseForeachExpr(bool has_paren) {
  if (has_paren) {
    return RecurseExpr({
        Token(Punc, PuncRPar),
    });
  }

  return RecurseExpr({
      Token(Punc, PuncLCur),
      Token(Oper, OpArrow),
  });
}

FlowPtr<Stmt> Parser::PImpl::RecurseForeachBody() {
  if (next_if(OpArrow)) {
    return RecurseBlock(false, true, SafetyMode::Unknown);
  }

  return RecurseBlock(true, false, SafetyMode::Unknown);
}

FlowPtr<Stmt> Parser::PImpl::RecurseForeach() {
  bool foreach_has_paren = next_if(PuncLPar).has_value();

  if (auto iter_names = RecurseForeachNames()) {
    auto [index_name, value_name] = iter_names.value();

    if (next_if(OpIn)) [[likely]] {
      auto iter_expr = RecurseForeachExpr(foreach_has_paren);
      if (foreach_has_paren && !next_if(PuncRPar)) {
        Log << SyntaxError << current() << "Expected ')' in foreach statement";
      }

      auto body = RecurseForeachBody();

      return make<ForeachStmt>(index_name, value_name, iter_expr, body)();
    } else {
      Log << SyntaxError << current()
          << "Expected 'in' keyword in foreach statement";
    }
  } else {
    Log << SyntaxError << current()
        << "Expected identifier pair in foreach statement";
  }

  return MockStmt(QAST_FOREACH);
}
