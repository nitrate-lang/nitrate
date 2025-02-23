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
#include <nitrate-parser/ASTFwd.hh>
#include <source_location>
#include <variant>

namespace ncc::parse {
  class ASTFactory final {
    using SourceLocation = std::source_location;

    IMemory& m_pool;

  public:
    constexpr ASTFactory(IMemory& pool) : m_pool(pool) {}
    constexpr ~ASTFactory() = default;

    ///=========================================================================
    /// COMMON BASE CLASSES

    /**
     * @brief Construct a new node object
     * @return FlowPtr<Base> A pointer to the newly created base object
     * @note This function is thread-safe
     */
    [[gnu::pure, nodiscard]] auto CreateBase(SourceLocation dbgsrc = SourceLocation::current()) -> FlowPtr<Base>;

    ///=========================================================================
    /// EXPRESSIONS

    /**
     * @brief Construct a new binary expression object
     * @param lhs The left-hand side of the binary expression
     * @param op The operator of the binary expression
     * @param rhs The right-hand side of the binary expression
     * @return FlowPtr<Binary> A pointer to the newly created binary expression object
     * @note This function is thread-safe
     */
    [[gnu::pure, nodiscard]] auto CreateBinary(FlowPtr<Expr> lhs, lex::Operator op, FlowPtr<Expr> rhs,
                                               SourceLocation dbgsrc = SourceLocation::current()) -> FlowPtr<Binary>;

    /**
     * @brief Construct a new unary expression object
     * @param op The operator of the unary expression
     * @param rhs The right-hand side of the unary expression
     * @return FlowPtr<Unary> A pointer to the newly created unary expression object
     * @note This function is thread-safe
     */
    [[gnu::pure, nodiscard]] auto CreateUnary(lex::Operator op, FlowPtr<Expr> rhs,
                                              SourceLocation dbgsrc = SourceLocation::current()) -> FlowPtr<Unary>;

    /**
     * @brief Construct a new post-unary expression object
     * @param lhs The left-hand side of the post-unary expression
     * @param op The operator of the post-unary expression
     * @return FlowPtr<PostUnary> A pointer to the newly created post-unary expression object
     * @note This function is thread-safe
     */
    [[gnu::pure, nodiscard]] auto CreatePostUnary(
        FlowPtr<Expr> lhs, lex::Operator op, SourceLocation dbgsrc = SourceLocation::current()) -> FlowPtr<PostUnary>;

    /**
     * @brief Construct a new ternary expression object
     * @param condition The condition of the ternary expression
     * @param then The 'then' branch of the ternary expression
     * @param ele The 'else' branch of the ternary expression
     * @return FlowPtr<Ternary> A pointer to the newly created ternary expression object
     * @note This function is thread-safe
     */
    [[gnu::pure, nodiscard]] auto CreateTernary(FlowPtr<Expr> condition, FlowPtr<Expr> then, FlowPtr<Expr> ele,
                                                SourceLocation dbgsrc = SourceLocation::current()) -> FlowPtr<Ternary>;

    /**
     * @brief Construct a new integer expression object
     * @param x The value of the integer expression
     * @return FlowPtr<Integer> A pointer to the newly created integer expression object
     * @note This function is thread-safe
     */
    [[gnu::pure, nodiscard]] auto CreateInteger(const boost::multiprecision::uint128_type& x,
                                                SourceLocation dbgsrc = SourceLocation::current()) -> FlowPtr<Integer>;

    /**
     * @brief Construct a new integer expression object
     * @param x The value of the integer expression
     * @return FlowPtr<Integer> A pointer to the newly created integer expression object, or std::nullopt if the value
     * is not a valid integer in range
     * @note This function is thread-safe
     */
    [[gnu::pure, nodiscard]] auto CreateInteger(string x, SourceLocation dbgsrc = SourceLocation::current())
        -> std::optional<FlowPtr<Integer>>;

    /**
     * @brief Construct a new integer expression object
     * @param x The value of the integer expression
     * @return FlowPtr<Integer> A pointer to the newly created integer expression object, or std::nullopt if the value
     * is not a valid integer in range
     * @note This function is thread-safe
     */
    [[gnu::pure, nodiscard]] auto CreateInteger(const boost::multiprecision::cpp_int& x,
                                                SourceLocation dbgsrc = SourceLocation::current())
        -> std::optional<FlowPtr<Integer>>;

    /**
     * @brief Construct a new float expression object
     * @param x The value of the float expression
     * @return FlowPtr<Float> A pointer to the newly created float expression object
     * @note This function is thread-safe
     */
    [[gnu::pure, nodiscard]] auto CreateFloat(double x,
                                              SourceLocation dbgsrc = SourceLocation::current()) -> FlowPtr<Float>;

    /**
     * @brief Construct a new float expression object
     * @param x The value of the float expression
     * @return FlowPtr<Float> A pointer to the newly created float expression object, or std::nullopt if the value is
     * not a valid float
     * @note This function is thread-safe
     */
    [[gnu::pure, nodiscard]] auto CreateFloat(string x, SourceLocation dbgsrc = SourceLocation::current())
        -> std::optional<FlowPtr<Float>>;

    /**
     * @brief Construct a new string expression object
     * @param x The value of the string expression
     * @return FlowPtr<String> A pointer to the newly created string expression object
     * @note This function is thread-safe
     */
    [[gnu::pure, nodiscard]] auto CreateString(string x,
                                               SourceLocation dbgsrc = SourceLocation::current()) -> FlowPtr<String>;

    /**
     * @brief Construct a new string expression object
     * @param x The value of the string expression
     * @return FlowPtr<String> A pointer to the newly created string expression object
     * @note This function is thread-safe
     */
    template <typename CharType>
    [[gnu::pure, nodiscard]] auto CreateString(std::span<CharType> x,
                                               SourceLocation dbgsrc = SourceLocation::current()) -> FlowPtr<String> {
      static_assert(sizeof(CharType) == 1, "CharType must be 1 byte in size");

      return CreateString(string(x.begin(), x.end()), dbgsrc);
    }

    /**
     * @brief Construct a new character expression object
     * @param x The value of the character expression
     * @return FlowPtr<Character> A pointer to the newly created character expression object
     * @note This function is thread-safe
     */
    [[gnu::pure, nodiscard]] auto CreateCharacter(char8_t x, SourceLocation dbgsrc = SourceLocation::current())
        -> FlowPtr<Character>;

    /**
     * @brief Construct a new boolean expression object
     * @param x The value of the boolean expression
     * @return FlowPtr<Boolean> A pointer to the newly created boolean expression object
     * @note This function is thread-safe
     */
    [[gnu::pure, nodiscard]] auto CreateBoolean(bool x,
                                                SourceLocation dbgsrc = SourceLocation::current()) -> FlowPtr<Boolean>;

