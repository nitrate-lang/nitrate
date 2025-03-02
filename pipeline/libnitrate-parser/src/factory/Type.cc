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
#include <nitrate-parser/ASTType.hh>
#include <unordered_set>
#include <variant>
#include <vector>

using namespace ncc::parse;

auto ASTFactory::CreateReference(FlowPtr<Type> to, bool volatil, NullableFlowPtr<Expr> bits, NullableFlowPtr<Expr> min,
                                 NullableFlowPtr<Expr> max, SourceLocation origin) -> FlowPtr<RefTy> {
  auto node = CreateInstance<RefTy>(to, volatil)(m_pool, origin);

  node->SetWidth(std::move(bits));
  node->SetRangeBegin(std::move(min));
  node->SetRangeEnd(std::move(max));

  return node;
}

auto ASTFactory::CreateU1(NullableFlowPtr<Expr> bits, NullableFlowPtr<Expr> min, NullableFlowPtr<Expr> max,
                          SourceLocation origin) -> FlowPtr<U1> {
  auto node = CreateInstance<U1>()(m_pool, origin);

  node->SetWidth(std::move(bits));
  node->SetRangeBegin(std::move(min));
  node->SetRangeEnd(std::move(max));

  return node;
}

auto ASTFactory::CreateU8(NullableFlowPtr<Expr> bits, NullableFlowPtr<Expr> min, NullableFlowPtr<Expr> max,
                          SourceLocation origin) -> FlowPtr<U8> {
  auto node = CreateInstance<U8>()(m_pool, origin);

  node->SetWidth(std::move(bits));
  node->SetRangeBegin(std::move(min));
  node->SetRangeEnd(std::move(max));

  return node;
}

auto ASTFactory::CreateU16(NullableFlowPtr<Expr> bits, NullableFlowPtr<Expr> min, NullableFlowPtr<Expr> max,
                           SourceLocation origin) -> FlowPtr<U16> {
  auto node = CreateInstance<U16>()(m_pool, origin);

  node->SetWidth(std::move(bits));
  node->SetRangeBegin(std::move(min));
  node->SetRangeEnd(std::move(max));

  return node;
}

auto ASTFactory::CreateU32(NullableFlowPtr<Expr> bits, NullableFlowPtr<Expr> min, NullableFlowPtr<Expr> max,
                           SourceLocation origin) -> FlowPtr<U32> {
  auto node = CreateInstance<U32>()(m_pool, origin);

  node->SetWidth(std::move(bits));
  node->SetRangeBegin(std::move(min));
  node->SetRangeEnd(std::move(max));

  return node;
}

auto ASTFactory::CreateU64(NullableFlowPtr<Expr> bits, NullableFlowPtr<Expr> min, NullableFlowPtr<Expr> max,
                           SourceLocation origin) -> FlowPtr<U64> {
  auto node = CreateInstance<U64>()(m_pool, origin);

  node->SetWidth(std::move(bits));
  node->SetRangeBegin(std::move(min));
  node->SetRangeEnd(std::move(max));

  return node;
}

auto ASTFactory::CreateU128(NullableFlowPtr<Expr> bits, NullableFlowPtr<Expr> min, NullableFlowPtr<Expr> max,
                            SourceLocation origin) -> FlowPtr<U128> {
  auto node = CreateInstance<U128>()(m_pool, origin);

  node->SetWidth(std::move(bits));
  node->SetRangeBegin(std::move(min));
  node->SetRangeEnd(std::move(max));

  return node;
}

auto ASTFactory::CreateI8(NullableFlowPtr<Expr> bits, NullableFlowPtr<Expr> min, NullableFlowPtr<Expr> max,
                          SourceLocation origin) -> FlowPtr<I8> {
  auto node = CreateInstance<I8>()(m_pool, origin);

  node->SetWidth(std::move(bits));
  node->SetRangeBegin(std::move(min));
  node->SetRangeEnd(std::move(max));

  return node;
}

auto ASTFactory::CreateI16(NullableFlowPtr<Expr> bits, NullableFlowPtr<Expr> min, NullableFlowPtr<Expr> max,
                           SourceLocation origin) -> FlowPtr<I16> {
  auto node = CreateInstance<I16>()(m_pool, origin);

  node->SetWidth(std::move(bits));
  node->SetRangeBegin(std::move(min));
  node->SetRangeEnd(std::move(max));

  return node;
}

