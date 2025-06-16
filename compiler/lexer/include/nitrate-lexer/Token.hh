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

#include <nitrate-lexer/Comment.hh>
#include <nitrate-lexer/Identifier.hh>
#include <nitrate-lexer/Keyword.hh>
#include <nitrate-lexer/Number.hh>
#include <nitrate-lexer/Operator.hh>
#include <nitrate-lexer/Punctor.hh>
#include <nitrate-lexer/String.hh>
#include <variant>

namespace nitrate::compiler::lexer {
  enum class TokenType : uint8_t {
    Identifier,
    Keyword,
    Operator,
    Punctor,
    StringLiteral,
    NumberLiteral,
    Comment,
  };

  static inline constexpr size_t TOKEN_TYPE_COUNT = 7;

  class FileSourceLocation {
    uint32_t m_line = 0;
    uint32_t m_column = 0;
    uint32_t m_offset = 0;

  public:
    FileSourceLocation() = default;
    FileSourceLocation(uint32_t line, uint32_t column, uint32_t offset)
        : m_line(line), m_column(column), m_offset(offset) {}

    [[nodiscard]] constexpr auto operator<=>(const FileSourceLocation& other) const -> std::strong_ordering = default;

    [[nodiscard]] auto line() const -> uint32_t { return m_line; }
    [[nodiscard]] auto column() const -> uint32_t { return m_column; }
    [[nodiscard]] auto offset() const -> uint32_t { return m_offset; }
  };

  class FileSourceRange {
    StringData m_file;
    FileSourceLocation m_begin;
    FileSourceLocation m_end;

  public:
    FileSourceRange() = default;
    FileSourceRange(StringData file, FileSourceLocation begin, FileSourceLocation end)
        : m_file(std::move(file)), m_begin(begin), m_end(end) {}

    [[nodiscard]] auto operator<=>(const FileSourceRange& other) const -> std::strong_ordering = default;

    [[nodiscard]] auto file() const -> const std::string& { return m_file.get(); }
    [[nodiscard]] auto begin() const -> FileSourceLocation { return m_begin; }
    [[nodiscard]] auto end() const -> FileSourceLocation { return m_end; }
  };

  inline auto hash_value(const FileSourceRange& range) -> size_t {
    return std::hash<std::string>()(range.file()) ^ std::hash<uint32_t>()(range.begin().offset()) ^
           std::hash<uint32_t>()(range.end().offset());
  }

  class Token {
  public:
    using TokenValue = std::variant<Identifier, Keyword, Operator, Punctor, StringLiteral, NumberLiteral, Comment>;

    [[nodiscard]] static auto from_identifier(Identifier id, FileSourceRange source_range) -> Token {
      return Token{std::move(id), std::move(source_range)};
    }

    [[nodiscard]] static auto from_keyword(Keyword kw, FileSourceRange source_range) -> Token {
      return Token{kw, std::move(source_range)};
    }

    [[nodiscard]] static auto from_operator(Operator op, FileSourceRange source_range) -> Token {
      return Token{op, std::move(source_range)};
    }

    [[nodiscard]] static auto from_punctor(Punctor pun, FileSourceRange source_range) -> Token {
      return Token{pun, std::move(source_range)};
    }

    [[nodiscard]] static auto from_string_literal(StringLiteral str, FileSourceRange source_range) -> Token {
      return Token{std::move(str), std::move(source_range)};
    }

    [[nodiscard]] static auto from_number_literal(NumberLiteral num, FileSourceRange source_range) -> Token {
      return Token{std::move(num), std::move(source_range)};
    }

    [[nodiscard]] static auto from_comment(Comment com, FileSourceRange source_range) -> Token {
      return Token{std::move(com), std::move(source_range)};
    }

    Token(const Token&) = default;
    Token(Token&&) = default;
    auto operator=(const Token&) -> Token& = default;
    auto operator=(Token&&) -> Token& = default;
    ~Token() = default;

    [[nodiscard]] constexpr auto value() const -> const TokenValue& { return m_value; }

    [[nodiscard]] constexpr auto type() const -> TokenType {
      static_assert(std::variant_size_v<TokenValue> == TOKEN_TYPE_COUNT);
      return static_cast<TokenType>(m_value.index());
    }

    [[nodiscard]] constexpr auto source_range() const -> const FileSourceRange& { return m_source_range; }

    auto dump(std::ostream& os, bool include_source_range = true) const -> std::ostream&;

  private:
    explicit Token(TokenValue value, FileSourceRange source_range)
        : m_value(std::move(value)), m_source_range(std::move(source_range)) {}

    TokenValue m_value;
    FileSourceRange m_source_range;
  };
}  // namespace nitrate::compiler::lexer