    /**
     * @brief Construct a new null expression object
     * @return FlowPtr<Null> A pointer to the newly created null expression object
     * @note This function is thread-safe
     */
    [[gnu::pure, nodiscard]] auto CreateNull(SourceLocation dbgsrc = SourceLocation::current()) -> FlowPtr<Null>;

    /**
     * @brief Construct a new undefined expression object
     * @return FlowPtr<Undefined> A pointer to the newly created undefined expression object
     * @note This function is thread-safe
     */
    [[gnu::pure, nodiscard]] auto CreateUndefined(SourceLocation dbgsrc = SourceLocation::current())
        -> FlowPtr<Undefined>;

    /** @brief Construct a new call expression object
     * @param callee The callee of the call expression
     * @param named_args The named arguments of the call expression. Positional arguments indexes start at 0, while
     * named arguments are indexed by their name.
     * @return FlowPtr<Call> A pointer to the newly created call expression object
     *
     * @note Checking is done to ensure that the specified arguments do not conflict.
     * @note This function is thread-safe
     */
    [[gnu::pure, nodiscard]] auto CreateCall(
        FlowPtr<Expr> callee, const std::unordered_map<std::variant<string, size_t>, FlowPtr<Expr>>& named_args,
        SourceLocation dbgsrc = SourceLocation::current()) -> std::optional<FlowPtr<Call>>;

    /**
     * @brief Construct a new call expression object
     * @param pos_args The positional arguments of the call expression
     * @param callee The callee of the call expression
     * @return FlowPtr<Call> A pointer to the newly created call expression object
     * @note This function is thread-safe
     */
    [[gnu::pure, nodiscard]] auto CreateCall(const std::vector<FlowPtr<Expr>>& pos_args, FlowPtr<Expr> callee,
                                             SourceLocation dbgsrc = SourceLocation::current()) -> FlowPtr<Call>;

    /**
     * @brief Construct a new call expression object
     * @param pos_args The positional arguments of the call expression
     * @param callee The callee of the call expression
     * @return FlowPtr<Call> A pointer to the newly created call expression object
     * @note This function is thread-safe
     */
    [[gnu::pure, nodiscard]] auto CreateCall(std::span<FlowPtr<Expr>> pos_args, FlowPtr<Expr> callee,
                                             SourceLocation dbgsrc = SourceLocation::current()) -> FlowPtr<Call>;

    /**
     * @brief Construct a new list expression object
     * @param ele The elements of the list expression
     * @return FlowPtr<List> A pointer to the newly created list expression object
     * @note This function is thread-safe
     */
    [[gnu::pure, nodiscard]] auto CreateList(std::span<FlowPtr<Expr>> ele,
                                             SourceLocation dbgsrc = SourceLocation::current()) -> FlowPtr<List>;

    /**
     * @brief Construct a new list expression object
     * @param ele The elements of the list expression
     * @return FlowPtr<List> A pointer to the newly created list expression object
     * @note This function is thread-safe
     */
    [[gnu::pure, nodiscard]] auto CreateList(const std::vector<FlowPtr<Expr>>& ele,
                                             SourceLocation dbgsrc = SourceLocation::current()) -> FlowPtr<List>;

    /**
     * @brief Construct a new association expression object
     * @param key The key of the association expression
     * @param x The value of the association expression
     * @return FlowPtr<Assoc> A pointer to the newly created association expression object
     * @note This function is thread-safe
     */
    [[gnu::pure, nodiscard]] auto CreateAssoc(FlowPtr<Expr> key, FlowPtr<Expr> x,
                                              SourceLocation dbgsrc = SourceLocation::current()) -> FlowPtr<Assoc>;

    /**
     * @brief Construct a new index expression object
     * @param base The base of the index expression
     * @param index The index of the index expression
     * @return FlowPtr<Index> A pointer to the newly created index expression object
     * @note This function is thread-safe
     */
    [[gnu::pure, nodiscard]] auto CreateIndex(FlowPtr<Expr> base, FlowPtr<Expr> index,
                                              SourceLocation dbgsrc = SourceLocation::current()) -> FlowPtr<Index>;

    /**
     * @brief Construct a new slice expression object
     * @param base The base of the slice expression
     * @param start The start of the slice expression
     * @param end The end of the slice expression
     * @return FlowPtr<Slice> A pointer to the newly created slice expression object
     * @note This function is thread-safe
     */
    [[gnu::pure, nodiscard]] auto CreateSlice(FlowPtr<Expr> base, FlowPtr<Expr> start, FlowPtr<Expr> end,
                                              SourceLocation dbgsrc = SourceLocation::current()) -> FlowPtr<Slice>;

    /**
     * @brief Construct a new format string expression object
     * @param x Plaintext string context
     * @return FlowPtr<FString> A pointer to the newly created format string expression object
     * @note This function is thread-safe
     */
    [[gnu::pure, nodiscard]] auto CreateFormatString(string x, SourceLocation dbgsrc = SourceLocation::current())
        -> FlowPtr<FString>;

    /**
     * @brief Construct a new format string expression object
     * @param parts The parts of the format string expression
     * @return FlowPtr<FString> A pointer to the newly created format string expression object
     * @note This function is thread-safe
     */
    [[gnu::pure, nodiscard]] auto CreateFormatString(std::span<std::variant<FlowPtr<Expr>, string>> parts,
                                                     SourceLocation dbgsrc = SourceLocation::current())
        -> FlowPtr<FString>;

    /**
     * @brief Construct a new format string expression object
     * @param parts The parts of the format string expression
     * @return FlowPtr<FString> A pointer to the newly created format string expression object
     * @note This function is thread-safe
     */
    [[gnu::pure, nodiscard]] auto CreateFormatString(const std::vector<std::variant<FlowPtr<Expr>, string>>& parts,
                                                     SourceLocation dbgsrc = SourceLocation::current())
        -> FlowPtr<FString>;

    /**
     * @brief Construct a new identifier expression object
     * @param name The name of the identifier expression
     * @return FlowPtr<Identifier> A pointer to the newly created identifier expression object
     * @note This function is thread-safe
     */
    [[gnu::pure, nodiscard]] auto CreateIdentifier(string name, SourceLocation dbgsrc = SourceLocation::current())
        -> FlowPtr<Identifier>;

    /**
     * @brief Construct a new sequence expression object
     * @param ele The elements of the sequence expression
     * @return FlowPtr<Sequence> A pointer to the newly created sequence expression object
     * @note This function is thread-safe
     */
    [[gnu::pure, nodiscard]] auto CreateSequence(
        std::span<FlowPtr<Expr>> ele, SourceLocation dbgsrc = SourceLocation::current()) -> FlowPtr<Sequence>;

