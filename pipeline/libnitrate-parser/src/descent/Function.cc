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

using namespace npar;

static Type *recurse_function_parameter_type(npar_t &S, qlex_t &rd) {
  if (next_if(qPuncColn)) {
    return recurse_type(S, rd);
  } else {
    let type = make<InferTy>();
    type->set_offset(current().start);
    return type;
  }
}

static std::optional<Expr *> recurse_function_parameter_value(npar_t &S,
                                                              qlex_t &rd) {
  if (next_if(qOpSet)) {
    return recurse_expr(
        S, rd,
        {qlex_tok_t(qPunc, qPuncComa), qlex_tok_t(qPunc, qPuncRPar),
         qlex_tok_t(qOper, qOpGT)});
  } else {
    return std::nullopt;
  }
}

static std::optional<FuncParam> recurse_function_parameter(npar_t &S,
                                                           qlex_t &rd) {
  if (let name = next_if(qName)) {
    let param_name = name->as_string(&rd);
    let param_type = recurse_function_parameter_type(S, rd);
    let param_value = recurse_function_parameter_value(S, rd);

    return FuncParam{SaveString(param_name), param_type,
                     param_value.value_or(nullptr)};
  } else {
    diagnostic << current() << "Expected a parameter name before ':'";
  }

  return std::nullopt;
}

std::optional<TemplateParameters> recurse_template_parameters(npar_t &S,
                                                              qlex_t &rd) {
  if (!next_if(qOpLT)) {
    return std::nullopt;
  }

  TemplateParameters params;

  while (true) {
    if (next_if(qEofF)) {
      diagnostic << current() << "Unexpected EOF in template parameters";
      return params;
    }

    if (next_if(qOpGT)) {
      break;
    }

    if (let param_opt = recurse_function_parameter(S, rd)) {
      let param = *param_opt;
      let tparam = TemplateParameter{std::get<0>(param), std::get<1>(param),
                                     std::get<2>(param)};

      params.push_back(std::move(tparam));

      next_if(qPuncComa);
    } else {
      diagnostic << current() << "Expected a template parameter";
    }
  }

  return params;
}

static FuncParams recurse_function_parameters(npar_t &S, qlex_t &rd) {
  FuncParams parameters;

  if (!next_if(qPuncLPar)) {
    diagnostic << current() << "Expected '(' after function name";
    return parameters;
  }

  while (true) {
    if (next_if(qEofF)) {
      diagnostic << current() << "Unexpected EOF in function parameters";
      return parameters;
    }

    if (next_if(qPuncRPar)) {
      break;
    }

    if (next_if(qOpEllipsis)) {
      parameters.is_variadic = true;
      if (!next_if(qPuncRPar)) {
        diagnostic << current() << "Expected ')' after variadic parameter";
      }
      break;
    }

    if (let param_opt = recurse_function_parameter(S, rd)) {
      let param = *param_opt;
      let tparam =
          FuncParam{std::get<0>(param), std::get<1>(param), std::get<2>(param)};

      parameters.params.push_back(std::move(tparam));

      next_if(qPuncComa);
    } else {
      diagnostic << current() << "Expected a function parameter";
    }
  }

  return parameters;
}

static void recurse_function_ambigouis(npar_t &S, qlex_t &rd,
                                       ExpressionList &attrs,
                                       FnCaptures &captures,
                                       FuncPurity &purity) {
  return;
  /// TODO: Implement function attributes
  qcore_implement();
}

static std::string_view recurse_function_name(qlex_t &rd) {
  if (let name = next_if(qName)) {
    return name->as_string(&rd);
  } else {
    return "";
  }
}

static Type *recurse_function_return_type(npar_t &S, qlex_t &rd) {
  if (next_if(qPuncColn)) {
    return recurse_type(S, rd);
  } else {
    let type = make<InferTy>();
    type->set_offset(current().start);

    return type;
  }
}

static std::optional<Stmt *> recurse_function_body(npar_t &S, qlex_t &rd,
                                                   bool restrict_decl_only) {
  if (restrict_decl_only || next_if(qPuncSemi)) {
    return std::nullopt;
  } else if (next_if(qOpArrow)) {
    return recurse_block(S, rd, false, true);
  } else {
    return recurse_block(S, rd, true, false);
  }
}

Stmt *npar::recurse_function(npar_t &S, qlex_t &rd, bool restrict_decl_only) {
  /* fn <attributes>? <modifiers>? <capture_list>?
   * <name><template_parameters>?(<parameters>?)<: return_type>? <body>? */

  let start_pos = current().start;

  ExpressionList attributes;
  FnCaptures captures;
  FuncPurity purity = FuncPurity::IMPURE_THREAD_UNSAFE;

  recurse_function_ambigouis(S, rd, attributes, captures, purity);

  let function_name = recurse_function_name(rd);
  let template_parameters = recurse_template_parameters(S, rd);
  let parameters = recurse_function_parameters(S, rd);
  let return_type = recurse_function_return_type(S, rd);
  let body = recurse_function_body(S, rd, restrict_decl_only);

  /// TODO: Implement function contract pre and post conditions

  let function =
      make<Function>(attributes, purity, captures, SaveString(function_name),
                     template_parameters, parameters, return_type, std::nullopt,
                     std::nullopt, body);
  function->set_offset(start_pos);

  return function;
}
