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

static const auto IS_OPERATOR_BYTE = []() {
  std::array<bool, 256> map = {};
  map.fill(false);

  for (const auto &op : OPERATOR_MAP) {
    for (auto ch : op.right) {
      const auto byte = static_cast<uint8_t>(ch);
      if (!IS_IDENTIFIER_BODY_CHAR[byte]) {
        map[byte] = true;
      }
    }
  }

  return map;
}();

/// https://stackoverflow.com/questions/1031645/how-to-detect-utf-8-in-plain-c
static auto is_utf8(const char *string) -> bool {
  if (string == nullptr) {
    return false;
  }

  const auto *bytes = (const unsigned char *)string;
  while (*bytes != 0U) {
    if ((  // ASCII
           // use bytes[0] <= 0x7F to allow ASCII control characters
            bytes[0] == 0x09 || bytes[0] == 0x0A || bytes[0] == 0x0D || (0x20 <= bytes[0] && bytes[0] <= 0x7E))) {
      bytes += 1;
      continue;
    }

    if ((  // non-overlong 2-byte
            (0xC2 <= bytes[0] && bytes[0] <= 0xDF) && (0x80 <= bytes[1] && bytes[1] <= 0xBF))) {
      bytes += 2;
      continue;
    }

    if ((  // excluding overlongs
            bytes[0] == 0xE0 && (0xA0 <= bytes[1] && bytes[1] <= 0xBF) && (0x80 <= bytes[2] && bytes[2] <= 0xBF)) ||
        (  // straight 3-byte
            ((0xE1 <= bytes[0] && bytes[0] <= 0xEC) || bytes[0] == 0xEE || bytes[0] == 0xEF) &&
            (0x80 <= bytes[1] && bytes[1] <= 0xBF) && (0x80 <= bytes[2] && bytes[2] <= 0xBF)) ||
        (  // excluding surrogates
            bytes[0] == 0xED && (0x80 <= bytes[1] && bytes[1] <= 0x9F) && (0x80 <= bytes[2] && bytes[2] <= 0xBF))) {
      bytes += 3;
      continue;
    }

    if ((  // planes 1-3
            bytes[0] == 0xF0 && (0x90 <= bytes[1] && bytes[1] <= 0xBF) && (0x80 <= bytes[2] && bytes[2] <= 0xBF) &&
            (0x80 <= bytes[3] && bytes[3] <= 0xBF)) ||
        (  // planes 4-15
            (0xF1 <= bytes[0] && bytes[0] <= 0xF3) && (0x80 <= bytes[1] && bytes[1] <= 0xBF) &&
            (0x80 <= bytes[2] && bytes[2] <= 0xBF) && (0x80 <= bytes[3] && bytes[3] <= 0xBF)) ||
        (  // plane 16
            bytes[0] == 0xF4 && (0x80 <= bytes[1] && bytes[1] <= 0x8F) && (0x80 <= bytes[2] && bytes[2] <= 0xBF) &&
            (0x80 <= bytes[3] && bytes[3] <= 0xBF))) {
      bytes += 4;
      continue;
    }

    return false;
  }

  return true;
}

class Lexer::LexicalParser {
  Lexer &m_lexer;

public:
  constexpr LexicalParser(Lexer &lexer) : m_lexer(lexer) {}

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

  auto parse_atypical_literal_after_tick() -> std::optional<std::string> {
    auto identifier = [this]() -> std::optional<std::string> {
      std::string value;
      const auto consume_body_predicate = [&](auto ch) {
        if (ch == '`') {
          return false;
        }

        value += static_cast<char>(ch);

        return true;
      };

      if (!consume_while(consume_body_predicate)) {
        return std::nullopt;
      }

      return value;
    }();

    if (!identifier.has_value()) [[unlikely]] {
      spdlog::error("[Lexer] Failed to read a atypical identifier in its entirety");
      return std::nullopt;
    }

    if (auto tick = m_lexer.next_byte(); !tick.has_value() || *tick != '`') [[unlikely]] {
      spdlog::error("[Lexer] Expected a closing tick terminator for atypical identifier, but found: '{}'",
                    tick.value_or(' '));
      return std::nullopt;
    }

    return identifier;
  }

