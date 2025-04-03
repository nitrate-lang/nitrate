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
#include <nitrate-parser/ASTType.hh>

using namespace ncc;
using namespace ncc::lex;
using namespace ncc::parse;

static auto RecurseVariableTypeSpecifier(GeneralParser::Context& m) -> FlowPtr<parse::Type> {
  if (m.NextIf<PuncColn>()) {
    return m.RecurseType();
  }

  return m.CreateInferredType();
}

static auto RecurseVariableInitializer(GeneralParser::Context& m) -> NullableFlowPtr<Expr> {
  if (m.NextIf<OpSet>()) {
    return m.RecurseExpr({
        Token(PuncComa),
        Token(PuncSemi),
    });
  }

  return std::nullopt;
}

static auto RecurseVariableInstance(GeneralParser::Context& m, VariableType decl_type) -> FlowPtr<Expr> {
  auto attributes = m.RecurseAttributes("variable");
  auto name = m.RecurseName();
  if (!name) {
    Log << ParserSignal << m.Next() << "No variable name found in variable declaration";
  }
  auto type = RecurseVariableTypeSpecifier(m);
  auto init = RecurseVariableInitializer(m);
  auto variable = m.CreateVariable(decl_type, name, attributes, type, init);

  return variable;
}

auto GeneralParser::Context::RecurseVariable(VariableType decl_type) -> std::vector<FlowPtr<Expr>> {
  std::vector<FlowPtr<Expr>> variables;

  while (true) {
    if (m.IsEof()) [[unlikely]] {
      Log << ParserSignal << Current() << "Unexpected EOF in variable declaration";
      return variables;
    }

    auto variable = RecurseVariableInstance(m, decl_type);
    variables.push_back(variable);

    if (NextIf<PuncComa>()) {
      continue;
    }

    if (NextIf<PuncSemi>()) {
      return variables;
    }

    Log << ParserSignal << Next() << "Expected comma or semicolon after variable declaration";
    return variables;
  }
}
