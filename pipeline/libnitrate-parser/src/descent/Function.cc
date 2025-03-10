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

auto GeneralParser::PImpl::RecurseFunctionParameterType() -> FlowPtr<parse::Type> {
  if (NextIf<PuncColn>()) {
    return RecurseType();
  }

  return m_fac.CreateUnknownType();
}

auto GeneralParser::PImpl::RecurseFunctionParameterValue() -> NullableFlowPtr<Expr> {
  if (NextIf<OpSet>()) {
    return RecurseExpr({
        Token(Punc, PuncComa),
        Token(Punc, PuncRPar),
        Token(Oper, OpGT),
    });
  }

  return std::nullopt;
}

auto GeneralParser::PImpl::RecurseFunctionParameter() -> std::optional<FuncParam> {
  if (auto param_name = RecurseName()) [[likely]] {
    auto param_type = RecurseFunctionParameterType();
    auto param_value = RecurseFunctionParameterValue();

    return FuncParam{param_name, param_type, param_value};
  }

  Log << SyntaxError << Next() << "Expected a parameter name before ':'";

  return std::nullopt;
}

auto GeneralParser::PImpl::RecurseTemplateParameters() -> std::optional<std::vector<TemplateParameter>> {
  if (!NextIf<OpLT>()) {
    return std::nullopt;
  }

  std::vector<TemplateParameter> params;

  while (true) {
    if (m_rd.IsEof()) [[unlikely]] {
      Log << SyntaxError << Current() << "Unexpected EOF in template parameters";
      return params;
    }

    if (NextIf<OpGT>()) {
      break;
    }

    if (auto param_opt = RecurseFunctionParameter()) {
      auto [param_name, param_type, param_value] = param_opt.value();

      params.emplace_back(param_name, param_type, param_value);
    } else {
      Log << SyntaxError << Next() << "Expected a template parameter";
    }

    NextIf<PuncComa>();
  }

  return params;
}

auto GeneralParser::PImpl::RecurseFunctionParameters()
    -> std::pair<std::vector<ASTFactory::FactoryFunctionParameter>, bool> {
  std::pair<std::vector<ASTFactory::FactoryFunctionParameter>, bool> parameters;

  if (!NextIf<PuncLPar>()) [[unlikely]] {
    Log << SyntaxError << Current() << "Expected '(' after function name";

    return parameters;
  }

  bool is_variadic = false;

  while (true) {
    if (m_rd.IsEof()) [[unlikely]] {
      Log << SyntaxError << Current() << "Unexpected EOF in function parameters";

      return parameters;
    }

    if (NextIf<PuncRPar>()) {
      break;
    }

    if (NextIf<OpEllipsis>()) {
      is_variadic = true;

      if (!Peek().Is<PuncRPar>()) {
        Log << SyntaxError << Current() << "Expected ')' after variadic parameter";
      }
      continue;
    }

    if (auto parameter = RecurseFunctionParameter()) {
      auto [param_name, param_type, param_value] = parameter.value();
      parameters.first.emplace_back(param_name, param_type, param_value);
    } else {
      Log << SyntaxError << Next() << "Expected a function parameter";
    }

    NextIf<PuncComa>();
  }

  parameters.second = is_variadic;

  return parameters;
}

auto GeneralParser::PImpl::RecurseFunctionCapture() -> std::optional<std::pair<string, bool>> {
  bool is_ref = NextIf<OpBitAnd>().has_value();

  if (auto name = RecurseName()) {
    return {{name, is_ref}};
  }

  Log << SyntaxError << Next() << "Expected a capture name";

  return std::nullopt;
}

