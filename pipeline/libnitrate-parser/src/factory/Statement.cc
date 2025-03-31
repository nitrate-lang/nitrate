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

#include <nitrate-parser/ASTFactory.hh>
#include <nitrate-parser/ASTStmt.hh>
#include <nitrate-parser/ASTType.hh>
#include <variant>

using namespace ncc::parse;

auto ASTFactory::CreateTypedef(string name, FlowPtr<Type> base, SourceLocation origin) -> FlowPtr<Typedef> {
  return CreateInstance<Typedef>(name, base)(m_pool, origin);
}

auto ASTFactory::CreateStruct(CompositeType comp_type, string name,
                              const std::optional<std::vector<TemplateParameter>>& tparams,
                              const std::vector<StructField>& fields, const std::vector<StructFunction>& methods,
                              const std::vector<string>& constraints, const std::vector<FlowPtr<Expr>>& attributes,
                              SourceLocation origin) -> FlowPtr<Struct> {
  auto fields_copy = AllocateArray<StructField>(fields.size());
  std::copy(fields.begin(), fields.end(), fields_copy.begin());

  auto methods_copy = AllocateArray<StructFunction>(methods.size());
  std::copy(methods.begin(), methods.end(), methods_copy.begin());

  auto attributes_copy = AllocateArray<FlowPtr<Expr>>(attributes.size());
  std::copy(attributes.begin(), attributes.end(), attributes_copy.begin());

  auto constraints_copy = AllocateArray<string>(constraints.size());
  std::copy(constraints.begin(), constraints.end(), constraints_copy.begin());

  auto tparams_copy = tparams ? std::make_optional(AllocateArray<TemplateParameter>(tparams->size())) : std::nullopt;
  if (tparams) {
    std::copy(tparams->begin(), tparams->end(), tparams_copy->begin());
  }

  return CreateInstance<Struct>(comp_type, attributes_copy, name, tparams_copy, constraints_copy, fields_copy,
                                methods_copy)(m_pool, origin);
}

auto ASTFactory::CreateEnum(string name, const std::vector<FactoryEnumItem>& ele, NullableFlowPtr<Type> ele_ty,
                            SourceLocation origin) -> FlowPtr<Enum> {
  auto ele_copy = AllocateArray<std::pair<string, NullableFlowPtr<Expr>>>(ele.size());
  for (size_t i = 0; i < ele.size(); ++i) {
    ele_copy[i] = {ele[i].m_name, ele[i].m_value};
  }

  return CreateInstance<Enum>(name, ele_ty, ele_copy)(m_pool, origin);
}

auto ASTFactory::CreateEnum(string name, std::span<const std::pair<string, NullableFlowPtr<Expr>>> ele,
                            NullableFlowPtr<Type> ele_ty, SourceLocation origin) -> FlowPtr<Enum> {
  auto ele_copy = AllocateArray<std::pair<string, NullableFlowPtr<Expr>>>(ele.size());
  for (size_t i = 0; i < ele.size(); ++i) {
    ele_copy[i] = {ele[i].first, ele[i].second};
  }

  return CreateInstance<Enum>(name, ele_ty, ele_copy)(m_pool, origin);
}

auto ASTFactory::CreateFunction(string name, NullableFlowPtr<Type> ret_ty,
                                const std::vector<FactoryFunctionParameter>& params, bool variadic,
                                NullableFlowPtr<Expr> body, const std::vector<FlowPtr<Expr>>& attributes,
                                const std::optional<std::vector<TemplateParameter>>& template_parameters,
                                SourceLocation origin) -> std::optional<FlowPtr<Function>> {
  auto params_copy = AllocateArray<std::tuple<string, FlowPtr<Type>, NullableFlowPtr<Expr>>>(params.size());
  for (size_t i = 0; i < params.size(); ++i) {
    auto the_name = std::holds_alternative<string>(params[i].m_name)
                        ? std::get<string>(params[i].m_name)
                        : string(std::to_string(std::get<size_t>(params[i].m_name)));

    params_copy[i] = {the_name, params[i].m_type, params[i].m_default_value};
  }

  auto attributes_copy = AllocateArray<FlowPtr<Expr>>(attributes.size());
  std::copy(attributes.begin(), attributes.end(), attributes_copy.begin());

  if (!ret_ty.has_value()) {
    ret_ty = CreateInferredType(nullptr, nullptr, nullptr, origin);
  }

  auto template_parameters_copy =
      template_parameters ? std::make_optional(AllocateArray<TemplateParameter>(template_parameters->size()))
                          : std::nullopt;
  if (template_parameters) {
    std::copy(template_parameters->begin(), template_parameters->end(), template_parameters_copy->begin());
  }

  return CreateInstance<Function>(attributes_copy, name, template_parameters_copy, params_copy, variadic,
                                  ret_ty.value(), body)(m_pool, origin);
}

auto ASTFactory::CreateAnonymousFunction(NullableFlowPtr<Type> ret_ty,
                                         const std::vector<FactoryFunctionParameter>& params, bool variadic,
                                         NullableFlowPtr<Expr> body, const std::vector<FlowPtr<Expr>>& attributes,
                                         SourceLocation origin) -> std::optional<FlowPtr<Function>> {
  return CreateFunction("", std::move(ret_ty), params, variadic, std::move(body), attributes, std::nullopt, origin);
}

auto ASTFactory::CreateScope(string name, FlowPtr<Expr> body, const std::vector<string>& tags,
                             SourceLocation origin) -> FlowPtr<Scope> {
  return CreateScope(name, std::move(body), std::span(tags), origin);
}

