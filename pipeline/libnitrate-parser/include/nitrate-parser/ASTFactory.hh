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

#ifndef __NITRATE_AST_FACTORY_H__
#define __NITRATE_AST_FACTORY_H__

#include <boost/multiprecision/cpp_int.hpp>
#include <nitrate-core/AllocateFwd.hh>
#include <nitrate-core/FlowPtr.hh>
#include <nitrate-core/NullableFlowPtr.hh>
#include <nitrate-core/String.hh>
#include <nitrate-lexer/Enums.hh>
#include <nitrate-parser/AST.hh>
#include <nitrate-parser/ASTData.hh>
#include <nitrate-parser/ASTFwd.hh>
#include <nitrate-parser/ASTStmt.hh>
#include <source_location>
#include <utility>
#include <variant>

namespace ncc::parse {
  class NCC_EXPORT ASTFactory final {
    using SourceLocation = std::source_location;

    std::pmr::memory_resource& m_pool;

    template <typename T, typename... Args>
    constexpr static inline auto CreateInstance(Args&&... args) {
      return [&](std::pmr::memory_resource& pool, SourceLocation origin) {
        FlowPtr<T> obj = MakeFlowPtr<T>(new (pool.allocate(sizeof(T), alignof(T))) T(std::forward<Args>(args)...));
        obj.SetTracking(origin);
        return obj;
      };
    }

    template <typename T>
    constexpr auto AllocateArray(auto size) -> std::span<T> {
      const auto bytes_needed = sizeof(T) * size;
      if (bytes_needed == 0) {
        return {};
      }
      auto* buffer = static_cast<T*>(m_pool.allocate(bytes_needed, alignof(T)));
      return std::span<T>(buffer, size);
    }

    [[gnu::pure, nodiscard]] auto CreateMockInstance(
        ASTNodeKind kind, SourceLocation origin = SourceLocation::current()) -> FlowPtr<Expr>;

  public:
    constexpr ASTFactory(std::pmr::memory_resource& pool) : m_pool(pool) {}
    constexpr ~ASTFactory() = default;

    ///=========================================================================
    /// FACTORY TYPES

    struct FactoryFunctionParameter {
      std::variant<size_t, string> m_name;
      FlowPtr<Type> m_type;
      NullableFlowPtr<Expr> m_default_value;

      FactoryFunctionParameter(string name, FlowPtr<Type> type, NullableFlowPtr<Expr> default_value = nullptr)
          : m_name(name), m_type(std::move(type)), m_default_value(std::move(default_value)) {}

      FactoryFunctionParameter(size_t pos, FlowPtr<Type> type, NullableFlowPtr<Expr> default_value = nullptr)
          : m_name(pos), m_type(std::move(type)), m_default_value(std::move(default_value)) {}
    };

    struct FactoryEnumItem {
      string m_name;
      NullableFlowPtr<Expr> m_value;

      FactoryEnumItem(string name, NullableFlowPtr<Expr> value = nullptr) : m_name(name), m_value(std::move(value)) {}
    };

    ///=========================================================================

    template <typename NodeClass>
    [[gnu::pure, nodiscard]] auto CreateMockInstance(ASTNodeKind kind = Expr::GetTypeCode<NodeClass>(),
                                                     SourceLocation origin = SourceLocation::current())
        -> FlowPtr<NodeClass> {
      return CreateMockInstance(kind, origin).As<NodeClass>();
    }

    template <typename NodeClass>
    [[gnu::pure, nodiscard]] static inline auto CreateMockInstance(std::pmr::memory_resource& m,
                                                                   ASTNodeKind kind = Expr::GetTypeCode<NodeClass>(),
                                                                   SourceLocation origin = SourceLocation::current()) {
      return ASTFactory(m).CreateMockInstance(kind, origin).As<NodeClass>();
    }

    ///=========================================================================
    /// EXPRESSIONS

    [[gnu::pure, nodiscard]] auto CreateBinary(FlowPtr<Expr> lhs, lex::Operator op, FlowPtr<Expr> rhs,
                                               SourceLocation origin = SourceLocation::current()) -> FlowPtr<Binary>;

    [[gnu::pure, nodiscard]] auto CreateUnary(lex::Operator op, FlowPtr<Expr> rhs, bool is_postfix = false,
                                              SourceLocation origin = SourceLocation::current()) -> FlowPtr<Unary>;

