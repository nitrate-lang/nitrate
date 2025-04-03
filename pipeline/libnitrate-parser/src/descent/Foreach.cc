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

static auto RecurseForeachNames(GeneralParser::Context& m) -> std::optional<std::pair<string, string>> {
  if (auto name_a = m.RecurseName()) [[likely]] {
    if (m.NextIf<PuncComa>()) {
      if (auto name_b = m.RecurseName()) [[likely]] {
        return std::make_pair(name_a, name_b);
      } else {
        Log << ParserSignal << m.Current() << "Expected identifier in foreach statement";
      }
    } else {
      return std::make_pair("", name_a);
    }
  } else {
    Log << ParserSignal << m.Current() << "Expected identifier in foreach statement";
  }

  return std::nullopt;
}

static auto RecurseForeachExpr(GeneralParser::Context& m, bool has_paren) -> FlowPtr<Expr> {
  if (has_paren) {
    return m.RecurseExpr({
        Token(Punc, PuncRPar),
    });
  }

  return m.RecurseExpr({
      Token(Punc, PuncLCur),
      Token(Oper, OpArrow),
  });
}

static auto RecurseForeachBody(GeneralParser::Context& m) -> FlowPtr<Expr> {
  if (m.NextIf<OpArrow>()) {
    return m.RecurseBlock(false, true, BlockMode::Unknown);
  }

  return m.RecurseBlock(true, false, BlockMode::Unknown);
}

auto GeneralParser::Context::RecurseForeach() -> FlowPtr<Expr> {
  bool foreach_has_paren = NextIf<PuncLPar>().has_value();

  if (auto iter_names = RecurseForeachNames(m)) {
    auto [index_name, value_name] = iter_names.value();

    if (NextIf<OpIn>()) [[likely]] {
      auto iter_expr = RecurseForeachExpr(m, foreach_has_paren);
      if (foreach_has_paren && !NextIf<PuncRPar>()) {
        Log << ParserSignal << Current() << "Expected ')' in foreach statement";
      }

      auto body = RecurseForeachBody(m);

      return CreateForeach(index_name, value_name, iter_expr, body);
    } else {
      Log << ParserSignal << Current() << "Expected 'in' keyword in foreach statement";
    }
  } else {
    Log << ParserSignal << Current() << "Expected identifier pair in foreach statement";
  }

  return CreateMockInstance<Foreach>();
}
