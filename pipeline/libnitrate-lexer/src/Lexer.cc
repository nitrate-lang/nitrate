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
///   The Nitrate Toolchain is free software; you can redistribute it or     ///
///   modify it under the terms of the GNU Lesser General Public             ///
///   License as published by the Free Software Foundation; either           ///
///   version 2.1 of the License, or (at your option) any later version.     ///
///                                                                          ///
///   The Nitrate Toolcain is distributed in the hope that it will be        ///
///   useful, but WITHOUT ANY WARRANTY; without even the implied warranty of ///
///   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      ///
///   Lesser General Public License for more details.                        ///
///                                                                          ///
///   You should have received a copy of the GNU Lesser General Public       ///
///   License along with the Nitrate Toolchain; if not, see                  ///
///   <https://www.gnu.org/licenses/>.                                       ///
///                                                                          ///
////////////////////////////////////////////////////////////////////////////////

#include <array>
#include <boost/bimap.hpp>
#include <boost/unordered_map.hpp>
#include <cctype>
#include <charconv>
#include <cmath>
#include <csetjmp>
#include <cstdint>
#include <cstdio>
#include <google/dense_hash_map>
#include <google/dense_hash_set>
#include <iostream>
#include <nitrate-core/Logger.hh>
#include <nitrate-core/Macro.hh>
#include <nitrate-core/String.hh>
#include <nitrate-lexer/Lexer.hh>
#include <nitrate-lexer/Token.hh>
#include <queue>
#include <ranges>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

using namespace ncc;
using namespace ncc::lex;
using namespace ncc::lex::detail;

static constexpr size_t kFloatingPointDigits = 128;
static constexpr size_t kGetcBufferSize = 512;

enum class NumberType : uint8_t {
  Default,
  DecimalExplicit,
  Hexadecimal,
  Binary,
  Octal,
  Floating,
};

enum class LexState : uint8_t {
  Identifier,
  String,
  Integer,
  CommentStart,
  MacroStart,
  Other,
};

static const auto OPERATOR_SET = []() {
  google::dense_hash_set<std::string_view> set;
  set.set_empty_key("");
  for (const auto &op : LEXICAL_OPERATORS.left) {
    set.insert(op.first);
  }
  return set;
}();

static const auto WORD_OPERATORS = []() {
  google::dense_hash_map<std::string_view, Operator> map;
  map.set_empty_key("");

  for (const auto &op : LEXICAL_OPERATORS.left) {
    std::string_view sv = op.first;
    bool is_word = std::all_of(sv.begin(), sv.end(), [](auto c) {
      return std::isalnum(c) || c == '_';
    });

    if (is_word) {
      map[op.first] = op.second;
    }
  }

  return map;
}();

static const auto KEYWORDS_MAP = []() {
  google::dense_hash_map<std::string_view, Keyword> map;
  map.set_empty_key("");

  for (const auto &kw : LEXICAL_KEYWORDS.left) {
    map[kw.first] = kw.second;
  }

  return map;
}();

static const auto OPERATORS_MAP = []() {
  google::dense_hash_map<std::string_view, Operator> map;
  map.set_empty_key("");

  for (const auto &kw : LEXICAL_OPERATORS.left) {
    map[kw.first] = kw.second;
  }

  return map;
}();

static const auto PUNCTORS_MAP = []() {
  google::dense_hash_map<std::string_view, Punctor> map;
  map.set_empty_key("");

  for (const auto &kw : LEXICAL_PUNCTORS.left) {
    map[kw.first] = kw.second;
  }

  return map;
}();

static constexpr auto kNibbleTable = []() {
  std::array<uint8_t, 256> hextable = {};
  hextable['0'] = 0;
  hextable['1'] = 1;
  hextable['2'] = 2;
  hextable['3'] = 3;
  hextable['4'] = 4;
  hextable['5'] = 5;
  hextable['6'] = 6;
  hextable['7'] = 7;
  hextable['8'] = 8;
  hextable['9'] = 9;
  hextable['A'] = 10;
  hextable['B'] = 11;
  hextable['C'] = 12;
  hextable['D'] = 13;
  hextable['E'] = 14;
  hextable['F'] = 15;
  hextable['a'] = 10;
  hextable['b'] = 11;
  hextable['c'] = 12;
  hextable['d'] = 13;
  hextable['e'] = 14;
  hextable['f'] = 15;
  return hextable;
}();

