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

static auto RecurseForInitExpr(GeneralParser::Context& m) -> NullableFlowPtr<Expr> {
  if (m.NextIf<PuncSemi>()) {
    return std::nullopt;
  }

  auto var_kind = m.Peek();
  if (!var_kind.Is<Let>() && !var_kind.Is<Var>() && !var_kind.Is<Const>()) {
    return m.RecurseExpr({
        Token(Punc, PuncSemi),
    });
  }

  m.Next();

  std::vector<FlowPtr<Expr>> variables;

  switch (var_kind.GetKeyword()) {
    case Keyword::Let: {
      variables = m.RecurseVariable(VariableType::Let);
      break;
    }

    case Keyword::Var: {
      variables = m.RecurseVariable(VariableType::Var);
      break;
    }

    case Keyword::Const: {
      variables = m.RecurseVariable(VariableType::Const);
      break;
    }

    default: {
      break;
    }
  }

  if (variables.size() == 1) {
    return variables[0];
  }

  Log << ParserSignal << m.Current() << "Expected exactly one variable in for loop";

  return std::nullopt;
}

static auto RecurseForCondition(GeneralParser::Context& m) -> NullableFlowPtr<Expr> {
  if (m.NextIf<PuncSemi>()) {
    return std::nullopt;
  }

  auto condition = m.RecurseExpr({
      Token(Punc, PuncSemi),
  });

  if (!m.NextIf<PuncSemi>()) {
    Log << ParserSignal << m.Current() << "Expected semicolon after condition expression";
  }

  return condition;
}

static auto RecurseForStepExpr(GeneralParser::Context& m, bool has_paren) -> NullableFlowPtr<Expr> {
  if (has_paren) {
    if (m.Peek().Is<PuncRPar>()) {
      return std::nullopt;
    }

    return m.RecurseExpr({
        Token(Punc, PuncRPar),
    });
  }

  if (m.Peek().Is<PuncLCur>()) {
    return std::nullopt;
  }

  return m.RecurseExpr({Token(Punc, PuncLCur)});
}

auto GeneralParser::Context::RecurseFor() -> FlowPtr<Expr> {
  bool for_with_paren = NextIf<PuncLPar>().has_value();
  auto for_init = RecurseForInitExpr(m);
  auto for_cond = RecurseForCondition(m);
  auto for_step = RecurseForStepExpr(m, for_with_paren);

  if (for_with_paren && !NextIf<PuncRPar>()) {
    Log << ParserSignal << Current() << "Expected closing parenthesis in for statement";
  }

  auto for_body = RecurseBlock(true, false, BlockMode::Unknown);

  return CreateFor(for_init, for_cond, for_step, for_body);
}