    [[gnu::pure, nodiscard]] auto CreateTernary(FlowPtr<Expr> condition, FlowPtr<Expr> then, FlowPtr<Expr> ele,
                                                SourceLocation origin = SourceLocation::current()) -> FlowPtr<Ternary>;

    [[gnu::pure, nodiscard]] auto CreateInteger(const boost::multiprecision::uint128_type& x,
                                                SourceLocation origin = SourceLocation::current()) -> FlowPtr<Integer>;

    [[gnu::pure, nodiscard]] auto CreateInteger(string x, SourceLocation origin = SourceLocation::current())
        -> std::optional<FlowPtr<Integer>>;

    [[gnu::pure, nodiscard]] auto CreateIntegerUnchecked(string x, SourceLocation origin = SourceLocation::current())
        -> FlowPtr<Integer>;

    [[gnu::pure, nodiscard]] auto CreateInteger(const boost::multiprecision::cpp_int& x,
                                                SourceLocation origin = SourceLocation::current())
        -> std::optional<FlowPtr<Integer>>;

    [[gnu::pure, nodiscard]] auto CreateFloat(double x,
                                              SourceLocation origin = SourceLocation::current()) -> FlowPtr<Float>;

    [[gnu::pure, nodiscard]] auto CreateFloat(string x, SourceLocation origin = SourceLocation::current())
        -> std::optional<FlowPtr<Float>>;

    [[gnu::pure, nodiscard]] auto CreateFloatUnchecked(string x, SourceLocation origin = SourceLocation::current())
        -> FlowPtr<Float>;

    [[gnu::pure, nodiscard]] auto CreateString(string x,
                                               SourceLocation origin = SourceLocation::current()) -> FlowPtr<String>;

    template <typename CharType>
    [[gnu::pure, nodiscard]] auto CreateString(std::span<CharType> x,
                                               SourceLocation origin = SourceLocation::current()) -> FlowPtr<String> {
      static_assert(sizeof(CharType) == 1, "CharType must be 1 byte in size");

      return CreateString(string(x.begin(), x.end()), origin);
    }

    [[gnu::pure, nodiscard]] auto CreateCharacter(char8_t x, SourceLocation origin = SourceLocation::current())
        -> FlowPtr<Character>;

    [[gnu::pure, nodiscard]] auto CreateBoolean(bool x,
                                                SourceLocation origin = SourceLocation::current()) -> FlowPtr<Boolean>;

    [[gnu::pure, nodiscard]] auto CreateNull(SourceLocation origin = SourceLocation::current()) -> FlowPtr<Null>;

    [[gnu::pure, nodiscard]] auto CreateUndefined(SourceLocation origin = SourceLocation::current())
        -> FlowPtr<Undefined>;

    [[gnu::pure, nodiscard]] auto CreateCall(
        FlowPtr<Expr> callee, const std::unordered_map<std::variant<string, size_t>, FlowPtr<Expr>>& named_args = {},
        SourceLocation origin = SourceLocation::current()) -> std::optional<FlowPtr<Call>>;

    [[gnu::pure, nodiscard]] auto CreateCall(const std::vector<FlowPtr<Expr>>& pos_args, FlowPtr<Expr> callee,
                                             SourceLocation origin = SourceLocation::current()) -> FlowPtr<Call>;

    [[gnu::pure, nodiscard]] auto CreateCall(std::span<const FlowPtr<Expr>> pos_args, FlowPtr<Expr> callee,
                                             SourceLocation origin = SourceLocation::current()) -> FlowPtr<Call>;

    [[gnu::pure, nodiscard]] auto CreateCall(std::span<const std::pair<string, FlowPtr<Expr>>> arguments,
                                             FlowPtr<Expr> callee,
                                             SourceLocation origin = SourceLocation::current()) -> FlowPtr<Call>;

    [[gnu::pure, nodiscard]] auto CreateList(std::span<const FlowPtr<Expr>> ele = {},
                                             SourceLocation origin = SourceLocation::current()) -> FlowPtr<List>;

    [[gnu::pure, nodiscard]] auto CreateList(const std::vector<FlowPtr<Expr>>& ele,
                                             SourceLocation origin = SourceLocation::current()) -> FlowPtr<List>;

    [[gnu::pure, nodiscard]] auto CreateAssociation(
        FlowPtr<Expr> key, FlowPtr<Expr> value, SourceLocation origin = SourceLocation::current()) -> FlowPtr<Assoc>;

