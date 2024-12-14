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

/// TODO: Cleanup this code; it's a mess from refactoring.

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

Stmt *npar::recurse_function(npar_t &S, qlex_t &rd) {
  /* fn <attributes>? <modifiers>? <capture_list>? <name>?(<parameters>?)<:
   * return_type>? <body>? */

  /// TODO: Implement function parsing
  qcore_implement();
  (void)S;
  (void)rd;
}