    /**
     * @brief Construct a new sequence expression object
     * @param ele The elements of the sequence expression
     * @return FlowPtr<Sequence> A pointer to the newly created sequence expression object
     * @note This function is thread-safe
     */
    [[gnu::pure, nodiscard]] auto CreateSequence(
        const std::vector<FlowPtr<Expr>>& ele, SourceLocation dbgsrc = SourceLocation::current()) -> FlowPtr<Sequence>;

    /// TODO: Add support for creating a new lambda expression object
    [[gnu::pure, nodiscard]] auto CreateLambdaExpr(SourceLocation dbgsrc = SourceLocation::current())
        -> FlowPtr<LambdaExpr>;

    /// TODO: Add support for creating a new template expression object
    [[gnu::pure, nodiscard]] auto CreateTemplateCall(SourceLocation dbgsrc = SourceLocation::current())
        -> FlowPtr<TemplateCall>;

    ///=========================================================================
    /// TYPES

    /**
     * @brief Construct a new reference type object
     * @param to The type that this reference points to
     * @param volatil Whether or not this reference is volatile
     * @param bits The bits of this reference type itself
     * @param min The minimum value that this reference can hold
     * @param max The maximum value that this reference can hold
     * @return FlowPtr<RefTy> A pointer to the newly created reference type object
     * @note This function is thread-safe
     */
    [[gnu::pure, nodiscard]] auto CreateRefTy(FlowPtr<Type> to, bool volatil = false,
                                              NullableFlowPtr<Expr> bits = nullptr, NullableFlowPtr<Expr> min = nullptr,
                                              NullableFlowPtr<Expr> max = nullptr,
                                              SourceLocation dbgsrc = SourceLocation::current()) -> FlowPtr<RefTy>;

    /**
     * @brief Construct a new 1-bit unsigned integer type object
     * @param bits The bits of this type
     * @param min The minimum value that this type can hold
     * @param max The maximum value that this type can hold
     * @return FlowPtr<U1> A pointer to the newly created 1-bit unsigned integer type object
     * @note This function is thread-safe
     */
    [[gnu::pure, nodiscard]] auto CreateU1(NullableFlowPtr<Expr> bits = nullptr, NullableFlowPtr<Expr> min = nullptr,
                                           NullableFlowPtr<Expr> max = nullptr,
                                           SourceLocation dbgsrc = SourceLocation::current()) -> FlowPtr<U1>;

    /**
     * @brief Construct a new 8-bit unsigned integer type object
     * @param bits The bits of this type
     * @param min The minimum value that this type can hold
     * @param max The maximum value that this type can hold
     * @return FlowPtr<U8> A pointer to the newly created 8-bit unsigned integer type object
     * @note This function is thread-safe
     */
    [[gnu::pure, nodiscard]] auto CreateU8(NullableFlowPtr<Expr> bits = nullptr, NullableFlowPtr<Expr> min = nullptr,
                                           NullableFlowPtr<Expr> max = nullptr,
                                           SourceLocation dbgsrc = SourceLocation::current()) -> FlowPtr<U8>;

    /**
     * @brief Construct a new 16-bit unsigned integer type object
     * @param bits The bits of this type
     * @param min The minimum value that this type can hold
     * @param max The maximum value that this type can hold
     * @return FlowPtr<U16> A pointer to the newly created 16-bit unsigned integer type object
     * @note This function is thread-safe
     */
    [[gnu::pure, nodiscard]] auto CreateU16(NullableFlowPtr<Expr> bits = nullptr, NullableFlowPtr<Expr> min = nullptr,
                                            NullableFlowPtr<Expr> max = nullptr,
                                            SourceLocation dbgsrc = SourceLocation::current()) -> FlowPtr<U16>;

    /**
     * @brief Construct a new 32-bit unsigned integer type object
     * @param bits The bits of this type
     * @param min The minimum value that this type can hold
     * @param max The maximum value that this type can hold
     * @return FlowPtr<U32> A pointer to the newly created 32-bit unsigned integer type object
     * @note This function is thread-safe
     */
    [[gnu::pure, nodiscard]] auto CreateU32(NullableFlowPtr<Expr> bits = nullptr, NullableFlowPtr<Expr> min = nullptr,
                                            NullableFlowPtr<Expr> max = nullptr,
                                            SourceLocation dbgsrc = SourceLocation::current()) -> FlowPtr<U32>;

    /**
     * @brief Construct a new 64-bit unsigned integer type object
     * @param bits The bits of this type
     * @param min The minimum value that this type can hold
     * @param max The maximum value that this type can hold
     * @return FlowPtr<U64> A pointer to the newly created 64-bit unsigned integer type object
     * @note This function is thread-safe
     */
    [[gnu::pure, nodiscard]] auto CreateU64(NullableFlowPtr<Expr> bits = nullptr, NullableFlowPtr<Expr> min = nullptr,
                                            NullableFlowPtr<Expr> max = nullptr,
                                            SourceLocation dbgsrc = SourceLocation::current()) -> FlowPtr<U64>;

    /**
     * @brief Construct a new 128-bit unsigned integer type object
     * @param bits The bits of this type
     * @param min The minimum value that this type can hold
     * @param max The maximum value that this type can hold
     * @return FlowPtr<U128> A pointer to the newly created 128-bit unsigned integer type object
     * @note This function is thread-safe
     */
    [[gnu::pure, nodiscard]] auto CreateU128(NullableFlowPtr<Expr> bits = nullptr, NullableFlowPtr<Expr> min = nullptr,
                                             NullableFlowPtr<Expr> max = nullptr,
                                             SourceLocation dbgsrc = SourceLocation::current()) -> FlowPtr<U128>;

    /**
     * @brief Construct a new 8-bit signed integer type object
     * @param bits The bits of this type
     * @param min The minimum value that this type can hold
     * @param max The maximum value that this type can hold
     * @return FlowPtr<I1> A pointer to the newly created 8-bit signed integer type object
     * @note This function is thread-safe
     */
    [[gnu::pure, nodiscard]] auto CreateI8(NullableFlowPtr<Expr> bits = nullptr, NullableFlowPtr<Expr> min = nullptr,
                                           NullableFlowPtr<Expr> max = nullptr,
                                           SourceLocation dbgsrc = SourceLocation::current()) -> FlowPtr<I8>;

    /**
     * @brief Construct a new 16-bit signed integer type object
     * @param bits The bits of this type
     * @param min The minimum value that this type can hold
     * @param max The maximum value that this type can hold
     * @return FlowPtr<I16> A pointer to the newly created 16-bit signed integer type object
     * @note This function is thread-safe
     */
    [[gnu::pure, nodiscard]] auto CreateI16(NullableFlowPtr<Expr> bits = nullptr, NullableFlowPtr<Expr> min = nullptr,
                                            NullableFlowPtr<Expr> max = nullptr,
                                            SourceLocation dbgsrc = SourceLocation::current()) -> FlowPtr<I16>;

