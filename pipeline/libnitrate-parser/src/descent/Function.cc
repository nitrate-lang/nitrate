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

/// TODO: Source location

/// TODO: Cleanup this code; it's a mess from refactoring.

#include <descent/Recurse.hh>

using namespace npar;

static std::optional<FuncParam> recurse_fn_parameter(npar_t &S, qlex_t &rd) {
  auto tok = next();

  std::string name;
  Type *type = nullptr;

  if (!tok.is(qName)) {
    diagnostic << tok << "Expected a parameter name before ':'";
  }

  name = tok.as_string(&rd);
  tok = peek();

  if (tok.is<qPuncColn>()) {
    next();

    type = recurse_type(S, rd);

    tok = peek();
  } else {
    type = make<InferTy>();
  }

  if (tok.is<qOpSet>()) {
    next();
    tok = peek();

    Expr *value =
        recurse_expr(S, rd,
                     {qlex_tok_t(qPunc, qPuncComa),
                      qlex_tok_t(qPunc, qPuncRPar), qlex_tok_t(qOper, qOpGT)});

    return FuncParam(SaveString(name), type, value);
  } else {
    return FuncParam(SaveString(name), type, nullptr);
  }
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

    if (let param_opt = recurse_fn_parameter(S, rd)) {
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
  /// TODO: Implement function parsing
  qcore_implement();
  (void)S;
  (void)rd;
}