    [[gnu::pure, nodiscard]] auto CreateIndex(FlowPtr<Expr> base, FlowPtr<Expr> index,
                                              SourceLocation origin = SourceLocation::current()) -> FlowPtr<Index>;

    [[gnu::pure, nodiscard]] auto CreateSlice(FlowPtr<Expr> base, FlowPtr<Expr> start, FlowPtr<Expr> end,
                                              SourceLocation origin = SourceLocation::current()) -> FlowPtr<Slice>;

    [[gnu::pure, nodiscard]] auto CreateFormatString(string x, SourceLocation origin = SourceLocation::current())
        -> FlowPtr<FString>;

    [[gnu::pure, nodiscard]] auto CreateFormatString(std::span<const std::variant<string, FlowPtr<Expr>>> parts = {},
                                                     SourceLocation origin = SourceLocation::current())
        -> FlowPtr<FString>;

    [[gnu::pure, nodiscard]] auto CreateFormatString(const std::vector<std::variant<string, FlowPtr<Expr>>>& parts,
                                                     SourceLocation origin = SourceLocation::current())
        -> FlowPtr<FString>;

    [[gnu::pure, nodiscard]] auto CreateIdentifier(string name, SourceLocation origin = SourceLocation::current())
        -> FlowPtr<Identifier>;

    [[gnu::pure, nodiscard]] auto CreateTemplateCall(
        FlowPtr<Expr> callee, const std::unordered_map<std::variant<string, size_t>, FlowPtr<Expr>>& template_args = {},
        const std::unordered_map<std::variant<string, size_t>, FlowPtr<Expr>>& named_args = {},
        SourceLocation origin = SourceLocation::current()) -> std::optional<FlowPtr<TemplateCall>>;

    [[gnu::pure, nodiscard]] auto CreateTemplateCall(
        const std::vector<FlowPtr<Expr>>& template_args, const std::vector<FlowPtr<Expr>>& pos_args,
        FlowPtr<Expr> callee, SourceLocation origin = SourceLocation::current()) -> FlowPtr<TemplateCall>;

    [[gnu::pure, nodiscard]] auto CreateTemplateCall(
        std::span<const FlowPtr<Expr>> template_args, std::span<const FlowPtr<Expr>> pos_args, FlowPtr<Expr> callee,
        SourceLocation origin = SourceLocation::current()) -> FlowPtr<TemplateCall>;

    [[gnu::pure, nodiscard]] auto CreateTemplateCall(
        std::span<const std::pair<string, FlowPtr<Expr>>> template_args,
        std::span<const std::pair<string, FlowPtr<Expr>>> args, FlowPtr<Expr> callee,
        SourceLocation origin = SourceLocation::current()) -> FlowPtr<TemplateCall>;

    [[gnu::pure, nodiscard]] auto CreateImport(string name, ImportMode mode, FlowPtr<Expr> subtree,
                                               SourceLocation origin = SourceLocation::current()) -> FlowPtr<Import>;

    ///=========================================================================
    /// TYPES

    [[gnu::pure, nodiscard]] auto CreateReference(FlowPtr<Type> to, bool volatil = false,
                                                  NullableFlowPtr<Expr> bits = nullptr,
                                                  NullableFlowPtr<Expr> min = nullptr,
                                                  NullableFlowPtr<Expr> max = nullptr,
                                                  SourceLocation origin = SourceLocation::current()) -> FlowPtr<RefTy>;

    [[gnu::pure, nodiscard]] auto CreateU1(NullableFlowPtr<Expr> bits = nullptr, NullableFlowPtr<Expr> min = nullptr,
                                           NullableFlowPtr<Expr> max = nullptr,
                                           SourceLocation origin = SourceLocation::current()) -> FlowPtr<U1>;

    [[gnu::pure, nodiscard]] auto CreateU8(NullableFlowPtr<Expr> bits = nullptr, NullableFlowPtr<Expr> min = nullptr,
                                           NullableFlowPtr<Expr> max = nullptr,
                                           SourceLocation origin = SourceLocation::current()) -> FlowPtr<U8>;

    [[gnu::pure, nodiscard]] auto CreateU16(NullableFlowPtr<Expr> bits = nullptr, NullableFlowPtr<Expr> min = nullptr,
                                            NullableFlowPtr<Expr> max = nullptr,
                                            SourceLocation origin = SourceLocation::current()) -> FlowPtr<U16>;