    /**
     * @brief Construct a new 32-bit signed integer type object
     * @param bits The bits of this type
     * @param min The minimum value that this type can hold
     * @param max The maximum value that this type can hold
     * @return FlowPtr<I32> A pointer to the newly created 32-bit signed integer type object
     * @note This function is thread-safe
     */
    [[gnu::pure, nodiscard]] auto CreateI32(NullableFlowPtr<Expr> bits = nullptr, NullableFlowPtr<Expr> min = nullptr,
                                            NullableFlowPtr<Expr> max = nullptr,
                                            SourceLocation dbgsrc = SourceLocation::current()) -> FlowPtr<I32>;

    /**
     * @brief Construct a new 64-bit signed integer type object
     * @param bits The bits of this type
     * @param min The minimum value that this type can hold
     * @param max The maximum value that this type can hold
     * @return FlowPtr<I64> A pointer to the newly created 64-bit signed integer type object
     * @note This function is thread-safe
     */
    [[gnu::pure, nodiscard]] auto CreateI64(NullableFlowPtr<Expr> bits = nullptr, NullableFlowPtr<Expr> min = nullptr,
                                            NullableFlowPtr<Expr> max = nullptr,
                                            SourceLocation dbgsrc = SourceLocation::current()) -> FlowPtr<I64>;

    /**
     * @brief Construct a new 128-bit signed integer type object
     * @param bits The bits of this type
     * @param min The minimum value that this type can hold
     * @param max The maximum value that this type can hold
     * @return FlowPtr<I128> A pointer to the newly created 128-bit signed integer type object
     * @note This function is thread-safe
     */
    [[gnu::pure, nodiscard]] auto CreateI128(NullableFlowPtr<Expr> bits = nullptr, NullableFlowPtr<Expr> min = nullptr,
                                             NullableFlowPtr<Expr> max = nullptr,
                                             SourceLocation dbgsrc = SourceLocation::current()) -> FlowPtr<I128>;

    /**
     * @brief Construct a new 16-bit floating-point type object
     * @param bits The bits of this type
     * @param min The minimum value that this type can hold
     * @param max The maximum value that this type can hold
     * @return FlowPtr<F16> A pointer to the newly created 16-bit floating-point type object
     * @note This function is thread-safe
     */
    [[gnu::pure, nodiscard]] auto CreateF16(NullableFlowPtr<Expr> bits = nullptr, NullableFlowPtr<Expr> min = nullptr,
                                            NullableFlowPtr<Expr> max = nullptr,
                                            SourceLocation dbgsrc = SourceLocation::current()) -> FlowPtr<F16>;

    /**
     * @brief Construct a new 32-bit floating-point type object
     * @param bits The bits of this type
     * @param min The minimum value that this type can hold
     * @param max The maximum value that this type can hold
     * @return FlowPtr<F32> A pointer to the newly created 32-bit floating-point type object
     * @note This function is thread-safe
     */
    [[gnu::pure, nodiscard]] auto CreateF32(NullableFlowPtr<Expr> bits = nullptr, NullableFlowPtr<Expr> min = nullptr,
                                            NullableFlowPtr<Expr> max = nullptr,
                                            SourceLocation dbgsrc = SourceLocation::current()) -> FlowPtr<F32>;

    /**
     * @brief Construct a new 64-bit floating-point type object
     * @param bits The bits of this type
     * @param min The minimum value that this type can hold
     * @param max The maximum value that this type can hold
     * @return FlowPtr<F64> A pointer to the newly created 64-bit floating-point type object
     * @note This function is thread-safe
     */
    [[gnu::pure, nodiscard]] auto CreateF64(NullableFlowPtr<Expr> bits = nullptr, NullableFlowPtr<Expr> min = nullptr,
                                            NullableFlowPtr<Expr> max = nullptr,
                                            SourceLocation dbgsrc = SourceLocation::current()) -> FlowPtr<F64>;

    /**
     * @brief Construct a new 128-bit floating-point type object
     * @param bits The bits of this type
     * @param min The minimum value that this type can hold
     * @param max The maximum value that this type can hold
     * @return FlowPtr<F128> A pointer to the newly created 128-bit floating-point type object
     * @note This function is thread-safe
     */
    [[gnu::pure, nodiscard]] auto CreateF128(NullableFlowPtr<Expr> bits = nullptr, NullableFlowPtr<Expr> min = nullptr,
                                             NullableFlowPtr<Expr> max = nullptr,
                                             SourceLocation dbgsrc = SourceLocation::current()) -> FlowPtr<F128>;

    /**
     * @brief Construct a new void type object
     * @param bits The bits of this type
     * @param min The minimum value that this type can hold
     * @param max The maximum value that this type can hold
     * @return FlowPtr<VoidTy> A pointer to the newly created void type object
     * @note This function is thread-safe
     */
    [[gnu::pure, nodiscard]] auto CreateVoidTy(NullableFlowPtr<Expr> bits = nullptr,
                                               NullableFlowPtr<Expr> min = nullptr, NullableFlowPtr<Expr> max = nullptr,
                                               SourceLocation dbgsrc = SourceLocation::current()) -> FlowPtr<VoidTy>;

    /**
     * @brief Construct a new pointer type object
     * @param to The type that this pointer points to
     * @param volatil Whether or not this pointer is volatile
     * @param bits The bits of this pointer type itself
     * @param min The minimum value that this pointer can hold
     * @param max The maximum value that this pointer can hold
     * @return FlowPtr<PtrTy> A pointer to the newly created pointer type object
     * @note This function is thread-safe
     */
    [[gnu::pure, nodiscard]] auto CreatePtrTy(FlowPtr<Type> to, bool volatil = false,
                                              NullableFlowPtr<Expr> bits = nullptr, NullableFlowPtr<Expr> min = nullptr,
                                              NullableFlowPtr<Expr> max = nullptr,
                                              SourceLocation dbgsrc = SourceLocation::current()) -> FlowPtr<PtrTy>;

    /**
     * @brief Construct a new opaque type object
     * @param name The name of this opaque type
     * @param bits The bits of this type
     * @param min The minimum value that this type can hold
     * @param max The maximum value that this type can hold
     * @return FlowPtr<OpaqueTy> A pointer to the newly created opaque type object
     * @note This function is thread-safe
     */
    [[gnu::pure, nodiscard]] auto CreateOpaqueTy(
        string name, NullableFlowPtr<Expr> bits = nullptr, NullableFlowPtr<Expr> min = nullptr,
        NullableFlowPtr<Expr> max = nullptr, SourceLocation dbgsrc = SourceLocation::current()) -> FlowPtr<OpaqueTy>;

