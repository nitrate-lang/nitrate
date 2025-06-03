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

#include <iostream>
#include <nitrate-lexer/Lexer.hh>

using namespace nitrate::compiler::lexer;

static constexpr auto IS_WHITESPACE = []() {
  std::array<bool, 256> arr;
  arr.fill(false);

  arr[' '] = true;   // Space
  arr['\t'] = true;  // Tab
  arr['\n'] = true;  // Newline
  arr['\r'] = true;  // Carriage return
  arr['\v'] = true;  // Vertical tab
  arr['\f'] = true;  // Form feed
  arr['\0'] = true;  // Null character
  arr['\\'] = true;  // Backslash

  return arr;
}();

static constexpr auto IS_IDENTIFIER_FIRST_CHAR = []() {
  std::array<bool, 256> map = {};
  map.fill(false);

  for (uint8_t c = 'a'; c <= 'z'; ++c) {
    map[c] = true;
  }

  for (uint8_t c = 'A'; c <= 'Z'; ++c) {
    map[c] = true;
  }

  map['_'] = true;
  map['`'] = true;  // For raw identifiers

  /* Support UTF-8 */
  for (uint8_t c = 0x80; c < 0xff; c++) {
    map[c] = true;
  }

  return map;
}();

static constexpr auto IS_IDENTIFIER_BODY_CHAR = []() {
  std::array<bool, 256> map = {};
  map.fill(false);

  for (uint8_t c = 'a'; c <= 'z'; ++c) {
    map[c] = true;
  }

  for (uint8_t c = 'A'; c <= 'Z'; ++c) {
    map[c] = true;
  }

  for (uint8_t c = '0'; c <= '9'; ++c) {
    map[c] = true;
  }

  map['_'] = true;

  /* Support UTF-8 */
  for (uint8_t c = 0x80; c < 0xff; c++) {
    map[c] = true;
  }

  return map;
}();

static constexpr auto IS_BASE10_DIGIT = []() {
  std::array<bool, 256> map = {};
  map.fill(false);

  for (uint8_t c = '0'; c <= '9'; ++c) {
    map[c] = true;
  }

  return map;
}();

class Lexer::LexicalParser {
  Lexer& m_lexer;

public:
  constexpr LexicalParser(Lexer& lexer) : m_lexer(lexer) {}

  [[nodiscard]] auto skip_bytes_while(auto predicate) -> bool {
    while (true) {
      auto ch = m_lexer.peek_byte();
      if (!ch.has_value() || !predicate(*ch)) {
        break;
      }

      if (!m_lexer.next_byte().has_value()) [[unlikely]] {
        return false;  // End of input || error
      }
    }

    return true;
  }

  [[nodiscard]] auto consume_while(auto predicate) -> bool {
    while (true) {
      auto ch = m_lexer.peek_byte();
      if (!ch.has_value() || !predicate(*ch)) {
        break;
      }

      if (!m_lexer.next_byte().has_value()) [[unlikely]] {
        return false;  // End of input || error
      }
    }

    return true;
  }

  auto parse_identifier_or_keyword() -> std::optional<Token> {
    const auto first_ch = m_lexer.next_byte();
    if (!first_ch.has_value()) [[unlikely]] {
      return std::nullopt;
    }

    if (*first_ch == '`') {
      // TODO: Handle raw identifiers
      return std::nullopt;
    }

    auto utf8_unverified_identifier = [this, first_ch]() -> std::optional<std::string> {
      std::string identifier;
      identifier += static_cast<char>(*first_ch);

      const auto consume_body_predicate = [&](auto ch) {
        if (!IS_IDENTIFIER_BODY_CHAR[ch]) {
          return false;
        }

        identifier += static_cast<char>(ch);

        return true;
      };

      if (!consume_while(consume_body_predicate)) {
        return std::nullopt;
      }

      return identifier;
    }();

    if (!utf8_unverified_identifier.has_value()) [[unlikely]] {
      return std::nullopt;
    }

    const FileSourceRange source_range = {};  // TODO: Set the actual source range

    if (is_keyword(*utf8_unverified_identifier)) {
      const auto keyword = *keyword_from_string(*utf8_unverified_identifier);

      return Token::from_keyword(keyword, source_range);
    }

    // TODO: Do utf8 verification here
    // For now, we assume the identifier is valid UTF-8
    auto flyweight_identifier = boost::flyweight<std::string>(std::move(*utf8_unverified_identifier));
    auto identifier = Identifier(std::move(flyweight_identifier), IdentifierType::Typical);

    return Token::from_identifier(std::move(identifier), source_range);
  }

  auto parse_string_literal() -> std::optional<Token> {
    /// TODO: Implement string literal parsing logic
    return std::nullopt;  // Placeholder implementation
  }

  auto parse_number_literal() -> std::optional<Token> {
    // TODO: Implement number literal parsing logic
    return std::nullopt;  // Placeholder implementation
  }

  auto parse_comment() -> std::optional<Token> {
    // TODO: Implement comment parsing logic
    return std::nullopt;  // Placeholder implementation
  }

  auto parse_operator_or_punctor_or_comment() -> std::optional<Token> {
    // TODO: Implement logic to parse operators, punctuation, etc.
    return std::nullopt;  // Placeholder implementation
  }
};

auto Lexer::parse_next_token() -> std::optional<Token> {
  auto lex = LexicalParser(*this);

  if (!lex.skip_bytes_while([](auto ch) { return IS_WHITESPACE[ch]; })) {
    return std::nullopt;
  }

  const auto first_ch = peek_byte();
  if (!first_ch.has_value()) {
    return std::nullopt;
  }

  const auto ch = *first_ch;

  if (IS_IDENTIFIER_FIRST_CHAR[ch]) {
    return lex.parse_identifier_or_keyword();
  }

  if (ch == '"' || ch == '\'') {
    return lex.parse_string_literal();
  }

  if (IS_BASE10_DIGIT[ch]) {
    return lex.parse_number_literal();
  }

  if (ch == '#') {
    return lex.parse_comment();
  }

  return lex.parse_operator_or_punctor_or_comment();
}
