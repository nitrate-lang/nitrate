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
#include <nitrate-lexer/Lexer.hh>
#include <nitrate-parser/ParseTree.hh>
#include <nitrate-parser/SymbolTable.hh>

namespace nitrate::compiler::parser {
  // FIXME: Create a seperate variant for types
  using Type = Expr;

  class Parser {
  public:
    Parser(lexer::Lexer& lexer);
    Parser(const Parser&) = delete;
    Parser(Parser&&) = delete;
    auto operator=(const Parser&) -> Parser& = delete;
    auto operator=(Parser&&) -> Parser& = delete;
    ~Parser() = default;

    [[nodiscard]] auto parse_type() -> Type;
    [[nodiscard]] auto parse_expression() -> Expr;

    [[nodiscard]] auto parse() -> Expr;

    [[nodiscard]] auto symbol_table() -> std::shared_ptr<SymbolTable> { return m_symbol_table; }
    [[nodiscard]] auto symbol_table() const -> std::shared_ptr<const SymbolTable> { return m_symbol_table; }
    [[nodiscard]] constexpr auto lexer() -> lexer::Lexer& { return m_lexer; }
    [[nodiscard]] constexpr auto lexer() const -> const lexer::Lexer& { return m_lexer; }

  private:
    lexer::Lexer& m_lexer;
    std::shared_ptr<SymbolTable> m_symbol_table;
  };
}  // namespace nitrate::compiler::parser
