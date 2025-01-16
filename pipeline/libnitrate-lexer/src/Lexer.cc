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
#include <nitrate-core/Logger.hh>
#include <nitrate-core/Macro.hh>
#include <nitrate-core/String.hh>
#include <nitrate-lexer/Lexer.hh>
#include <nitrate-lexer/Token.hh>
#include <queue>
#include <ranges>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

using namespace ncc;
using namespace ncc::lex;
using namespace ncc::lex::detail;

static constexpr size_t kFloatingPointDigits = 100;
static constexpr size_t kGetcBufferSize = 512;

enum class NumberLiteralType : char {
  Decimal,
  DecimalExplicit,
  Hexadecimal,
  Binary,
  Octal,
  Floating,
};

enum class LexState : char {
  Start,
  Identifier,
  String,
  Integer,
  CommentStart,
  CommentSingleLine,
  CommentMultiLine,
  MacroStart,
  SingleLineMacro,
  BlockMacro,
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

static constexpr auto kHextable = []() {
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
  tab['\0'] = true;  // Null byte is also a whitespace character

  return tab;
}();

///============================================================================///

static NCC_FORCE_INLINE auto ValidateIdentifier(std::string_view id) -> bool {
  /*
   * This state machine checks if the identifier looks
   * like 'a::b::c::d_::e::f'.
   */

  int state = 0;

  for (char c : id) {
    switch (state) {
      case 0:
        if ((std::isalnum(c) != 0) || c == '_') {
          continue;
        }
        if (c == ':') {
          state = 1;
          continue;
        }
        return false;
      case 1:
        if (c == ':') {
          state = 0;
          continue;
        }
        return false;
    }
  }

  return state == 0;
}

// We use this layer of indirection to ensure that the compiler can have full
// optimization capabilities as if the functions has static linkage.
class Tokenizer::Impl {
public:
  std::string buf;  /// NOLINT

  std::queue<char> m_fifo;

  uint32_t m_offset = 0;
  uint32_t m_line = 0;
  uint32_t m_column = 0;

  uint32_t m_getc_buffer_pos = kGetcBufferSize;
  std::array<char, kGetcBufferSize> m_getc_buffer;

  string m_filename;

  std::istream &m_file;
  std::shared_ptr<Environment> m_env;

  ///============================================================================///

  Impl(std::istream &file) : m_file(file) {}

  void ResetAutomaton() { m_fifo = std::queue<char>(); }

  void RefillCharacterBuffer() {
    m_file.read(m_getc_buffer.data(), kGetcBufferSize);
    auto gcount = m_file.gcount();

    if (gcount == 0) [[unlikely]] {
      // Benchmarks show that this is the fastest way to signal EOF
      // for large files.
      throw ScannerEOF();
    }

    // Fill extra buffer with '#' with is a comment character
    memset(m_getc_buffer.data() + gcount, '\n', kGetcBufferSize - gcount);
    m_getc_buffer_pos = 0;
  }

  NCC_FORCE_INLINE auto GetChar() -> char {
    if (m_getc_buffer_pos == kGetcBufferSize) [[unlikely]] {
      RefillCharacterBuffer();
    }

    auto c = m_getc_buffer[m_getc_buffer_pos++];

    m_offset += 1;
    m_line = m_line + static_cast<uint64_t>(c == '\n');
    m_column = static_cast<uint64_t>(c != '\n') * (m_column + 1);

    return c;
  }

