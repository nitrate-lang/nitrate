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

std::optional<std::pair<string, string>> Parser::recurse_foreach_names() {
  if (auto ident1 = next_if(Name)) {
    auto ident1_value = ident1->as_string();

    if (next_if(PuncComa)) {
      if (auto ident2 = next_if(Name)) {
        auto ident2_value = ident2->as_string();
        return std::make_pair(ident1_value, ident2_value);
      } else {
        diagnostic << current() << "Expected identifier in foreach statement";
      }
    } else {
      return std::make_pair("", ident1_value);
    }
  } else {
    diagnostic << current() << "Expected identifier in foreach statement";
  }

  return std::nullopt;
}

FlowPtr<Expr> Parser::recurse_foreach_expr(bool has_paren) {
  if (has_paren) {
    return recurse_expr({Token(Punc, PuncRPar)});
  } else {
    return recurse_expr({Token(Punc, PuncLCur), Token(Oper, OpArrow)});
  }
}

FlowPtr<Stmt> Parser::recurse_foreach_body() {
  if (next_if(OpArrow)) {
    return recurse_block(false, true, SafetyMode::Unknown);
  } else {
    return recurse_block(true, false, SafetyMode::Unknown);
  }
}

FlowPtr<Stmt> Parser::recurse_foreach() {
  /**
   * Syntax examples:
   *   `foreach (i, v in arr) { }`, `foreach (v in arr) { }`
   *   `foreach (i, v in arr) => v.put(i);`, `foreach (v in arr) => v.put();`
   */

  bool has_paren = next_if(PuncLPar).has_value();

  if (auto ident_pair_opt = recurse_foreach_names()) {
    auto [index_name, value_name] = ident_pair_opt.value();

    if (next_if(OpIn)) {
      auto iter_expr = recurse_foreach_expr(has_paren);
      if (has_paren) {
        if (!next_if(PuncRPar)) {
          diagnostic << current() << "Expected ')' in foreach statement";
        }
      }

      auto body = recurse_foreach_body();

      return make<ForeachStmt>(index_name, value_name, iter_expr, body)();
    } else {
      diagnostic << current() << "Expected 'in' keyword in foreach statement";
    }
  } else {
    diagnostic << current() << "Expected identifier pair in foreach statement";
  }

  return mock_stmt(QAST_FOREACH);
}