    [[gnu::pure, nodiscard]] auto CreateU32(NullableFlowPtr<Expr> bits = nullptr, NullableFlowPtr<Expr> min = nullptr,
                                            NullableFlowPtr<Expr> max = nullptr,
                                            SourceLocation origin = SourceLocation::current()) -> FlowPtr<U32>;

    [[gnu::pure, nodiscard]] auto CreateU64(NullableFlowPtr<Expr> bits = nullptr, NullableFlowPtr<Expr> min = nullptr,
                                            NullableFlowPtr<Expr> max = nullptr,
                                            SourceLocation origin = SourceLocation::current()) -> FlowPtr<U64>;

    [[gnu::pure, nodiscard]] auto CreateU128(NullableFlowPtr<Expr> bits = nullptr, NullableFlowPtr<Expr> min = nullptr,
                                             NullableFlowPtr<Expr> max = nullptr,
                                             SourceLocation origin = SourceLocation::current()) -> FlowPtr<U128>;

    [[gnu::pure, nodiscard]] auto CreateI8(NullableFlowPtr<Expr> bits = nullptr, NullableFlowPtr<Expr> min = nullptr,
                                           NullableFlowPtr<Expr> max = nullptr,
                                           SourceLocation origin = SourceLocation::current()) -> FlowPtr<I8>;

    [[gnu::pure, nodiscard]] auto CreateI16(NullableFlowPtr<Expr> bits = nullptr, NullableFlowPtr<Expr> min = nullptr,
                                            NullableFlowPtr<Expr> max = nullptr,
                                            SourceLocation origin = SourceLocation::current()) -> FlowPtr<I16>;

    [[gnu::pure, nodiscard]] auto CreateI32(NullableFlowPtr<Expr> bits = nullptr, NullableFlowPtr<Expr> min = nullptr,
                                            NullableFlowPtr<Expr> max = nullptr,
                                            SourceLocation origin = SourceLocation::current()) -> FlowPtr<I32>;

    [[gnu::pure, nodiscard]] auto CreateI64(NullableFlowPtr<Expr> bits = nullptr, NullableFlowPtr<Expr> min = nullptr,
                                            NullableFlowPtr<Expr> max = nullptr,
                                            SourceLocation origin = SourceLocation::current()) -> FlowPtr<I64>;

    [[gnu::pure, nodiscard]] auto CreateI128(NullableFlowPtr<Expr> bits = nullptr, NullableFlowPtr<Expr> min = nullptr,
                                             NullableFlowPtr<Expr> max = nullptr,
                                             SourceLocation origin = SourceLocation::current()) -> FlowPtr<I128>;

    [[gnu::pure, nodiscard]] auto CreateF16(NullableFlowPtr<Expr> bits = nullptr, NullableFlowPtr<Expr> min = nullptr,
                                            NullableFlowPtr<Expr> max = nullptr,
                                            SourceLocation origin = SourceLocation::current()) -> FlowPtr<F16>;

    [[gnu::pure, nodiscard]] auto CreateF32(NullableFlowPtr<Expr> bits = nullptr, NullableFlowPtr<Expr> min = nullptr,
                                            NullableFlowPtr<Expr> max = nullptr,
                                            SourceLocation origin = SourceLocation::current()) -> FlowPtr<F32>;

    [[gnu::pure, nodiscard]] auto CreateF64(NullableFlowPtr<Expr> bits = nullptr, NullableFlowPtr<Expr> min = nullptr,
                                            NullableFlowPtr<Expr> max = nullptr,
                                            SourceLocation origin = SourceLocation::current()) -> FlowPtr<F64>;

    [[gnu::pure, nodiscard]] auto CreateF128(NullableFlowPtr<Expr> bits = nullptr, NullableFlowPtr<Expr> min = nullptr,
                                             NullableFlowPtr<Expr> max = nullptr,
                                             SourceLocation origin = SourceLocation::current()) -> FlowPtr<F128>;

    [[gnu::pure, nodiscard]] auto CreateVoid(NullableFlowPtr<Expr> bits = nullptr, NullableFlowPtr<Expr> min = nullptr,
                                             NullableFlowPtr<Expr> max = nullptr,
                                             SourceLocation origin = SourceLocation::current()) -> FlowPtr<VoidTy>;

