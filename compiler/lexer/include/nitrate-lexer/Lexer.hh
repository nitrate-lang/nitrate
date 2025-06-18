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
    class LexicalParser;
    friend class LexicalParser;

    std::istream& m_input_stream;
    std::deque<Token> m_token_queue;
    std::stack<uint32_t> m_rewind_checkpoints;

    uint32_t m_head_stream_position = 0, m_lead_stream_position = 0;
    uint32_t m_line_number = 0, m_column_number = 0;
    StringData m_current_file;

    [[nodiscard]] auto peek_byte() -> std::optional<uint8_t>;
    [[nodiscard]] auto next_byte() -> std::optional<uint8_t>;

    [[nodiscard]] auto current_source_location() const -> FileSourceLocation;
    [[nodiscard]] auto current_file() const -> const StringData& { return m_current_file; }

    [[nodiscard]] auto parse_next_token() -> std::optional<Token>;

  public:
    Lexer(std::istream& is, StringData file);
    Lexer(const Lexer&) = delete;
    Lexer(Lexer&&) = delete;
    auto operator=(const Lexer&) -> Lexer& = delete;
    auto operator=(Lexer&&) -> Lexer& = delete;
    ~Lexer() = default;

    [[nodiscard]] auto next_token() -> std::optional<Token>;
    [[nodiscard]] auto peek_token(uint8_t k = 1) -> std::optional<Token>;

    [[nodiscard]] auto push_rewind_checkpoint() -> bool;
    [[nodiscard]] auto pop_rewind_checkpoint() -> bool;
  };
}  // namespace nitrate::compiler::lexer
