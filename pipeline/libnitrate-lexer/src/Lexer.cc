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
#include <boost/multiprecision/cpp_dec_float.hpp>
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
#include <sstream>
#include <stack>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "EC.hh"

using namespace ncc;
using namespace ncc::lex;
using namespace ncc::lex::detail;

static constexpr size_t kFloatingPointDigits = 128;
static constexpr size_t kGetcBufferSize = 512;

enum NumberKind : uint8_t {
  DecNum,
  ExpDec,
  HexNum,
  BinNum,
  OctNum,
  Double,
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
    bool is_word = std::all_of(sv.begin(), sv.end(), [](auto c) { return std::isalnum(c) || c == '_'; });

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

  /* Support UTF-8 */
  for (uint8_t c = 0x80; c < 0xff; c++) {
    tab[c] = true;
  }

  return tab;
}();

static constexpr auto kMacroCallCharTable = []() {
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
  tab[':'] = true;

  /* Support UTF-8 */
  for (uint8_t c = 0x80; c < 0xff; c++) {
    tab[c] = true;
  }

  return tab;
}();

static constexpr auto kNumberKindMap = []() {
  std::array<NumberKind, std::numeric_limits<uint16_t>::max() + 1> map = {};
  map.fill(DecNum);

  /* [0x, 0X, 0d, 0D, 0b, 0B, 0o, 0O] */
  map['0' << 8 | 'x'] = HexNum;
  map['0' << 8 | 'X'] = HexNum;
  map['0' << 8 | 'd'] = ExpDec;
  map['0' << 8 | 'D'] = ExpDec;
  map['0' << 8 | 'b'] = BinNum;
  map['0' << 8 | 'B'] = BinNum;
  map['0' << 8 | 'o'] = OctNum;
  map['0' << 8 | 'O'] = OctNum;

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

static constexpr auto kOctalDigitsTable = []() {
  std::array<bool, 256> map = {};
  map.fill(false);

  for (uint8_t c = '0'; c <= '7'; ++c) {
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

static constexpr auto kIdentiferStartTable = []() {
  std::array<bool, 256> map = {};
  map.fill(false);

  for (uint8_t c = 'a'; c <= 'z'; ++c) {
    map[c] = true;
  }

  for (uint8_t c = 'A'; c <= 'Z'; ++c) {
    map[c] = true;
  }

  map['_'] = true;

  /* Support UTF-8 */
  for (uint8_t c = 0x80; c < 0xff; c++) {
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

/// https://stackoverflow.com/questions/1031645/how-to-detect-utf-8-in-plain-c
static auto IsUtf8(const char *string) -> bool {
  qcore_assert(string != nullptr);

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

///============================================================================///

// We use this layer of indirection to ensure that the compiler can have full
// optimization capabilities as if the functions has static linkage.
class Tokenizer::Impl {
public:
  std::string m_buf;

  std::stack<char> m_rewind;

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

  Impl(std::istream &file, std::function<void()> on_eof) : m_file(file), m_on_eof(std::move(on_eof)) {}

  [[nodiscard]] [[gnu::noinline]] std::string LogSource() const {
    std::stringstream ss;
    ss << "at (";

    ss << (m_filename.empty() ? "?" : *m_filename) << ":";
    ss << m_line + 1 << ":" << m_column + 1 << "): ";

    return ss.str();
  }

  [[gnu::noinline]] void ResetAutomaton() {
    m_rewind = {};
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

    if (!m_rewind.empty()) {
      c = m_rewind.top();

      if (c != cmp) {
        return false;
      }

      m_rewind.pop();
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

    if (!m_rewind.empty()) {
      c = m_rewind.top();
      m_rewind.pop();
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

    if (!m_rewind.empty()) {
      c = m_rewind.top();
    } else {
      if (m_getc_buffer_pos == kGetcBufferSize) [[unlikely]] {
        RefillCharacterBuffer();
      }

      c = m_getc_buffer[m_getc_buffer_pos];
    }

    return c;
  }

  static bool ValidateFloat(std::string_view buf) {
    long double _ = 0;
    return std::from_chars(buf.data(), buf.data() + buf.size(), _).ptr == buf.data() + buf.size();
  }

  auto CanonicalizeFloat(std::string &buf) const -> bool {
    const auto e_pos = buf.find('e');
    if (e_pos == std::string::npos) [[likely]] {
      return ValidateFloat(buf);
    }

    qcore_assert(e_pos != 0 && e_pos != buf.size() - 1);

    const std::string_view number = buf;
    const std::string_view mantissa_view = number.substr(0, e_pos);
    const std::string_view exponent_view = number.substr(e_pos + 1);

    if (!ValidateFloat(mantissa_view)) [[unlikely]] {
      Log << InvalidMantissa << LogSource() << "Invalid mantissa in floating point literal";
      return false;
    }

    if (!ValidateFloat(exponent_view)) [[unlikely]] {
      Log << InvalidExponent << LogSource() << "Invalid exponent in floating point literal";
      return false;
    }

    const boost::multiprecision::cpp_dec_float_100 mantissa(mantissa_view);
    const boost::multiprecision::cpp_dec_float_100 exponent(exponent_view);
    const boost::multiprecision::cpp_dec_float_100 float_value = mantissa * boost::multiprecision::pow(10.0, exponent);

    buf = float_value.str(kFloatingPointDigits, std::ios_base::fixed);

    const auto dot_pos = buf.find('.');
    while (buf.size() >= dot_pos + 3 && buf.back() == '0') {
      buf.pop_back();
    }

    return true;
  }

  auto CanonicalizeNumber(std::string &buf, NumberKind type) const -> bool {
    std::transform(buf.begin(), buf.end(), buf.begin(), ::tolower);
    std::erase(buf, '_');

    boost::uint128_type x = 0;

    switch (type) {
      case HexNum: {
        for (uint8_t i : buf) {
          if (((x >> 64) & 0xF000000000000000) != 0U) [[unlikely]] {
            Log << LiteralOutOfRange << LogSource() << "Hexadecimal literal '0x" << buf << "' is out of range";
            Log << LiteralOutOfRange << Notice << LogSource()
                << "The maximum value for a hexadecimal literal is (2^128 - 1) "
                   "aka uint128_t";

            return false;
          }

          qcore_assert(kHexDigitsTable[i]);

          x = (x << 4) + kNibbleTable[i];
        }
        break;
      }

      case BinNum: {
        for (char i : buf) {
          if (((x >> 64) & 0x8000000000000000) != 0U) [[unlikely]] {
            Log << LiteralOutOfRange << LogSource() << "Binary literal '0b" << buf << "' is out of range";
            Log << LiteralOutOfRange << Notice << LogSource()
                << "The maximum value for a binary literal is (2^128 - 1) aka "
                   "uint128_t";

            return false;
          }

          if (i != '0' && i != '1') [[unlikely]] {
            Log << InvalidBinaryDigit << LogSource() << "Unexpected '" << i << "' in binary literal '0b" << buf << "'";
            return false;
          }

          x = (x << 1) + (i - '0');
        }
        break;
      }

      case OctNum: {
        for (char i : buf) {
          if (((x >> 64) & 0xE000000000000000) != 0U) [[unlikely]] {
            Log << LiteralOutOfRange << LogSource() << "Octal literal '0o" << buf << "' is out of range";
            Log << LiteralOutOfRange << Notice << LogSource()
                << "The maximum value for an octal literal is (2^128 - 1) aka "
                   "uint128_t";
            return false;
          }

          if (i < '0' || i > '7') [[unlikely]] {
            Log << InvalidOctalDigit << LogSource() << "Unexpected '" << i << "' in octal literal '0o" << buf << "'";
            return false;
          }

          x = (x << 3) + (i - '0');
        }
        break;
      }

      case ExpDec: {
        for (char i : buf) {
          if (i < '0' || i > '9') [[unlikely]] {
            Log << InvalidDecimalDigit << LogSource() << "Unexpected '" << i << "' in decimal literal '0d" << buf
                << "'";
            return false;
          }

          auto tmp = x;
          x = (x * 10) + (i - '0');

          if (x < tmp) [[unlikely]] {
            Log << LiteralOutOfRange << LogSource() << "Decimal literal '0d" << buf << "' is out of range";
            Log << LiteralOutOfRange << Notice << LogSource()
                << "The maximum value for an explicit decimal literal is "
                   "(2^128 - 1) aka uint128_t";
            return false;
          }
        }
        break;
      }

      case DecNum: {
        for (char i : buf) {
          if (i < '0' || i > '9') [[unlikely]] {
            Log << InvalidDecimalDigit << LogSource() << "Unexpected '" << i << "' in decimal literal '" << buf << "'";
            return false;
          }

          auto tmp = x;
          x = (x * 10) + (i - '0');

          if (x < tmp) [[unlikely]] {
            Log << LiteralOutOfRange << LogSource() << "Decimal literal '" << buf << "' is out of range";
            Log << LiteralOutOfRange << Notice << LogSource()
                << "The maximum value for a decimal literal is (2^128 - 1) aka "
                   "uint128_t";
            return false;
          }
        }
        break;
      }

      case Double:
        __builtin_unreachable();
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
    if (c == 'f') {
      if (auto peekc = PeekChar(); peekc == '"' || peekc == '\'') {
        return {KeyW, __FString, start_pos};
      }
    }

    while (kIdentifierCharTable[c]) {
      m_buf += c;
      c = NextChar();
    }

    m_rewind.push(c);

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

    if (!IsUtf8(m_buf.c_str())) [[unlikely]] {
      Log << InvalidUTF8 << LogSource() << "Invalid UTF-8 sequence in identifier";
      return Token::EndOfFile();
    }

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

              Log << InvalidHexDigit << LogSource() << "Unexpected '" << hex[0] << hex[1]
                  << "' in hexadecimal escape sequence";

              return Token::EndOfFile();
            }

            m_buf += (kNibbleTable[hex[0]] << 4) | kNibbleTable[hex[1]];

            break;
          }

          case 'u': {
            c = NextChar();
            if (c != '{') [[unlikely]] {
              Log << MissingUnicodeBrace << LogSource() << "Missing '{' in unicode escape sequence";

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
                Log << InvalidHexDigit << LogSource() << "Unexpected '" << c << "' in unicode escape sequence";

                ResetAutomaton();
                return Token::EndOfFile();
              }

              hex += c;
            }

            uint32_t codepoint;
            if (std::from_chars(hex.data(), hex.data() + hex.size(), codepoint, 16).ec != std::errc()) {
              Log << InvalidUnicodeCodepoint << LogSource() << "Invalid unicode codepoint";
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

            if (std::from_chars(reinterpret_cast<const char *>(oct.data()),
                                reinterpret_cast<const char *>(oct.data() + oct.size()), val, 8)
                    .ec != std::errc()) {
              Log << InvalidOctalDigit << LogSource() << "Invalid octal digit in escape sequence '" << oct[0] << oct[1]
                  << oct[2] << "'";

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
        NextChar();
        c = NextChar();
        continue;
      }

      if (quote == '\'' && m_buf.size() == 1) {
        return {Char, string(std::string_view(m_buf.data(), 1)), start_pos};
      }

      return {Text, string(std::move(m_buf)), start_pos};
    }
  }

  void ParseIntegerUnwind(uint8_t c, NumberKind kind, std::string &buf) {
    m_rewind.push(c);

    switch (kind) {
      case DecNum:
      case ExpDec: {
        while (!buf.empty()) {
          auto ch = buf.back();
          if (!kDigitsTable[ch]) {
            m_rewind.push(ch);
            buf.pop_back();
          } else {
            break;
          }
        }

        break;
      }

      case HexNum: {
        qcore_assert(!buf.empty() && kHexDigitsTable[buf.back()]);
        break;
      }

      case BinNum: {
        while (!buf.empty()) {
          auto ch = buf.back();
          if (ch != '0' && ch != '1') {
            m_rewind.push(ch);
            buf.pop_back();
          } else {
            break;
          }
        }

        break;
      }

      case OctNum: {
        while (!buf.empty()) {
          auto ch = buf.back();
          if (!kOctalDigitsTable[ch]) {
            m_rewind.push(ch);
            buf.pop_back();
          } else {
            break;
          }
        }

        break;
      }

      case Double: {
        while (!buf.empty()) {
          auto ch = buf.back();
          if (!kDigitsTable[ch]) {
            m_rewind.push(ch);
            buf.pop_back();
          } else {
            break;
          }
        }

        break;
      }
    }
  }

  auto ParseInteger(uint8_t c, LocationID start_pos) -> Token {
    const auto prefix = static_cast<uint16_t>(c) << 8 | PeekChar();
    auto kind = kNumberKindMap[prefix];
    bool end_of_float = false;
    bool is_lexing = true;

    if (kind != DecNum) {
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
        case DecNum: {
          if (c == '.' || c == 'e' || c == 'E' || c == '-' || c == '+') {
            m_buf += c;
            kind = Double;
            c = NextChar();
          } else {
            is_lexing = false;
          }

          break;
        }

        case Double: {
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

    ParseIntegerUnwind(c, kind, m_buf);
    if (m_buf.empty()) {
      Log << InvalidNumber << LogSource() << "Invalid number literal";
      return Token::EndOfFile();
    }

    if (kind == DecNum || kind == Double) {
      kind =
          std::any_of(m_buf.begin(), m_buf.end(), [](auto c) { return c == 'e' || c == 'E' || c == '.' || c == '-'; })
              ? Double
              : DecNum;
    }

    if (kind == Double) {
      if (CanonicalizeFloat(m_buf)) {
        return {NumL, string(std::move(m_buf)), start_pos};
      }

      Log << InvalidNumber << LogSource() << "Floating point literal is not valid";
    } else if (CanonicalizeNumber(m_buf, kind)) [[likely]] {
      return {IntL, string(std::move(m_buf)), start_pos};
    } else {
      Log << InvalidNumber << LogSource() << "Non-floating point literal is not valid";
    }

    ResetAutomaton();

    return Token::EndOfFile();
  }

  auto ParseCommentSingleLine(LocationID start_pos) -> Token {
    uint8_t c;

    while (true) {
      c = NextChar();
      if (c == '\n') {
        break;
      }

      m_buf += c;
    }

    return {Note, string(std::move(m_buf)), start_pos};
  };

  auto ParseCommentMultiLine(LocationID start_pos) -> Token {
    size_t depth = 1;

    while (true) {
      auto c = NextChar();

      /* Support for nested comments */
      if (c == '/' && NextCharIf('*')) [[unlikely]] {
        ++depth;
        m_buf += "/*";
        continue;
      }

      if (c == '*' && NextCharIf('/')) {
        if (--depth == 0) {
          return {Note, string(std::move(m_buf)), start_pos};
        }

        m_buf += "*/";
        continue;
      }

      m_buf += c;
    }
  }

  auto ParseSingleLineMacro(LocationID start_pos) -> Token {
    while (true) {
      auto c = PeekChar();

      if (!kMacroCallCharTable[c]) {
        break;
      }

      m_buf += c;
      NextChar();
    }

    if (!IsUtf8(m_buf.c_str())) [[unlikely]] {
      Log << InvalidUTF8 << LogSource() << "Invalid UTF-8 sequence in macro";
      return Token::EndOfFile();
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

    Log << UnexpectedEOF << LogSource() << "Unexpected EOF while parsing block macro";

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
    if ((m_buf[0] == '~' && NextCharIf('>'))) {
      m_buf.clear();
      return ParseCommentSingleLine(start_pos);
    }

    bool was_match = false;
    while (OPERATOR_SET.count(m_buf) != 0) {
      was_match = true;

      m_buf += NextChar();
    }

    if (!was_match) {
      Log << LexicalGarbage << LogSource() << "Unexpected lexical garbage in source code";
      ResetAutomaton();

      return Token::EndOfFile();
    }

    m_rewind.push(m_buf.back());

    return {Oper, OPERATORS_MAP.find(m_buf.substr(0, m_buf.size() - 1))->second, start_pos};
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

  auto start_pos = InternLocation(Location(impl.m_offset, impl.m_line, impl.m_column, impl.m_filename));

  LexState state;
  if (kIdentiferStartTable[c]) {
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
    case LexState::Identifier:
      token = impl.ParseIdentifier(c, start_pos);
      break;

    case LexState::String:
      token = impl.ParseString(c, start_pos);
      break;

    case LexState::Integer:
      token = impl.ParseInteger(c, start_pos);
      break;

    case LexState::CommentStart:
      if (impl.NextCharIf('/')) {
        token = impl.ParseCommentSingleLine(start_pos);
        break;
      }

      if (impl.NextCharIf('*')) {
        token = impl.ParseCommentMultiLine(start_pos);
        break;
      }

      token = impl.ParseOther('/', start_pos);
      break;

    case LexState::MacroStart:
      if (impl.NextCharIf('(')) {
        token = impl.ParseBlockMacro(start_pos);
        break;
      }

      token = impl.ParseSingleLineMacro(start_pos);
      break;

    case LexState::Other:
      token = impl.ParseOther(c, start_pos);
      break;
  }

  if (token.Is(EofF)) [[unlikely]] {
    SetFailBit();
  }

  return token;
}

Tokenizer::Tokenizer(std::istream &source_file, std::shared_ptr<IEnvironment> env)
    : IScanner(std::move(env)), m_impl(new Impl(source_file, [&]() { SetFailBit(); })) {}

Tokenizer::Tokenizer(Tokenizer &&o) noexcept : IScanner(o.m_env), m_impl(std::move(o.m_impl)) {}

Tokenizer::~Tokenizer() = default;

auto Tokenizer::GetSourceWindow(Point start, Point end, char fillchar) -> std::optional<std::vector<std::string>> {
  Impl &impl = *m_impl;

  if (start.m_x > end.m_x || (start.m_x == end.m_x && start.m_y > end.m_y)) {
    Log << "Invalid source window range";
    return std::nullopt;
  }

  impl.m_file.clear();
  auto current_source_offset = impl.m_file.tellg();
  if (!impl.m_file) {
    Log << "Failed to get the current file offset";
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
      if (line == start.m_x && column == start.m_y) [[unlikely]] {
        is_begin = true;
      } else {
        switch (ch) {
          case '\n': {
            if (line == start.m_x && start.m_y == -1) [[unlikely]] {
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
          long current_line = lines.size() + start.m_x;
          long current_column = line_buf.size();

          if (current_line == end.m_x && current_column == end.m_y) [[unlikely]] {
            spinning = false;
          } else {
            switch (ch) {
              case '\n': {
                if (current_line == end.m_x && end.m_y == -1) [[unlikely]] {
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
    Log << "Failed to seek to the beginning of the file";
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