auto ASTFactory::CreateI32(NullableFlowPtr<Expr> bits, NullableFlowPtr<Expr> min, NullableFlowPtr<Expr> max,
                           SourceLocation origin) -> FlowPtr<I32> {
  auto node = CreateInstance<I32>()(m_pool, origin);

  node->SetWidth(std::move(bits));
  node->SetRangeBegin(std::move(min));
  node->SetRangeEnd(std::move(max));

  return node;
}

auto ASTFactory::CreateI64(NullableFlowPtr<Expr> bits, NullableFlowPtr<Expr> min, NullableFlowPtr<Expr> max,
                           SourceLocation origin) -> FlowPtr<I64> {
  auto node = CreateInstance<I64>()(m_pool, origin);

  node->SetWidth(std::move(bits));
  node->SetRangeBegin(std::move(min));
  node->SetRangeEnd(std::move(max));

  return node;
}

auto ASTFactory::CreateI128(NullableFlowPtr<Expr> bits, NullableFlowPtr<Expr> min, NullableFlowPtr<Expr> max,
                            SourceLocation origin) -> FlowPtr<I128> {
  auto node = CreateInstance<I128>()(m_pool, origin);

  node->SetWidth(std::move(bits));
  node->SetRangeBegin(std::move(min));
  node->SetRangeEnd(std::move(max));

  return node;
}

auto ASTFactory::CreateF16(NullableFlowPtr<Expr> bits, NullableFlowPtr<Expr> min, NullableFlowPtr<Expr> max,
                           SourceLocation origin) -> FlowPtr<F16> {
  auto node = CreateInstance<F16>()(m_pool, origin);

  node->SetWidth(std::move(bits));
  node->SetRangeBegin(std::move(min));
  node->SetRangeEnd(std::move(max));

  return node;
}

auto ASTFactory::CreateF32(NullableFlowPtr<Expr> bits, NullableFlowPtr<Expr> min, NullableFlowPtr<Expr> max,
                           SourceLocation origin) -> FlowPtr<F32> {
  auto node = CreateInstance<F32>()(m_pool, origin);

  node->SetWidth(std::move(bits));
  node->SetRangeBegin(std::move(min));
  node->SetRangeEnd(std::move(max));

  return node;
}

auto ASTFactory::CreateF64(NullableFlowPtr<Expr> bits, NullableFlowPtr<Expr> min, NullableFlowPtr<Expr> max,
                           SourceLocation origin) -> FlowPtr<F64> {
  auto node = CreateInstance<F64>()(m_pool, origin);

  node->SetWidth(std::move(bits));
  node->SetRangeBegin(std::move(min));
  node->SetRangeEnd(std::move(max));

  return node;
}

auto ASTFactory::CreateF128(NullableFlowPtr<Expr> bits, NullableFlowPtr<Expr> min, NullableFlowPtr<Expr> max,
                            SourceLocation origin) -> FlowPtr<F128> {
  auto node = CreateInstance<F128>()(m_pool, origin);

  node->SetWidth(std::move(bits));
  node->SetRangeBegin(std::move(min));
  node->SetRangeEnd(std::move(max));

  return node;
}

auto ASTFactory::CreateVoid(NullableFlowPtr<Expr> bits, NullableFlowPtr<Expr> min, NullableFlowPtr<Expr> max,
                            SourceLocation origin) -> FlowPtr<VoidTy> {
  auto node = CreateInstance<VoidTy>()(m_pool, origin);

  node->SetWidth(std::move(bits));
  node->SetRangeBegin(std::move(min));
  node->SetRangeEnd(std::move(max));

  return node;
}

auto ASTFactory::CreatePointer(FlowPtr<Type> to, bool volatil, NullableFlowPtr<Expr> bits, NullableFlowPtr<Expr> min,
                               NullableFlowPtr<Expr> max, SourceLocation origin) -> FlowPtr<PtrTy> {
  auto node = CreateInstance<PtrTy>(to, volatil)(m_pool, origin);

  node->SetWidth(std::move(bits));
  node->SetRangeBegin(std::move(min));
  node->SetRangeEnd(std::move(max));

  return node;
}

auto ASTFactory::CreateOpaque(string name, NullableFlowPtr<Expr> bits, NullableFlowPtr<Expr> min,
                              NullableFlowPtr<Expr> max, SourceLocation origin) -> FlowPtr<OpaqueTy> {
  auto node = CreateInstance<OpaqueTy>(name)(m_pool, origin);

  node->SetWidth(std::move(bits));
  node->SetRangeBegin(std::move(min));
  node->SetRangeEnd(std::move(max));

  return node;
}

