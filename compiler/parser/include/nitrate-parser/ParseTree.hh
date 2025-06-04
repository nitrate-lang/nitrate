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

#include <boost/flyweight/flyweight_fwd.hpp>
#include <cmath>
#include <nitrate-lexer/Token.hh>
#include <nitrate-parser/ParseTreeFwd.hh>
#include <nitrate-parser/Visitor.hh>
#include <ostream>

namespace nitrate::compiler::parser {
  class Expr {
    static constexpr size_t M_KIND_BITS = 6;  // 64 kinds max
    static_assert(static_cast<size_t>(ASTKIND_MAX) <= (1 << M_KIND_BITS) - 1,
                  "ASTKind must fit in 6 bits for Expr::m_kind");

    ASTKind m_kind : M_KIND_BITS;
    bool m_is_parenthesized : 1 = false;
    boost::flyweight<lexer::FileSourceRange> m_source_range;

  public:
    Expr(ASTKind kind) : m_kind(kind) {}

    Expr(const Expr&) = delete;
    Expr(Expr&&) = delete;
    auto operator=(const Expr&) -> Expr& = delete;
    auto operator=(Expr&&) -> Expr& = delete;
    ~Expr();

    [[nodiscard]] constexpr auto get_kind() const -> ASTKind { return m_kind; }
    [[nodiscard]] constexpr auto is_discarded() const -> bool { return m_kind == ASTKind::Discarded; }
    constexpr auto discard() -> void { m_kind = ASTKind::Discarded; }

    [[nodiscard]] constexpr auto is_parenthesized() const -> bool { return m_is_parenthesized; }
    constexpr auto set_parenthesized(bool is_parenthesized) -> void { m_is_parenthesized = is_parenthesized; }

    [[nodiscard]] constexpr auto source_range() const -> const lexer::FileSourceRange& { return m_source_range.get(); }

    /** Perform minimal required semantic analysis */
    [[nodiscard]] auto check(const SymbolTable& symbol_table) const -> bool;

    constexpr auto accept(ConstVisitor& visitor) const -> void;
    constexpr auto accept(Visitor& visitor) -> void;

    auto dump(std::ostream& os) const -> std::ostream&;
  };

  class Break : public Expr {
  public:
    Break() : Expr(ASTKind::Break) {}
    ~Break() = default;
  };

#define W_NITRATE_PARSER_EXPR_ACCEPT_METHOD(visitor_name, constness)     \
  constexpr auto Expr::accept(visitor_name& visitor) constness -> void { \
    constness Expr& node = *this;                                        \
                                                                         \
    switch (get_kind()) {                                                \
      case ASTKind::Discarded:                                           \
        visitor.visit(static_cast<constness Expr&>(node));               \
        break;                                                           \
      case ASTKind::Break:                                               \
        visitor.visit(static_cast<constness Break&>(node));              \
        break;                                                           \
    }                                                                    \
  }

  W_NITRATE_PARSER_EXPR_ACCEPT_METHOD(Visitor, )
  W_NITRATE_PARSER_EXPR_ACCEPT_METHOD(ConstVisitor, const)

#undef W_NITRATE_PARSER_EXPR_ACCEPT_METHOD
}  // namespace nitrate::compiler::parser