auto ASTFactory::CreateScope(string name, FlowPtr<Expr> body, std::span<const string> tags,
                             SourceLocation origin) -> FlowPtr<Scope> {
  auto tags_copy = AllocateArray<string>(tags.size());
  std::copy(tags.begin(), tags.end(), tags_copy.begin());

  return CreateInstance<Scope>(name, body, tags_copy)(m_pool, origin);
}

auto ASTFactory::CreateExport(FlowPtr<Block> symbol, const std::vector<FlowPtr<Expr>>& attributes, Vis vis, string abi,
                              SourceLocation origin) -> FlowPtr<Export> {
  return CreateExport(std::move(symbol), std::span(attributes), vis, abi, origin);
}

auto ASTFactory::CreateExport(FlowPtr<Block> symbol, std::span<const FlowPtr<Expr>> attributes, Vis vis, string abi,
                              SourceLocation origin) -> FlowPtr<Export> {
  auto attributes_copy = AllocateArray<FlowPtr<Expr>>(attributes.size());
  std::copy(attributes.begin(), attributes.end(), attributes_copy.begin());

  return CreateInstance<Export>(symbol, abi, vis, attributes_copy)(m_pool, origin);
}

auto ASTFactory::CreateBlock(std::span<const FlowPtr<Expr>> items, BlockMode safety,
                             SourceLocation origin) -> FlowPtr<Block> {
  auto items_copy = AllocateArray<FlowPtr<Expr>>(items.size());
  std::copy(items.begin(), items.end(), items_copy.begin());

  return CreateInstance<Block>(items_copy, safety)(m_pool, origin);
}

auto ASTFactory::CreateBlock(const std::vector<FlowPtr<Expr>>& items, BlockMode safety,
                             SourceLocation origin) -> FlowPtr<Block> {
  return CreateBlock(std::span(items.data(), items.size()), safety, origin);
}

auto ASTFactory::CreateVariable(VariableType variant, string name, const std::vector<FlowPtr<Expr>>& attributes,
                                NullableFlowPtr<Type> type, NullableFlowPtr<Expr> init,
                                SourceLocation origin) -> FlowPtr<Variable> {
  return CreateVariable(variant, name, std::span(attributes), std::move(type), std::move(init), origin);
}

auto ASTFactory::CreateVariable(VariableType variant, string name, std::span<const FlowPtr<Expr>> attributes,
                                NullableFlowPtr<Type> type, NullableFlowPtr<Expr> init,
                                SourceLocation origin) -> FlowPtr<Variable> {
  auto attributes_copy = AllocateArray<FlowPtr<Expr>>(attributes.size());
  std::copy(attributes.begin(), attributes.end(), attributes_copy.begin());

  if (!type.has_value()) {
    type = CreateInferredType(nullptr, nullptr, nullptr, origin);
  }

  return CreateInstance<Variable>(name, type.value(), init, variant, attributes_copy)(m_pool, origin);
}

auto ASTFactory::CreateAssembly(string asm_code, SourceLocation origin) -> FlowPtr<Assembly> {
  return CreateInstance<Assembly>(asm_code, std::span<FlowPtr<Expr>>())(m_pool, origin);
}

auto ASTFactory::CreateReturn(NullableFlowPtr<Expr> value, SourceLocation origin) -> FlowPtr<Return> {
  return CreateInstance<Return>(value)(m_pool, origin);
}

auto ASTFactory::CreateBreak(SourceLocation origin) -> FlowPtr<Break> {
  return CreateInstance<Break>()(m_pool, origin);
}

auto ASTFactory::CreateContinue(SourceLocation origin) -> FlowPtr<Continue> {
  return CreateInstance<Continue>()(m_pool, origin);
}

auto ASTFactory::CreateIf(FlowPtr<Expr> cond, FlowPtr<Expr> then, NullableFlowPtr<Expr> ele,
                          SourceLocation origin) -> FlowPtr<If> {
  return CreateInstance<If>(cond, then, ele)(m_pool, origin);
}

auto ASTFactory::CreateWhile(FlowPtr<Expr> cond, FlowPtr<Expr> body, SourceLocation origin) -> FlowPtr<While> {
  return CreateInstance<While>(cond, body)(m_pool, origin);
}

auto ASTFactory::CreateFor(NullableFlowPtr<Expr> init, NullableFlowPtr<Expr> cond, NullableFlowPtr<Expr> step,
                           FlowPtr<Expr> body, SourceLocation origin) -> FlowPtr<For> {
  return CreateInstance<For>(init, cond, step, body)(m_pool, origin);
}

auto ASTFactory::CreateForeach(string key_name, string val_name, FlowPtr<Expr> iterable, FlowPtr<Expr> body,
                               SourceLocation origin) -> FlowPtr<Foreach> {
  return CreateInstance<Foreach>(key_name, val_name, iterable, body)(m_pool, origin);
}

auto ASTFactory::CreateCase(FlowPtr<Expr> match, FlowPtr<Expr> body, SourceLocation origin) -> FlowPtr<Case> {
  return CreateInstance<Case>(match, body)(m_pool, origin);
}

auto ASTFactory::CreateSwitch(FlowPtr<Expr> match, NullableFlowPtr<Expr> defaul,
                              const std::vector<FlowPtr<Case>>& cases, SourceLocation origin) -> FlowPtr<Switch> {
  return CreateSwitch(std::move(match), std::move(defaul), std::span(cases), origin);
}

auto ASTFactory::CreateSwitch(FlowPtr<Expr> match, NullableFlowPtr<Expr> defaul, std::span<const FlowPtr<Case>> cases,
                              SourceLocation origin) -> FlowPtr<Switch> {
  auto cases_copy = AllocateArray<FlowPtr<Case>>(cases.size());
  std::copy(cases.begin(), cases.end(), cases_copy.begin());

  return CreateInstance<Switch>(match, cases_copy, defaul)(m_pool, origin);
}