    /**
     * @brief Construct a new array type object
     * @param element_type The type of the elements in this array
     * @param element_count The number of elements in this array
     * @param bits The bits of this type
     * @param min The minimum value that this type can hold
     * @param max The maximum value that this type can hold
     * @return FlowPtr<ArrayTy> A pointer to the newly created array type object
     * @note This function is thread-safe
     */
    [[gnu::pure, nodiscard]] auto CreateArrayTy(FlowPtr<Type> element_type, FlowPtr<Expr> element_count,
                                                NullableFlowPtr<Expr> bits = nullptr,
                                                NullableFlowPtr<Expr> min = nullptr,
                                                NullableFlowPtr<Expr> max = nullptr,
                                                SourceLocation dbgsrc = SourceLocation::current()) -> FlowPtr<ArrayTy>;

    /**
     * @brief Construct a new tuple type object
     * @param ele The types of the elements in this tuple
     * @param bits The bits of this type
     * @param min The minimum value that this type can hold
     * @param max The maximum value that this type can hold
     * @return FlowPtr<TupleTy> A pointer to the newly created tuple type object
     * @note This function is thread-safe
     */
    [[gnu::pure, nodiscard]] auto CreateTupleTy(std::span<FlowPtr<Type>> ele, NullableFlowPtr<Expr> bits = nullptr,
                                                NullableFlowPtr<Expr> min = nullptr,
                                                NullableFlowPtr<Expr> max = nullptr,
                                                SourceLocation dbgsrc = SourceLocation::current()) -> FlowPtr<TupleTy>;

    /**
     * @brief Construct a new tuple type object
     * @param ele The types of the elements in this tuple
     * @param bits The bits of this type
     * @param min The minimum value that this type can hold
     * @param max The maximum value that this type can hold
     * @return FlowPtr<TupleTy> A pointer to the newly created tuple type object
     * @note This function is thread-safe
     */
    [[gnu::pure, nodiscard]] auto CreateTupleTy(const std::vector<FlowPtr<Type>>& ele,
                                                NullableFlowPtr<Expr> bits = nullptr,
                                                NullableFlowPtr<Expr> min = nullptr,
                                                NullableFlowPtr<Expr> max = nullptr,
                                                SourceLocation dbgsrc = SourceLocation::current()) -> FlowPtr<TupleTy>;

    /// TODO: Add support for creating a new function type object
    [[gnu::pure, nodiscard]] auto CreateFuncTy(SourceLocation dbgsrc = SourceLocation::current()) -> FlowPtr<FuncTy>;

    /**
     * @brief Construct a new unresolved named type object
     * @param name The name of this named type
     * @param bits The bits of this type
     * @param min The minimum value that this type can hold
     * @param max The maximum value that this type can hold
     * @return FlowPtr<NamedTy> A pointer to the newly created named type object
     * @note This function is thread-safe
     */
    [[gnu::pure, nodiscard]] auto CreateNamedTy(string name, NullableFlowPtr<Expr> bits = nullptr,
                                                NullableFlowPtr<Expr> min = nullptr,
                                                NullableFlowPtr<Expr> max = nullptr,
                                                SourceLocation dbgsrc = SourceLocation::current()) -> FlowPtr<NamedTy>;

    /**
     * @brief Construct a new inferred type object
     * @param bits The bits of this type
     * @param min The minimum value that this type can hold
     * @param max The maximum value that this type can hold
     * @return FlowPtr<InferTy> A pointer to the newly created inferred type object
     * @note This function is thread-safe
     */
    [[gnu::pure, nodiscard]] auto CreateInferTy(NullableFlowPtr<Expr> bits = nullptr,
                                                NullableFlowPtr<Expr> min = nullptr,
                                                NullableFlowPtr<Expr> max = nullptr,
                                                SourceLocation dbgsrc = SourceLocation::current()) -> FlowPtr<InferTy>;

    /// TODO: Add support for creating a new template type object
    [[gnu::pure, nodiscard]] auto CreateTemplateType(SourceLocation dbgsrc = SourceLocation::current())
        -> FlowPtr<TemplateType>;

    ///=========================================================================
    /// STATEMENTS
    [[gnu::pure, nodiscard]] auto CreateTypedef() -> FlowPtr<Typedef>;
    [[gnu::pure, nodiscard]] auto CreateStruct() -> FlowPtr<Struct>;
    [[gnu::pure, nodiscard]] auto CreateEnum() -> FlowPtr<Enum>;
    [[gnu::pure, nodiscard]] auto CreateFunction() -> FlowPtr<Function>;
    [[gnu::pure, nodiscard]] auto CreateScope() -> FlowPtr<Scope>;
    [[gnu::pure, nodiscard]] auto CreateExport() -> FlowPtr<Export>;
    [[gnu::pure, nodiscard]] auto CreateBlock() -> FlowPtr<Block>;
    [[gnu::pure, nodiscard]] auto CreateVariable() -> FlowPtr<Variable>;
    [[gnu::pure, nodiscard]] auto CreateAssembly() -> FlowPtr<Assembly>;
    [[gnu::pure, nodiscard]] auto CreateReturn() -> FlowPtr<Return>;
    [[gnu::pure, nodiscard]] auto CreateReturnIf() -> FlowPtr<ReturnIf>;
    [[gnu::pure, nodiscard]] auto CreateBreak() -> FlowPtr<Break>;
    [[gnu::pure, nodiscard]] auto CreateContinue() -> FlowPtr<Continue>;
    [[gnu::pure, nodiscard]] auto CreateIf() -> FlowPtr<If>;
    [[gnu::pure, nodiscard]] auto CreateWhile() -> FlowPtr<While>;
    [[gnu::pure, nodiscard]] auto CreateFor() -> FlowPtr<For>;
    [[gnu::pure, nodiscard]] auto CreateForeach() -> FlowPtr<Foreach>;
    [[gnu::pure, nodiscard]] auto CreateCase() -> FlowPtr<Case>;
    [[gnu::pure, nodiscard]] auto CreateSwitch() -> FlowPtr<Switch>;
    [[gnu::pure, nodiscard]] auto CreateExprStmt() -> FlowPtr<ExprStmt>;

    ///=========================================================================
    /// STATIC FACTORY FUNCTIONS

