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

auto Parser::PImpl::RecurseFunctionParameterType() -> FlowPtr<parse::Type> {
  if (NextIf(PuncColn)) {
    return RecurseType();
  }

  return CreateNode<InferTy>()();
}

auto Parser::PImpl::RecurseFunctionParameterValue() -> NullableFlowPtr<Expr> {
  if (NextIf(OpSet)) {
    return RecurseExpr({
        Token(Punc, PuncComa),
        Token(Punc, PuncRPar),
        Token(Oper, OpGT),
    });
  }

  return std::nullopt;
}

auto Parser::PImpl::RecurseFunctionParameter() -> std::optional<FuncParam> {
  if (auto param_name = RecurseName(); !param_name.empty()) [[likely]] {
    auto param_type = RecurseFunctionParameterType();
    auto param_value = RecurseFunctionParameterValue();

    return FuncParam{param_name, param_type, param_value};
  }

  Log << SyntaxError << next() << "Expected a parameter name before ':'";

  return std::nullopt;
}

auto Parser::PImpl::RecurseTemplateParameters() -> std::optional<TemplateParameters> {
  if (!NextIf(OpLT)) {
    return std::nullopt;
  }

  TemplateParameters params;

  while (true) {
    if (NextIf(EofF)) [[unlikely]] {
      Log << SyntaxError << current() << "Unexpected EOF in template parameters";
      return params;
    }

    if (NextIf(OpGT)) {
      break;
    }

    if (auto param_opt = RecurseFunctionParameter()) {
      auto [param_name, param_type, param_value] = param_opt.value();

      params.emplace_back(param_name, param_type, param_value);
    } else {
      Log << SyntaxError << next() << "Expected a template parameter";
    }

    NextIf(PuncComa);
  }

  return params;
}

auto Parser::PImpl::RecurseFunctionParameters() -> std::pair<FuncParams, bool> {
  std::pair<FuncParams, bool> parameters;

  if (!NextIf(PuncLPar)) [[unlikely]] {
    Log << SyntaxError << current() << "Expected '(' after function name";

    return parameters;
  }

  bool is_variadic = false;

  while (true) {
    if (NextIf(EofF)) [[unlikely]] {
      Log << SyntaxError << current() << "Unexpected EOF in function parameters";

      return parameters;
    }

    if (NextIf(PuncRPar)) {
      break;
    }

    if (NextIf(OpEllipsis)) {
      is_variadic = true;

      if (!peek().Is<PuncRPar>()) {
        Log << SyntaxError << current() << "Expected ')' after variadic parameter";
      }
      continue;
    }

    if (auto parameter = RecurseFunctionParameter()) {
      auto [param_name, param_type, param_value] = parameter.value();
      parameters.first.emplace_back(param_name, param_type, param_value);
    } else {
      Log << SyntaxError << next() << "Expected a function parameter";
    }

    NextIf(PuncComa);
  }

  parameters.second = is_variadic;

  return parameters;
}

auto Parser::PImpl::GetPuritySpecifier(Token start_pos, bool is_thread_safe, bool is_pure, bool is_impure,
                                       bool is_quasi, bool is_retro) -> Purity {
  /* Ensure that there is no duplication of purity specifiers */
  if ((static_cast<int>(is_impure) + static_cast<int>(is_pure) + static_cast<int>(is_quasi) +
       static_cast<int>(is_retro)) > 1) {
    Log << SyntaxError << start_pos << "Conflicting purity specifiers";

    return Purity::Impure;
  }

  /** Thread safety does not conflict with purity.
   *  Purity implies thread safety.
   */
  if (is_pure) {
    return Purity::Pure;
  }

  if (is_quasi) {
    return Purity::Quasi;
  }

  if (is_retro) {
    return Purity::Retro;
  }

  if (is_thread_safe) {
    return Purity::Impure_TSafe;
  }

  return Purity::Impure;
}

auto Parser::PImpl::RecurseFunctionCapture() -> std::optional<std::pair<string, bool>> {
  bool is_ref = NextIf(OpBitAnd).has_value();

  if (auto name = RecurseName(); !name.empty()) {
    return {{name, is_ref}};
  }

  Log << SyntaxError << next() << "Expected a capture name";

  return std::nullopt;
}