    [[gnu::pure, nodiscard]] auto CreatePointer(FlowPtr<Type> to, bool volatil = false,
                                                NullableFlowPtr<Expr> bits = nullptr,
                                                NullableFlowPtr<Expr> min = nullptr,
                                                NullableFlowPtr<Expr> max = nullptr,
                                                SourceLocation origin = SourceLocation::current()) -> FlowPtr<PtrTy>;

    [[gnu::pure, nodiscard]] auto CreateOpaque(string name, NullableFlowPtr<Expr> bits = nullptr,
                                               NullableFlowPtr<Expr> min = nullptr, NullableFlowPtr<Expr> max = nullptr,
                                               SourceLocation origin = SourceLocation::current()) -> FlowPtr<OpaqueTy>;

    [[gnu::pure, nodiscard]] auto CreateArray(FlowPtr<Type> element_type, FlowPtr<Expr> element_count,
                                              NullableFlowPtr<Expr> bits = nullptr, NullableFlowPtr<Expr> min = nullptr,
                                              NullableFlowPtr<Expr> max = nullptr,
                                              SourceLocation origin = SourceLocation::current()) -> FlowPtr<ArrayTy>;

    [[gnu::pure, nodiscard]] auto CreateTuple(std::span<const FlowPtr<Type>> ele = {},
                                              NullableFlowPtr<Expr> bits = nullptr, NullableFlowPtr<Expr> min = nullptr,
                                              NullableFlowPtr<Expr> max = nullptr,
                                              SourceLocation origin = SourceLocation::current()) -> FlowPtr<TupleTy>;

    [[gnu::pure, nodiscard]] auto CreateTuple(const std::vector<FlowPtr<Type>>& ele,
                                              NullableFlowPtr<Expr> bits = nullptr, NullableFlowPtr<Expr> min = nullptr,
                                              NullableFlowPtr<Expr> max = nullptr,
                                              SourceLocation origin = SourceLocation::current()) -> FlowPtr<TupleTy>;

    [[gnu::pure, nodiscard]] auto CreateFunctionType(
        FlowPtr<Type> ret_ty, const std::vector<FactoryFunctionParameter>& params, bool variadic = false,
        std::vector<FlowPtr<Expr>> attributes = {}, NullableFlowPtr<Expr> bits = nullptr,
        NullableFlowPtr<Expr> min = nullptr, NullableFlowPtr<Expr> max = nullptr,
        SourceLocation origin = SourceLocation::current()) -> std::optional<FlowPtr<FuncTy>>;

    [[gnu::pure, nodiscard]] auto CreateFunctionType(
        FlowPtr<Type> ret_ty, std::span<const FuncParam> params = {}, bool variadic = false,
        std::span<const FlowPtr<Expr>> attributes = {}, NullableFlowPtr<Expr> bits = nullptr,
        NullableFlowPtr<Expr> min = nullptr, NullableFlowPtr<Expr> max = nullptr,
        SourceLocation origin = SourceLocation::current()) -> std::optional<FlowPtr<FuncTy>>;

    [[gnu::pure, nodiscard]] auto CreateNamed(string name, NullableFlowPtr<Expr> bits = nullptr,
                                              NullableFlowPtr<Expr> min = nullptr, NullableFlowPtr<Expr> max = nullptr,
                                              SourceLocation origin = SourceLocation::current()) -> FlowPtr<NamedTy>;

    [[gnu::pure, nodiscard]] auto CreateUnknownType(
        NullableFlowPtr<Expr> bits = nullptr, NullableFlowPtr<Expr> min = nullptr, NullableFlowPtr<Expr> max = nullptr,
        SourceLocation origin = SourceLocation::current()) -> FlowPtr<InferTy>;

    [[gnu::pure, nodiscard]] auto CreateTemplateType(
        FlowPtr<Type> base, const std::unordered_map<std::variant<string, size_t>, FlowPtr<Expr>>& named_args = {},
        NullableFlowPtr<Expr> bits = nullptr, NullableFlowPtr<Expr> min = nullptr, NullableFlowPtr<Expr> max = nullptr,
        SourceLocation origin = SourceLocation::current()) -> std::optional<FlowPtr<TemplateType>>;

    [[gnu::pure, nodiscard]] auto CreateTemplateType(
        const std::vector<FlowPtr<Expr>>& pos_args, FlowPtr<Type> base, NullableFlowPtr<Expr> bits = nullptr,
        NullableFlowPtr<Expr> min = nullptr, NullableFlowPtr<Expr> max = nullptr,
        SourceLocation origin = SourceLocation::current()) -> FlowPtr<TemplateType>;