  static auto CanonicalizeFloat(std::string &buf) -> bool {
    size_t e_pos = buf.find('e');
    if (e_pos == std::string::npos) [[likely]] {
      return true;
    }

    long double mantissa = 0;
    if (std::from_chars(buf.data(), buf.data() + e_pos, mantissa).ec !=
        std::errc()) [[unlikely]] {
      return false;
    }

    long double exponent = 0;
    if (std::from_chars(buf.data() + e_pos + 1, buf.data() + buf.size(),
                        exponent)
            .ec != std::errc()) [[unlikely]] {
      return false;
    }

    long double x = mantissa * std::pow(10.0, exponent);

    std::array<char, kFloatingPointDigits> buffer;
    static_assert(kFloatingPointDigits >= 10,
                  "Floating point precision must be at least 10");

    if (std::snprintf(buffer.data(), buffer.size(), "%.*Lf",
                      (int)(kFloatingPointDigits - 1), x) < 0) [[unlikely]] {
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

  static auto CanonicalizeNumber(std::string &buf,
                                 NumberLiteralType type) -> bool {
    boost::uint128_type x = 0;
    boost::uint128_type i = 0;

    std::transform(buf.begin(), buf.end(), buf.begin(), ::tolower);
    std::erase(buf, '_');

    switch (type) {
      case NumberLiteralType::Hexadecimal: {
        for (i = 2; i < buf.size(); ++i) {
          // Check for overflow
          if (((x >> 64) & 0xF000000000000000) != 0U) [[unlikely]] {
            return false;
          }

          if (buf[i] >= '0' && buf[i] <= '9') {
            x = (x << 4) + (buf[i] - '0');
          } else if (buf[i] >= 'a' && buf[i] <= 'f') {
            x = (x << 4) + (buf[i] - 'a' + 10);
          } else [[unlikely]] {
            return false;
          }
        }
        break;
      }
      case NumberLiteralType::Binary: {
        for (i = 2; i < buf.size(); ++i) {
          // Check for overflow
          if (((x >> 64) & 0x8000000000000000) != 0U) [[unlikely]] {
            return false;
          }
          if (buf[i] != '0' && buf[i] != '1') [[unlikely]] {
            return false;
          }

          x = (x << 1) + (buf[i] - '0');
        }
        break;
      }
      case NumberLiteralType::Octal: {
        for (i = 2; i < buf.size(); ++i) {
          // Check for overflow
          if (((x >> 64) & 0xE000000000000000) != 0U) [[unlikely]] {
            return false;
          }
          if (buf[i] < '0' || buf[i] > '7') [[unlikely]] {
            return false;
          }

          x = (x << 3) + (buf[i] - '0');
        }
        break;
      }
      case NumberLiteralType::DecimalExplicit: {
        for (i = 2; i < buf.size(); ++i) {
          if (buf[i] < '0' || buf[i] > '9') [[unlikely]] {
            return false;
          }

          // check for overflow
          auto tmp = x;
          x = (x * 10) + (buf[i] - '0');
          if (x < tmp) [[unlikely]] {
            return false;
          }
        }
        break;
      }
      case NumberLiteralType::Decimal: {
        for (i = 0; i < buf.size(); ++i) {
          if (buf[i] < '0' || buf[i] > '9') [[unlikely]] {
            return false;
          }

          // check for overflow
          auto tmp = x;
          x = (x * 10) + (buf[i] - '0');
          if (x < tmp) [[unlikely]] {
            return false;
          }
        }
        break;
      }
      case NumberLiteralType::Floating: {
        __builtin_unreachable();
      }
    }

    std::stringstream ss;
    if (x == 0) {
      ss << '0';
    }

    for (i = x; i > 0; i /= 10) {
      ss << (char)('0' + i % 10);
    }

    buf.assign(ss.str());
    std::reverse(buf.begin(), buf.end());

    return true;
  }

  NCC_FORCE_INLINE auto ParseIdentifier(char c, LocationID start_pos) -> Token {
    { /* Read in what is hopefully an identifier */
      int colon_state = 0;

      while ((std::isalnum(c) != 0) || c == '_' || c == ':') {
        if (c != ':' && colon_state == 1) {
          if (!buf.ends_with("::")) {
            char tc = buf.back();
            buf.pop_back();
            m_fifo.push(tc);
            break;
          }
          colon_state = 0;
        } else if (c == ':') {
          colon_state = 1;
        }

        buf += c;
        c = GetChar();
      }
    }

    /* Check for f-string */
    if (buf == "f" && c == '"') {
      m_fifo.push(c);
      return {KeyW, __FString, start_pos};
    }

    /* We overshot; this must be a punctor ':' */
    if (!buf.empty() && buf.back() == ':') {
      char tc = buf.back();
      buf.pop_back();
      m_fifo.push(tc);
    }
    m_fifo.push(c);

    { /* Determine if it's a keyword or an identifier */
      auto it = KEYWORDS_MAP.find(buf);
      if (it != KEYWORDS_MAP.end()) {
        return {KeyW, it->second, start_pos};
      }
    }

    { /* Check if it's an operator */
      auto it = WORD_OPERATORS.find(buf);
      if (it != WORD_OPERATORS.end()) {
        return {Oper, it->second, start_pos};
      }
    }

    /* Ensure it's a valid identifier */
    if (!ValidateIdentifier(buf)) {
      ResetAutomaton();
      return Token::EndOfFile();
    }

    /* For compiler internal debugging */
    qcore_assert(buf != "__builtin_lexer_crash",
                 "The source code invoked a compiler panic API.");

    if (buf == "__builtin_lexer_abort") {
      return Token::EndOfFile();
    }

    /* Return the identifier */
    return {Name, string(buf), start_pos};
  };

  auto ParseString(char c, LocationID start_pos) -> Token {
    while (true) {
      while (c != buf[0]) {
        /* Normal character */
        if (c != '\\') {
          buf += c;
        } else {
          /* String escape sequences */
          c = GetChar();
          switch (c) {
            case 'n':
              buf += '\n';
              break;
            case 't':
              buf += '\t';
              break;
            case 'r':
              buf += '\r';
              break;
            case '0':
              buf += '\0';
              break;
            case '\\':
              buf += '\\';
              break;
            case '\'':
              buf += '\'';
              break;
            case '\"':
              buf += '\"';
              break;
            case 'x': {
              std::array<char, 2> hex = {GetChar(), GetChar()};
              if ((std::isxdigit(hex[0]) == 0) ||
                  (std::isxdigit(hex[1]) == 0)) {
                ResetAutomaton();
                return Token::EndOfFile();
              }
              buf += (kHextable[(uint8_t)hex[0]] << 4) |
                     kHextable[(uint8_t)hex[1]];
              break;
            }
            case 'u': {
              c = GetChar();
              if (c != '{') {
                ResetAutomaton();
                return Token::EndOfFile();
              }

              std::string hex;

              while (true) {
                c = GetChar();
                if (c == '}') {
                  break;
                }

                if (std::isxdigit(c) == 0) {
                  ResetAutomaton();
                  return Token::EndOfFile();
                }

                hex += c;
              }

              uint32_t codepoint;
              if (std::from_chars(hex.data(), hex.data() + hex.size(),
                                  codepoint, 16)
                      .ec != std::errc()) {
                ResetAutomaton();
                return Token::EndOfFile();
              }

              if (codepoint < 0x80) {
                buf += (char)codepoint;
              } else if (codepoint < 0x800) {
                buf += (char)(0xC0 | (codepoint >> 6));
                buf += (char)(0x80 | (codepoint & 0x3F));
              } else if (codepoint < 0x10000) {
                buf += (char)(0xE0 | (codepoint >> 12));
                buf += (char)(0x80 | ((codepoint >> 6) & 0x3F));
                buf += (char)(0x80 | (codepoint & 0x3F));
              } else if (codepoint < 0x110000) {
                buf += (char)(0xF0 | (codepoint >> 18));
                buf += (char)(0x80 | ((codepoint >> 12) & 0x3F));
                buf += (char)(0x80 | ((codepoint >> 6) & 0x3F));
                buf += (char)(0x80 | (codepoint & 0x3F));
              } else {
                ResetAutomaton();
                return Token::EndOfFile();
              }

              break;
            }
            case 'o': {
              std::array<char, 3> oct = {GetChar(), GetChar(), GetChar()};
              int val;
              if (std::from_chars(oct.data(), oct.data() + oct.size(), val, 8)
                      .ec != std::errc()) {
                ResetAutomaton();
                return Token::EndOfFile();
              }

              buf += val;
              break;
            }
            case 'b': {
              c = GetChar();
              if (c != '{') {
                ResetAutomaton();
                return Token::EndOfFile();
              }

              std::string bin;

              while (true) {
                c = GetChar();
                if (c == '}') {
                  break;
                }

                if ((c != '0' && c != '1') || bin.size() >= 64) {
                  ResetAutomaton();
                  return Token::EndOfFile();
                }

                bin += c;
              }

              uint64_t codepoint = 0;
              for (char i : bin) {
                codepoint = (codepoint << 1) | (i - '0');
              }

              if (codepoint > 0xFFFFFFFF) {
                buf += (char)(codepoint >> 56);
                buf += (char)(codepoint >> 48);
                buf += (char)(codepoint >> 40);
                buf += (char)(codepoint >> 32);
                buf += (char)(codepoint >> 24);
                buf += (char)(codepoint >> 16);
                buf += (char)(codepoint >> 8);
                buf += (char)(codepoint);
              } else if (codepoint > 0xFFFF) {
                buf += (char)(codepoint >> 24);
                buf += (char)(codepoint >> 16);
                buf += (char)(codepoint >> 8);
                buf += (char)(codepoint);
              } else if (codepoint > 0xFF) {
                buf += (char)(codepoint >> 8);
                buf += (char)(codepoint);
              } else {
                buf += (char)(codepoint);
              }

              break;
            }
            default:
              buf += c;
              break;
          }
        }

        c = GetChar();
      }

      /* Skip over characters permitted in between multi-part strings */
      do {
        c = GetChar();
      } while (kWhitespaceTable[c] || c == '\\');

      /* Check for a multi-part string */
      if (c == buf[0]) {
        c = GetChar();
        continue;
      }

      m_fifo.push(c);
      /* Character or string */
      if (buf.front() == '\'' && buf.size() == 2) {
        return {Char, string(std::string(1, buf[1])), start_pos};
      }
      return {Text, string(buf.substr(1, buf.size() - 1)), start_pos};
    }
  }

  void ParseIntegerUnwind(char c, std::string &buf) {
    bool spinning = true;

    std::vector<char> q;

    while (!buf.empty() && spinning) {
      switch (char ch = buf.back()) {
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

    q.push_back(c);

    for (char &it : std::ranges::reverse_view(q)) {
      m_fifo.push(it);
    }
  }

  auto ParseInteger(char c, LocationID start_pos) -> Token {
    NumberLiteralType type = NumberLiteralType::Decimal;

    /* [0x, 0X, 0d, 0D, 0b, 0B, 0o, 0O] */
    if (buf.size() == 1 && buf[0] == '0') [[unlikely]] {
      switch (c) {
        case 'x':
        case 'X': {
          type = NumberLiteralType::Hexadecimal;
          buf += c;
          c = GetChar();
          break;
        }

        case 'b':
        case 'B': {
          type = NumberLiteralType::Binary;
          buf += c;
          c = GetChar();
          break;
        }

        case 'o':
        case 'O': {
          type = NumberLiteralType::Octal;
          buf += c;
          c = GetChar();
          break;
        }

        case 'd':
        case 'D': {
          type = NumberLiteralType::DecimalExplicit;
          buf += c;
          c = GetChar();
          break;
        }
      }
    }

    enum class FloatPart : uint8_t {
      MantissaPost,
      ExponentSign,
      ExponentPre,
      ExponentPost,
    } float_lit_state = FloatPart::MantissaPost;

    bool is_lexing = true;
    while (is_lexing) {
      /* Skip over the integer syntax formatting */
      if (c == '_') [[unlikely]] {
        while (kWhitespaceTable[c] || c == '_' || c == '\\') [[unlikely]] {
          c = GetChar();
        }
      } else if (kWhitespaceTable[c]) [[unlikely]] {
        break;
      }

      switch (type) {
        case NumberLiteralType::Decimal: {
          if (std::isdigit(c) != 0) [[likely]] {
            buf += c;
          } else if (c == '.') {
            buf += c;
            type = NumberLiteralType::Floating;
            float_lit_state = FloatPart::MantissaPost;
          } else if (c == 'e' || c == 'E') {
            buf += 'e';
            type = NumberLiteralType::Floating;
            float_lit_state = FloatPart::ExponentSign;
          } else {
            ParseIntegerUnwind(c, buf);
            is_lexing = false;
          }
          break;
        }

        case NumberLiteralType::DecimalExplicit: {
          if (std::isdigit(c) != 0) [[likely]] {
            buf += c;
          } else {
            ParseIntegerUnwind(c, buf);
            is_lexing = false;
          }
          break;
        }

        case NumberLiteralType::Hexadecimal: {
          if (std::isxdigit(c) != 0) [[likely]] {
            buf += c;
          } else {
            ParseIntegerUnwind(c, buf);
            is_lexing = false;
          }
          break;
        }

        case NumberLiteralType::Binary: {
          if (c == '0' || c == '1') [[likely]] {
            buf += c;
          } else {
            ParseIntegerUnwind(c, buf);
            is_lexing = false;
          }
          break;
        }

        case NumberLiteralType::Octal: {
          if (c >= '0' && c <= '7') [[likely]] {
            buf += c;
          } else {
            ParseIntegerUnwind(c, buf);
            is_lexing = false;
          }
          break;
        }

        case NumberLiteralType::Floating: {
          switch (float_lit_state) {
            case FloatPart::MantissaPost: {
              if (std::isdigit(c) != 0) {
                buf += c;
              } else if (c == 'e' || c == 'E') {
                buf += 'e';
                float_lit_state = FloatPart::ExponentSign;
              } else {
                ParseIntegerUnwind(c, buf);
                is_lexing = false;
              }
              break;
            }

            case FloatPart::ExponentSign: {
              if (c == '-') {
                buf += c;
              } else if (c == '+') {
                float_lit_state = FloatPart::ExponentPre;
              } else if (std::isdigit(c) != 0) {
                buf += c;
                float_lit_state = FloatPart::ExponentPre;
              } else {
                ParseIntegerUnwind(c, buf);
                is_lexing = false;
              }
              break;
            }

            case FloatPart::ExponentPre: {
              if (std::isdigit(c) != 0) {
                buf += c;
              } else if (c == '.') {
                buf += c;
                float_lit_state = FloatPart::ExponentPost;
              } else {
                ParseIntegerUnwind(c, buf);
                is_lexing = false;
              }
              break;
            }

            case FloatPart::ExponentPost: {
              if (std::isdigit(c) != 0) {
                buf += c;
              } else {
                ParseIntegerUnwind(c, buf);
                is_lexing = false;
              }
              break;
            }
          }

          break;
        }
      }

      if (is_lexing) [[likely]] {
        c = GetChar();
      }
    }

    if (type == NumberLiteralType::Floating) {
      if (CanonicalizeFloat(buf)) {
        return {NumL, string(std::move(buf)), start_pos};
      }
    } else if (CanonicalizeNumber(buf, type)) [[likely]] {
      return {IntL, string(std::move(buf)), start_pos};
    }

    ResetAutomaton();
    return Token::EndOfFile();
  }

  auto ParseCommentSingleLine(char c, LocationID start_pos) -> Token {
    while (c != '\n') [[likely]] {
      buf += c;
      c = GetChar();
    }

    return {Note, string(std::move(buf)), start_pos};
  };

  auto ParseCommentMultiLine(char c, LocationID start_pos) -> Token {
    size_t level = 1;

    while (true) {
      if (c == '/') {
        char tmp = GetChar();
        if (tmp == '*') {
          level++;
          buf += "/*";
        } else {
          buf += c;
          buf += tmp;
        }

        c = GetChar();
      } else if (c == '*') {
        char tmp = GetChar();
        if (tmp == '/') {
          level--;
          if (level == 0) {
            return {Note, string(std::move(buf)), start_pos};
          }
          buf += "*";
          buf += tmp;

        } else {
          buf += c;
          buf += tmp;
        }
        c = GetChar();
      } else {
        buf += c;
        c = GetChar();
      }
    }
  }

  auto ParseSingleLineMacro(char c, LocationID start_pos) -> Token {
    while ((std::isalnum(c) != 0) || c == '_' || c == ':') [[likely]] {
      buf += c;
      c = GetChar();
    }

    m_fifo.push(c);

    return {Macr, string(std::move(buf)), start_pos};
  }

  auto ParseBlockMacro(char c, LocationID start_pos) -> Token {
    uint32_t state_parens = 1;

    while (true) {
      if (c == '(') {
        state_parens++;
      } else if (c == ')') {
        state_parens--;
      }

      if (state_parens == 0) {
        return {MacB, string(std::move(buf)), start_pos};
      }

      buf += c;

      c = GetChar();
    }
  }

  auto ParseOther(char c, LocationID start_pos, LexState &state,
                  Token &token) -> bool {
    /* Check if it's a punctor */
    if (buf.size() == 1) {
      auto it = PUNCTORS_MAP.find(buf);
      if (it != PUNCTORS_MAP.end()) {
        m_fifo.push(c);
        token = Token(Punc, it->second, start_pos);
        return true;
      }
    }

    /* Special case for a comment */
    if ((buf[0] == '~' && c == '>')) {
      buf.clear();
      state = LexState::CommentSingleLine;
      return false;
    }

    /* Special case for a comment */
    if (buf[0] == '#') {
      buf.clear();
      m_fifo.push(c);
      state = LexState::CommentSingleLine;
      return false;
    }

    bool found = false;
    while (true) {
      bool contains = false;
      if (OPERATOR_SET.count(buf) != 0U) {
        contains = true;
        found = true;
      }

      if (contains) {
        buf += c;
        if (buf.size() > 4) { /* Handle infinite error case */
          ResetAutomaton();
          return false;
        }
        c = GetChar();
      } else {
        break;
      }
    }

    if (!found) {
      ResetAutomaton();
      return false;
    }

    m_fifo.push(buf.back());
    m_fifo.push(c);
    token =
        Token(Oper, OPERATORS_MAP.find(buf.substr(0, buf.size() - 1))->second,
              start_pos);

    return true;
  }
};

auto Tokenizer::GetNext() -> Token {
  /**
   * **WARNING**: Do not just start editing this function without
   * having a holistic understanding of all code that depends on the lexer
   * (most of the pipeline).
   *
   * This function provides various undocumented invariant guarantees that
   * if broken will likely result in internal runtime corruption and other
   * undefined behavior.
   * */

  LocationID start_pos = 0;
  LexState state = LexState::Start;
  char c{};

  Impl &impl = *m_impl;

  impl.buf.clear();

  while (true) {
    { /* If the Lexer over-consumed, we will return the saved character */
      if (impl.m_fifo.empty()) {
        c = impl.GetChar();
      } else {
        c = impl.m_fifo.front();
        impl.m_fifo.pop();
      }
    }

    switch (state) {
      case LexState::Start: {
        if (kWhitespaceTable[c]) {
          continue;
        }

        start_pos =
            InternLocation(Location(impl.m_offset - 1, impl.m_line,
                                    impl.m_column - 1, impl.m_filename));

        if ((std::isalpha(c) != 0) || c == '_') {
          /* Identifier or keyword or operator */

          impl.buf += c, state = LexState::Identifier;
          continue;
        }
        if (c == '/') {
          state = LexState::CommentStart; /* Comment or operator */
          continue;
        }
        if (std::isdigit(c) != 0) {
          impl.buf += c, state = LexState::Integer;
          continue;
        }
        if (c == '"' || c == '\'') {
          impl.buf += c, state = LexState::String;
          continue;
        }
        if (c == '@') {
          state = LexState::MacroStart;
          continue;
        } /* Operator or punctor or invalid */
        impl.buf += c;
        state = LexState::Other;
        continue;
      }
      case LexState::Identifier: {
        return impl.ParseIdentifier(c, start_pos);
      }
      case LexState::String: {
        return impl.ParseString(c, start_pos);
      }
      case LexState::Integer: {
        return impl.ParseInteger(c, start_pos);
      }
      case LexState::CommentStart: {
        if (c == '/') { /* Single line comment */
          state = LexState::CommentSingleLine;
          continue;
        }
        if (c == '*') { /* Multi-line comment */
          state = LexState::CommentMultiLine;
          continue;
        } /* Divide operator */
        impl.m_fifo.push(c);
        return {Oper, OpSlash, start_pos};
      }
      case LexState::CommentSingleLine: {
        return impl.ParseCommentSingleLine(c, start_pos);
      }
      case LexState::CommentMultiLine: {
        return impl.ParseCommentMultiLine(c, start_pos);
      }
      case LexState::MacroStart: {
        if (c == '(') {
          state = LexState::BlockMacro;
          continue;
        }
        state = LexState::SingleLineMacro;
        impl.buf += c;
        continue;

        break;
      }
      case LexState::SingleLineMacro: {
        return impl.ParseSingleLineMacro(c, start_pos);
      }
      case LexState::BlockMacro: {
        return impl.ParseBlockMacro(c, start_pos);
      }
      case LexState::Other: {
        Token token;
        if (impl.ParseOther(c, start_pos, state, token)) {
          return token;
        }
        break;
      }
    }
  }
}

Tokenizer::Tokenizer(std::istream &source_file,
                     std::shared_ptr<Environment> env)
    : IScanner(std::move(env)), m_impl(new Impl(source_file)) {
  m_impl->m_filename = m_env->Get("FILE").value_or("");
}

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
