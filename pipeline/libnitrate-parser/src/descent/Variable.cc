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

auto GeneralParser::PImpl::RecurseVariableAttributes() -> std::vector<FlowPtr<Expr>> {
  std::vector<FlowPtr<Expr>> attributes;

  if (!NextIf<PuncLBrk>()) {
    return attributes;
  }

  while (true) {
    if (m_rd.IsEof()) [[unlikely]] {
      Log << ParserSignal << Current() << "Encountered EOF while parsing variable attribute";
      return attributes;
    }

    if (NextIf<PuncRBrk>()) {
      return attributes;
    }

    auto attribute = RecurseExpr({
        Token(Punc, PuncComa),
        Token(Punc, PuncRBrk),
    });

    attributes.push_back(attribute);

    NextIf<PuncComa>();
  }
}

auto GeneralParser::PImpl::RecurseVariableType() -> FlowPtr<parse::Type> {
  if (NextIf<PuncColn>()) {
    return RecurseType();
  }

  return m_fac.CreateUnknownType();
}

auto GeneralParser::PImpl::RecurseVariableValue() -> NullableFlowPtr<Expr> {
  if (NextIf<OpSet>()) {
    return RecurseExpr({
        Token(Punc, PuncComa),
        Token(Punc, PuncSemi),
    });
  }

  return std::nullopt;
}

auto GeneralParser::PImpl::RecurseVariableInstance(VariableType decl_type) -> NullableFlowPtr<Expr> {
  auto symbol_attributes_opt = RecurseVariableAttributes();
  if (auto variable_name = RecurseName()) {
    auto variable_type = RecurseVariableType();
    auto variable_initial = RecurseVariableValue();

    return m_fac.CreateVariable(decl_type, variable_name, symbol_attributes_opt, variable_type, variable_initial);
  }

  Log << ParserSignal << Current() << "Expected variable name";

  return std::nullopt;
}

auto GeneralParser::PImpl::RecurseVariable(VariableType decl_type) -> std::vector<FlowPtr<Expr>> {
  std::vector<FlowPtr<Expr>> variables;

  while (true) {
    if (m_rd.IsEof()) [[unlikely]] {
      Log << ParserSignal << Current() << "Unexpected EOF in variable declaration";
      break;
    }

    if (auto variable_opt = RecurseVariableInstance(decl_type)) {
      variables.push_back(variable_opt.value());
    } else {
      Log << ParserSignal << Current() << "Failed to parse variable declaration";
      break;
    }

    if (NextIf<PuncSemi>()) {
      return variables;
    }

    if (!NextIf<PuncComa>()) {
      Log << ParserSignal << Current() << "Expected comma or semicolon after variable declaration";
      break;
    }
  }

  return {m_fac.CreateMockInstance<Variable>()};
}