    [[gnu::pure, nodiscard]] auto CreateTemplateType(
        std::span<const FlowPtr<Expr>> pos_args, FlowPtr<Type> base, NullableFlowPtr<Expr> bits = nullptr,
        NullableFlowPtr<Expr> min = nullptr, NullableFlowPtr<Expr> max = nullptr,
        SourceLocation origin = SourceLocation::current()) -> FlowPtr<TemplateType>;

    [[gnu::pure, nodiscard]] auto CreateTemplateType(
        std::span<const std::pair<string, FlowPtr<Expr>>> named_args, FlowPtr<Type> base,
        NullableFlowPtr<Expr> bits = nullptr, NullableFlowPtr<Expr> min = nullptr, NullableFlowPtr<Expr> max = nullptr,
        SourceLocation origin = SourceLocation::current()) -> FlowPtr<TemplateType>;

    ///=========================================================================
    /// STATEMENTS

    [[gnu::pure, nodiscard]] auto CreateTypedef(string name, FlowPtr<Type> base,
                                                SourceLocation origin = SourceLocation::current()) -> FlowPtr<Typedef>;

    [[gnu::pure, nodiscard]] auto CreateStruct(
        CompositeType comp_type = CompositeType::Class, string name = "",
        const std::optional<std::vector<TemplateParameter>>& tparams = std::nullopt,
        const std::vector<StructField>& fields = {}, const std::vector<StructFunction>& methods = {},
        const std::vector<StructFunction>& static_methods = {}, const std::vector<string>& constraints = {},
        const std::vector<FlowPtr<Expr>>& attributes = {},
        SourceLocation origin = SourceLocation::current()) -> FlowPtr<Struct>;

    [[gnu::pure, nodiscard]] auto CreateEnum(string name, const std::vector<FactoryEnumItem>& ele,
                                             NullableFlowPtr<Type> ele_ty = nullptr,
                                             SourceLocation origin = SourceLocation::current()) -> FlowPtr<Enum>;

    [[gnu::pure, nodiscard]] auto CreateEnum(string name,
                                             std::span<const std::pair<string, NullableFlowPtr<Expr>>> ele = {},
                                             NullableFlowPtr<Type> ele_ty = nullptr,
                                             SourceLocation origin = SourceLocation::current()) -> FlowPtr<Enum>;

    [[gnu::pure, nodiscard]] auto CreateFunction(
        string name, NullableFlowPtr<Type> ret_ty = nullptr, const std::vector<FactoryFunctionParameter>& params = {},
        bool variadic = false, NullableFlowPtr<Expr> body = nullptr, const std::vector<FlowPtr<Expr>>& attributes = {},
        NullableFlowPtr<Expr> precond = nullptr, NullableFlowPtr<Expr> postcond = nullptr,

        const std::optional<std::vector<TemplateParameter>>& template_parameters = std::nullopt,
        SourceLocation origin = SourceLocation::current()) -> std::optional<FlowPtr<Function>>;

    [[gnu::pure, nodiscard]] auto CreateAnonymousFunction(
        NullableFlowPtr<Type> ret_ty = nullptr, const std::vector<FactoryFunctionParameter>& params = {},
        bool variadic = false, NullableFlowPtr<Expr> body = nullptr, const std::vector<FlowPtr<Expr>>& attributes = {},
        NullableFlowPtr<Expr> precond = nullptr, NullableFlowPtr<Expr> postcond = nullptr,
        SourceLocation origin = SourceLocation::current()) -> std::optional<FlowPtr<Function>>;

    [[gnu::pure, nodiscard]] auto CreateScope(string name, FlowPtr<Expr> body, const std::vector<string>& tags,
                                              SourceLocation origin = SourceLocation::current()) -> FlowPtr<Scope>;

    [[gnu::pure, nodiscard]] auto CreateScope(string name, FlowPtr<Expr> body, std::span<const string> tags = {},
                                              SourceLocation origin = SourceLocation::current()) -> FlowPtr<Scope>;

    [[gnu::pure, nodiscard]] auto CreateExport(FlowPtr<Expr> symbol, const std::vector<FlowPtr<Expr>>& attributes,
                                               Vis vis = Vis::Pub, string abi = "",
                                               SourceLocation origin = SourceLocation::current()) -> FlowPtr<Export>;