    [[gnu::pure, nodiscard]] static inline auto CreateBase(IMemory& m,
                                                           SourceLocation dbgsrc = SourceLocation::current()) {
      return ASTFactory(m).CreateBase(dbgsrc);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateBinary(IMemory& m, FlowPtr<Expr> lhs, lex::Operator op,
                                                             FlowPtr<Expr> rhs,
                                                             SourceLocation dbgsrc = SourceLocation::current()) {
      return ASTFactory(m).CreateBinary(std::move(lhs), op, std::move(rhs), dbgsrc);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateUnary(IMemory& m, lex::Operator op, FlowPtr<Expr> rhs,
                                                            SourceLocation dbgsrc = SourceLocation::current()) {
      return ASTFactory(m).CreateUnary(op, std::move(rhs), dbgsrc);
    }

    [[gnu::pure, nodiscard]] static inline auto CreatePostUnary(IMemory& m, FlowPtr<Expr> lhs, lex::Operator op,
                                                                SourceLocation dbgsrc = SourceLocation::current()) {
      return ASTFactory(m).CreatePostUnary(std::move(lhs), op, dbgsrc);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateTernary(IMemory& m, FlowPtr<Expr> condition, FlowPtr<Expr> then,
                                                              FlowPtr<Expr> ele,
                                                              SourceLocation dbgsrc = SourceLocation::current()) {
      return ASTFactory(m).CreateTernary(std::move(condition), std::move(then), std::move(ele), dbgsrc);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateInteger(IMemory& m, const boost::multiprecision::uint128_type& x,
                                                              SourceLocation dbgsrc = SourceLocation::current()) {
      return ASTFactory(m).CreateInteger(x, dbgsrc);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateInteger(IMemory& m, string x,
                                                              SourceLocation dbgsrc = SourceLocation::current()) {
      return ASTFactory(m).CreateInteger(x, dbgsrc);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateInteger(IMemory& m, const boost::multiprecision::cpp_int& x,
                                                              SourceLocation dbgsrc = SourceLocation::current()) {
      return ASTFactory(m).CreateInteger(x, dbgsrc);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateFloat(IMemory& m, double x,
                                                            SourceLocation dbgsrc = SourceLocation::current()) {
      return ASTFactory(m).CreateFloat(x, dbgsrc);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateFloat(IMemory& m, string x,
                                                            SourceLocation dbgsrc = SourceLocation::current()) {
      return ASTFactory(m).CreateFloat(x, dbgsrc);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateString(IMemory& m, string x,
                                                             SourceLocation dbgsrc = SourceLocation::current()) {
      return ASTFactory(m).CreateString(x, dbgsrc);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateCharacter(IMemory& m, char8_t x,
                                                                SourceLocation dbgsrc = SourceLocation::current()) {
      return ASTFactory(m).CreateCharacter(x, dbgsrc);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateBoolean(IMemory& m, bool x,
                                                              SourceLocation dbgsrc = SourceLocation::current()) {
      return ASTFactory(m).CreateBoolean(x, dbgsrc);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateNull(IMemory& m,
                                                           SourceLocation dbgsrc = SourceLocation::current()) {
      return ASTFactory(m).CreateNull(dbgsrc);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateUndefined(IMemory& m,
                                                                SourceLocation dbgsrc = SourceLocation::current()) {
      return ASTFactory(m).CreateUndefined(dbgsrc);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateCall(
        IMemory& m, FlowPtr<Expr> callee,
        const std::unordered_map<std::variant<string, size_t>, FlowPtr<Expr>>& named_args,
        SourceLocation dbgsrc = SourceLocation::current()) {
      return ASTFactory(m).CreateCall(std::move(callee), named_args, dbgsrc);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateCall(IMemory& m, const std::vector<FlowPtr<Expr>>& pos_args,
                                                           FlowPtr<Expr> callee,
                                                           SourceLocation dbgsrc = SourceLocation::current()) {
      return ASTFactory(m).CreateCall(pos_args, std::move(callee), dbgsrc);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateCall(IMemory& m, std::span<FlowPtr<Expr>> pos_args,
                                                           FlowPtr<Expr> callee,
                                                           SourceLocation dbgsrc = SourceLocation::current()) {
      return ASTFactory(m).CreateCall(pos_args, std::move(callee), dbgsrc);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateList(IMemory& m, std::span<FlowPtr<Expr>> ele,
                                                           SourceLocation dbgsrc = SourceLocation::current()) {
      return ASTFactory(m).CreateList(ele, dbgsrc);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateList(IMemory& m, const std::vector<FlowPtr<Expr>>& ele,
                                                           SourceLocation dbgsrc = SourceLocation::current()) {
      return ASTFactory(m).CreateList(ele, dbgsrc);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateAssoc(IMemory& m, FlowPtr<Expr> key, FlowPtr<Expr> x,
                                                            SourceLocation dbgsrc = SourceLocation::current()) {
      return ASTFactory(m).CreateAssoc(std::move(key), std::move(x), dbgsrc);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateIndex(IMemory& m, FlowPtr<Expr> base, FlowPtr<Expr> index,
                                                            SourceLocation dbgsrc = SourceLocation::current()) {
      return ASTFactory(m).CreateIndex(std::move(base), std::move(index), dbgsrc);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateSlice(IMemory& m, FlowPtr<Expr> base, FlowPtr<Expr> start,
                                                            FlowPtr<Expr> end,
                                                            SourceLocation dbgsrc = SourceLocation::current()) {
      return ASTFactory(m).CreateSlice(std::move(base), std::move(start), std::move(end), dbgsrc);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateFormatString(IMemory& m, string x,
                                                                   SourceLocation dbgsrc = SourceLocation::current()) {
      return ASTFactory(m).CreateFormatString(x, dbgsrc);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateFormatString(IMemory& m,
                                                                   std::span<std::variant<FlowPtr<Expr>, string>> parts,
                                                                   SourceLocation dbgsrc = SourceLocation::current()) {
      return ASTFactory(m).CreateFormatString(parts, dbgsrc);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateFormatString(
        IMemory& m, const std::vector<std::variant<FlowPtr<Expr>, string>>& parts,
        SourceLocation dbgsrc = SourceLocation::current()) {
      return ASTFactory(m).CreateFormatString(parts, dbgsrc);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateIdentifier(IMemory& m, string name,
                                                                 SourceLocation dbgsrc = SourceLocation::current()) {
      return ASTFactory(m).CreateIdentifier(name, dbgsrc);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateSequence(IMemory& m, std::span<FlowPtr<Expr>> ele,
                                                               SourceLocation dbgsrc = SourceLocation::current()) {
      return ASTFactory(m).CreateSequence(ele, dbgsrc);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateSequence(IMemory& m, const std::vector<FlowPtr<Expr>>& ele,
                                                               SourceLocation dbgsrc = SourceLocation::current()) {
      return ASTFactory(m).CreateSequence(ele, dbgsrc);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateLambdaExpr(IMemory& m,
                                                                 SourceLocation dbgsrc = SourceLocation::current()) {
      return ASTFactory(m).CreateLambdaExpr(dbgsrc);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateTemplateCall(IMemory& m,
                                                                   SourceLocation dbgsrc = SourceLocation::current()) {
      return ASTFactory(m).CreateTemplateCall(dbgsrc);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateRefTy(IMemory& m, FlowPtr<Type> to, bool volatil = false,
                                                            NullableFlowPtr<Expr> bits = nullptr,
                                                            NullableFlowPtr<Expr> min = nullptr,
                                                            NullableFlowPtr<Expr> max = nullptr,
                                                            SourceLocation dbgsrc = SourceLocation::current()) {
      return ASTFactory(m).CreateRefTy(std::move(to), volatil, std::move(bits), std::move(min), std::move(max), dbgsrc);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateU1(IMemory& m, NullableFlowPtr<Expr> bits = nullptr,
                                                         NullableFlowPtr<Expr> min = nullptr,
                                                         NullableFlowPtr<Expr> max = nullptr,
                                                         SourceLocation dbgsrc = SourceLocation::current()) {
      return ASTFactory(m).CreateU1(std::move(bits), std::move(min), std::move(max), dbgsrc);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateU8(IMemory& m, NullableFlowPtr<Expr> bits = nullptr,
                                                         NullableFlowPtr<Expr> min = nullptr,
                                                         NullableFlowPtr<Expr> max = nullptr,
                                                         SourceLocation dbgsrc = SourceLocation::current()) {
      return ASTFactory(m).CreateU8(std::move(bits), std::move(min), std::move(max), dbgsrc);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateU16(IMemory& m, NullableFlowPtr<Expr> bits = nullptr,
                                                          NullableFlowPtr<Expr> min = nullptr,
                                                          NullableFlowPtr<Expr> max = nullptr,
                                                          SourceLocation dbgsrc = SourceLocation::current()) {
      return ASTFactory(m).CreateU16(std::move(bits), std::move(min), std::move(max), dbgsrc);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateU32(IMemory& m, NullableFlowPtr<Expr> bits = nullptr,
                                                          NullableFlowPtr<Expr> min = nullptr,
                                                          NullableFlowPtr<Expr> max = nullptr,
                                                          SourceLocation dbgsrc = SourceLocation::current()) {
      return ASTFactory(m).CreateU32(std::move(bits), std::move(min), std::move(max), dbgsrc);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateU64(IMemory& m, NullableFlowPtr<Expr> bits = nullptr,
                                                          NullableFlowPtr<Expr> min = nullptr,
                                                          NullableFlowPtr<Expr> max = nullptr,
                                                          SourceLocation dbgsrc = SourceLocation::current()) {
      return ASTFactory(m).CreateU64(std::move(bits), std::move(min), std::move(max), dbgsrc);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateU128(IMemory& m, NullableFlowPtr<Expr> bits = nullptr,
                                                           NullableFlowPtr<Expr> min = nullptr,
                                                           NullableFlowPtr<Expr> max = nullptr,
                                                           SourceLocation dbgsrc = SourceLocation::current()) {
      return ASTFactory(m).CreateU128(std::move(bits), std::move(min), std::move(max), dbgsrc);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateI8(IMemory& m, NullableFlowPtr<Expr> bits = nullptr,
                                                         NullableFlowPtr<Expr> min = nullptr,
                                                         NullableFlowPtr<Expr> max = nullptr,
                                                         SourceLocation dbgsrc = SourceLocation::current()) {
      return ASTFactory(m).CreateI8(std::move(bits), std::move(min), std::move(max), dbgsrc);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateI16(IMemory& m, NullableFlowPtr<Expr> bits = nullptr,
                                                          NullableFlowPtr<Expr> min = nullptr,
                                                          NullableFlowPtr<Expr> max = nullptr,
                                                          SourceLocation dbgsrc = SourceLocation::current()) {
      return ASTFactory(m).CreateI16(std::move(bits), std::move(min), std::move(max), dbgsrc);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateI32(IMemory& m, NullableFlowPtr<Expr> bits = nullptr,
                                                          NullableFlowPtr<Expr> min = nullptr,
                                                          NullableFlowPtr<Expr> max = nullptr,
                                                          SourceLocation dbgsrc = SourceLocation::current()) {
      return ASTFactory(m).CreateI32(std::move(bits), std::move(min), std::move(max), dbgsrc);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateI64(IMemory& m, NullableFlowPtr<Expr> bits = nullptr,
                                                          NullableFlowPtr<Expr> min = nullptr,
                                                          NullableFlowPtr<Expr> max = nullptr,
                                                          SourceLocation dbgsrc = SourceLocation::current()) {
      return ASTFactory(m).CreateI64(std::move(bits), std::move(min), std::move(max), dbgsrc);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateI128(IMemory& m, NullableFlowPtr<Expr> bits = nullptr,
                                                           NullableFlowPtr<Expr> min = nullptr,
                                                           NullableFlowPtr<Expr> max = nullptr,
                                                           SourceLocation dbgsrc = SourceLocation::current()) {
      return ASTFactory(m).CreateI128(std::move(bits), std::move(min), std::move(max), dbgsrc);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateF16(IMemory& m, NullableFlowPtr<Expr> bits = nullptr,
                                                          NullableFlowPtr<Expr> min = nullptr,
                                                          NullableFlowPtr<Expr> max = nullptr,
                                                          SourceLocation dbgsrc = SourceLocation::current()) {
      return ASTFactory(m).CreateF16(std::move(bits), std::move(min), std::move(max), dbgsrc);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateF32(IMemory& m, NullableFlowPtr<Expr> bits = nullptr,
                                                          NullableFlowPtr<Expr> min = nullptr,
                                                          NullableFlowPtr<Expr> max = nullptr,
                                                          SourceLocation dbgsrc = SourceLocation::current()) {
      return ASTFactory(m).CreateF32(std::move(bits), std::move(min), std::move(max), dbgsrc);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateF64(IMemory& m, NullableFlowPtr<Expr> bits = nullptr,
                                                          NullableFlowPtr<Expr> min = nullptr,
                                                          NullableFlowPtr<Expr> max = nullptr,
                                                          SourceLocation dbgsrc = SourceLocation::current()) {
      return ASTFactory(m).CreateF64(std::move(bits), std::move(min), std::move(max), dbgsrc);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateF128(IMemory& m, NullableFlowPtr<Expr> bits = nullptr,
                                                           NullableFlowPtr<Expr> min = nullptr,
                                                           NullableFlowPtr<Expr> max = nullptr,
                                                           SourceLocation dbgsrc = SourceLocation::current()) {
      return ASTFactory(m).CreateF128(std::move(bits), std::move(min), std::move(max), dbgsrc);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateVoidTy(IMemory& m, NullableFlowPtr<Expr> bits = nullptr,
                                                             NullableFlowPtr<Expr> min = nullptr,
                                                             NullableFlowPtr<Expr> max = nullptr,
                                                             SourceLocation dbgsrc = SourceLocation::current()) {
      return ASTFactory(m).CreateVoidTy(std::move(bits), std::move(min), std::move(max), dbgsrc);
    }

    [[gnu::pure, nodiscard]] static inline auto CreatePtrTy(IMemory& m, FlowPtr<Type> to, bool volatil = false,
                                                            NullableFlowPtr<Expr> bits = nullptr,
                                                            NullableFlowPtr<Expr> min = nullptr,
                                                            NullableFlowPtr<Expr> max = nullptr,
                                                            SourceLocation dbgsrc = SourceLocation::current()) {
      return ASTFactory(m).CreatePtrTy(std::move(to), volatil, std::move(bits), std::move(min), std::move(max), dbgsrc);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateOpaqueTy(IMemory& m, string name,
                                                               NullableFlowPtr<Expr> bits = nullptr,
                                                               NullableFlowPtr<Expr> min = nullptr,
                                                               NullableFlowPtr<Expr> max = nullptr,
                                                               SourceLocation dbgsrc = SourceLocation::current()) {
      return ASTFactory(m).CreateOpaqueTy(name, std::move(bits), std::move(min), std::move(max), dbgsrc);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateArrayTy(IMemory& m, FlowPtr<Type> element_type,
                                                              FlowPtr<Expr> element_count,
                                                              NullableFlowPtr<Expr> bits = nullptr,
                                                              NullableFlowPtr<Expr> min = nullptr,
                                                              NullableFlowPtr<Expr> max = nullptr,
                                                              SourceLocation dbgsrc = SourceLocation::current()) {
      return ASTFactory(m).CreateArrayTy(std::move(element_type), std::move(element_count), std::move(bits),
                                         std::move(min), std::move(max), dbgsrc);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateTupleTy(IMemory& m, std::span<FlowPtr<Type>> ele,
                                                              NullableFlowPtr<Expr> bits = nullptr,
                                                              NullableFlowPtr<Expr> min = nullptr,
                                                              NullableFlowPtr<Expr> max = nullptr,
                                                              SourceLocation dbgsrc = SourceLocation::current()) {
      return ASTFactory(m).CreateTupleTy(ele, std::move(bits), std::move(min), std::move(max), dbgsrc);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateTupleTy(IMemory& m, const std::vector<FlowPtr<Type>>& ele,
                                                              NullableFlowPtr<Expr> bits = nullptr,
                                                              NullableFlowPtr<Expr> min = nullptr,
                                                              NullableFlowPtr<Expr> max = nullptr,
                                                              SourceLocation dbgsrc = SourceLocation::current()) {
      return ASTFactory(m).CreateTupleTy(ele, std::move(bits), std::move(min), std::move(max), dbgsrc);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateFuncTy(IMemory& m,
                                                             SourceLocation dbgsrc = SourceLocation::current()) {
      return ASTFactory(m).CreateFuncTy(dbgsrc);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateNamedTy(IMemory& m, string name,
                                                              NullableFlowPtr<Expr> bits = nullptr,
                                                              NullableFlowPtr<Expr> min = nullptr,
                                                              NullableFlowPtr<Expr> max = nullptr,
                                                              SourceLocation dbgsrc = SourceLocation::current()) {
      return ASTFactory(m).CreateNamedTy(name, std::move(bits), std::move(min), std::move(max), dbgsrc);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateInferTy(IMemory& m, NullableFlowPtr<Expr> bits = nullptr,
                                                              NullableFlowPtr<Expr> min = nullptr,
                                                              NullableFlowPtr<Expr> max = nullptr,
                                                              SourceLocation dbgsrc = SourceLocation::current()) {
      return ASTFactory(m).CreateInferTy(std::move(bits), std::move(min), std::move(max), dbgsrc);
    }

    [[gnu::pure, nodiscard]] static inline auto CreateTemplateType(IMemory& m,
                                                                   SourceLocation dbgsrc = SourceLocation::current()) {
      return ASTFactory(m).CreateTemplateType(dbgsrc);
    }

    ///=========================================================================
    /// STATEMENTS
    [[gnu::pure, nodiscard]] static inline auto CreateTypedef(IMemory& m) { return ASTFactory(m).CreateTypedef(); }
    [[gnu::pure, nodiscard]] static inline auto CreateStruct(IMemory& m) { return ASTFactory(m).CreateStruct(); }
    [[gnu::pure, nodiscard]] static inline auto CreateEnum(IMemory& m) { return ASTFactory(m).CreateEnum(); }
    [[gnu::pure, nodiscard]] static inline auto CreateFunction(IMemory& m) { return ASTFactory(m).CreateFunction(); }
    [[gnu::pure, nodiscard]] static inline auto CreateScope(IMemory& m) { return ASTFactory(m).CreateScope(); }
    [[gnu::pure, nodiscard]] static inline auto CreateExport(IMemory& m) { return ASTFactory(m).CreateExport(); }
    [[gnu::pure, nodiscard]] static inline auto CreateBlock(IMemory& m) { return ASTFactory(m).CreateBlock(); }
    [[gnu::pure, nodiscard]] static inline auto CreateVariable(IMemory& m) { return ASTFactory(m).CreateVariable(); }
    [[gnu::pure, nodiscard]] static inline auto CreateAssembly(IMemory& m) { return ASTFactory(m).CreateAssembly(); }
    [[gnu::pure, nodiscard]] static inline auto CreateReturn(IMemory& m) { return ASTFactory(m).CreateReturn(); }
    [[gnu::pure, nodiscard]] static inline auto CreateReturnIf(IMemory& m) { return ASTFactory(m).CreateReturnIf(); }
    [[gnu::pure, nodiscard]] static inline auto CreateBreak(IMemory& m) { return ASTFactory(m).CreateBreak(); }
    [[gnu::pure, nodiscard]] static inline auto CreateContinue(IMemory& m) { return ASTFactory(m).CreateContinue(); }
    [[gnu::pure, nodiscard]] static inline auto CreateIf(IMemory& m) { return ASTFactory(m).CreateIf(); }
    [[gnu::pure, nodiscard]] static inline auto CreateWhile(IMemory& m) { return ASTFactory(m).CreateWhile(); }
    [[gnu::pure, nodiscard]] static inline auto CreateFor(IMemory& m) { return ASTFactory(m).CreateFor(); }
    [[gnu::pure, nodiscard]] static inline auto CreateForeach(IMemory& m) { return ASTFactory(m).CreateForeach(); }
    [[gnu::pure, nodiscard]] static inline auto CreateCase(IMemory& m) { return ASTFactory(m).CreateCase(); }
    [[gnu::pure, nodiscard]] static inline auto CreateSwitch(IMemory& m) { return ASTFactory(m).CreateSwitch(); }
    [[gnu::pure, nodiscard]] static inline auto CreateExprStmt(IMemory& m) { return ASTFactory(m).CreateExprStmt(); }
  };
}  // namespace ncc::parse

#endif

#include <nitrate-core/Allocate.hh>
#include <nitrate-parser/ASTExpr.hh>

void Example() {
  auto m = ncc::DynamicArena();
  auto b = ncc::parse::ASTFactory(m);

  auto f = b.CreateFormatString({b.CreateString(""), std::string(""), b.CreateInteger(0)});
}