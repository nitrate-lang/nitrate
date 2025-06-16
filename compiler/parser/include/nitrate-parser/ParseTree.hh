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
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <boost/flyweight.hpp>
#include <memory>
#include <nitrate-lexer/Token.hh>

namespace nitrate::compiler::parser {
  enum class ASTKind : uint8_t {
    /* General Expression Nodes */
    gBinExpr,
    gUnaryExpr,
    gNumber,
    gFString,
    gString,
    gChar,
    gList,
    gIdent,
    gIndex,
    gSlice,
    gCall,
    gTemplateCall,
    gIf,
    gElse,
    gFor,
    gWhile,
    gDo,
    gSwitch,
    gBreak,
    gContinue,
    gReturn,
    gForeach,
    gTry,
    gCatch,
    gThrow,
    gAwait,
    gAsm,

    /* Type Nodes */
    tInfer,
    tOpaque,
    tNamed,
    tRef,
    tPtr,
    tArray,
    tTuple,
    tTemplate,
    tLambda,

    /* Symbol Nodes */
    sVar,
    sFn,
    sEnum,
    sStruct,
    sUnion,
    sContract,
    sTrait,
    sTypeDef,
    sScope,
    sImport,
    sUnitTest,
  };

  class BinExpr;
  class UnaryExpr;
  class Number;
  class FString;
  class String;
  class Char;
  class List;
  class Ident;
  class Index;
  class Slice;
  class Call;
  class TemplateCall;
  class If;
  class Else;
  class For;
  class While;
  class Do;
  class Switch;
  class Break;
  class Continue;
  class Return;
  class Foreach;
  class Try;
  class Catch;
  class Throw;
  class Await;
  class Asm;
  class InferTy;
  class OpaqueTy;
  class NamedTy;
  class RefTy;
  class PtrTy;
  class ArrayTy;
  class TupleTy;
  class TemplateTy;
  class LambdaTy;

  using Expr = std::variant<BinExpr, UnaryExpr, String>;

  using Type = std::variant<InferTy>;

  namespace detail {
    class SourceLocationTag {
      uint64_t m_id : 48;

    public:
      constexpr SourceLocationTag() : m_id(0) {}
      SourceLocationTag(boost::flyweight<lexer::FileSourceRange> source_range);
      SourceLocationTag(const SourceLocationTag&);
      SourceLocationTag(SourceLocationTag&& o) : m_id(o.m_id) { o.m_id = 0; }
      auto operator=(const SourceLocationTag&) -> SourceLocationTag&;
      auto operator=(SourceLocationTag&& o) -> SourceLocationTag& {
        m_id = o.m_id;
        o.m_id = 0;
        return *this;
      }
      ~SourceLocationTag();

      [[nodiscard]] constexpr auto operator<=>(const SourceLocationTag&) const -> std::weak_ordering = default;

      [[nodiscard]] auto get() const -> const lexer::FileSourceRange&;
    } __attribute__((packed));

#define W_NITRATE_AST_PARENTHESIZED_TRAIT()                                                    \
  [[nodiscard]] constexpr auto is_parenthesized() const -> bool { return m_is_parenthesized; } \
  constexpr auto set_parenthesized(bool b) -> void { m_is_parenthesized = b; }

#define W_NITRATE_AST_SOURCE_RANGE_TRAIT()                                                                            \
  [[nodiscard]] constexpr auto source_range() const -> const lexer::FileSourceRange& { return m_source_range.get(); } \
  auto set_source_range(lexer::FileSourceRange source_range) -> void {                                                \
    m_source_range = detail::SourceLocationTag(boost::flyweight<lexer::FileSourceRange>(std::move(source_range)));    \
  }

    inline auto clone_expr(const std::unique_ptr<Expr>& expr) -> std::unique_ptr<Expr>;
  }  // namespace detail

#define W_PLACEHOLDER_IMPL(name, type) \
  class name {                         \
  public:                              \
    name() {}                          \
  };

  class BinExpr final {
  public:
    using LHS = Expr;
    using RHS = Expr;
    using Op = lexer::Operator;