  auto parse_typical_literal_body(uint8_t first_byte) -> std::optional<std::string> {
    auto identifier = [this, first_byte]() -> std::optional<std::string> {
      std::string value;
      value += static_cast<char>(first_byte);

      const auto consume_body_predicate = [&](auto ch) {
        if (!IS_IDENTIFIER_BODY_CHAR[ch]) {
          return false;
        }

        value += static_cast<char>(ch);

        return true;
      };

      if (!consume_while(consume_body_predicate)) {
        return std::nullopt;
      }

      return value;
    }();

    if (!identifier.has_value()) [[unlikely]] {
      spdlog::error("[Lexer] Failed to read an identifier or keyword in its entirety");
      return std::nullopt;
    }

    return identifier;
  }

  auto parse_identifier_or_keyword() -> std::optional<Token> {
    const auto start_position = m_lexer.current_source_location();

    const auto first_ch = m_lexer.next_byte();
    if (!first_ch.has_value()) [[unlikely]] {
      spdlog::error("[Lexer] Failed to read the first byte of an identifier or keyword");
      return std::nullopt;
    }

    const auto identifier_type = *first_ch == '`' ? IdentifierType::Atypical : IdentifierType::Typical;

    auto identifier_value = [&]() {
      if (identifier_type == IdentifierType::Atypical) {
        return parse_atypical_literal_after_tick();
      }

      return parse_typical_literal_body(*first_ch);
    }();
    if (!identifier_value.has_value()) [[unlikely]] {
      spdlog::error("[Lexer] Failed to read the identifier or keyword body");
      return std::nullopt;
    }

    const auto end_position = m_lexer.current_source_location();
    const auto source_range = FileSourceRange(m_lexer.current_file(), start_position, end_position);

    if (identifier_type == IdentifierType::Typical) {
      if (is_keyword(*identifier_value)) {
        const auto keyword = *keyword_from_string(*identifier_value);
        return Token::from_keyword(keyword, source_range);
      }

      if (is_operator(*identifier_value)) {
        const auto op = *operator_from_string(*identifier_value);
        return Token::from_operator(op, source_range);
      }

      if (!is_utf8(identifier_value->c_str())) [[unlikely]] {
        spdlog::error("[Lexer] Identifier '{}' is not valid UTF-8", *identifier_value);
        return std::nullopt;
      }
    }

    auto flyweight_identifier = boost::flyweight<std::string>(std::move(*identifier_value));
    auto identifier = Identifier(std::move(flyweight_identifier), identifier_type);

    return Token::from_identifier(std::move(identifier), source_range);
  }

  auto parse_string_literal() -> std::optional<Token> {
    /// TODO: Implement string literal parsing logic
    spdlog::warn("[Lexer] String literal parsing is not yet implemented");
    return std::nullopt;  // Placeholder implementation
  }

  auto parse_number_literal() -> std::optional<Token> {
    // TODO: Implement number literal parsing logic
    spdlog::warn("[Lexer] Number literal parsing is not yet implemented");
    return std::nullopt;  // Placeholder implementation
  }

  auto parse_comment() -> std::optional<Token> {
    const auto start_position = m_lexer.current_source_location();

    const auto first_ch = m_lexer.next_byte();
    if (!first_ch.has_value()) [[unlikely]] {
      spdlog::error("[Lexer] Failed to read the first byte of a comment");
      return std::nullopt;
    }

    if (*first_ch != '#') [[unlikely]] {
      spdlog::error("[Lexer] Expected a comment to start with '#', but found: '{}'", *first_ch);
      return std::nullopt;
    }

    // Consume the rest of the comment until a newline or end of input
    std::string comment_value;
    const auto consume_comment_body = [&](auto ch) {
      if (ch == '\n' || ch == '\r' || !ch) {
        return false;  // Stop at newline or end of input
      }

      comment_value += static_cast<char>(ch);
      return true;
    };

    if (!consume_while(consume_comment_body)) {
      return std::nullopt;
    }

    const auto end_position = m_lexer.current_source_location();
    auto source_range = FileSourceRange(m_lexer.current_file(), start_position, end_position);

    auto flyweight_identifier = boost::flyweight<std::string>(std::move(comment_value));
    auto comment = Comment(std::move(flyweight_identifier), CommentType::SingleLine);

    return Token::from_comment(std::move(comment), std::move(source_range));
  }

