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

#include <nitrate-lexer/Lexer.hh>

using namespace nitrate::compiler::lexer;

BOOST_SYMBOL_EXPORT Lexer::Lexer(std::istream& is, boost::flyweight<std::string> file)
    : m_input_stream(is), m_current_file(std::move(file)) {}

auto Lexer::peek_byte() -> std::optional<uint8_t> {
  const auto byte = m_input_stream.peek();

  if (byte == std::char_traits<char>::eof()) [[unlikely]] {
    return std::nullopt;  // If we reach the end of the stream
  }

  return static_cast<uint8_t>(byte);
}

auto Lexer::next_byte() -> std::optional<uint8_t> {
  const auto byte = m_input_stream.get();

  if (byte == std::char_traits<char>::eof()) [[unlikely]] {
    return std::nullopt;  // If we reach the end of the stream
  }

  ++m_lead_stream_position;

  if (byte == '\n') {
    ++m_line_number;
    m_column_number = 0;  // Reset column number on new line
  } else {
    ++m_column_number;  // Increment column number for other characters
  }

  return static_cast<uint8_t>(byte);
}

auto Lexer::current_source_location() const -> FileSourceLocation {
  return {m_line_number, m_column_number, m_lead_stream_position};
}

BOOST_SYMBOL_EXPORT auto Lexer::next_token() -> std::optional<Token> {
  auto token = [this]() -> std::optional<Token> {
    if (!m_token_queue.empty()) {
      auto token = std::move(m_token_queue.front());
      m_token_queue.pop_front();

      return token;
    }

    return parse_next_token();
  }();

  m_head_stream_position = token->source_range().end().offset() + 1;

  return token;
}

BOOST_SYMBOL_EXPORT auto Lexer::peek_token(uint8_t k) -> std::optional<Token> {
  assert(k > 0 && "k must be greater than 0 to peek at tokens.");

  while (m_token_queue.size() < k) {
    auto token = parse_next_token();
    if (!token.has_value()) [[unlikely]] {
      return std::nullopt;  // No more tokens available
    }
    m_token_queue.push_back(std::move(token.value()));
  }

  const size_t index = k - 1;  // k is 1-based, so we access k-1
  assert(index < m_token_queue.size() && "Not enough tokens in the queue to peek at k-th token.");

  return m_token_queue[index];
}

BOOST_SYMBOL_EXPORT auto Lexer::push_rewind_checkpoint() -> bool {
  m_rewind_checkpoints.push(m_head_stream_position);
  return true;
}

BOOST_SYMBOL_EXPORT auto Lexer::pop_rewind_checkpoint() -> bool {
  assert(!m_rewind_checkpoints.empty() && "Attempted to pop a lexical checkpoint from an empty stack.");

  const auto checkpoint = m_rewind_checkpoints.top();
  if (!m_input_stream.seekg(checkpoint, std::ios::beg)) [[unlikely]] {
    spdlog::error("[Lexer] Failed to seek to the rewind checkpoint at position {}", checkpoint);
    return false;
  }

  m_rewind_checkpoints.pop();

  m_head_stream_position = checkpoint;
  m_lead_stream_position = checkpoint;
  m_token_queue.clear();

  return true;
}