static constexpr auto kWhitespaceTable = []() {
  std::array<bool, 256> tab = {};

  tab[' '] = true;
  tab['\f'] = true;
  tab['\n'] = true;
  tab['\r'] = true;
  tab['\t'] = true;
  tab['\v'] = true;
  tab['\\'] = true;
  tab['\0'] = true;  // Null byte is also a whitespace character

  return tab;
}();

static constexpr auto kIdentifierCharTable = []() {
  std::array<bool, 256> tab = {};

  for (uint8_t c = 'a'; c <= 'z'; ++c) {
    tab[c] = true;
  }

  for (uint8_t c = 'A'; c <= 'Z'; ++c) {
    tab[c] = true;
  }

  for (uint8_t c = '0'; c <= '9'; ++c) {
    tab[c] = true;
  }

  tab['_'] = true;

  return tab;
}();

static constexpr auto kNumberTypeMap = []() {
  std::array<NumberType, std::numeric_limits<uint16_t>::max() + 1> map = {};
  map.fill(NumberType::Default);

  /* [0x, 0X, 0d, 0D, 0b, 0B, 0o, 0O] */
  map['0' << 8 | 'x'] = NumberType::Hexadecimal;
  map['0' << 8 | 'X'] = NumberType::Hexadecimal;
  map['0' << 8 | 'd'] = NumberType::DecimalExplicit;
  map['0' << 8 | 'D'] = NumberType::DecimalExplicit;
  map['0' << 8 | 'b'] = NumberType::Binary;
  map['0' << 8 | 'B'] = NumberType::Binary;
  map['0' << 8 | 'o'] = NumberType::Octal;
  map['0' << 8 | 'O'] = NumberType::Octal;

  return map;
}();

static constexpr auto kHexDigitsTable = []() {
  std::array<bool, 256> map = {};
  map.fill(false);

  for (uint8_t c = '0'; c <= '9'; ++c) {
    map[c] = true;
  }

  for (uint8_t c = 'a'; c <= 'f'; ++c) {
    map[c] = true;
  }

  for (uint8_t c = 'A'; c <= 'F'; ++c) {
    map[c] = true;
  }

  return map;
}();

static constexpr auto kDigitsTable = []() {
  std::array<bool, 256> map = {};
  map.fill(false);

  for (uint8_t c = '0'; c <= '9'; ++c) {
    map[c] = true;
  }

  return map;
}();

static constexpr auto kAlphabetTable = []() {
  std::array<bool, 256> map = {};
  map.fill(false);

  for (uint8_t c = 'a'; c <= 'z'; ++c) {
    map[c] = true;
  }

  for (uint8_t c = 'A'; c <= 'Z'; ++c) {
    map[c] = true;
  }

  return map;
}();

constexpr auto kCharMap = []() constexpr {
  std::array<uint8_t, 256> map = {};
  for (size_t i = 0; i < 256; ++i) {
    map[i] = (uint8_t)i;
  }

  map['0'] = '\0';
  map['a'] = '\a';
  map['b'] = '\b';
  map['t'] = '\t';
  map['n'] = '\n';
  map['v'] = '\v';
  map['f'] = '\f';
  map['r'] = '\r';

  map['x'] = -1;
  map['u'] = -1;
  map['o'] = -1;

  return map;
}();

///============================================================================///

static std::string LexerECFormatter(std::string_view msg, Sev sev) {
  if (sev <= ncc::Debug) {
    return "\x1b[37;1m[\x1b[0m\x1b[34;1mLexer\x1b[0m\x1b[37;1m]: debug: " +
           std::string(msg) + "\x1b[0m";
  }

  return "\x1b[37;1m[\x1b[0m\x1b[34;1mLexer\x1b[0m\x1b[37;1m]: " +
         std::string(msg) + "\x1b[0m";
}

NCC_EC_GROUP(Lexer);

NCC_EC_EX(Lexer, UserRequest, LexerECFormatter);
NCC_EC_EX(Lexer, UnexpectedEOF, LexerECFormatter);

NCC_EC_EX(Lexer, LiteralOutOfRange, LexerECFormatter);
NCC_EC_EX(Lexer, InvalidNumber, LexerECFormatter);
NCC_EC_EX(Lexer, InvalidMantissa, LexerECFormatter);
NCC_EC_EX(Lexer, InvalidExponent, LexerECFormatter);