auto GeneralParser::PImpl::RecurseFunctionAmbigouis()
    -> std::tuple<std::vector<FlowPtr<Expr>>, std::vector<std::pair<string, bool>>, string> {
  enum class State : uint8_t {
    Ground,
    AttributesSection,
    CaptureSection,
    End,
  } state = State::Ground;

  std::vector<FlowPtr<Expr>> attributes;
  std::vector<std::pair<string, bool>> captures;
  string function_name;
  bool already_parsed_attributes = false;
  bool already_parsed_captures = false;

  while (state != State::End) {
    if (m_rd.IsEof()) [[unlikely]] {
      Log << SyntaxError << Current() << "Unexpected EOF in function attributes";
      break;
    }

    switch (state) {
      case State::Ground: {
        if (auto some_word = RecurseName()) {
          if (some_word == "pure" || some_word == "impure" || some_word == "tsafe" || some_word == "quasi" ||
              some_word == "retro" || some_word == "inline" || some_word == "foreign") {
            attributes.push_back(m_fac.CreateIdentifier(some_word));
          } else {
            function_name = some_word;
            state = State::End;
          }
        } else if (NextIf<PuncLBrk>()) {
          if (already_parsed_attributes && already_parsed_captures) {
            Log << SyntaxError << Current() << "Unexpected '[' after function attributes and captures";
          } else if (already_parsed_attributes && !already_parsed_captures) {
            state = State::CaptureSection;
          } else if (!already_parsed_attributes && already_parsed_captures) {
            state = State::AttributesSection;
          } else {
            qcore_assert(!already_parsed_attributes && !already_parsed_captures);

            auto tok = Peek();

            /* No attribute expression may begin with '&' */
            if (tok.Is<OpBitAnd>()) {
              state = State::CaptureSection;
            } else if (tok.Is(Name)) {
              state = State::CaptureSection;
            } else { /* Ambiguous edge case */
              state = State::AttributesSection;
            }
          }
        } else if (auto tok = Peek(); tok.Is<PuncLPar>() || tok.Is<OpLT>()) {
          state = State::End; /* Begin parsing parameters or template options */
        } else {
          Log << SyntaxError << Next() << "Unexpected token in function declaration";
        }

        break;
      }

      case State::AttributesSection: {
        already_parsed_attributes = true;

        while (true) {
          if (m_rd.IsEof()) [[unlikely]] {
            Log << SyntaxError << Current() << "Unexpected EOF in function attributes";
            break;
          }

          if (NextIf<PuncRBrk>()) {
            state = State::Ground;
            break;
          }

          auto attribute = RecurseExpr({
              Token(Punc, PuncComa),
              Token(Punc, PuncRBrk),
          });

          attributes.push_back(attribute);

          NextIf<PuncComa>();
        }

        break;
      }

      case State::CaptureSection: {
        already_parsed_captures = true;

        while (true) {
          if (m_rd.IsEof()) [[unlikely]] {
            Log << SyntaxError << Current() << "Unexpected EOF in function captures";
            break;
          }

          if (NextIf<PuncRBrk>()) {
            state = State::Ground;
            break;
          }

          if (auto capture = RecurseFunctionCapture()) {
            captures.emplace_back(capture->first, capture->second);
          }

          NextIf<PuncComa>();
        }

        break;
      }

      case State::End: {
        break;
      }
    }
  }

  return {attributes, captures, function_name};
}

auto GeneralParser::PImpl::RecurseFunctionReturnType() -> FlowPtr<parse::Type> {
  if (NextIf<PuncColn>()) {
    return RecurseType();
  }

  return m_fac.CreateUnknownType();
}

auto GeneralParser::PImpl::RecurseFunctionBody(bool parse_declaration_only) -> NullableFlowPtr<Expr> {
  if (parse_declaration_only || NextIf<PuncSemi>()) {
    return std::nullopt;
  }

  if (NextIf<OpArrow>()) {
    return RecurseBlock(false, true, BlockMode::Unknown);
  }

  return RecurseBlock(true, false, BlockMode::Unknown);
}

auto GeneralParser::PImpl::RecurseFunction(bool parse_declaration_only) -> FlowPtr<Expr> {
  auto start_pos = Current().GetStart();

  auto [function_attributes, function_captures, function_name] = RecurseFunctionAmbigouis();
  auto function_template_parameters = RecurseTemplateParameters();
  auto function_parameters = RecurseFunctionParameters();
  auto function_return_type = RecurseFunctionReturnType();
  auto function_body = RecurseFunctionBody(parse_declaration_only);

  auto function = m_fac.CreateFunction(function_name, function_return_type, function_parameters.first,
                                       function_parameters.second, function_body, function_attributes, std::nullopt,
                                       std::nullopt, function_captures, function_template_parameters);
  if (!function.has_value()) [[unlikely]] {
    function = m_fac.CreateMockInstance<Function>();
  }

  function.value()->SetOffset(start_pos);

  return function.value();
}
