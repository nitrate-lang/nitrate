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

FlowPtr<parse::Type> Parser::recurse_function_parameter_type() {
  if (next_if(PuncColn)) {
    return recurse_type();
  } else {
    return make<InferTy>()();
  }
}

NullableFlowPtr<Expr> Parser::recurse_function_parameter_value() {
  if (next_if(OpSet)) {
    return recurse_expr({
        Token(Punc, PuncComa),
        Token(Punc, PuncRPar),
        Token(Oper, OpGT),
    });
  } else {
    return std::nullopt;
  }
}

std::optional<FuncParam> Parser::recurse_function_parameter() {
  if (auto param_name = next_if(Name)) [[likely]] {
    auto param_type = recurse_function_parameter_type();
    auto param_value = recurse_function_parameter_value();

    return FuncParam{param_name->as_string(), param_type, param_value};
  } else {
    log << SyntaxError << next() << "Expected a parameter name before ':'";
  }

  return std::nullopt;
}

std::optional<TemplateParameters> Parser::recurse_template_parameters() {
  if (!next_if(OpLT)) {
    return std::nullopt;
  }

  TemplateParameters params;

  while (true) {
    if (next_if(EofF)) [[unlikely]] {
      log << SyntaxError << current()
          << "Unexpected EOF in template parameters";
      return params;
    }

    if (next_if(OpGT)) {
      break;
    }

    if (auto param_opt = recurse_function_parameter()) {
      auto [param_name, param_type, param_value] = param_opt.value();

      params.push_back({param_name, param_type, param_value});
    } else {
      log << SyntaxError << next() << "Expected a template parameter";
    }

    next_if(PuncComa);
  }

  return params;
}

std::pair<FuncParams, bool> Parser::recurse_function_parameters() {
  std::pair<FuncParams, bool> parameters;

  if (!next_if(PuncLPar)) [[unlikely]] {
    log << SyntaxError << current() << "Expected '(' after function name";
    return parameters;
  }

  bool is_variadic = false;

  while (true) {
    if (next_if(EofF)) [[unlikely]] {
      log << SyntaxError << current()
          << "Unexpected EOF in function parameters";
      return parameters;
    }

    if (next_if(PuncRPar)) {
      break;
    }

    if (next_if(OpEllipsis)) {
      is_variadic = true;

      if (!peek().is<PuncRPar>()) {
        log << SyntaxError << current()
            << "Expected ')' after variadic parameter";
      }
      continue;
    }

    if (auto parameter = recurse_function_parameter()) {
      auto [param_name, param_type, param_value] = parameter.value();
      parameters.first.push_back({param_name, param_type, param_value});

    } else {
      log << SyntaxError << next() << "Expected a function parameter";
    }

    next_if(PuncComa);
  }

  parameters.second = is_variadic;

  return parameters;
}

Purity Parser::get_purity_specifier(Token start_pos, bool is_thread_safe,
                                    bool is_pure, bool is_impure, bool is_quasi,
                                    bool is_retro) {
  /* Ensure that there is no duplication of purity specifiers */
  if ((is_impure + is_pure + is_quasi + is_retro) > 1) {
    log << SyntaxError << start_pos << "Conflicting purity specifiers";
    return Purity::Impure;
  }

  /** Thread safety does not conflict with purity.
   *  Purity implies thread safety.
   */
  if (is_pure) {
    return Purity::Pure;
  } else if (is_quasi) {
    return Purity::Quasi;
  } else if (is_retro) {
    return Purity::Retro;
  } else if (is_thread_safe) {
    return Purity::Impure_TSafe;
  } else {
    return Purity::Impure;
  }
}

std::optional<std::pair<string, bool>> Parser::recurse_function_capture() {
  bool is_ref = next_if(OpBitAnd).has_value();

  if (auto name = next_if(Name)) {
    return {{name->as_string(), is_ref}};
  } else {
    log << SyntaxError << next() << "Expected a capture name";
    return std::nullopt;
  }
}