NCC_EC_EX(Lexer, InvalidHexDigit, LexerECFormatter);
NCC_EC_EX(Lexer, InvalidDecimalDigit, LexerECFormatter);
NCC_EC_EX(Lexer, InvalidOctalDigit, LexerECFormatter);
NCC_EC_EX(Lexer, InvalidBinaryDigit, LexerECFormatter);

NCC_EC_EX(Lexer, MissingUnicodeBrace, LexerECFormatter);
NCC_EC_EX(Lexer, InvalidUnicodeCodepoint, LexerECFormatter);
NCC_EC_EX(Lexer, LexicalGarbage, LexerECFormatter);

// We use this layer of indirection to ensure that the compiler can have full
// optimization capabilities as if the functions has static linkage.
class Tokenizer::Impl {
public:
  std::string m_buf;

  std::queue<char> m_fifo;

  uint32_t m_offset = 0;
  uint32_t m_line = 0;
  uint32_t m_column = 0;

  uint32_t m_getc_buffer_pos = kGetcBufferSize;
  std::istream &m_file;
  std::array<char, kGetcBufferSize> m_getc_buffer;

  string m_filename;
  bool m_at_end = false;
  bool m_parsing = false;

  std::function<void()> m_on_eof;

  ///============================================================================///

  Impl(std::istream &file, std::function<void()> on_eof)
      : m_file(file), m_on_eof(std::move(on_eof)) {}

  [[nodiscard]] [[gnu::noinline]] std::string LogSource() const {
    std::stringstream ss;
    ss << "at (";

    ss << (m_filename->empty() ? "?" : *m_filename) << ":";
    ss << m_line + 1 << ":" << m_column + 1 << "): ";

    return ss.str();
  }

  [[gnu::noinline]] void ResetAutomaton() {
    m_fifo = {};
    m_buf.clear();
  }

  void RefillCharacterBuffer() {
    m_file.read(m_getc_buffer.data(), kGetcBufferSize);
    auto gcount = m_file.gcount();

    if (gcount == 0) [[unlikely]] {
      if (m_at_end) [[unlikely]] {
        if (m_parsing) [[unlikely]] {
          Log << UnexpectedEOF << "Unexpected EOF while reading file";
          m_on_eof();
        }

        // Benchmarks show that this is the fastest way to signal EOF
        // for large files.
        throw ScannerEOF();
      }

      m_at_end = true;
    }

    // Fill extra buffer with '#' with is a comment character
    memset(m_getc_buffer.data() + gcount, '\n', kGetcBufferSize - gcount);
    m_getc_buffer_pos = 0;
  }

  auto NextCharIf(uint8_t cmp) -> bool {
    uint8_t c;

    if (!m_fifo.empty()) {
      c = m_fifo.front();

      if (c != cmp) {
        return false;
      }

      m_fifo.pop();
    } else {
      if (m_getc_buffer_pos == kGetcBufferSize) [[unlikely]] {
        RefillCharacterBuffer();
      }

      c = m_getc_buffer[m_getc_buffer_pos];

      if (c != cmp) {
        return false;
      }

      m_getc_buffer_pos++;
    }

    m_offset += 1;
    m_line = m_line + static_cast<uint32_t>(c == '\n');
    m_column = static_cast<uint32_t>(c != '\n') * (m_column + 1);

    return true;
  }

  auto NextChar() -> uint8_t {
    uint8_t c;

    if (!m_fifo.empty()) {
      c = m_fifo.front();
      m_fifo.pop();
    } else {
      if (m_getc_buffer_pos == kGetcBufferSize) [[unlikely]] {
        RefillCharacterBuffer();
      }

      c = m_getc_buffer[m_getc_buffer_pos++];
    }

    m_offset += 1;
    m_line = m_line + static_cast<uint32_t>(c == '\n');
    m_column = static_cast<uint32_t>(c != '\n') * (m_column + 1);

    return c;
  }

  auto PeekChar() -> uint8_t {
    uint8_t c;

    if (!m_fifo.empty()) {
      c = m_fifo.front();
    } else {
      if (m_getc_buffer_pos == kGetcBufferSize) [[unlikely]] {
        RefillCharacterBuffer();
      }

      c = m_getc_buffer[m_getc_buffer_pos];
    }

    return c;
  }