auto ASTFactory::CreateArray(FlowPtr<Type> element_type, FlowPtr<Expr> element_count, NullableFlowPtr<Expr> bits,
                             NullableFlowPtr<Expr> min, NullableFlowPtr<Expr> max,
                             SourceLocation origin) -> FlowPtr<ArrayTy> {
  auto node = CreateInstance<ArrayTy>(element_type, element_count)(m_pool, origin);

  node->SetWidth(std::move(bits));
  node->SetRangeBegin(std::move(min));
  node->SetRangeEnd(std::move(max));

  return node;
}

auto ASTFactory::CreateTuple(std::span<const FlowPtr<Type>> ele, NullableFlowPtr<Expr> bits, NullableFlowPtr<Expr> min,
                             NullableFlowPtr<Expr> max, SourceLocation origin) -> FlowPtr<TupleTy> {
  auto ele_copy = AllocateArray<FlowPtr<Type>>(ele.size());
  std::copy(ele.begin(), ele.end(), ele_copy.begin());

  auto node = CreateInstance<TupleTy>(ele_copy)(m_pool, origin);

  node->SetWidth(std::move(bits));
  node->SetRangeBegin(std::move(min));
  node->SetRangeEnd(std::move(max));

  return node;
}

auto ASTFactory::CreateTuple(const std::vector<FlowPtr<Type>>& ele, NullableFlowPtr<Expr> bits,
                             NullableFlowPtr<Expr> min, NullableFlowPtr<Expr> max,
                             SourceLocation origin) -> FlowPtr<TupleTy> {
  return CreateTuple(std::span(ele), std::move(bits), std::move(min), std::move(max), origin);
}

auto ASTFactory::CreateFunctionType(FlowPtr<Type> ret_ty, const std::vector<FactoryFunctionParameter>& params,
                                    bool variadic, Purity purity, std::vector<FlowPtr<Expr>> attributes,
                                    NullableFlowPtr<Expr> bits, NullableFlowPtr<Expr> min, NullableFlowPtr<Expr> max,
                                    SourceLocation origin) -> std::optional<FlowPtr<FuncTy>> {
  auto params_copy = AllocateArray<FuncParam>(params.size());

  {
    std::unordered_set<string> unique_indices;

    for (size_t i = 0; const auto& [key, value, default_] : params) {
      const string key_string =
          std::holds_alternative<size_t>(key) ? string(std::to_string(std::get<size_t>(key))) : std::get<string>(key);

      // Check for duplicate indices
      if (unique_indices.contains(key_string)) {
        return std::nullopt;
      }

      unique_indices.insert(key_string);

      std::get<0>(params_copy[i]) = key_string;
      std::get<1>(params_copy[i]) = value;
      std::get<2>(params_copy[i]) = default_;

      i++;
    }
  }

  auto attributes_copy = AllocateArray<FlowPtr<Expr>>(attributes.size());
  std::copy(attributes.begin(), attributes.end(), attributes_copy.begin());

  auto node = CreateInstance<FuncTy>(ret_ty, params_copy, variadic, purity, attributes_copy)(m_pool, origin);

  node->SetWidth(std::move(bits));
  node->SetRangeBegin(std::move(min));
  node->SetRangeEnd(std::move(max));

  return node;
}

auto ASTFactory::CreateFunctionType(FlowPtr<Type> ret_ty, std::span<const FuncParam> params, bool variadic,
                                    Purity purity, std::span<const FlowPtr<Expr>> attributes,
                                    NullableFlowPtr<Expr> bits, NullableFlowPtr<Expr> min, NullableFlowPtr<Expr> max,
                                    SourceLocation origin) -> std::optional<FlowPtr<FuncTy>> {
  auto params_copy = AllocateArray<FuncParam>(params.size());

  {
    std::unordered_set<string> unique_indices;

    for (size_t i = 0; const auto& [name, value, default_] : params) {
      if (unique_indices.contains(name)) {
        return std::nullopt;
      }

      unique_indices.insert(name);

      std::get<0>(params_copy[i]) = name;
      std::get<1>(params_copy[i]) = value;
      std::get<2>(params_copy[i]) = default_;

      i++;
    }
  }

  auto attributes_copy = AllocateArray<FlowPtr<Expr>>(attributes.size());
  std::copy(attributes.begin(), attributes.end(), attributes_copy.begin());

  auto node = CreateInstance<FuncTy>(ret_ty, params_copy, variadic, purity, attributes_copy)(m_pool, origin);

  node->SetWidth(std::move(bits));
  node->SetRangeBegin(std::move(min));
  node->SetRangeEnd(std::move(max));

  return node;
}