    BinExpr() = delete;
    inline BinExpr(LHS lhs, RHS rhs, Op op);
    BinExpr(std::unique_ptr<LHS> lhs, std::unique_ptr<RHS> rhs, Op op)
        : m_op(op), m_lhs(std::move(lhs)), m_rhs(std::move(rhs)) {}
    BinExpr(const BinExpr& o)
        : m_source_range(o.m_source_range),
          m_is_parenthesized(o.m_is_parenthesized),
          m_op(o.m_op),
          m_lhs(detail::clone_expr(o.m_lhs)),
          m_rhs(detail::clone_expr(o.m_rhs)) {}
    BinExpr(BinExpr&&) = default;
    auto operator=(const BinExpr& o) -> BinExpr& {
      m_source_range = o.m_source_range;
      m_is_parenthesized = o.m_is_parenthesized;
      m_op = o.m_op;
      m_lhs = detail::clone_expr(o.m_lhs);
      m_rhs = detail::clone_expr(o.m_rhs);
      return *this;
    }
    auto operator=(BinExpr&& o) -> BinExpr& = default;
    ~BinExpr() = default;

    [[nodiscard]] constexpr auto operator==(const BinExpr& o) const -> bool;
    [[nodiscard]] constexpr auto operator<=>(const BinExpr& o) const -> std::weak_ordering;

    [[nodiscard]] constexpr auto get_lhs() const -> const LHS&;
    [[nodiscard]] constexpr auto get_lhs() -> LHS&;
    auto set_lhs(LHS lhs) -> void;

    [[nodiscard]] constexpr auto get_rhs() const -> const RHS&;
    [[nodiscard]] constexpr auto get_rhs() -> RHS&;
    auto set_rhs(RHS rhs) -> void;

    [[nodiscard]] constexpr auto get_op() const -> Op { return m_op; }
    auto set_op(Op op) -> void { m_op = op; }

    W_NITRATE_AST_PARENTHESIZED_TRAIT();
    W_NITRATE_AST_SOURCE_RANGE_TRAIT();

  protected:
    using MemberTuple = std::tuple<const detail::SourceLocationTag&, const bool&, const Op&, const LHS&, const RHS&>;

    [[nodiscard]] constexpr auto state() const -> MemberTuple;

  private:
    detail::SourceLocationTag m_source_range;
    bool m_is_parenthesized : 1 = false;
    Op m_op : 7;
    std::unique_ptr<LHS> m_lhs;
    std::unique_ptr<RHS> m_rhs;
  };

  class UnaryExpr final {
  public:
    using Op = lexer::Operator;
    using Operand = Expr;

    UnaryExpr() = delete;
    inline UnaryExpr(Operand operand, Op op, bool is_postfix);
    UnaryExpr(std::unique_ptr<Operand> operand, Op op, bool is_postfix)
        : m_op(op), m_is_postfix(is_postfix), m_operand(std::move(operand)) {}
    UnaryExpr(const UnaryExpr& o)
        : m_source_range(o.m_source_range),
          m_is_parenthesized(o.m_is_parenthesized),
          m_op(o.m_op),
          m_is_postfix(o.m_is_postfix),
          m_operand(detail::clone_expr(o.m_operand)) {}
    UnaryExpr(UnaryExpr&&) = default;
    auto operator=(const UnaryExpr& o) -> UnaryExpr& {
      m_source_range = o.m_source_range;
      m_is_parenthesized = o.m_is_parenthesized;
      m_op = o.m_op;
      m_is_postfix = o.m_is_postfix;
      m_operand = detail::clone_expr(o.m_operand);
      return *this;
    }
    auto operator=(UnaryExpr&&) -> UnaryExpr& = default;
    ~UnaryExpr() = default;

    [[nodiscard]] constexpr auto operator==(const UnaryExpr&) const -> bool;
    [[nodiscard]] constexpr auto operator<=>(const UnaryExpr&) const -> std::weak_ordering;

    [[nodiscard]] constexpr auto get_operand() const -> const Operand&;
    [[nodiscard]] constexpr auto get_operand() -> Operand&;
    auto set_operand(Operand operand) -> void;