  auto CanonicalizeFloat(std::string &buf) const -> bool {
    auto e_pos = buf.find('e');
    if (e_pos == std::string::npos) [[likely]] {
      return true;
    }

    long double mantissa = 0;

    if (std::from_chars(buf.data(), buf.data() + e_pos, mantissa).ec !=
        std::errc()) [[unlikely]] {
      Log << InvalidMantissa << LogSource()
          << "Invalid mantissa in floating point literal";
      return false;
    }

    long double exponent = 0;
    if (std::from_chars(buf.data() + e_pos + 1, buf.data() + buf.size(),
                        exponent)
            .ec != std::errc()) [[unlikely]] {
      Log << InvalidExponent << LogSource()
          << "Invalid exponent in floating point literal";
      return false;
    }

    long double float_value = mantissa * std::pow(10.0, exponent);

    static_assert(kFloatingPointDigits >= 1,
                  "Floating point precision must be at least 1");

    std::array<char, kFloatingPointDigits + 2> buffer;
    if (std::snprintf(buffer.data(), buffer.size(), "%.*Lf",
                      static_cast<int>(kFloatingPointDigits), float_value) < 0)
        [[unlikely]] {
      Log << InvalidNumber << LogSource()
          << "Failed to serialize floating point literal";
      return false;
    }

    std::string str(buffer.data());

    if (str.find('.') != std::string::npos) {
      while (!str.empty() && str.back() == '0') {
        str.pop_back();
      }

      if (!str.empty() && str.back() == '.') {
        str.pop_back();
      }
    }

    buf = std::move(str);

    return true;
  }

  auto CanonicalizeNumber(std::string &buf, NumberType type) const -> bool {
    std::transform(buf.begin(), buf.end(), buf.begin(), ::tolower);
    std::erase(buf, '_');

    boost::uint128_type x = 0;

    switch (type) {
      case NumberType::Hexadecimal: {
        for (uint8_t i : buf) {
          if (((x >> 64) & 0xF000000000000000) != 0U) [[unlikely]] {
            Log << LiteralOutOfRange << LogSource() << "Hexadecimal literal '0x"
                << buf << "' is out of range";
            Log << LiteralOutOfRange << Notice << LogSource()
                << "The maximum value for a hexadecimal literal is (2^128 - 1) "
                   "aka uint128_t";

            return false;
          }

          if (!kHexDigitsTable[i]) [[unlikely]] {
            Log << InvalidHexDigit << LogSource() << "Unexpected '" << i
                << "' in hexadecimal literal '0x" << buf << "'";
            return false;
          }

          x = (x << 4) + kNibbleTable[i];
        }
        break;
      }

      case NumberType::Binary: {
        for (char i : buf) {
          if (((x >> 64) & 0x8000000000000000) != 0U) [[unlikely]] {
            Log << LiteralOutOfRange << LogSource() << "Binary literal '0b"
                << buf << "' is out of range";
            Log << LiteralOutOfRange << Notice << LogSource()
                << "The maximum value for a binary literal is (2^128 - 1) aka "
                   "uint128_t";

            return false;
          }

          if (i != '0' && i != '1') [[unlikely]] {
            Log << InvalidBinaryDigit << LogSource() << "Unexpected '" << i
                << "' in binary literal '0b" << buf << "'";
            return false;
          }

          x = (x << 1) + (i - '0');
        }
        break;
      }

      case NumberType::Octal: {
        for (char i : buf) {
          if (((x >> 64) & 0xE000000000000000) != 0U) [[unlikely]] {
            Log << LiteralOutOfRange << LogSource() << "Octal literal '0o"
                << buf << "' is out of range";
            Log << LiteralOutOfRange << Notice << LogSource()
                << "The maximum value for an octal literal is (2^128 - 1) aka "
                   "uint128_t";
            return false;
          }

          if (i < '0' || i > '7') [[unlikely]] {
            Log << InvalidOctalDigit << LogSource() << "Unexpected '" << i
                << "' in octal literal '0o" << buf << "'";
            return false;
          }

          x = (x << 3) + (i - '0');
        }
        break;
      }

      case NumberType::DecimalExplicit: {
        for (char i : buf) {
          if (i < '0' || i > '9') [[unlikely]] {
            Log << InvalidDecimalDigit << LogSource() << "Unexpected '" << i
                << "' in decimal literal '0d" << buf << "'";
            return false;
          }

          auto tmp = x;
          x = (x * 10) + (i - '0');

          if (x < tmp) [[unlikely]] {
            Log << LiteralOutOfRange << LogSource() << "Decimal literal '0d"
                << buf << "' is out of range";
            Log << LiteralOutOfRange << Notice << LogSource()
                << "The maximum value for an explicit decimal literal is "
                   "(2^128 - 1) aka uint128_t";
            return false;
          }
        }
        break;
      }

      case NumberType::Default: {
        for (char i : buf) {
          if (i < '0' || i > '9') [[unlikely]] {
            Log << InvalidDecimalDigit << LogSource() << "Unexpected '" << i
                << "' in decimal literal '" << buf << "'";
            return false;
          }

          auto tmp = x;
          x = (x * 10) + (i - '0');

          if (x < tmp) [[unlikely]] {
            Log << LiteralOutOfRange << LogSource() << "Decimal literal '"
                << buf << "' is out of range";
            Log << LiteralOutOfRange << Notice << LogSource()
                << "The maximum value for a decimal literal is (2^128 - 1) aka "
                   "uint128_t";
            return false;
          }
        }
        break;
      }

      case NumberType::Floating: {
        __builtin_unreachable();
      }
    }

    if (x == 0) {
      buf = "0";
      return true;
    }

    std::array<char, sizeof("340282366920938463463374607431768211455")> buffer;
    char *buffer_ptr = buffer.data();

    for (auto i = x; i > 0; i /= 10) {
      *buffer_ptr++ = '0' + i % 10;
    }

    std::reverse(buffer.data(), buffer_ptr);
    buf.assign(buffer.data(), buffer_ptr);

    return true;
  }

