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

std::optional<ExpressionList> Parser::PImpl::RecurseVariableAttributes() {
  ExpressionList attributes;

  if (!next_if(PuncLBrk)) {
    return attributes;
  }

  while (true) {
    if (next_if(EofF)) [[unlikely]] {
      Log << SyntaxError << current()
          << "Encountered EOF while parsing variable attribute";
      break;
    }

    if (next_if(PuncRBrk)) {
      return attributes;
    }

    auto attribute = RecurseExpr({
        Token(Punc, PuncComa),
        Token(Punc, PuncRBrk),
    });

    attributes.push_back(attribute);

    next_if(PuncComa);
  }

  return std::nullopt;
}

NullableFlowPtr<parse::Type> Parser::PImpl::RecurseVariableType() {
  if (next_if(PuncColn)) {
    return RecurseType();
  }

  return std::nullopt;
}

NullableFlowPtr<Expr> Parser::PImpl::RecurseVariableValue() {
  if (next_if(OpSet)) {
    return RecurseExpr({
        Token(Punc, PuncComa),
        Token(Punc, PuncSemi),
    });
  }

  return std::nullopt;
}

NullableFlowPtr<Stmt> Parser::PImpl::RecurseVariableInstance(
    VarDeclType decl_type) {
  if (auto symbol_attributes_opt = RecurseVariableAttributes()) {
    if (auto tok = next_if(Name)) {
      auto variable_name = tok->GetString();
      auto variable_type = RecurseVariableType();
      auto variable_initial = RecurseVariableValue();

      return make<VarDecl>(variable_name, variable_type, variable_initial,
                           decl_type, symbol_attributes_opt.value())();
    }

    Log << SyntaxError << current() << "Expected variable name";

    return std::nullopt;
  }

  Log << SyntaxError << current() << "Malformed variable attributes";

  return MockStmt(QAST_VAR);
}

std::vector<FlowPtr<Stmt>> Parser::PImpl::RecurseVariable(
    VarDeclType decl_type) {
  std::vector<FlowPtr<Stmt>> variables;

  while (true) {
    if (next_if(EofF)) [[unlikely]] {
      Log << SyntaxError << current()
          << "Unexpected EOF in variable declaration";
      break;
    }

    if (next_if(PuncSemi)) {
      return variables;
    }

    if (auto variable_opt = RecurseVariableInstance(decl_type)) {
      variables.push_back(variable_opt.value());
    } else {
      Log << SyntaxError << current() << "Failed to parse variable declaration";
      break;
    }

    next_if(PuncComa);
  }

  return {MockStmt(QAST_VAR)};
}