auto ASTFactory::CreateNamed(string name, NullableFlowPtr<Expr> bits, NullableFlowPtr<Expr> min,
                             NullableFlowPtr<Expr> max, SourceLocation origin) -> FlowPtr<NamedTy> {
  auto node = CreateInstance<NamedTy>(name)(m_pool, origin);

  node->SetWidth(std::move(bits));
  node->SetRangeBegin(std::move(min));
  node->SetRangeEnd(std::move(max));

  return node;
}

auto ASTFactory::CreateUnknownType(NullableFlowPtr<Expr> bits, NullableFlowPtr<Expr> min, NullableFlowPtr<Expr> max,
                                   SourceLocation origin) -> FlowPtr<InferTy> {
  auto node = CreateInstance<InferTy>()(m_pool, origin);

  node->SetWidth(std::move(bits));
  node->SetRangeBegin(std::move(min));
  node->SetRangeEnd(std::move(max));

  return node;
}

auto ASTFactory::CreateTemplateType(FlowPtr<Type> base,
                                    const std::unordered_map<std::variant<string, size_t>, FlowPtr<Expr>>& named_args,
                                    NullableFlowPtr<Expr> bits, NullableFlowPtr<Expr> min, NullableFlowPtr<Expr> max,
                                    SourceLocation origin) -> std::optional<FlowPtr<TemplateType>> {
  auto args_copy = AllocateArray<CallArg>(named_args.size());

  {
    std::unordered_set<string> unique_indices;

    size_t i = 0;

    for (const auto& [key, value] : named_args) {
      const string key_string =
          std::holds_alternative<size_t>(key) ? string(std::to_string(std::get<size_t>(key))) : std::get<string>(key);

      // Check for duplicate indices
      if (unique_indices.contains(key_string)) {
        return std::nullopt;
      }

      unique_indices.insert(key_string);

      args_copy[i].first = key_string;
      args_copy[i].second = value;

      i++;
    }
  }

  auto node = CreateInstance<TemplateType>(base, args_copy)(m_pool, origin);

  node->SetWidth(std::move(bits));
  node->SetRangeBegin(std::move(min));
  node->SetRangeEnd(std::move(max));

  return node;
}

auto ASTFactory::CreateTemplateType(const std::vector<FlowPtr<Expr>>& pos_args, FlowPtr<Type> base,
                                    NullableFlowPtr<Expr> bits, NullableFlowPtr<Expr> min, NullableFlowPtr<Expr> max,
                                    SourceLocation origin) -> FlowPtr<TemplateType> {
  return CreateTemplateType(std::span(pos_args), std::move(base), std::move(bits), std::move(min), std::move(max),
                            origin);
}

auto ASTFactory::CreateTemplateType(std::span<const FlowPtr<Expr>> pos_args, FlowPtr<Type> base,
                                    NullableFlowPtr<Expr> bits, NullableFlowPtr<Expr> min, NullableFlowPtr<Expr> max,
                                    SourceLocation origin) -> FlowPtr<TemplateType> {
  auto args_copy = AllocateArray<CallArg>(pos_args.size());
  for (size_t i = 0; i < pos_args.size(); i++) {
    args_copy[i].first = std::to_string(i);
    args_copy[i].second = pos_args[i];
  }

  auto node = CreateInstance<TemplateType>(base, args_copy)(m_pool, origin);

  node->SetWidth(std::move(bits));
  node->SetRangeBegin(std::move(min));
  node->SetRangeEnd(std::move(max));

  return node;
}

auto ASTFactory::CreateTemplateType(std::span<const std::pair<string, FlowPtr<Expr>>> named_args, FlowPtr<Type> base,
                                    NullableFlowPtr<Expr> bits, NullableFlowPtr<Expr> min, NullableFlowPtr<Expr> max,
                                    SourceLocation origin) -> FlowPtr<TemplateType> {
  auto args_copy = AllocateArray<CallArg>(named_args.size());
  std::copy(named_args.begin(), named_args.end(), args_copy.begin());

  auto node = CreateInstance<TemplateType>(base, args_copy)(m_pool, origin);

  node->SetWidth(std::move(bits));
  node->SetRangeBegin(std::move(min));
  node->SetRangeEnd(std::move(max));

  return node;
}
