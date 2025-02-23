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

    IMemory& m_pool;

    template <typename T, typename... Args>
    constexpr static inline auto CreateInstance(Args&&... args) {
      return [&](IMemory& pool, SourceLocation origin) {
        FlowPtr<T> obj = MakeFlowPtr<T>(new (pool.allocate(sizeof(T), alignof(T))) T(std::forward<Args>(args)...));
        obj.SetTracking(origin);
        return obj;
      };
    }

    template <typename T>
    constexpr auto AllocateArray(auto size) -> std::span<T> {
      const auto bytes_needed = sizeof(T);
      auto* buffer = static_cast<T*>(m_pool.allocate(bytes_needed, alignof(T)));
      return std::span<T>(buffer, size);
    }

  public:
    constexpr ASTFactory(IMemory& pool) : m_pool(pool) {}
    constexpr ~ASTFactory() = default;

    [[gnu::pure, nodiscard]] auto CreateMockInstance(
        ASTNodeKind kind, SourceLocation origin = SourceLocation::current()) -> FlowPtr<Expr>;

    ///=========================================================================
    /// EXPRESSIONS

    [[gnu::pure, nodiscard]] auto CreateBinary(FlowPtr<Expr> lhs, lex::Operator op, FlowPtr<Expr> rhs,
                                               SourceLocation origin = SourceLocation::current()) -> FlowPtr<Binary>;

    [[gnu::pure, nodiscard]] auto CreateUnary(lex::Operator op, FlowPtr<Expr> rhs,
                                              SourceLocation origin = SourceLocation::current()) -> FlowPtr<Unary>;

    [[gnu::pure, nodiscard]] auto CreatePostUnary(
        FlowPtr<Expr> lhs, lex::Operator op, SourceLocation origin = SourceLocation::current()) -> FlowPtr<PostUnary>;

    [[gnu::pure, nodiscard]] auto CreateTernary(FlowPtr<Expr> condition, FlowPtr<Expr> then, FlowPtr<Expr> ele,
                                                SourceLocation origin = SourceLocation::current()) -> FlowPtr<Ternary>;

    [[gnu::pure, nodiscard]] auto CreateInteger(const boost::multiprecision::uint128_type& x,
                                                SourceLocation origin = SourceLocation::current()) -> FlowPtr<Integer>;

    [[gnu::pure, nodiscard]] auto CreateInteger(string x, SourceLocation origin = SourceLocation::current())
        -> std::optional<FlowPtr<Integer>>;

    [[gnu::pure, nodiscard]] auto CreateInteger(const boost::multiprecision::cpp_int& x,
                                                SourceLocation origin = SourceLocation::current())
        -> std::optional<FlowPtr<Integer>>;

    [[gnu::pure, nodiscard]] auto CreateFloat(double x,
                                              SourceLocation origin = SourceLocation::current()) -> FlowPtr<Float>;

    [[gnu::pure, nodiscard]] auto CreateFloat(string x, SourceLocation origin = SourceLocation::current())
        -> std::optional<FlowPtr<Float>>;

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

    [[gnu::pure, nodiscard]] auto CreateSequence(std::span<const FlowPtr<Expr>> ele = {},
                                                 SourceLocation origin = SourceLocation::current())
        -> FlowPtr<Sequence>;

    [[gnu::pure, nodiscard]] auto CreateSequence(
        const std::vector<FlowPtr<Expr>>& ele, SourceLocation origin = SourceLocation::current()) -> FlowPtr<Sequence>;

    [[gnu::pure, nodiscard]] auto CreateTemplateCall(
        FlowPtr<Expr> callee, const std::unordered_map<std::variant<string, size_t>, FlowPtr<Expr>>& template_args = {},
        const std::unordered_map<std::variant<string, size_t>, FlowPtr<Expr>>& named_args = {},
        SourceLocation origin = SourceLocation::current()) -> std::optional<FlowPtr<Call>>;

    [[gnu::pure, nodiscard]] auto CreateTemplateCall(
        const std::vector<FlowPtr<Expr>>& template_args, const std::vector<FlowPtr<Expr>>& pos_args,
        FlowPtr<Expr> callee, SourceLocation origin = SourceLocation::current()) -> FlowPtr<Call>;

    [[gnu::pure, nodiscard]] auto CreateTemplateCall(
        std::span<const FlowPtr<Expr>> template_args, std::span<const FlowPtr<Expr>> pos_args, FlowPtr<Expr> callee,
        SourceLocation origin = SourceLocation::current()) -> FlowPtr<Call>;

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

    struct FactoryFunctionParameter {
      std::variant<size_t, string> m_name;
      FlowPtr<Type> m_type;
      NullableFlowPtr<Expr> m_default_value;

      FactoryFunctionParameter(std::variant<size_t, string> name, FlowPtr<Type> type,
                               NullableFlowPtr<Expr> default_value = nullptr)
          : m_name(name), m_type(std::move(type)), m_default_value(std::move(default_value)) {}
    };

    [[gnu::pure, nodiscard]] auto CreateFunc(
        FlowPtr<Type> ret_ty, const std::vector<FactoryFunctionParameter>& params = {}, bool variadic = false,
        Purity purity = Purity::Impure, std::vector<FlowPtr<Expr>> attributes = {},
        NullableFlowPtr<Expr> bits = nullptr, NullableFlowPtr<Expr> min = nullptr, NullableFlowPtr<Expr> max = nullptr,
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

    ///=========================================================================
    /// STATEMENTS

    [[gnu::pure, nodiscard]] auto CreateTypedef(string name, FlowPtr<Type> base,
                                                SourceLocation origin = SourceLocation::current()) -> FlowPtr<Typedef>;

    /// TODO: Finish declaration
    [[gnu::pure, nodiscard]] auto CreateStruct(SourceLocation origin = SourceLocation::current()) -> FlowPtr<Struct>;

    struct FactoryEnumItem {
      string m_name;
      NullableFlowPtr<Expr> m_value;

      FactoryEnumItem(string name, NullableFlowPtr<Expr> value = nullptr) : m_name(name), m_value(std::move(value)) {}
    };

    [[gnu::pure, nodiscard]] auto CreateEnum(string name, NullableFlowPtr<Type> ele_ty = nullptr,
                                             const std::vector<FactoryEnumItem>& ele = {},
                                             SourceLocation origin = SourceLocation::current()) -> FlowPtr<Enum>;

    [[gnu::pure, nodiscard]] auto CreateFunction(
        string name, NullableFlowPtr<Type> ret_ty = nullptr, const std::vector<FactoryFunctionParameter>& params = {},
        bool variadic = false, NullableFlowPtr<Expr> body = nullptr, Purity purity = Purity::Impure,
        const std::vector<FlowPtr<Expr>>& attributes = {}, NullableFlowPtr<Expr> precond = nullptr,
        NullableFlowPtr<Expr> postcond = nullptr, const std::vector<std::pair<string, bool>>& captures = {},
        const std::optional<std::span<TemplateParameter>>& template_parameters = std::nullopt,
        SourceLocation origin = SourceLocation::current()) -> std::optional<FlowPtr<Function>>;

    [[gnu::pure, nodiscard]] auto CreateAnonymousFunction(
        Purity purity = Purity::Impure, const std::vector<std::pair<string, bool>>& captures = {},
        NullableFlowPtr<Type> ret_ty = nullptr, const std::vector<FactoryFunctionParameter>& params = {},
        bool variadic = false, NullableFlowPtr<Expr> body = nullptr, const std::vector<FlowPtr<Expr>>& attributes = {},
        NullableFlowPtr<Expr> precond = nullptr, NullableFlowPtr<Expr> postcond = nullptr,
        SourceLocation origin = SourceLocation::current()) -> std::optional<FlowPtr<Function>>;

    [[gnu::pure, nodiscard]] auto CreateScope(string name, FlowPtr<Expr> body, const std::vector<string>& tags = {},
                                              SourceLocation origin = SourceLocation::current()) -> FlowPtr<Scope>;

    [[gnu::pure, nodiscard]] auto CreateExport(FlowPtr<Expr> symbol, Vis vis = Vis::Pub, string abi = "",
                                               const std::vector<FlowPtr<Expr>>& attributes = {},
                                               SourceLocation origin = SourceLocation::current()) -> FlowPtr<Export>;

    [[gnu::pure, nodiscard]] auto CreateBlock(std::span<const FlowPtr<Expr>> items = {},
                                              SafetyMode safety = SafetyMode::Unknown,
                                              SourceLocation origin = SourceLocation::current()) -> FlowPtr<Block>;

    [[gnu::pure, nodiscard]] auto CreateBlock(const std::vector<FlowPtr<Expr>>& items,
                                              SafetyMode safety = SafetyMode::Unknown,
                                              SourceLocation origin = SourceLocation::current()) -> FlowPtr<Block>;

    [[gnu::pure, nodiscard]] auto CreateVariable(
        VariableType variant, string name, NullableFlowPtr<Type> type = nullptr, NullableFlowPtr<Expr> init = nullptr,
        const std::vector<FlowPtr<Expr>>& attributes = {},
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

    [[gnu::pure, nodiscard]] auto CreateFor(NullableFlowPtr<Expr> init, NullableFlowPtr<Expr> step,
                                            NullableFlowPtr<Expr> cond, FlowPtr<Expr> body,
                                            SourceLocation origin = SourceLocation::current()) -> FlowPtr<For>;

    [[gnu::pure, nodiscard]] auto CreateForeach(string key_name, string val_name, FlowPtr<Expr> iterable,
                                                FlowPtr<Expr> body,
                                                SourceLocation origin = SourceLocation::current()) -> FlowPtr<Foreach>;

    [[gnu::pure, nodiscard]] auto CreateCase(FlowPtr<Expr> match, FlowPtr<Expr> body,
                                             SourceLocation origin = SourceLocation::current()) -> FlowPtr<Case>;

    [[gnu::pure, nodiscard]] auto CreateSwitch(FlowPtr<Expr> match, NullableFlowPtr<Expr> defaul = nullptr,
                                               const std::vector<FlowPtr<Case>>& cases = {},
                                               SourceLocation origin = SourceLocation::current()) -> FlowPtr<Switch>;

    ///=========================================================================
    /// STATIC FACTORY FUNCTIONS

    [[gnu::pure, nodiscard]] static inline auto CreateMockInstance(IMemory& m, ASTNodeKind kind) -> FlowPtr<Expr> {
      return ASTFactory(m).CreateMockInstance(kind);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateBinary(IMemory& m, FlowPtr<Expr> lhs, lex::Operator op,
                                                             FlowPtr<Expr> rhs,
                                                             SourceLocation origin = SourceLocation::current()) {
      return ASTFactory(m).CreateBinary(std::move(lhs), op, std::move(rhs), origin);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateUnary(IMemory& m, lex::Operator op, FlowPtr<Expr> rhs,
                                                            SourceLocation origin = SourceLocation::current()) {
      return ASTFactory(m).CreateUnary(op, std::move(rhs), origin);
    }

    [[gnu::pure, nodiscard]] static inline auto CreatePostUnary(IMemory& m, FlowPtr<Expr> lhs, lex::Operator op,
                                                                SourceLocation origin = SourceLocation::current()) {
      return ASTFactory(m).CreatePostUnary(std::move(lhs), op, origin);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateTernary(IMemory& m, FlowPtr<Expr> condition, FlowPtr<Expr> then,
                                                              FlowPtr<Expr> ele,
                                                              SourceLocation origin = SourceLocation::current()) {
      return ASTFactory(m).CreateTernary(std::move(condition), std::move(then), std::move(ele), origin);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateInteger(IMemory& m, const boost::multiprecision::uint128_type& x,
                                                              SourceLocation origin = SourceLocation::current()) {
      return ASTFactory(m).CreateInteger(x, origin);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateInteger(IMemory& m, string x,
                                                              SourceLocation origin = SourceLocation::current()) {
      return ASTFactory(m).CreateInteger(x, origin);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateInteger(IMemory& m, const boost::multiprecision::cpp_int& x,
                                                              SourceLocation origin = SourceLocation::current()) {
      return ASTFactory(m).CreateInteger(x, origin);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateFloat(IMemory& m, double x,
                                                            SourceLocation origin = SourceLocation::current()) {
      return ASTFactory(m).CreateFloat(x, origin);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateFloat(IMemory& m, string x,
                                                            SourceLocation origin = SourceLocation::current()) {
      return ASTFactory(m).CreateFloat(x, origin);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateString(IMemory& m, string x,
                                                             SourceLocation origin = SourceLocation::current()) {
      return ASTFactory(m).CreateString(x, origin);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateCharacter(IMemory& m, char8_t x,
                                                                SourceLocation origin = SourceLocation::current()) {
      return ASTFactory(m).CreateCharacter(x, origin);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateBoolean(IMemory& m, bool x,
                                                              SourceLocation origin = SourceLocation::current()) {
      return ASTFactory(m).CreateBoolean(x, origin);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateNull(IMemory& m,
                                                           SourceLocation origin = SourceLocation::current()) {
      return ASTFactory(m).CreateNull(origin);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateUndefined(IMemory& m,
                                                                SourceLocation origin = SourceLocation::current()) {
      return ASTFactory(m).CreateUndefined(origin);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateCall(
        IMemory& m, FlowPtr<Expr> callee,
        const std::unordered_map<std::variant<string, size_t>, FlowPtr<Expr>>& named_args = {},
        SourceLocation origin = SourceLocation::current()) {
      return ASTFactory(m).CreateCall(std::move(callee), named_args, origin);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateCall(IMemory& m, const std::vector<FlowPtr<Expr>>& pos_args,
                                                           FlowPtr<Expr> callee,
                                                           SourceLocation origin = SourceLocation::current()) {
      return ASTFactory(m).CreateCall(pos_args, std::move(callee), origin);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateCall(IMemory& m, std::span<const FlowPtr<Expr>> pos_args,
                                                           FlowPtr<Expr> callee,
                                                           SourceLocation origin = SourceLocation::current()) {
      return ASTFactory(m).CreateCall(pos_args, std::move(callee), origin);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateList(IMemory& m, std::span<const FlowPtr<Expr>> ele = {},
                                                           SourceLocation origin = SourceLocation::current()) {
      return ASTFactory(m).CreateList(ele, origin);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateList(IMemory& m, const std::vector<FlowPtr<Expr>>& ele,
                                                           SourceLocation origin = SourceLocation::current()) {
      return ASTFactory(m).CreateList(ele, origin);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateAssociation(IMemory& m, FlowPtr<Expr> key, FlowPtr<Expr> value,
                                                                  SourceLocation origin = SourceLocation::current()) {
      return ASTFactory(m).CreateAssociation(std::move(key), std::move(value), origin);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateIndex(IMemory& m, FlowPtr<Expr> base, FlowPtr<Expr> index,
                                                            SourceLocation origin = SourceLocation::current()) {
      return ASTFactory(m).CreateIndex(std::move(base), std::move(index), origin);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateSlice(IMemory& m, FlowPtr<Expr> base, FlowPtr<Expr> start,
                                                            FlowPtr<Expr> end,
                                                            SourceLocation origin = SourceLocation::current()) {
      return ASTFactory(m).CreateSlice(std::move(base), std::move(start), std::move(end), origin);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateFormatString(IMemory& m, string x,
                                                                   SourceLocation origin = SourceLocation::current()) {
      return ASTFactory(m).CreateFormatString(x, origin);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateFormatString(
        IMemory& m, std::span<const std::variant<string, FlowPtr<Expr>>> parts = {},
        SourceLocation origin = SourceLocation::current()) {
      return ASTFactory(m).CreateFormatString(parts, origin);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateFormatString(
        IMemory& m, const std::vector<std::variant<string, FlowPtr<Expr>>>& parts,
        SourceLocation origin = SourceLocation::current()) {
      return ASTFactory(m).CreateFormatString(parts, origin);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateIdentifier(IMemory& m, string name,
                                                                 SourceLocation origin = SourceLocation::current()) {
      return ASTFactory(m).CreateIdentifier(name, origin);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateSequence(IMemory& m, std::span<const FlowPtr<Expr>> ele = {},
                                                               SourceLocation origin = SourceLocation::current()) {
      return ASTFactory(m).CreateSequence(ele, origin);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateSequence(IMemory& m, const std::vector<FlowPtr<Expr>>& ele,
                                                               SourceLocation origin = SourceLocation::current()) {
      return ASTFactory(m).CreateSequence(ele, origin);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateTemplateCall(
        IMemory& m, FlowPtr<Expr> callee,
        const std::unordered_map<std::variant<string, size_t>, FlowPtr<Expr>>& template_args = {},
        const std::unordered_map<std::variant<string, size_t>, FlowPtr<Expr>>& named_args = {},
        SourceLocation origin = SourceLocation::current()) {
      return ASTFactory(m).CreateTemplateCall(std::move(callee), template_args, named_args, origin);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateTemplateCall(IMemory& m,
                                                                   std::span<const FlowPtr<Expr>> template_args,
                                                                   std::span<const FlowPtr<Expr>> pos_args,
                                                                   FlowPtr<Expr> callee,
                                                                   SourceLocation origin = SourceLocation::current()) {
      return ASTFactory(m).CreateTemplateCall(template_args, pos_args, std::move(callee), origin);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateTemplateCall(IMemory& m,
                                                                   const std::vector<FlowPtr<Expr>>& template_args,
                                                                   const std::vector<FlowPtr<Expr>>& pos_args,
                                                                   FlowPtr<Expr> callee,
                                                                   SourceLocation origin = SourceLocation::current()) {
      return ASTFactory(m).CreateTemplateCall(template_args, pos_args, std::move(callee), origin);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateReference(IMemory& m, FlowPtr<Type> to, bool volatil = false,
                                                                NullableFlowPtr<Expr> bits = nullptr,
                                                                NullableFlowPtr<Expr> min = nullptr,
                                                                NullableFlowPtr<Expr> max = nullptr,
                                                                SourceLocation origin = SourceLocation::current()) {
      return ASTFactory(m).CreateReference(std::move(to), volatil, std::move(bits), std::move(min), std::move(max),
                                           origin);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateU1(IMemory& m, NullableFlowPtr<Expr> bits = nullptr,
                                                         NullableFlowPtr<Expr> min = nullptr,
                                                         NullableFlowPtr<Expr> max = nullptr,
                                                         SourceLocation origin = SourceLocation::current()) {
      return ASTFactory(m).CreateU1(std::move(bits), std::move(min), std::move(max), origin);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateU8(IMemory& m, NullableFlowPtr<Expr> bits = nullptr,
                                                         NullableFlowPtr<Expr> min = nullptr,
                                                         NullableFlowPtr<Expr> max = nullptr,
                                                         SourceLocation origin = SourceLocation::current()) {
      return ASTFactory(m).CreateU8(std::move(bits), std::move(min), std::move(max), origin);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateU16(IMemory& m, NullableFlowPtr<Expr> bits = nullptr,
                                                          NullableFlowPtr<Expr> min = nullptr,
                                                          NullableFlowPtr<Expr> max = nullptr,
                                                          SourceLocation origin = SourceLocation::current()) {
      return ASTFactory(m).CreateU16(std::move(bits), std::move(min), std::move(max), origin);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateU32(IMemory& m, NullableFlowPtr<Expr> bits = nullptr,
                                                          NullableFlowPtr<Expr> min = nullptr,
                                                          NullableFlowPtr<Expr> max = nullptr,
                                                          SourceLocation origin = SourceLocation::current()) {
      return ASTFactory(m).CreateU32(std::move(bits), std::move(min), std::move(max), origin);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateU64(IMemory& m, NullableFlowPtr<Expr> bits = nullptr,
                                                          NullableFlowPtr<Expr> min = nullptr,
                                                          NullableFlowPtr<Expr> max = nullptr,
                                                          SourceLocation origin = SourceLocation::current()) {
      return ASTFactory(m).CreateU64(std::move(bits), std::move(min), std::move(max), origin);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateU128(IMemory& m, NullableFlowPtr<Expr> bits = nullptr,
                                                           NullableFlowPtr<Expr> min = nullptr,
                                                           NullableFlowPtr<Expr> max = nullptr,
                                                           SourceLocation origin = SourceLocation::current()) {
      return ASTFactory(m).CreateU128(std::move(bits), std::move(min), std::move(max), origin);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateI8(IMemory& m, NullableFlowPtr<Expr> bits = nullptr,
                                                         NullableFlowPtr<Expr> min = nullptr,
                                                         NullableFlowPtr<Expr> max = nullptr,
                                                         SourceLocation origin = SourceLocation::current()) {
      return ASTFactory(m).CreateI8(std::move(bits), std::move(min), std::move(max), origin);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateI16(IMemory& m, NullableFlowPtr<Expr> bits = nullptr,
                                                          NullableFlowPtr<Expr> min = nullptr,
                                                          NullableFlowPtr<Expr> max = nullptr,
                                                          SourceLocation origin = SourceLocation::current()) {
      return ASTFactory(m).CreateI16(std::move(bits), std::move(min), std::move(max), origin);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateI32(IMemory& m, NullableFlowPtr<Expr> bits = nullptr,
                                                          NullableFlowPtr<Expr> min = nullptr,
                                                          NullableFlowPtr<Expr> max = nullptr,
                                                          SourceLocation origin = SourceLocation::current()) {
      return ASTFactory(m).CreateI32(std::move(bits), std::move(min), std::move(max), origin);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateI64(IMemory& m, NullableFlowPtr<Expr> bits = nullptr,
                                                          NullableFlowPtr<Expr> min = nullptr,
                                                          NullableFlowPtr<Expr> max = nullptr,
                                                          SourceLocation origin = SourceLocation::current()) {
      return ASTFactory(m).CreateI64(std::move(bits), std::move(min), std::move(max), origin);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateI128(IMemory& m, NullableFlowPtr<Expr> bits = nullptr,
                                                           NullableFlowPtr<Expr> min = nullptr,
                                                           NullableFlowPtr<Expr> max = nullptr,
                                                           SourceLocation origin = SourceLocation::current()) {
      return ASTFactory(m).CreateI128(std::move(bits), std::move(min), std::move(max), origin);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateF16(IMemory& m, NullableFlowPtr<Expr> bits = nullptr,
                                                          NullableFlowPtr<Expr> min = nullptr,
                                                          NullableFlowPtr<Expr> max = nullptr,
                                                          SourceLocation origin = SourceLocation::current()) {
      return ASTFactory(m).CreateF16(std::move(bits), std::move(min), std::move(max), origin);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateF32(IMemory& m, NullableFlowPtr<Expr> bits = nullptr,
                                                          NullableFlowPtr<Expr> min = nullptr,
                                                          NullableFlowPtr<Expr> max = nullptr,
                                                          SourceLocation origin = SourceLocation::current()) {
      return ASTFactory(m).CreateF32(std::move(bits), std::move(min), std::move(max), origin);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateF64(IMemory& m, NullableFlowPtr<Expr> bits = nullptr,
                                                          NullableFlowPtr<Expr> min = nullptr,
                                                          NullableFlowPtr<Expr> max = nullptr,
                                                          SourceLocation origin = SourceLocation::current()) {
      return ASTFactory(m).CreateF64(std::move(bits), std::move(min), std::move(max), origin);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateF128(IMemory& m, NullableFlowPtr<Expr> bits = nullptr,
                                                           NullableFlowPtr<Expr> min = nullptr,
                                                           NullableFlowPtr<Expr> max = nullptr,
                                                           SourceLocation origin = SourceLocation::current()) {
      return ASTFactory(m).CreateF128(std::move(bits), std::move(min), std::move(max), origin);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateVoid(IMemory& m, NullableFlowPtr<Expr> bits = nullptr,
                                                           NullableFlowPtr<Expr> min = nullptr,
                                                           NullableFlowPtr<Expr> max = nullptr,
                                                           SourceLocation origin = SourceLocation::current()) {
      return ASTFactory(m).CreateVoid(std::move(bits), std::move(min), std::move(max), origin);
    }

    [[gnu::pure, nodiscard]] static inline auto CreatePointer(IMemory& m, FlowPtr<Type> to, bool volatil = false,
                                                              NullableFlowPtr<Expr> bits = nullptr,
                                                              NullableFlowPtr<Expr> min = nullptr,
                                                              NullableFlowPtr<Expr> max = nullptr,
                                                              SourceLocation origin = SourceLocation::current()) {
      return ASTFactory(m).CreatePointer(std::move(to), volatil, std::move(bits), std::move(min), std::move(max),
                                         origin);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateOpaque(IMemory& m, string name,
                                                             NullableFlowPtr<Expr> bits = nullptr,
                                                             NullableFlowPtr<Expr> min = nullptr,
                                                             NullableFlowPtr<Expr> max = nullptr,
                                                             SourceLocation origin = SourceLocation::current()) {
      return ASTFactory(m).CreateOpaque(name, std::move(bits), std::move(min), std::move(max), origin);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateArray(IMemory& m, FlowPtr<Type> element_type,
                                                            FlowPtr<Expr> element_count,
                                                            NullableFlowPtr<Expr> bits = nullptr,
                                                            NullableFlowPtr<Expr> min = nullptr,
                                                            NullableFlowPtr<Expr> max = nullptr,
                                                            SourceLocation origin = SourceLocation::current()) {
      return ASTFactory(m).CreateArray(std::move(element_type), std::move(element_count), std::move(bits),
                                       std::move(min), std::move(max), origin);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateTuple(IMemory& m, std::span<const FlowPtr<Type>> ele = {},
                                                            NullableFlowPtr<Expr> bits = nullptr,
                                                            NullableFlowPtr<Expr> min = nullptr,
                                                            NullableFlowPtr<Expr> max = nullptr,
                                                            SourceLocation origin = SourceLocation::current()) {
      return ASTFactory(m).CreateTuple(ele, std::move(bits), std::move(min), std::move(max), origin);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateTuple(IMemory& m, const std::vector<FlowPtr<Type>>& ele,
                                                            NullableFlowPtr<Expr> bits = nullptr,
                                                            NullableFlowPtr<Expr> min = nullptr,
                                                            NullableFlowPtr<Expr> max = nullptr,
                                                            SourceLocation origin = SourceLocation::current()) {
      return ASTFactory(m).CreateTuple(ele, std::move(bits), std::move(min), std::move(max), origin);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateFunc(
        IMemory& m, FlowPtr<Type> ret_ty, const std::vector<FactoryFunctionParameter>& params = {},
        bool variadic = false, Purity purity = Purity::Impure, std::vector<FlowPtr<Expr>> attributes = {},
        NullableFlowPtr<Expr> bits = nullptr, NullableFlowPtr<Expr> min = nullptr, NullableFlowPtr<Expr> max = nullptr,
        SourceLocation origin = SourceLocation::current()) {
      return ASTFactory(m).CreateFunc(std::move(ret_ty), params, variadic, purity, std::move(attributes),
                                      std::move(bits), std::move(min), std::move(max), origin);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateNamed(IMemory& m, string name,
                                                            NullableFlowPtr<Expr> bits = nullptr,
                                                            NullableFlowPtr<Expr> min = nullptr,
                                                            NullableFlowPtr<Expr> max = nullptr,
                                                            SourceLocation origin = SourceLocation::current()) {
      return ASTFactory(m).CreateNamed(name, std::move(bits), std::move(min), std::move(max), origin);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateUnknownType(IMemory& m, NullableFlowPtr<Expr> bits = nullptr,
                                                                  NullableFlowPtr<Expr> min = nullptr,
                                                                  NullableFlowPtr<Expr> max = nullptr,
                                                                  SourceLocation origin = SourceLocation::current()) {
      return ASTFactory(m).CreateUnknownType(std::move(bits), std::move(min), std::move(max), origin);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateTemplateType(
        IMemory& m, FlowPtr<Type> base,
        const std::unordered_map<std::variant<string, size_t>, FlowPtr<Expr>>& named_args,
        NullableFlowPtr<Expr> bits = nullptr, NullableFlowPtr<Expr> min = nullptr, NullableFlowPtr<Expr> max = nullptr,
        SourceLocation origin = SourceLocation::current()) {
      return ASTFactory(m).CreateTemplateType(std::move(base), named_args, std::move(bits), std::move(min),
                                              std::move(max), origin);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateTemplateType(
        IMemory& m, const std::vector<FlowPtr<Expr>>& pos_args, FlowPtr<Type> base,
        NullableFlowPtr<Expr> bits = nullptr, NullableFlowPtr<Expr> min = nullptr, NullableFlowPtr<Expr> max = nullptr,
        SourceLocation origin = SourceLocation::current()) {
      return ASTFactory(m).CreateTemplateType(pos_args, std::move(base), std::move(bits), std::move(min),
                                              std::move(max), origin);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateTemplateType(IMemory& m, std::span<const FlowPtr<Expr>> pos_args,
                                                                   FlowPtr<Type> base,
                                                                   NullableFlowPtr<Expr> bits = nullptr,
                                                                   NullableFlowPtr<Expr> min = nullptr,
                                                                   NullableFlowPtr<Expr> max = nullptr,
                                                                   SourceLocation origin = SourceLocation::current()) {
      return ASTFactory(m).CreateTemplateType(pos_args, std::move(base), std::move(bits), std::move(min),
                                              std::move(max), origin);
    }

    ///=========================================================================
    /// STATEMENTS
    [[gnu::pure, nodiscard]] static inline auto CreateTypedef(IMemory& m, string name, FlowPtr<Type> base,
                                                              SourceLocation origin = SourceLocation::current()) {
      return ASTFactory(m).CreateTypedef(name, std::move(base), origin);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateStruct(IMemory& m,
                                                             SourceLocation origin = SourceLocation::current()) {
      return ASTFactory(m).CreateStruct(origin);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateEnum(IMemory& m, string name,
                                                           NullableFlowPtr<Type> ele_ty = nullptr,
                                                           const std::vector<FactoryEnumItem>& ele = {},
                                                           SourceLocation origin = SourceLocation::current()) {
      return ASTFactory(m).CreateEnum(name, std::move(ele_ty), ele, origin);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateFunction(
        IMemory& m, string name, NullableFlowPtr<Type> ret_ty = nullptr,
        const std::vector<FactoryFunctionParameter>& params = {}, bool variadic = false,
        NullableFlowPtr<Expr> body = nullptr, Purity purity = Purity::Impure,
        const std::vector<FlowPtr<Expr>>& attributes = {}, NullableFlowPtr<Expr> precond = nullptr,
        NullableFlowPtr<Expr> postcond = nullptr, const std::vector<std::pair<string, bool>>& captures = {},
        const std::optional<std::span<TemplateParameter>>& template_parameters = std::nullopt,
        SourceLocation origin = SourceLocation::current()) {
      return ASTFactory(m).CreateFunction(name, std::move(ret_ty), params, variadic, std::move(body), purity,
                                          attributes, std::move(precond), std::move(postcond), captures,
                                          template_parameters, origin);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateAnonymousFunction(
        IMemory& m, Purity purity = Purity::Impure, const std::vector<std::pair<string, bool>>& captures = {},
        NullableFlowPtr<Type> ret_ty = nullptr, const std::vector<FactoryFunctionParameter>& params = {},
        bool variadic = false, NullableFlowPtr<Expr> body = nullptr, const std::vector<FlowPtr<Expr>>& attributes = {},
        NullableFlowPtr<Expr> precond = nullptr, NullableFlowPtr<Expr> postcond = nullptr,
        SourceLocation origin = SourceLocation::current()) {
      return ASTFactory(m).CreateAnonymousFunction(purity, captures, std::move(ret_ty), params, variadic,
                                                   std::move(body), attributes, std::move(precond), std::move(postcond),
                                                   origin);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateScope(IMemory& m, string name, FlowPtr<Expr> body,
                                                            const std::vector<string>& tags = {},
                                                            SourceLocation origin = SourceLocation::current()) {
      return ASTFactory(m).CreateScope(name, std::move(body), tags, origin);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateExport(IMemory& m, FlowPtr<Expr> symbol, Vis vis = Vis::Pub,
                                                             string abi = "",
                                                             const std::vector<FlowPtr<Expr>>& attributes = {},
                                                             SourceLocation origin = SourceLocation::current()) {
      return ASTFactory(m).CreateExport(std::move(symbol), vis, abi, attributes, origin);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateBlock(IMemory& m, std::span<const FlowPtr<Expr>> items = {},
                                                            SafetyMode safety = SafetyMode::Unknown,
                                                            SourceLocation origin = SourceLocation::current()) {
      return ASTFactory(m).CreateBlock(items, safety, origin);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateBlock(IMemory& m, const std::vector<FlowPtr<Expr>>& items,
                                                            SafetyMode safety = SafetyMode::Unknown,
                                                            SourceLocation origin = SourceLocation::current()) {
      return ASTFactory(m).CreateBlock(items, safety, origin);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateVariable(IMemory& m, VariableType variant, string name,
                                                               NullableFlowPtr<Type> type = nullptr,
                                                               NullableFlowPtr<Expr> init = nullptr,
                                                               const std::vector<FlowPtr<Expr>>& attributes = {},
                                                               SourceLocation origin = SourceLocation::current()) {
      return ASTFactory(m).CreateVariable(variant, name, std::move(type), std::move(init), attributes, origin);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateAssembly(IMemory& m, string asm_code,
                                                               SourceLocation origin = SourceLocation::current()) {
      return ASTFactory(m).CreateAssembly(asm_code, origin);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateReturn(IMemory& m, NullableFlowPtr<Expr> value = nullptr,
                                                             SourceLocation origin = SourceLocation::current()) {
      return ASTFactory(m).CreateReturn(std::move(value), origin);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateReturnIf(IMemory& m, FlowPtr<Expr> cond,
                                                               NullableFlowPtr<Expr> value = nullptr,
                                                               SourceLocation origin = SourceLocation::current()) {
      return ASTFactory(m).CreateReturnIf(std::move(cond), std::move(value), origin);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateBreak(IMemory& m,
                                                            SourceLocation origin = SourceLocation::current()) {
      return ASTFactory(m).CreateBreak(origin);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateContinue(IMemory& m,
                                                               SourceLocation origin = SourceLocation::current()) {
      return ASTFactory(m).CreateContinue(origin);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateIf(IMemory& m, FlowPtr<Expr> cond, FlowPtr<Expr> then,
                                                         NullableFlowPtr<Expr> ele = nullptr,
                                                         SourceLocation origin = SourceLocation::current()) {
      return ASTFactory(m).CreateIf(std::move(cond), std::move(then), std::move(ele), origin);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateWhile(IMemory& m, FlowPtr<Expr> cond, FlowPtr<Expr> body,
                                                            SourceLocation origin = SourceLocation::current()) {
      return ASTFactory(m).CreateWhile(std::move(cond), std::move(body), origin);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateFor(IMemory& m, NullableFlowPtr<Expr> init,
                                                          NullableFlowPtr<Expr> step, NullableFlowPtr<Expr> cond,
                                                          FlowPtr<Expr> body,
                                                          SourceLocation origin = SourceLocation::current()) {
      return ASTFactory(m).CreateFor(std::move(init), std::move(step), std::move(cond), std::move(body), origin);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateForeach(IMemory& m, string key_name, string val_name,
                                                              FlowPtr<Expr> iterable, FlowPtr<Expr> body,
                                                              SourceLocation origin = SourceLocation::current()) {
      return ASTFactory(m).CreateForeach(key_name, val_name, std::move(iterable), std::move(body), origin);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateCase(IMemory& m, FlowPtr<Expr> match, FlowPtr<Expr> body,
                                                           SourceLocation origin = SourceLocation::current()) {
      return ASTFactory(m).CreateCase(std::move(match), std::move(body), origin);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateSwitch(IMemory& m, FlowPtr<Expr> match,
                                                             NullableFlowPtr<Expr> defaul = nullptr,
                                                             const std::vector<FlowPtr<Case>>& cases = {},
                                                             SourceLocation origin = SourceLocation::current()) {
      return ASTFactory(m).CreateSwitch(std::move(match), std::move(defaul), cases, origin);
    }
  };
}  // namespace ncc::parse

#endif