  auto ParseIdentifier(uint8_t c, LocationID start_pos) -> Token {
    if (c == 'f' && PeekChar() == '"') {
      return {KeyW, __FString, start_pos};
    }

    while (kIdentifierCharTable[c]) {
      m_buf += c;
      c = NextChar();
    }

    m_fifo.push(c);

    { /* Determine if it's a keyword */
      auto it = KEYWORDS_MAP.find(m_buf);
      if (it != KEYWORDS_MAP.end()) {
        return {KeyW, it->second, start_pos};
      }
    }

    { /* Determine if it's an operator */
      auto it = WORD_OPERATORS.find(m_buf);
      if (it != WORD_OPERATORS.end()) {
        return {Oper, it->second, start_pos};
      }
    }

    /* For compiler internal debugging */
    qcore_assert(m_buf != "__builtin_lexer_crash",
                 "The source code invoked a compiler panic API.");

    if (m_buf == "__builtin_lexer_abort") {
      Log << Debug << UserRequest << LogSource()
          << "The source code requested lexical truncation";
      return Token::EndOfFile();
    }

    /* Return the identifier */
    return {Name, string(std::move(m_buf)), start_pos};
  };

  auto ParseString(uint8_t c, LocationID start_pos) -> Token {
    auto quote = c;
    c = NextChar();

    while (true) {
      while (c != quote) [[likely]] {
        if (c != '\\') [[likely]] {
          m_buf += c;
          c = NextChar();
          continue;
        }

        c = NextChar();

        if (kCharMap[c] != 0xff) {
          m_buf += kCharMap[c];
          c = NextChar();
          continue;
        }

        switch (c) {
          case 'x': {
            std::array hex = {NextChar(), NextChar()};
            if (!kHexDigitsTable[hex[0]] || !kHexDigitsTable[hex[1]]) {
              ResetAutomaton();

              Log << InvalidHexDigit << LogSource() << "Unexpected '" << hex[0]
                  << hex[1] << "' in hexadecimal escape sequence";

              return Token::EndOfFile();
            }

            m_buf += (kNibbleTable[hex[0]] << 4) | kNibbleTable[hex[1]];

            break;
          }

          case 'u': {
            c = NextChar();
            if (c != '{') [[unlikely]] {
              Log << MissingUnicodeBrace << LogSource()
                  << "Missing '{' in unicode escape sequence";

              ResetAutomaton();
              return Token::EndOfFile();
            }

            std::string hex;

            while (true) {
              c = NextChar();
              if (c == '}') {
                break;
              }

              if (!kHexDigitsTable[c]) {
                Log << InvalidHexDigit << LogSource() << "Unexpected '" << c
                    << "' in unicode escape sequence";

                ResetAutomaton();
                return Token::EndOfFile();
              }

              hex += c;
            }

            uint32_t codepoint;
            if (std::from_chars(hex.data(), hex.data() + hex.size(), codepoint,
                                16)
                    .ec != std::errc()) {
              Log << InvalidUnicodeCodepoint << LogSource()
                  << "Invalid unicode codepoint";
              ResetAutomaton();

              return Token::EndOfFile();
            }

            if (codepoint < 0x80) {
              m_buf += (char)codepoint;
            } else if (codepoint < 0x800) {
              m_buf += (char)(0xC0 | (codepoint >> 6));
              m_buf += (char)(0x80 | (codepoint & 0x3F));
            } else if (codepoint < 0x10000) {
              m_buf += (char)(0xE0 | (codepoint >> 12));
              m_buf += (char)(0x80 | ((codepoint >> 6) & 0x3F));
              m_buf += (char)(0x80 | (codepoint & 0x3F));
            } else if (codepoint < 0x110000) {
              m_buf += (char)(0xF0 | (codepoint >> 18));
              m_buf += (char)(0x80 | ((codepoint >> 12) & 0x3F));
              m_buf += (char)(0x80 | ((codepoint >> 6) & 0x3F));
              m_buf += (char)(0x80 | (codepoint & 0x3F));
            } else {
              Log << InvalidUnicodeCodepoint << LogSource()
                  << "Codepoint does not embody a "
                     "valid UTF-8 character";

              ResetAutomaton();

              return Token::EndOfFile();
            }

            break;
          }

          case 'o': {
            std::array<uint8_t, 3> oct = {NextChar(), NextChar(), NextChar()};
            uint8_t val;

            if (std::from_chars(
                    reinterpret_cast<const char *>(oct.data()),
                    reinterpret_cast<const char *>(oct.data() + oct.size()),
                    val, 8)
                    .ec != std::errc()) {
              Log << InvalidOctalDigit << LogSource()
                  << "Invalid octal digit in escape sequence '" << oct[0]
                  << oct[1] << oct[2] << "'";

              ResetAutomaton();

              return Token::EndOfFile();
            }

            m_buf += val;
            break;
          }
        }

        c = NextChar();
      }

      while (!m_at_end) {
        c = PeekChar();
        if (kWhitespaceTable[c]) {
          NextChar();
        } else {
          break;
        }
      }

      /* Check for a multi-part string */
      if (c == quote) {
        c = NextChar();
        continue;
      }

      if (quote == '\'' && m_buf.size() == 1) {
        return {Char, string(std::string_view(m_buf.data(), 1)), start_pos};
      }

      return {Text, string(std::move(m_buf)), start_pos};
    }
  }

