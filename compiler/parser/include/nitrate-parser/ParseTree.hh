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

#include <memory>
#include <nitrate-lexer/Token.hh>

namespace nitrate::compiler::parser {
  class BinExpr;
  class UnaryExpr;
  class Number;        // TODO: Implement this class
  class FString;       // TODO: Implement this class
  class String;        // TODO: Implement this class
  class Char;          // TODO: Implement this class
  class List;          // TODO: Implement this class
  class Ident;         // TODO: Implement this class
  class Index;         // TODO: Implement this class
  class Slice;         // TODO: Implement this class
  class Call;          // TODO: Implement this class
  class TemplateCall;  // TODO: Implement this class
  class If;            // TODO: Implement this class
  class Else;          // TODO: Implement this class
  class For;           // TODO: Implement this class
  class While;         // TODO: Implement this class
  class Do;            // TODO: Implement this class
  class Switch;        // TODO: Implement this class
  class Break;         // TODO: Implement this class
  class Continue;      // TODO: Implement this class
  class Return;        // TODO: Implement this class
  class Foreach;       // TODO: Implement this class
  class Try;           // TODO: Implement this class
  class Catch;         // TODO: Implement this class
  class Throw;         // TODO: Implement this class
  class Await;         // TODO: Implement this class
  class Asm;           // TODO: Implement this class
  class InferTy;       // TODO: Implement this class
  class OpaqueTy;      // TODO: Implement this class
  class NamedTy;       // TODO: Implement this class
  class RefTy;         // TODO: Implement this class
  class PtrTy;         // TODO: Implement this class
  class ArrayTy;       // TODO: Implement this class
  class TupleTy;       // TODO: Implement this class
  class TemplateTy;    // TODO: Implement this class
  class LambdaTy;      // TODO: Implement this class

  using Expr = std::variant<BinExpr, UnaryExpr, String>;

  namespace detail {
    class LocationTag final {
      uint64_t m_id : 48;

    public:
      constexpr LocationTag() : m_id(0) {}
      LocationTag(lexer::FileSourceRange source_range);
      LocationTag(const LocationTag&);
      LocationTag(LocationTag&& o) : m_id(o.m_id) { o.m_id = 0; }
      auto operator=(const LocationTag&) -> LocationTag&;
      auto operator=(LocationTag&& o) -> LocationTag& {
        m_id = o.m_id;
        o.m_id = 0;
        return *this;
      }
      ~LocationTag();

      [[nodiscard]] constexpr auto operator<=>(const LocationTag&) const -> std::strong_ordering = default;
      [[nodiscard]] constexpr auto operator==(const LocationTag& o) const -> bool { return get() == o.get(); }

      [[nodiscard]] auto get() const -> const lexer::FileSourceRange&;
    } __attribute__((packed));

    inline auto clone_expr(const std::unique_ptr<Expr>& expr) -> std::unique_ptr<Expr>;
  }  // namespace detail

  /*------------------------------------------------------------------------------------------------------------*/

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
      if (this != &o) {
        m_source_range = o.m_source_range;
        m_is_parenthesized = o.m_is_parenthesized;
        m_op = o.m_op;
        m_lhs = detail::clone_expr(o.m_lhs);
        m_rhs = detail::clone_expr(o.m_rhs);
      }