    [[nodiscard]] constexpr auto get_op() const -> Op { return m_op; }
    constexpr auto set_op(Op op) -> void { m_op = op; }

    [[nodiscard]] constexpr auto is_postfix() const -> bool { return m_is_postfix; }
    constexpr auto set_postfix(bool is_postfix) -> void { m_is_postfix = is_postfix; }

    W_NITRATE_AST_PARENTHESIZED_TRAIT();
    W_NITRATE_AST_SOURCE_RANGE_TRAIT();

  protected:
    using MemberTuple =
        std::tuple<const detail::SourceLocationTag&, const bool&, const Op&, const bool&, const Operand&>;

    [[nodiscard]] constexpr auto state() const -> MemberTuple;

  private:
    detail::SourceLocationTag m_source_range;
    bool m_is_parenthesized : 1 = false;
    Op m_op : 7;
    bool m_is_postfix : 1;
    std::unique_ptr<Operand> m_operand;
  };

  class String final {
  public:
    using ValueType = std::pmr::string;

    String(ValueType value = "") : m_value(std::move(value)) {}
    String(const String&) = default;
    String(String&&) = default;
    auto operator=(const String&) -> String& = default;
    auto operator=(String&&) -> String& = default;
    ~String() = default;

    [[nodiscard]] constexpr auto operator==(const String& o) const -> bool = default;
    [[nodiscard]] constexpr auto operator<=>(const String& o) const -> std::strong_ordering = default;

    [[nodiscard]] constexpr auto get_value() const -> const ValueType& { return m_value; }
    [[nodiscard]] constexpr auto get_value() -> ValueType& { return m_value; }
    [[nodiscard]] constexpr auto is_empty() const -> bool { return m_value.empty(); }
    [[nodiscard]] constexpr auto size() const -> size_t { return m_value.size(); }

    auto set_value(ValueType value) -> void { m_value = std::move(value); }

  private:
    ValueType m_value;
  };

  class InferTy {
  public:
    constexpr InferTy() = default;
  };

  /*------------------------------------------------------------------------------------------------------------*/

  inline auto detail::clone_expr(const std::unique_ptr<Expr>& expr) -> std::unique_ptr<Expr> {
    return expr ? std::make_unique<Expr>(*expr) : nullptr;
  }

  /*------------------------------------------------------------------------------------------------------------*/

  BinExpr::BinExpr(LHS lhs, RHS rhs, Op op)
      : m_op(op), m_lhs(std::make_unique<LHS>(std::move(lhs))), m_rhs(std::make_unique<RHS>(std::move(rhs))) {}

  constexpr auto BinExpr::state() const -> MemberTuple {
    return std::tie(m_source_range, m_is_parenthesized, m_op, *m_lhs, *m_rhs);
  }

  constexpr auto BinExpr::operator==(const BinExpr& o) const -> bool { return state() == o.state(); }
  constexpr auto BinExpr::operator<=>(const BinExpr& o) const -> std::weak_ordering { return state() <=> o.state(); }

  /*------------------------------------------------------------------------------------------------------------*/

  UnaryExpr::UnaryExpr(Operand operand, Op op, bool is_postfix)
      : m_op(op), m_is_postfix(is_postfix), m_operand(std::make_unique<Operand>(std::move(operand))) {}

  constexpr auto UnaryExpr::state() const -> MemberTuple {
    return std::tie(m_source_range, m_is_parenthesized, m_op, m_is_postfix, *m_operand);
  }

  constexpr auto UnaryExpr::operator==(const UnaryExpr& o) const -> bool { return state() == o.state(); }
  constexpr auto UnaryExpr::operator<=>(const UnaryExpr& o) const -> std::weak_ordering {
    return state() <=> o.state();
  }

  /*------------------------------------------------------------------------------------------------------------*/

  namespace detail {
#undef W_NITRATE_AST_SOURCE_RANGE_TRAIT
#undef W_NITRATE_AST_PARENTHESIZED_TRAIT
  }  // namespace detail
}  // namespace nitrate::compiler::parser
