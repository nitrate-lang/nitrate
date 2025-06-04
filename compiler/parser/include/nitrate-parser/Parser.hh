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
#include <nitrate-parser/SymbolTable.hh>

namespace nitrate::compiler::parser {
  class Parser {
  public:
    Parser(lexer::Lexer& lexer);
    Parser(const Parser&) = delete;
    Parser(Parser&&) = delete;
    auto operator=(const Parser&) -> Parser& = delete;
    auto operator=(Parser&&) -> Parser& = delete;
    ~Parser() = default;

    [[nodiscard]] auto parse_type() -> std::unique_ptr<Type>;
    [[nodiscard]] auto parse_expression() -> std::unique_ptr<Expr>;

    [[nodiscard]] auto parse() -> std::unique_ptr<Expr> { return parse_expression(); }

    [[nodiscard]] constexpr auto symbol_table() -> SymbolTable& { return m_symbol_table; }
    [[nodiscard]] constexpr auto symbol_table() const -> const SymbolTable& { return m_symbol_table; }

  private:
    lexer::Lexer& m_lexer;
    SymbolTable m_symbol_table;
  };
}  // namespace nitrate::compiler::parser