    [[gnu::pure, nodiscard]] auto CreateExport(FlowPtr<Expr> symbol, std::span<const FlowPtr<Expr>> attributes = {},
                                               Vis vis = Vis::Pub, string abi = "",
                                               SourceLocation origin = SourceLocation::current()) -> FlowPtr<Export>;

    [[gnu::pure, nodiscard]] auto CreateBlock(std::span<const FlowPtr<Expr>> items = {},
                                              BlockMode safety = BlockMode::Unknown,
                                              SourceLocation origin = SourceLocation::current()) -> FlowPtr<Block>;

    [[gnu::pure, nodiscard]] auto CreateBlock(const std::vector<FlowPtr<Expr>>& items,
                                              BlockMode safety = BlockMode::Unknown,
                                              SourceLocation origin = SourceLocation::current()) -> FlowPtr<Block>;

    [[gnu::pure, nodiscard]] auto CreateVariable(
        VariableType variant, string name, const std::vector<FlowPtr<Expr>>& attributes,
        NullableFlowPtr<Type> type = nullptr, NullableFlowPtr<Expr> init = nullptr,
        SourceLocation origin = SourceLocation::current()) -> FlowPtr<Variable>;

    [[gnu::pure, nodiscard]] auto CreateVariable(
        VariableType variant, string name, std::span<const FlowPtr<Expr>> attributes = {},
        NullableFlowPtr<Type> type = nullptr, NullableFlowPtr<Expr> init = nullptr,
        SourceLocation origin = SourceLocation::current()) -> FlowPtr<Variable>;

    [[gnu::pure, nodiscard]] auto CreateAssembly(string asm_code, SourceLocation origin = SourceLocation::current())
        -> FlowPtr<Assembly>;

    [[gnu::pure, nodiscard]] auto CreateReturn(NullableFlowPtr<Expr> value = nullptr,
                                               SourceLocation origin = SourceLocation::current()) -> FlowPtr<Return>;

    [[gnu::pure, nodiscard]] auto CreateReturnIf(FlowPtr<Expr> cond, NullableFlowPtr<Expr> value = nullptr,
                                                 SourceLocation origin = SourceLocation::current())
        -> FlowPtr<ReturnIf>;

    [[gnu::pure, nodiscard]] auto CreateBreak(SourceLocation origin = SourceLocation::current()) -> FlowPtr<Break>;

    [[gnu::pure, nodiscard]] auto CreateContinue(SourceLocation origin = SourceLocation::current())
        -> FlowPtr<Continue>;

    [[gnu::pure, nodiscard]] auto CreateIf(FlowPtr<Expr> cond, FlowPtr<Expr> then, NullableFlowPtr<Expr> ele = nullptr,
                                           SourceLocation origin = SourceLocation::current()) -> FlowPtr<If>;

    [[gnu::pure, nodiscard]] auto CreateWhile(FlowPtr<Expr> cond, FlowPtr<Expr> body,
                                              SourceLocation origin = SourceLocation::current()) -> FlowPtr<While>;

    [[gnu::pure, nodiscard]] auto CreateFor(NullableFlowPtr<Expr> init, NullableFlowPtr<Expr> cond,
                                            NullableFlowPtr<Expr> step, FlowPtr<Expr> body,
                                            SourceLocation origin = SourceLocation::current()) -> FlowPtr<For>;

    [[gnu::pure, nodiscard]] auto CreateForeach(string key_name, string val_name, FlowPtr<Expr> iterable,
                                                FlowPtr<Expr> body,
                                                SourceLocation origin = SourceLocation::current()) -> FlowPtr<Foreach>;

    [[gnu::pure, nodiscard]] auto CreateCase(FlowPtr<Expr> match, FlowPtr<Expr> body,
                                             SourceLocation origin = SourceLocation::current()) -> FlowPtr<Case>;

    [[gnu::pure, nodiscard]] auto CreateSwitch(FlowPtr<Expr> match, NullableFlowPtr<Expr> defaul,
                                               const std::vector<FlowPtr<Case>>& cases,
                                               SourceLocation origin = SourceLocation::current()) -> FlowPtr<Switch>;

    [[gnu::pure, nodiscard]] auto CreateSwitch(FlowPtr<Expr> match, NullableFlowPtr<Expr> defaul = nullptr,
                                               std::span<const FlowPtr<Case>> cases = {},
                                               SourceLocation origin = SourceLocation::current()) -> FlowPtr<Switch>;
  };
}  // namespace ncc::parse

#endif
