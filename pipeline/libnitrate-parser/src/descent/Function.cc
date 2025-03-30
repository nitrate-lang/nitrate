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
#include <nitrate-parser/ASTExpr.hh>
#include <nitrate-parser/ASTStmt.hh>
#include <nitrate-parser/ASTType.hh>
#include <unordered_set>

using namespace ncc;
using namespace ncc::lex;
using namespace ncc::parse;

auto GeneralParser::Context::RecurseFunctionParameterType() -> FlowPtr<parse::Type> {
  if (NextIf<PuncColn>()) {
    return RecurseType();
  }

  return CreateUnknownType();
}

auto GeneralParser::Context::RecurseFunctionParameterValue() -> NullableFlowPtr<Expr> {
  if (NextIf<OpSet>()) {
    return RecurseExpr({
        Token(Punc, PuncComa),
        Token(Punc, PuncRPar),
        Token(Oper, OpGT),
    });
  }

  return std::nullopt;
}

auto GeneralParser::Context::RecurseFunctionParameter() -> std::optional<FuncParam> {
  if (auto param_name = RecurseName()) [[likely]] {
    auto param_type = RecurseFunctionParameterType();
    auto param_value = RecurseFunctionParameterValue();

    return FuncParam{param_name, param_type, param_value};
  }

  Log << ParserSignal << Next() << "Expected a parameter name before ':'";

  return std::nullopt;
}

auto GeneralParser::Context::RecurseTemplateParameters() -> std::optional<std::vector<TemplateParameter>> {
  if (!NextIf<OpLT>()) {
    return std::nullopt;
  }

  std::vector<TemplateParameter> params;

  while (true) {
    if (m_rd.IsEof()) [[unlikely]] {
      Log << ParserSignal << Current() << "Unexpected EOF in template parameters";
      return params;
    }

    if (NextIf<OpGT>()) {
      break;
    }

    if (auto param_opt = RecurseFunctionParameter()) {
      auto [param_name, param_type, param_value] = param_opt.value();

      params.emplace_back(param_name, param_type, param_value);
    } else {
      Log << ParserSignal << Next() << "Expected a template parameter";
    }

    NextIf<PuncComa>();
  }

  return params;
}

auto GeneralParser::Context::RecurseFunctionParameters()
    -> std::pair<std::vector<ASTFactory::FactoryFunctionParameter>, bool> {
  std::pair<std::vector<ASTFactory::FactoryFunctionParameter>, bool> parameters;

  if (!NextIf<PuncLPar>()) [[unlikely]] {
    Log << ParserSignal << Current() << "Expected '(' after function name";

    return parameters;
  }

  bool is_variadic = false;

  while (true) {
    if (m_rd.IsEof()) [[unlikely]] {
      Log << ParserSignal << Current() << "Unexpected EOF in function parameters";

      return parameters;
    }

    if (NextIf<PuncRPar>()) {
      break;
    }

    if (NextIf<OpEllipsis>()) {
      is_variadic = true;

      if (!Peek().Is<PuncRPar>()) {
        Log << ParserSignal << Current() << "Expected ')' after variadic parameter";
      }
      continue;
    }

    if (auto parameter = RecurseFunctionParameter()) {
      auto [param_name, param_type, param_value] = parameter.value();
      parameters.first.emplace_back(param_name, param_type, param_value);
    } else {
      Log << ParserSignal << Next() << "Expected a function parameter";
    }

    NextIf<PuncComa>();
  }

  parameters.second = is_variadic;

  return parameters;
}

auto GeneralParser::Context::RecurseFunctionAttributes() -> std::vector<FlowPtr<Expr>> {
  static const std::unordered_set<Keyword> reserved_words = {
      Pure, Impure, Quasi, Retro, Inline, Foreign, Safe, Unsafe,
  };

  std::vector<FlowPtr<Expr>> attributes;

  while (true) {  // Parse some free-standing attributes
    auto tok = Peek();
    if (!tok.Is(KeyW)) {
      break;
    }

    if (!reserved_words.contains(tok.GetKeyword())) {
      break;
    }

    Next();
    attributes.emplace_back(CreateIdentifier(tok.AsString()));
  }

  if (NextIf<PuncLBrk>()) {
    while (true) {
      if (m_rd.IsEof()) [[unlikely]] {
        Log << ParserSignal << Current() << "Unexpected EOF in function attributes";
        return attributes;
      }

      if (NextIf<PuncRBrk>()) {
        break;
      }

      if (auto tok = Peek(); tok.Is(KeyW) && reserved_words.contains(tok.GetKeyword())) {
        Next();
        attributes.emplace_back(CreateIdentifier(tok.AsString()));
      } else {
        auto expr = RecurseExpr({Token(Punc, PuncComa), Token(Punc, PuncRBrk)});
        attributes.emplace_back(expr);
      }

      NextIf<PuncComa>();
    }
  }

  while (true) {  // Parse some free-standing attributes
    auto tok = Peek();
    if (!tok.Is(KeyW)) {
      break;
    }

    if (!reserved_words.contains(tok.GetKeyword())) {
      break;
    }

    Next();

    attributes.emplace_back(CreateIdentifier(tok.AsString()));
  }

  return attributes;
}

auto GeneralParser::Context::RecurseFunctionReturnType() -> FlowPtr<parse::Type> {
  if (NextIf<PuncColn>()) {
    return RecurseType();
  }

  return CreateUnknownType();
}

auto GeneralParser::Context::RecurseFunctionBody(bool parse_declaration_only) -> NullableFlowPtr<Expr> {
  if (parse_declaration_only || NextIf<PuncSemi>()) {
    return std::nullopt;
  }

  if (NextIf<OpArrow>()) {
    return RecurseBlock(false, true, BlockMode::Unknown);
  }

  return RecurseBlock(true, false, BlockMode::Unknown);
}

auto GeneralParser::Context::RecurseFunction(bool parse_declaration_only) -> FlowPtr<Function> {
  auto start_pos = Current().GetStart();

  auto function_attributes = RecurseFunctionAttributes();
  auto function_name = RecurseName();
  auto function_template_parameters = RecurseTemplateParameters();
  auto function_parameters = RecurseFunctionParameters();
  auto function_return_type = RecurseFunctionReturnType();
  auto function_body = RecurseFunctionBody(parse_declaration_only);

  auto function =
      CreateFunction(function_name, function_return_type, function_parameters.first, function_parameters.second,
                     function_body, function_attributes, function_template_parameters);
  if (!function.has_value()) [[unlikely]] {
    function = CreateMockInstance<Function>();
  }

  function.value()->SetOffset(start_pos);

  return function.value();
}