  void ParseIntegerUnwind(uint8_t c, std::string &buf) {
    bool spinning = true;

    std::vector<uint8_t> q;

    while (!buf.empty() && spinning) {
      switch (uint8_t ch = buf.back()) {
        case '.':
        case 'e':
        case 'E': {
          q.push_back(ch);
          buf.pop_back();
          break;
        }

        default: {
          spinning = false;
          break;
        }
      }
    }

    for (auto it : std::ranges::reverse_view(q)) {
      m_fifo.push(it);
    }

    m_fifo.push(c);
  }

  auto ParseInteger(uint8_t c, LocationID start_pos) -> Token {
    uint16_t prefix = static_cast<uint16_t>(c) << 8 | PeekChar();
    auto kind = kNumberTypeMap[prefix];
    bool end_of_float = false;
    bool is_lexing = true;

    if (kind != NumberType::Default) {
      NextChar();
      c = NextChar();
    }

    do {
      if (kWhitespaceTable[c]) {
        break;
      }

      if (c == '_') [[unlikely]] {
        while (!m_at_end && (kWhitespaceTable[c] || c == '_')) [[unlikely]] {
          c = NextChar();
        }
      }

      if (kHexDigitsTable[c]) [[likely]] {
        m_buf += c;
        c = NextChar();
        continue;
      }

      switch (kind) {
        case NumberType::Default: {
          if (c == '.') {
            m_buf += c;
            kind = NumberType::Floating;
            c = NextChar();
          } else {
            is_lexing = false;
          }

          break;
        }

        case NumberType::Floating: {
          if (end_of_float) {
            is_lexing = false;
            break;
          }

          if (c == '-') {
            m_buf += c;
            c = NextChar();
          } else if (c == '+') {
            // ignored
            c = NextChar();
          } else if (c == '.') {
            m_buf += c;
            end_of_float = true;
            c = NextChar();
          } else {
            is_lexing = false;
          }

          break;
        }

        default: {
          is_lexing = false;
          break;
        }
      }
    } while (is_lexing);

    ParseIntegerUnwind(c, m_buf);

    if (kind == NumberType::Default) {
      kind = std::any_of(m_buf.begin(), m_buf.end(),
                         [](auto c) { return c == 'e' || c == 'E'; })
                 ? NumberType::Floating
                 : NumberType::Default;
    }

    if (kind == NumberType::Floating) {
      if (CanonicalizeFloat(m_buf)) {
        return {NumL, string(std::move(m_buf)), start_pos};
      }

      Log << InvalidNumber << LogSource()
          << "Floating point literal is not valid";
    } else if (CanonicalizeNumber(m_buf, kind)) [[likely]] {
      return {IntL, string(std::move(m_buf)), start_pos};
    } else {
      Log << InvalidNumber << LogSource()
          << "Non-floating point literal is not valid";
    }

    ResetAutomaton();

    return Token::EndOfFile();
  }

