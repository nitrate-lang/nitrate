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

#include <deque>
#include <istream>
#include <nitrate-lexer/Token.hh>
#include <stack>

namespace nitrate::compiler::lexer {
  class Lexer {
    std::istream& m_input_stream;
    std::deque<Token> m_tokens;
    std::stack<uint32_t> m_rewind_checkpoints;
    bool m_eof_reached = false;

    [[nodiscard]] auto peek_byte() -> uint8_t;
    [[nodiscard]] auto next_byte() -> uint8_t;

  public:
    Lexer(std::istream& is);
    Lexer(const Lexer&) = delete;
    Lexer(Lexer&&) = delete;
    auto operator=(const Lexer&) -> Lexer& = delete;
    auto operator=(Lexer&&) -> Lexer& = delete;
    ~Lexer() = default;

    [[nodiscard]] auto next_token() -> Token;
    [[nodiscard]] auto peek_token(uint8_t k = 1) -> const Token&;
    [[nodiscard]] auto is_end_of_file() const -> bool { return m_eof_reached && m_tokens.empty(); }

    auto push_rewind_checkpoint() -> void;
    auto pop_rewind_checkpoint() -> void;
  };
}  // namespace nitrate::compiler::lexer