auto Parser::PImpl::RecurseFunctionAmbigouis() -> std::tuple<ExpressionList, FnCaptures, Purity, string> {
  enum class State : uint8_t {
    Ground,
    AttributesSection,
    CaptureSection,
    End,
  } state = State::Ground;

  auto start_pos = current();
  ExpressionList attributes;
  FnCaptures captures;
  string function_name;
  bool is_thread_safe = false;
  bool is_pure = false;
  bool is_impure = false;
  bool is_quasi = false;
  bool is_retro = false;
  bool already_parsed_attributes = false;
  bool already_parsed_captures = false;

  while (state != State::End) {
    if (NextIf(EofF)) [[unlikely]] {
      Log << SyntaxError << current() << "Unexpected EOF in function attributes";
      break;
    }

    switch (state) {
      case State::Ground: {
        if (auto some_word = RecurseName(); !some_word.empty()) {
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
            attributes.push_back(CreateNode<Identifier>(some_word)());
          } else {
            function_name = some_word;
            state = State::End;
          }
        } else if (NextIf(PuncLBrk)) {
          if (already_parsed_attributes && already_parsed_captures) {
            Log << SyntaxError << current() << "Unexpected '[' after function attributes and captures";
          } else if (already_parsed_attributes && !already_parsed_captures) {
            state = State::CaptureSection;
          } else if (!already_parsed_attributes && already_parsed_captures) {
            state = State::AttributesSection;
          } else {
            qcore_assert(!already_parsed_attributes && !already_parsed_captures);

            auto tok = peek();

            /* No attribute expression may begin with '&' */
            if (tok.Is<OpBitAnd>()) {
              state = State::CaptureSection;
            } else if (tok.Is(Name)) {
              state = State::CaptureSection;
            } else { /* Ambiguous edge case */
              state = State::AttributesSection;
            }
          }
        } else if (auto tok = peek(); tok.Is<PuncLPar>() || tok.Is<OpLT>()) {
          state = State::End; /* Begin parsing parameters or template options */
        } else {
          Log << SyntaxError << next() << "Unexpected token in function declaration";
        }

        break;
      }

      case State::AttributesSection: {
        already_parsed_attributes = true;

        while (true) {
          if (NextIf(EofF)) [[unlikely]] {
            Log << SyntaxError << current() << "Unexpected EOF in function attributes";
            break;
          }

          if (NextIf(PuncRBrk)) {
            state = State::Ground;
            break;
          }

          auto attribute = RecurseExpr({
              Token(Punc, PuncComa),
              Token(Punc, PuncRBrk),
          });

          attributes.push_back(attribute);

          NextIf(PuncComa);
        }

        break;
      }

      case State::CaptureSection: {
        already_parsed_captures = true;

        while (true) {
          if (NextIf(EofF)) [[unlikely]] {
            Log << SyntaxError << current() << "Unexpected EOF in function captures";
            break;
          }

          if (NextIf(PuncRBrk)) {
            state = State::Ground;
            break;
          }

          if (auto capture = RecurseFunctionCapture()) {
            captures.emplace_back(capture->first, capture->second);
          }

          NextIf(PuncComa);
        }

        break;
      }

      case State::End: {
        break;
      }
    }
  }

  auto purity = GetPuritySpecifier(start_pos, is_thread_safe, is_pure, is_impure, is_quasi, is_retro);

  return {attributes, captures, purity, function_name};
}

auto Parser::PImpl::RecurseFunctionReturnType() -> FlowPtr<parse::Type> {
  if (NextIf(PuncColn)) {
    return RecurseType();
  }

  return CreateNode<InferTy>()();
}

auto Parser::PImpl::RecurseFunctionBody(bool parse_declaration_only) -> NullableFlowPtr<Stmt> {
  if (parse_declaration_only || NextIf(PuncSemi)) {
    return std::nullopt;
  }

  if (NextIf(OpArrow)) {
    return RecurseBlock(false, true, SafetyMode::Unknown);
  }

  return RecurseBlock(true, false, SafetyMode::Unknown);
}

auto Parser::PImpl::RecurseFunction(bool parse_declaration_only) -> FlowPtr<Stmt> {
  auto start_pos = current().GetStart();

  auto [function_attributes, function_captures, function_purity, function_name] = RecurseFunctionAmbigouis();
  auto function_template_parameters = RecurseTemplateParameters();
  auto function_parameters = RecurseFunctionParameters();
  auto function_return_type = RecurseFunctionReturnType();
  auto function_body = RecurseFunctionBody(parse_declaration_only);

  auto function =
      CreateNode<Function>(function_attributes, function_purity, function_captures, function_name,
                           function_template_parameters, function_parameters.first, function_parameters.second,
                           function_return_type, std::nullopt, std::nullopt, function_body)();
  function->SetOffset(start_pos);

  return function;
}