std::tuple<ExpressionList, FnCaptures, Purity, string>
Parser::recurse_function_ambigouis() {
  enum class State {
    Ground,
    AttributesSection,
    CaptureSection,
    End,
  } state = State::Ground;

  auto start_pos = current();
  ExpressionList attributes;
  FnCaptures captures;
  string function_name;
  bool is_thread_safe = false, is_pure = false, is_impure = false,
       is_quasi = false, is_retro = false, already_parsed_attributes = false,
       already_parsed_captures = false;

  while (state != State::End) {
    if (next_if(EofF)) [[unlikely]] {
      log << SyntaxError << current()
          << "Unexpected EOF in function attributes";
      break;
    }

    switch (state) {
      case State::Ground: {
        if (auto some_identifier = next_if(Name)) {
          auto some_word = some_identifier->as_string();

          if (some_word == "pure") {
            is_pure = true;
          } else if (some_word == "impure") {
            is_impure = true;
          } else if (some_word == "tsafe") {
            is_thread_safe = true;
          } else if (some_word == "quasi") {
            is_quasi = true;
          } else if (some_word == "retro") {
            is_retro = true;
          } else if (some_word == "foreign" || some_word == "inline") {
            attributes.push_back(make<Ident>(some_word)());
          } else {
            function_name = some_word;
            state = State::End;
          }
        } else if (next_if(PuncLBrk)) {
          if (already_parsed_attributes && already_parsed_captures) {
            log << SyntaxError << current()
                << "Unexpected '[' after function attributes and captures";
          } else if (already_parsed_attributes && !already_parsed_captures) {
            state = State::CaptureSection;
          } else if (!already_parsed_attributes && already_parsed_captures) {
            state = State::AttributesSection;
          } else {
            qcore_assert(!already_parsed_attributes &&
                         !already_parsed_captures);

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
          log << SyntaxError << next()
              << "Unexpected token in function declaration";
        }

        break;
      }

      case State::AttributesSection: {
        already_parsed_attributes = true;

        while (true) {
          if (next_if(EofF)) [[unlikely]] {
            log << SyntaxError << current()
                << "Unexpected EOF in function attributes";
            break;
          }

          if (next_if(PuncRBrk)) {
            state = State::Ground;
            break;
          }

          auto attribute = recurse_expr({
              Token(Punc, PuncComa),
              Token(Punc, PuncRBrk),
          });

          attributes.push_back(attribute);

          next_if(PuncComa);
        }

        break;
      }

      case State::CaptureSection: {
        already_parsed_captures = true;

        while (true) {
          if (next_if(EofF)) [[unlikely]] {
            log << SyntaxError << current()
                << "Unexpected EOF in function captures";
            break;
          }

          if (next_if(PuncRBrk)) {
            state = State::Ground;
            break;
          }

          if (auto capture = recurse_function_capture()) {
            captures.push_back({capture->first, capture->second});
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

  auto purity = get_purity_specifier(start_pos, is_thread_safe, is_pure,
                                     is_impure, is_quasi, is_retro);

  return {attributes, captures, purity, function_name};
}

FlowPtr<parse::Type> Parser::Parser::recurse_function_return_type() {
  if (next_if(PuncColn)) {
    return recurse_type();
  } else {
    return make<InferTy>()();
  }
}

NullableFlowPtr<Stmt> Parser::recurse_function_body(
    bool parse_declaration_only) {
  if (parse_declaration_only || next_if(PuncSemi)) {
    return std::nullopt;
  } else if (next_if(OpArrow)) {
    return recurse_block(false, true, SafetyMode::Unknown);
  } else {
    return recurse_block(true, false, SafetyMode::Unknown);
  }
}

FlowPtr<Stmt> Parser::recurse_function(bool parse_declaration_only) {
  auto start_pos = current().get_start();

  auto [function_attributes, function_captures, function_purity,
        function_name] = recurse_function_ambigouis();
  auto function_template_parameters = recurse_template_parameters();
  auto function_parameters = recurse_function_parameters();
  auto function_return_type = recurse_function_return_type();
  auto function_body = recurse_function_body(parse_declaration_only);

  auto function = make<Function>(
      function_attributes, function_purity, function_captures, function_name,
      function_template_parameters, function_parameters.first,
      function_parameters.second, function_return_type, std::nullopt,
      std::nullopt, function_body)();
  function->set_offset(start_pos);

  return function;
}
