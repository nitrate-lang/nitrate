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
#include <nitrate-parser/ASTStmt.hh>

using namespace ncc;
using namespace ncc::lex;
using namespace ncc::parse;

auto GeneralParser::Context::RecurseForeachNames() -> std::optional<std::pair<string, string>> {
  if (auto name_a = RecurseName()) [[likely]] {
    if (NextIf<PuncComa>()) {
      if (auto name_b = RecurseName()) [[likely]] {
        return std::make_pair(name_a, name_b);
      } else {
        Log << ParserSignal << Current() << "Expected identifier in foreach statement";
      }
    } else {
      return std::make_pair("", name_a);
    }
  } else {
    Log << ParserSignal << Current() << "Expected identifier in foreach statement";
  }

  return std::nullopt;
}

auto GeneralParser::Context::RecurseForeachExpr(bool has_paren) -> FlowPtr<Expr> {
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

auto GeneralParser::Context::RecurseForeachBody() -> FlowPtr<Expr> {
  if (NextIf<OpArrow>()) {
    return RecurseBlock(false, true, BlockMode::Unknown);
  }

  return RecurseBlock(true, false, BlockMode::Unknown);
}

auto GeneralParser::Context::RecurseForeach() -> FlowPtr<Expr> {
  bool foreach_has_paren = NextIf<PuncLPar>().has_value();

  if (auto iter_names = RecurseForeachNames()) {
    auto [index_name, value_name] = iter_names.value();

    if (NextIf<OpIn>()) [[likely]] {
      auto iter_expr = RecurseForeachExpr(foreach_has_paren);
      if (foreach_has_paren && !NextIf<PuncRPar>()) {
        Log << ParserSignal << Current() << "Expected ')' in foreach statement";
      }

      auto body = RecurseForeachBody();

      return CreateForeach(index_name, value_name, iter_expr, body);
    } else {
      Log << ParserSignal << Current() << "Expected 'in' keyword in foreach statement";
    }
  } else {
    Log << ParserSignal << Current() << "Expected identifier pair in foreach statement";
  }

  return CreateMockInstance<Foreach>();
}
