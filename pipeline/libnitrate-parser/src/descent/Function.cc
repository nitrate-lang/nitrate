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

using namespace ncc;
using namespace ncc::lex;
using namespace ncc::parse;

FlowPtr<parse::Type> Parser::recurse_function_parameter_type() {
  if (next_if(PuncColn)) {
    return recurse_type();
  } else {
    auto type = make<InferTy>()();
    type->set_offset(current().get_start());
    return type;
  }
}

NullableFlowPtr<Expr> Parser::recurse_function_parameter_value() {
  if (next_if(OpSet)) {
    return recurse_expr(

        {Token(Punc, PuncComa), Token(Punc, PuncRPar), Token(Oper, OpGT)});
  } else {
    return std::nullopt;
  }
}

std::optional<FuncParam> Parser::recurse_function_parameter() {
  if (auto name = next_if(Name)) {
    auto param_name = name->as_string();
    auto param_type = recurse_function_parameter_type();
    auto param_value = recurse_function_parameter_value();

    return FuncParam{SaveString(param_name), param_type, param_value};
  } else {
    diagnostic << current() << "Expected a parameter name before ':'";
  }

  return std::nullopt;
}

std::optional<TemplateParameters> Parser::recurse_template_parameters() {
  if (!next_if(OpLT)) {
    return std::nullopt;
  }

  TemplateParameters params;

  while (true) {
    if (next_if(EofF)) {
      diagnostic << current() << "Unexpected EOF in template parameters";
      return params;
    }

    if (next_if(OpGT)) {
      break;
    }

    if (auto param_opt = recurse_function_parameter()) {
      auto param = *param_opt;
      auto tparam = TemplateParameter{std::get<0>(param), std::get<1>(param),
                                      std::get<2>(param)};

      params.push_back(std::move(tparam));

      next_if(PuncComa);
    } else {
      diagnostic << next() << "Expected a template parameter";
    }
  }

  return params;
}

std::pair<FuncParams, bool> Parser::recurse_function_parameters() {
  std::pair<FuncParams, bool> parameters;

  if (!next_if(PuncLPar)) {
    diagnostic << current() << "Expected '(' after function name";
    return parameters;
  }

  while (true) {
    if (next_if(EofF)) {
      diagnostic << current() << "Unexpected EOF in function parameters";
      return parameters;
    }

    if (next_if(PuncRPar)) {
      break;
    }

    if (next_if(OpEllipsis)) {
      parameters.second = true;
      if (!next_if(PuncRPar)) {
        diagnostic << current() << "Expected ')' after variadic parameter";
      }
      break;
    }

    if (auto param_opt = recurse_function_parameter()) {
      auto param = *param_opt;
      auto tparam =
          FuncParam{std::get<0>(param), std::get<1>(param), std::get<2>(param)};

      parameters.first.push_back(std::move(tparam));

      next_if(PuncComa);
    } else {
      diagnostic << next() << "Expected a function parameter";
    }
  }

  return parameters;
}

FuncPurity Parser::get_purity_specifier(Token &start_pos, bool is_thread_safe,
                                        bool is_pure, bool is_impure,
                                        bool is_quasi, bool is_retro) {
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

std::optional<std::pair<std::string_view, bool>>
Parser::recurse_function_capture() {
  bool is_ref = false;

  if (next_if(OpBitAnd)) {
    is_ref = true;
  }

  if (auto name = next_if(Name)) {
    return {{name->as_string(), is_ref}};
  } else {
    diagnostic << next() << "Expected a capture name";
  }

  return std::nullopt;
}

void Parser::recurse_function_ambigouis(ExpressionList &attributes,
                                        FnCaptures &captures,
                                        FuncPurity &purity,
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
  Token start_pos = current();

  while (state != State::End) {
    if (next_if(EofF)) {
      diagnostic << current() << "Unexpected EOF in function attributes";
      break;
    }

    switch (state) {
      case State::Main: {
        if (auto identifier = next_if(Name)) {
          static const std::unordered_set<std::string_view> reserved_words = {
              "foreign", "inline"};

          auto name = identifier->as_string();

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
            attributes.push_back(make<Ident>(SaveString(name))());
          } else {
            function_name = name;

            state = State::End;
          }
        } else if (next_if(PuncLBrk)) {
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

            auto tok = peek();

            /* No attribute expression may begin with '&' */
            if (tok.is<OpBitAnd>()) {
              state = State::CaptureSection;
            } else if (!tok.is(Name)) {
              state = State::AttributesSection;
            } else { /* Ambiguous edge case */
              state = State::CaptureSection;
            }
          }
        } else if (auto tok = peek(); tok.is<PuncLPar>() || tok.is<OpLT>()) {
          state = State::End; /* Begin parsing parameters or template options */
        } else {
          diagnostic << next() << "Unexpected token in function declaration";
        }

        break;
      }

      case State::AttributesSection: {
        parsed_attributes = true;

        while (true) {
          if (next_if(EofF)) {
            diagnostic << current() << "Unexpected EOF in function attributes";
            break;
          }

          if (next_if(PuncRBrk)) {
            state = State::Main;
            break;
          }

          auto attribute = recurse_expr(

              {Token(Punc, PuncComa), Token(Punc, PuncRBrk)});

          attributes.push_back(attribute);

          next_if(PuncComa);
        }

        break;
      }

      case State::CaptureSection: {
        parsed_captures = true;

        while (true) {
          if (next_if(EofF)) {
            diagnostic << current() << "Unexpected EOF in function captures";
            break;
          }

          if (next_if(PuncRBrk)) {
            state = State::Main;
            break;
          }

          if (auto capture = recurse_function_capture()) {
            captures.push_back({SaveString(capture->first), capture->second});
          }

          next_if(PuncComa);
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

FlowPtr<parse::Type> Parser::Parser::recurse_function_return_type() {
  if (next_if(PuncColn)) {
    return recurse_type();
  } else {
    auto type = make<InferTy>()();
    type->set_offset(current().get_start());

    return type;
  }
}

NullableFlowPtr<Stmt> Parser::recurse_function_body(bool restrict_decl_only) {
  if (restrict_decl_only || next_if(PuncSemi)) {
    return std::nullopt;
  } else if (next_if(OpArrow)) {
    return recurse_block(false, true, SafetyMode::Unknown);
  } else {
    return recurse_block(true, false, SafetyMode::Unknown);
  }
}

FlowPtr<Stmt> Parser::recurse_function(bool restrict_decl_only) {
  /* fn <attributes>? <modifiers>? <capture_list>?
   * <name><template_parameters>?(<parameters>?)<: return_type>? <body>? */

  auto start_pos = current().get_start();

  ExpressionList attributes;
  FnCaptures captures;
  FuncPurity purity = FuncPurity::IMPURE_THREAD_UNSAFE;
  std::string_view function_name;

  recurse_function_ambigouis(attributes, captures, purity, function_name);

  auto template_parameters = recurse_template_parameters();
  auto parameters = recurse_function_parameters();
  auto return_type = recurse_function_return_type();
  auto body = recurse_function_body(restrict_decl_only);

  /// TODO: Implement function contract pre and post conditions

  auto function =
      make<Function>(attributes, purity, captures, SaveString(function_name),
                     template_parameters, parameters.first, parameters.second,
                     return_type, std::nullopt, std::nullopt, body)();
  function->set_offset(start_pos);

  return function;
}