  auto parse_singleline_comment_after_slash() -> std::optional<std::string> {
    std::string comment_value;
    comment_value += '/';  // Include the initial slash

    const auto consume_body_predicate = [&](auto ch) {
      if (ch == '\n' || ch == '\r' || !ch) {
        return false;  // Stop at newline or end of input
      }

      comment_value += static_cast<char>(ch);
      return true;
    };

    if (!consume_while(consume_body_predicate)) {
      spdlog::error("[Lexer] Failed to read the body of a single-line comment");
      return std::nullopt;
    }

    return comment_value;
  }

  auto parse_multiline_comment_after_slash() -> std::optional<std::string> {
    std::string comment_value;
    comment_value += "/";  // Include the initial '/'

    while (true) {
      auto ch = m_lexer.next_byte();
      if (!ch.has_value()) [[unlikely]] {
        spdlog::error("[Lexer] Failed to read the next byte in a multiline comment");
        return std::nullopt;  // End of input or error
      }

      if (*ch == '*' && m_lexer.peek_byte() == '/') {
        comment_value += "*/";  // Include the closing '*/'
        if (!m_lexer.next_byte()) {
          spdlog::error("[Lexer] Failed to read the closing '*/' in a multiline comment");
          return std::nullopt;
        }

        break;
      }

      comment_value += static_cast<char>(*ch);
    }

    return comment_value;
  }

  auto parse_operator_or_punctor_or_comment() -> std::optional<Token> {
    const auto start_position = m_lexer.current_source_location();
    const auto first_ch = m_lexer.next_byte();
    if (!first_ch.has_value()) [[unlikely]] {
      spdlog::error("[Lexer] Failed to read the first byte of an operator, punctor, or comment");
      return std::nullopt;
    }

    const auto ch = *first_ch;
    if (is_punctor(ch) && m_lexer.peek_byte() != ':') {
      auto punctor = *punctor_from_byte(ch);
      const auto end_position = m_lexer.current_source_location();
      auto source_range = FileSourceRange(m_lexer.current_file(), start_position, end_position);

      return Token::from_punctor(punctor, std::move(source_range));
    }

    if (ch == '/') {
      const auto next_ch = m_lexer.peek_byte();
      const auto is_singleline_comment = next_ch == '/';
      const auto is_multiline_comment = next_ch == '*';

      auto comment_value = [&]() -> std::optional<std::string> {
        if (is_singleline_comment) {
          return parse_singleline_comment_after_slash();
        }

        if (is_multiline_comment) {
          return parse_multiline_comment_after_slash();
        }

        return std::nullopt;
      }();

      if (comment_value.has_value()) {
        const auto end_position = m_lexer.current_source_location();
        auto source_range = FileSourceRange(m_lexer.current_file(), start_position, end_position);

        auto flyweight_identifier = boost::flyweight<std::string>(std::move(*comment_value));
        auto comment = Comment(std::move(flyweight_identifier),
                               is_singleline_comment ? CommentType::SingleLine : CommentType::MultiLine);

        return Token::from_comment(std::move(comment), std::move(source_range));
      }
    }

    // If we reach here, it means we encountered an operator or garbage
    std::string operator_value;
    operator_value += static_cast<char>(ch);
    while (true) {
      auto next_ch = m_lexer.peek_byte();
      if (!next_ch.has_value() || !IS_OPERATOR_BYTE[*next_ch]) {
        break;  // Stop if the next byte is not part of an operator
      }

      operator_value += static_cast<char>(*next_ch);
      if (!m_lexer.next_byte().has_value()) [[unlikely]] {
        spdlog::error("[Lexer] Failed to read the next byte of an operator");
        return std::nullopt;  // End of input or error
      }
    }

    if (is_operator(operator_value)) {
      auto op = *operator_from_string(operator_value);
      const auto end_position = m_lexer.current_source_location();
      auto source_range = FileSourceRange(m_lexer.current_file(), start_position, end_position);

      return Token::from_operator(op, std::move(source_range));
    }

    spdlog::error("[Lexer] Unrecognized operator: '{}'", operator_value);

    return std::nullopt;
  }
};

auto Lexer::parse_next_token() -> std::optional<Token> {
  auto lex = LexicalParser(*this);

  if (!lex.skip_bytes_while([](auto ch) { return IS_WHITESPACE[ch]; })) {
    spdlog::error("[Lexer] Failed to skip whitespace");
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