  auto ParseCommentSingleLine(LocationID start_pos) -> Token {
    uint8_t c;

    do {
      c = NextChar();
      m_buf += c;
    } while (c != '\n');

    return {Note, string(std::move(m_buf)), start_pos};
  };

  auto ParseCommentMultiLine(LocationID start_pos) -> Token {
    while (true) {
      auto c = NextChar();

      if (c == '*') {
        if (NextCharIf('/')) {
          return {Note, string(std::move(m_buf)), start_pos};
        }
      }

      m_buf += c;
    }
  }

  auto ParseSingleLineMacro(LocationID start_pos) -> Token {
    while (true) {
      auto c = PeekChar();

      if (!kIdentifierCharTable[c]) {
        break;
      }

      m_buf += c;

      NextChar();
    }

    return {Macr, string(std::move(m_buf)), start_pos};
  }

  auto ParseBlockMacro(LocationID start_pos) -> Token {
    uint32_t state_parens = 1;

    while (!m_at_end) {
      auto c = NextChar();

      if (c == '(') {
        state_parens++;
      } else if (c == ')') {
        state_parens--;
      }

      if (state_parens == 0) {
        return {MacB, string(std::move(m_buf)), start_pos};
      }

      m_buf += c;
    }

    Log << UnexpectedEOF << LogSource()
        << "Unexpected EOF while parsing block macro";

    return Token::EndOfFile();
  }

  auto ParseOther(uint8_t c, LocationID start_pos) -> Token {
    if (c == '#') {
      return ParseCommentSingleLine(start_pos);
    }

    if (c == ':' && NextCharIf(':')) {
      return {Punc, PuncScope, start_pos};
    }

    {
      auto punc_sv = std::string_view(reinterpret_cast<char *>(&c), 1);
      auto it = PUNCTORS_MAP.find(punc_sv);
      if (it != PUNCTORS_MAP.end()) {
        return {Punc, it->second, start_pos};
      }
    }

    m_buf += c;
    c = NextChar();

    if ((m_buf[0] == '~' && c == '>')) {
      m_buf.clear();
      return ParseCommentSingleLine(start_pos);
    }

    auto found = false;
    while (true) {
      auto contains = false;
      if (OPERATOR_SET.count(m_buf) != 0U) {
        contains = true;
        found = true;
      }

      if (contains) {
        m_buf += c;
        if (m_buf.size() > 4) { /* Handle infinite error case */
          Log << LexicalGarbage << LogSource()
              << "Unexpected lexical garbage in source code";
          ResetAutomaton();

          return Token::EndOfFile();
        }
        c = NextChar();
      } else {
        break;
      }
    }

    if (!found) {
      Log << LexicalGarbage << LogSource()
          << "Unexpected lexical garbage in source code";
      ResetAutomaton();

      return Token::EndOfFile();
    }

    m_fifo.push(m_buf.back());
    m_fifo.push(c);

    return {Oper, OPERATORS_MAP.find(m_buf.substr(0, m_buf.size() - 1))->second,
            start_pos};
  }
};

