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

using namespace ncc::parse;

auto ASTFactory::CreateTypedef(string name, FlowPtr<Type> base, SourceLocation origin) -> FlowPtr<Typedef> {
  return CreateInstance<Typedef>(name, base)(m_pool, origin);
}

auto ASTFactory::CreateStruct(SourceLocation origin) -> FlowPtr<Struct> {
  /// TODO: Implement
  qcore_implement();
}

auto ASTFactory::CreateEnum(string name, NullableFlowPtr<Type> ele_ty, const std::vector<FactoryEnumItem>& ele,
                            SourceLocation origin) -> FlowPtr<Enum> {
  auto ele_copy = AllocateArray<EnumItem>(ele.size());
  for (size_t i = 0; i < ele.size(); ++i) {
    ele_copy[i] = {ele[i].m_name, ele[i].m_value};
  }

  return CreateInstance<Enum>(name, ele_ty, ele_copy)(m_pool, origin);
}

auto ASTFactory::CreateFunction(string name, NullableFlowPtr<Type> ret_ty,
                                const std::vector<FactoryFunctionParameter>& params, bool variadic,
                                NullableFlowPtr<Expr> body, Purity purity, const std::vector<FlowPtr<Expr>>& attributes,
                                NullableFlowPtr<Expr> precond, NullableFlowPtr<Expr> postcond,
                                const std::vector<std::pair<string, bool>>& captures,
                                const std::optional<std::span<TemplateParameter>>& template_parameters,
                                SourceLocation origin) -> std::optional<FlowPtr<Function>> {
  /// TODO: Implement
  qcore_implement();
}

auto ASTFactory::CreateAnonymousFunction(Purity purity, const std::vector<std::pair<string, bool>>& captures,
                                         NullableFlowPtr<Type> ret_ty,
                                         const std::vector<FactoryFunctionParameter>& params, bool variadic,
                                         NullableFlowPtr<Expr> body, const std::vector<FlowPtr<Expr>>& attributes,
                                         NullableFlowPtr<Expr> precond, NullableFlowPtr<Expr> postcond,
                                         SourceLocation origin) -> std::optional<FlowPtr<Function>> {
  /// TODO: Implement
  qcore_implement();
}

auto ASTFactory::CreateScope(string name, FlowPtr<Expr> body, const std::vector<string>& tags,
                             SourceLocation origin) -> FlowPtr<Scope> {
  auto tags_copy = AllocateArray<string>(tags.size());
  std::copy(tags.begin(), tags.end(), tags_copy.begin());

  return CreateInstance<Scope>(name, body, tags_copy)(m_pool, origin);
}

auto ASTFactory::CreateExport(FlowPtr<Expr> symbol, Vis vis, string abi, const std::vector<FlowPtr<Expr>>& attributes,
                              SourceLocation origin) -> FlowPtr<Export> {
  auto attributes_copy = AllocateArray<FlowPtr<Expr>>(attributes.size());
  std::copy(attributes.begin(), attributes.end(), attributes_copy.begin());

  return CreateInstance<Export>(symbol, abi, vis, attributes_copy)(m_pool, origin);
}

auto ASTFactory::CreateBlock(std::span<const FlowPtr<Expr>> items, SafetyMode safety,
                             SourceLocation origin) -> FlowPtr<Block> {
  auto items_copy = AllocateArray<FlowPtr<Expr>>(items.size());
  std::copy(items.begin(), items.end(), items_copy.begin());

  return CreateInstance<Block>(items_copy, safety)(m_pool, origin);
}

auto ASTFactory::CreateBlock(const std::vector<FlowPtr<Expr>>& items, SafetyMode safety,
                             SourceLocation origin) -> FlowPtr<Block> {
  return CreateBlock(std::span(items.data(), items.size()), safety, origin);
}

auto ASTFactory::CreateVariable(VariableType variant, string name, NullableFlowPtr<Type> type,
                                NullableFlowPtr<Expr> init, const std::vector<FlowPtr<Expr>>& attributes,
                                SourceLocation origin) -> FlowPtr<Variable> {
  auto attributes_copy = AllocateArray<FlowPtr<Expr>>(attributes.size());
  std::copy(attributes.begin(), attributes.end(), attributes_copy.begin());

  if (!type.has_value()) {
    type = CreateUnknownType(nullptr, nullptr, nullptr, origin);
  }

  return CreateInstance<Variable>(name, type.value(), init, variant, attributes_copy)(m_pool, origin);
}

auto ASTFactory::CreateAssembly(string asm_code, SourceLocation origin) -> FlowPtr<Assembly> {
  return CreateInstance<Assembly>(asm_code, std::span<FlowPtr<Expr>>())(m_pool, origin);
}

auto ASTFactory::CreateReturn(NullableFlowPtr<Expr> value, SourceLocation origin) -> FlowPtr<Return> {
  return CreateInstance<Return>(value)(m_pool, origin);
}

auto ASTFactory::CreateReturnIf(FlowPtr<Expr> cond, NullableFlowPtr<Expr> value,
                                SourceLocation origin) -> FlowPtr<ReturnIf> {
  return CreateInstance<ReturnIf>(cond, value)(m_pool, origin);
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

auto ASTFactory::CreateFor(NullableFlowPtr<Expr> init, NullableFlowPtr<Expr> step, NullableFlowPtr<Expr> cond,
                           FlowPtr<Expr> body, SourceLocation origin) -> FlowPtr<For> {
  return CreateInstance<For>(init, step, cond, body)(m_pool, origin);
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
  auto cases_copy = AllocateArray<FlowPtr<Case>>(cases.size());
  std::copy(cases.begin(), cases.end(), cases_copy.begin());

  return CreateInstance<Switch>(match, cases_copy, defaul)(m_pool, origin);
}
