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
#include <nitrate-parser/ASTData.hh>
#include <unordered_set>

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
      diagnostic << next() << "Expected a template parameter";
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
      diagnostic << next() << "Expected a function parameter";
    }
  }

  return parameters;
}

static FuncPurity get_purity_specifier(qlex_tok_t &start_pos,
                                       bool is_thread_safe, bool is_pure,
                                       bool is_impure, bool is_quasi,
                                       bool is_retro) {
  /** Thread safety does not conflict with purity.
   *  Purity implies thread safety.
   */
  (void)is_thread_safe;

  /* Ensure that there is no duplication of purity specifiers */
  if ((is_impure + is_pure + is_quasi + is_retro) > 1) {
    diagnostic << start_pos << "Conflicting purity specifiers";
    return FuncPurity::IMPURE_THREAD_UNSAFE;
  }

  if (is_pure) {
    return FuncPurity::PURE;
  } else if (is_quasi) {
    return FuncPurity::QUASI;
  } else if (is_retro) {
    return FuncPurity::RETRO;
  } else if (is_thread_safe) {
    return FuncPurity::IMPURE_THREAD_SAFE;
  } else {
    return FuncPurity::IMPURE_THREAD_UNSAFE;
  }
}

static std::optional<std::pair<std::string_view, bool>>
recurse_function_capture(qlex_t &rd) {
  bool is_ref = false;

  if (next_if(qOpBitAnd)) {
    is_ref = true;
  }

  if (let name = next_if(qName)) {
    return {{name->as_string(&rd), is_ref}};
  } else {
    diagnostic << next() << "Expected a capture name";
  }

  return std::nullopt;
}

static void recurse_function_ambigouis(npar_t &S, qlex_t &rd,
                                       ExpressionList &attributes,
                                       FnCaptures &captures, FuncPurity &purity,
                                       std::string_view &function_name) {
  enum class State {
    Main,
    AttributesSection,
    CaptureSection,
    End,
  } state = State::Main;

  bool is_thread_safe = false, is_pure = false, is_impure = false,
       is_quasi = false, is_retro = false;
  bool parsed_attributes = false, parsed_captures = false;
  qlex_tok_t start_pos = current();

  while (state != State::End) {
    if (next_if(qEofF)) {
      diagnostic << current() << "Unexpected EOF in function attributes";
      break;
    }

    switch (state) {
      case State::Main: {
        if (let identifier = next_if(qName)) {
          static const std::unordered_set<std::string_view> reserved_words = {
              "foreign", "inline"};

          let name = identifier->as_string(&rd);

          if (name == "pure") {
            is_pure = true;
          } else if (name == "impure") {
            is_impure = true;
          } else if (name == "tsafe") {
            is_thread_safe = true;
          } else if (name == "quasi") {
            is_quasi = true;
          } else if (name == "retro") {
            is_retro = true;
          } else if (reserved_words.contains(name)) {
            attributes.push_back(make<Ident>(SaveString(name)));
          } else {
            function_name = name;

            state = State::End;
          }
        } else if (next_if(qPuncLBrk)) {
          if (parsed_attributes && parsed_captures) {
            diagnostic
                << current()
                << "Unexpected '[' after function attributes and captures";
          } else if (parsed_attributes && !parsed_captures) {
            state = State::CaptureSection;
          } else if (!parsed_attributes && parsed_captures) {
            state = State::AttributesSection;
          } else {
            qcore_assert(!parsed_attributes && !parsed_captures);

            let tok = peek();

            /* No attribute expression may begin with '&' */
            if (tok.is<qOpBitAnd>()) {
              state = State::CaptureSection;
            } else if (!tok.is(qName)) {
              state = State::AttributesSection;
            } else { /* Ambiguous edge case */
              state = State::CaptureSection;
            }
          }
        } else if (let tok = peek(); tok.is<qPuncLPar>() || tok.is<qOpLT>()) {
          state = State::End; /* Begin parsing parameters or template options */
        } else {
          diagnostic << next() << "Unexpected token in function declaration";
        }

        break;
      }

      case State::AttributesSection: {
        parsed_attributes = true;

        while (true) {
          if (next_if(qEofF)) {
            diagnostic << current() << "Unexpected EOF in function attributes";
            break;
          }

          if (next_if(qPuncRBrk)) {
            state = State::Main;
            break;
          }

          let attribute = recurse_expr(
              S, rd,
              {qlex_tok_t(qPunc, qPuncComa), qlex_tok_t(qPunc, qPuncRBrk)});

          attributes.push_back(attribute);

          next_if(qPuncComa);
        }

        break;
      }

      case State::CaptureSection: {
        parsed_captures = true;

        while (true) {
          if (next_if(qEofF)) {
            diagnostic << current() << "Unexpected EOF in function captures";
            break;
          }

          if (next_if(qPuncRBrk)) {
            state = State::Main;
            break;
          }

          if (let capture = recurse_function_capture(rd)) {
            captures.push_back({SaveString(capture->first), capture->second});
          }

          next_if(qPuncComa);
        }

        break;
      }

      case State::End: {
        break;
      }
    }
  }

  purity = get_purity_specifier(start_pos, is_thread_safe, is_pure, is_impure,
                                is_quasi, is_retro);
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
    return recurse_block(S, rd, false, true, SafetyMode::Unknown);
  } else {
    return recurse_block(S, rd, true, false, SafetyMode::Unknown);
  }
}

Stmt *npar::recurse_function(npar_t &S, qlex_t &rd, bool restrict_decl_only) {
  /* fn <attributes>? <modifiers>? <capture_list>?
   * <name><template_parameters>?(<parameters>?)<: return_type>? <body>? */

  let start_pos = current().start;

  ExpressionList attributes;
  FnCaptures captures;
  FuncPurity purity = FuncPurity::IMPURE_THREAD_UNSAFE;
  std::string_view function_name;

  recurse_function_ambigouis(S, rd, attributes, captures, purity,
                             function_name);

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