auto Tokenizer::GetNext() -> Token {
  Impl &impl = *m_impl;
  impl.m_buf.clear();
  impl.m_parsing = false;

  uint8_t c;
  do {
    c = impl.NextChar();
  } while (kWhitespaceTable[c]);

  impl.m_parsing = true;

  auto start_pos = InternLocation(
      Location(impl.m_offset, impl.m_line, impl.m_column, impl.m_filename));

  LexState state;
  if (kAlphabetTable[c] || c == '_') {
    state = LexState::Identifier;
  } else if (c == '/') {
    state = LexState::CommentStart;
  } else if (kDigitsTable[c]) {
    state = LexState::Integer;
  } else if (c == '"' || c == '\'') {
    state = LexState::String;
  } else if (c == '@') {
    state = LexState::MacroStart;
  } else {
    state = LexState::Other;
  }

  Token token;

  switch (state) {
    case LexState::Identifier: {
      token = impl.ParseIdentifier(c, start_pos);
      break;
    }

    case LexState::String: {
      token = impl.ParseString(c, start_pos);
      break;
    }

    case LexState::Integer: {
      token = impl.ParseInteger(c, start_pos);
      break;
    }

    case LexState::CommentStart: {
      if (impl.NextCharIf('/')) {
        token = impl.ParseCommentSingleLine(start_pos);
        break;
      }

      if (impl.NextCharIf('*')) {
        token = impl.ParseCommentMultiLine(start_pos);
        break;
      }

      /* Divide operator */
      token = {Oper, OpSlash, start_pos};
      break;
    }

    case LexState::MacroStart: {
      if (impl.NextCharIf('(')) {
        token = impl.ParseBlockMacro(start_pos);
        break;
      }

      token = impl.ParseSingleLineMacro(start_pos);
      break;
    }

    case LexState::Other: {
      token = impl.ParseOther(c, start_pos);
      break;
    }
  }

  if (token.is(EofF)) [[unlikely]] {
    SetFailBit();
  }

  return token;
}

Tokenizer::Tokenizer(std::istream &source_file,
                     std::shared_ptr<Environment> env)
    : IScanner(std::move(env)),
      m_impl(new Impl(source_file, [&]() { SetFailBit(); })) {}

Tokenizer::~Tokenizer() = default;

auto Tokenizer::GetSourceWindow(Point start, Point end, char fillchar)
    -> std::optional<std::vector<std::string>> {
  Impl &impl = *m_impl;

  if (start.x > end.x || (start.x == end.x && start.y > end.y)) {
    qcore_print(QCORE_ERROR, "Invalid source window range");
    return std::nullopt;
  }

  impl.m_file.clear();
  auto current_source_offset = impl.m_file.tellg();
  if (!impl.m_file) {
    qcore_print(QCORE_ERROR, "Failed to get the current file offset");
    return std::nullopt;
  }

  if (impl.m_file.seekg(0, std::ios::beg)) {
    long line = 0;
    long column = 0;

    bool spinning = true;
    while (spinning) {
      int ch = impl.m_file.get();
      if (ch == EOF) [[unlikely]] {
        break;
      }

      bool is_begin = false;
      if (line == start.x && column == start.y) [[unlikely]] {
        is_begin = true;
      } else {
        switch (ch) {
          case '\n': {
            if (line == start.x && start.y == -1) [[unlikely]] {
              is_begin = true;
            }

            line++;
            column = 0;
            break;
          }

          default: {
            column++;
            break;
          }
        }
      }

      if (is_begin) [[unlikely]] {
        std::vector<std::string> lines;
        std::string line_buf;

        do {
          long current_line = lines.size() + start.x;
          long current_column = line_buf.size();

          if (current_line == end.x && current_column == end.y) [[unlikely]] {
            spinning = false;
          } else {
            switch (ch) {
              case '\n': {
                if (current_line == end.x && end.y == -1) [[unlikely]] {
                  spinning = false;
                }

                lines.push_back(line_buf);
                line_buf.clear();
                break;
              }

              default: {
                line_buf.push_back(ch);
                break;
              }
            }
          }

          ch = impl.m_file.get();
          if (ch == EOF) [[unlikely]] {
            spinning = false;
          }
        } while (spinning);

        if (!line_buf.empty()) {
          lines.push_back(line_buf);
        }

        size_t max_length = 0;
        for (const auto &line : lines) {
          max_length = std::max(max_length, line.size());
        }

        for (auto &line : lines) {
          if (line.size() < max_length) {
            line.append(max_length - line.size(), fillchar);
          }
        }

        impl.m_file.seekg(current_source_offset);
        return lines;
      }
    }
  } else {
    qcore_print(QCORE_ERROR, "Failed to seek to the beginning of the file");
  }

  impl.m_file.seekg(current_source_offset);

  return std::nullopt;
}

auto Tokenizer::SetCurrentFilename(string filename) -> string {
  auto old = m_impl->m_filename;
  m_impl->m_filename = filename;
  return old;
}

auto Tokenizer::GetCurrentFilename() -> string { return m_impl->m_filename; }
