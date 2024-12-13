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
#include <nitrate-parser/AST.hh>

using namespace npar;

static std::optional<ExpressionList> recurse_variable_attributes(npar_t &S,
                                                                 qlex_t &rd) {
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
          S, rd, {qlex_tok_t(qPunc, qPuncComa), qlex_tok_t(qPunc, qPuncRBrk)});

      attributes.push_back(attribute);

      next_if(qPuncComa);
    } else {
      diagnostic << tok << "Encountered EOF while parsing variable attributes";
      break;
    }
  }

  return std::nullopt;
}

static std::optional<Type *> recurse_variable_type(npar_t &S, qlex_t &rd) {
  if (next_if(qPuncColn)) {
    return recurse_type(S, rd);
  } else {
    return std::nullopt;
  }
}

static std::optional<Expr *> recurse_variable_value(npar_t &S, qlex_t &rd) {
  if (next_if(qOpSet)) {
    return recurse_expr(
        S, rd, {qlex_tok_t(qPunc, qPuncComa), qlex_tok_t(qPunc, qPuncSemi)});
  } else {
    return std::nullopt;
  }
}

static std::optional<Stmt *> recurse_variable_instance(npar_t &S, qlex_t &rd,
                                                       VarDeclType decl_type) {
  if (let attributes = recurse_variable_attributes(S, rd)) {
    if (let tok = next_if(qName)) {
      let name = tok->as_string(&rd);
      let type = recurse_variable_type(S, rd);
      let value = recurse_variable_value(S, rd);

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

std::vector<Stmt *> npar::recurse_variable(npar_t &S, qlex_t &rd,
                                           VarDeclType decl_type) {
  std::vector<Stmt *> variables;

  while (true) {
    if (peek().is(qEofF)) {
      diagnostic << current() << "Unexpected EOF in variable declaration";
      break;
    }

    if (next_if(qPuncSemi)) {
      return variables;
    }

    if (let var_opt = recurse_variable_instance(S, rd, decl_type)) {
      variables.push_back(var_opt.value());
    } else {
      diagnostic << current() << "Failed to parse variable declaration";
      break;
    }

    next_if(qPuncComa);
  }

  return {mock_stmt(QAST_VAR)};
}
