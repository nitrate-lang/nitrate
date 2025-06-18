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

#define FMT_HEADER_ONLY
#include <spdlog/spdlog.h>

#include <cstdlib>
#include <nitrate-parser/ParseTree.hh>
#include <nitrate-parser/Parser.hh>

using namespace nitrate::compiler::parser;

BOOST_SYMBOL_EXPORT Parser::Parser(lexer::Lexer& lexer)
    : m_lexer(lexer), m_symbol_table(std::make_shared<SymbolTable>()) {}

BOOST_SYMBOL_EXPORT auto Parser::parse_type() -> Type {
  // TODO: Implement type parsing logic

  spdlog::critical("Parser::parse_type() is not implemented yet.");
  abort();
}

BOOST_SYMBOL_EXPORT auto Parser::parse_expression() -> Expr {
  // TODO: Implement expression parsing logic

  spdlog::critical("Parser::parse_expression() is not implemented yet.");
  abort();
}

BOOST_SYMBOL_EXPORT auto Parser::parse() -> Expr { return parse_expression(); }
