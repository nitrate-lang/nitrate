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

using namespace ncc::parse;

std::optional<ExpressionList> Parser::recurse_variable_attributes() {
  ExpressionList attributes;

  if (!next_if(qPuncLBrk)) {
    return attributes;
  }

  while (true) {
    let tok = peek();

    if (!tok.is(qEofF)) {
      if (next_if(qPuncRBrk)) {
        return attributes;
      }

      let attribute = recurse_expr(
          {NCCToken(qPunc, qPuncComa), NCCToken(qPunc, qPuncRBrk)});

      attributes.push_back(attribute);

      next_if(qPuncComa);
    } else {
      diagnostic << tok << "Encountered EOF while parsing variable attributes";
      break;
    }
  }

  return std::nullopt;
}

std::optional<Type *> Parser::recurse_variable_type() {
  if (next_if(qPuncColn)) {
    return recurse_type();
  } else {
    return std::nullopt;
  }
}

std::optional<Expr *> Parser::recurse_variable_value() {
  if (next_if(qOpSet)) {
    return recurse_expr(
        {NCCToken(qPunc, qPuncComa), NCCToken(qPunc, qPuncSemi)});
  } else {
    return std::nullopt;
  }
}

std::optional<Stmt *> Parser::recurse_variable_instance(VarDeclType decl_type) {
  if (let attributes = recurse_variable_attributes()) {
    if (let tok = next_if(qName)) {
      let name = tok->as_string(&rd);
      let type = recurse_variable_type();
      let value = recurse_variable_value();

      return make<VarDecl>(SaveString(name), type.value_or(nullptr),
                           value.value_or(nullptr), decl_type,
                           std::move(attributes.value()));
    } else {
      diagnostic << current() << "Expected variable name";
      return std::nullopt;
    }
  } else {
    diagnostic << current() << "Malformed variable attributes";
  }

  return mock_stmt(QAST_VAR);
}

std::vector<Stmt *> Parser::recurse_variable(VarDeclType decl_type) {
  std::vector<Stmt *> variables;

  while (true) {
    if (next_if(qEofF)) {
      diagnostic << current() << "Unexpected EOF in variable declaration";
      break;
    }

    if (next_if(qPuncSemi)) {
      return variables;
    }

    if (let var_opt = recurse_variable_instance(decl_type)) {
      variables.push_back(var_opt.value());
    } else {
      diagnostic << current() << "Failed to parse variable declaration";
      break;
    }

    next_if(qPuncComa);
  }

  return {mock_stmt(QAST_VAR)};
}