      return *this;
    }
    auto operator=(BinExpr&& o) -> BinExpr& = default;
    ~BinExpr() = default;

    [[nodiscard]] constexpr auto operator==(const BinExpr& o) const -> bool;
    [[nodiscard]] constexpr auto operator<=>(const BinExpr& o) const -> std::weak_ordering;

    [[nodiscard]] constexpr auto get_lhs() const -> const LHS&;
    [[nodiscard]] constexpr auto get_lhs() -> LHS&;
    auto set_lhs(std::unique_ptr<LHS> lhs) -> void { m_lhs = std::move(lhs); }
    inline auto set_lhs(LHS lhs) -> void;

    [[nodiscard]] constexpr auto get_rhs() const -> const RHS&;
    [[nodiscard]] constexpr auto get_rhs() -> RHS&;
    auto set_rhs(std::unique_ptr<RHS> rhs) -> void { m_rhs = std::move(rhs); }
    inline auto set_rhs(RHS rhs) -> void;

    [[nodiscard]] constexpr auto get_op() const -> Op { return m_op; }
    auto set_op(Op op) -> void { m_op = op; }

    [[nodiscard]] constexpr auto is_parenthesized() const -> bool { return m_is_parenthesized; }
    constexpr auto set_parenthesized(bool b) -> void { m_is_parenthesized = b; }

    [[nodiscard]] constexpr auto source_range() const -> const auto& { return m_source_range.get(); }
    auto set_source_range(auto source_range) -> void { m_source_range = detail::LocationTag(std::move(source_range)); }

  protected:
    using MemberTuple = std::tuple<const detail::LocationTag&, const bool&, const Op&, const LHS&, const RHS&>;

    [[nodiscard]] constexpr auto state() const -> MemberTuple;

  private:
    detail::LocationTag m_source_range;
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
      if (this != &o) {
        m_source_range = o.m_source_range;
        m_is_parenthesized = o.m_is_parenthesized;
        m_op = o.m_op;
        m_is_postfix = o.m_is_postfix;
        m_operand = detail::clone_expr(o.m_operand);
      }
      return *this;
    }
    auto operator=(UnaryExpr&&) -> UnaryExpr& = default;
    ~UnaryExpr() = default;

    [[nodiscard]] constexpr auto operator==(const UnaryExpr&) const -> bool;
    [[nodiscard]] constexpr auto operator<=>(const UnaryExpr&) const -> std::weak_ordering;

    [[nodiscard]] constexpr auto get_operand() const -> const Operand&;
    [[nodiscard]] constexpr auto get_operand() -> Operand&;
    auto set_operand(std::unique_ptr<Operand> operand) -> void { m_operand = std::move(operand); }
    inline auto set_operand(Operand operand) -> void;

    [[nodiscard]] constexpr auto get_op() const -> Op { return m_op; }
    constexpr auto set_op(Op op) -> void { m_op = op; }

    [[nodiscard]] constexpr auto is_postfix() const -> bool { return m_is_postfix; }
    constexpr auto set_postfix(bool is_postfix) -> void { m_is_postfix = is_postfix; }

    [[nodiscard]] constexpr auto is_parenthesized() const -> bool { return m_is_parenthesized; }
    constexpr auto set_parenthesized(bool b) -> void { m_is_parenthesized = b; }

    [[nodiscard]] constexpr auto source_range() const -> const auto& { return m_source_range.get(); }
    auto set_source_range(auto source_range) -> void { m_source_range = detail::LocationTag(std::move(source_range)); }

  protected:
    using MemberTuple = std::tuple<const detail::LocationTag&, const bool&, const Op&, const bool&, const Operand&>;

    [[nodiscard]] constexpr auto state() const -> MemberTuple;

  private:
    detail::LocationTag m_source_range;
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
    [[nodiscard]] constexpr auto operator<=>(const String& o) const -> std::weak_ordering = default;

    [[nodiscard]] constexpr auto get_value() const -> const ValueType& { return m_value; }
    [[nodiscard]] constexpr auto get_value() -> ValueType& { return m_value; }
    [[nodiscard]] constexpr auto is_empty() const -> bool { return m_value.empty(); }
    [[nodiscard]] constexpr auto size() const -> size_t { return m_value.size(); }

    auto set_value(ValueType value) -> void { m_value = std::move(value); }

    [[nodiscard]] constexpr auto source_range() const -> const auto& { return m_source_range.get(); }
    auto set_source_range(auto source_range) -> void { m_source_range = detail::LocationTag(std::move(source_range)); }

  private:
    ValueType m_value;
    detail::LocationTag m_source_range;
  };

  /*------------------------------------------------------------------------------------------------------------*/

  BinExpr::BinExpr(LHS lhs, RHS rhs, Op op)
      : m_op(op), m_lhs(std::make_unique<LHS>(std::move(lhs))), m_rhs(std::make_unique<RHS>(std::move(rhs))) {}

  constexpr auto BinExpr::state() const -> MemberTuple {
    return std::tie(m_source_range, m_is_parenthesized, m_op, *m_lhs, *m_rhs);
  }

  constexpr auto BinExpr::operator==(const BinExpr& o) const -> bool { return state() == o.state(); }
  constexpr auto BinExpr::operator<=>(const BinExpr& o) const -> std::weak_ordering { return state() <=> o.state(); }

  auto BinExpr::set_lhs(LHS lhs) -> void {
    assert(m_lhs != nullptr && "Cannot set LHS of BinExpr when m_lhs is nullptr");
    *m_lhs = std::move(lhs);
  }

  auto BinExpr::set_rhs(RHS rhs) -> void {
    assert(m_rhs != nullptr && "Cannot set RHS of BinExpr when m_rhs is nullptr");
    *m_rhs = std::move(rhs);
  }

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

  auto UnaryExpr::set_operand(Operand operand) -> void {
    assert(m_operand != nullptr && "Cannot set Operand of UnaryExpr when m_operand is nullptr");
    *m_operand = std::move(operand);
  }

  /*------------------------------------------------------------------------------------------------------------*/

  namespace detail {
    inline auto clone_expr(const std::unique_ptr<Expr>& expr) -> std::unique_ptr<Expr> {
      return expr ? std::make_unique<Expr>(*expr) : nullptr;
    }
  }  // namespace detail

  /*------------------------------------------------------------------------------------------------------------*/
}  // namespace nitrate::compiler::parser
