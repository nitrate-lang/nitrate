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
#include <string>
#include <string_view>
#include <utility>
#include <vector>

using namespace ncc;
using namespace ncc::lex;
using namespace ncc::lex::detail;

constexpr size_t FLOATING_POINT_PRECISION = 100;

namespace ncc::lex {
  static const auto operator_set = []() {
    google::dense_hash_set<std::string_view> set;
    set.set_empty_key("");
    for (const auto &op : LexicalOperators.left) {
      set.insert(op.first);
    }
    return set;
  }();

  static const auto word_operators = []() {
    google::dense_hash_map<std::string_view, Operator> map;
    map.set_empty_key("");

    for (const auto &op : LexicalOperators.left) {
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

  static const auto KeywordsMap = []() {
    google::dense_hash_map<std::string_view, Keyword> map;
    map.set_empty_key("");

    for (const auto &kw : LexicalKeywords.left) {
      map[kw.first] = kw.second;
    }

    return map;
  }();

  static const auto OperatorsMap = []() {
    google::dense_hash_map<std::string_view, Operator> map;
    map.set_empty_key("");

    for (const auto &kw : LexicalOperators.left) {
      map[kw.first] = kw.second;
    }

    return map;
  }();

  static const auto PunctorsMap = []() {
    google::dense_hash_map<std::string_view, Punctor> map;
    map.set_empty_key("");

    for (const auto &kw : LexicalPunctors.left) {
      map[kw.first] = kw.second;
    }

    return map;
  }();

  static constexpr auto hextable = []() {
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

  static constexpr std::array<uint8_t, 256> whitespace_table = []() {
    std::array<uint8_t, 256> tab = {};
    tab[' '] = 1;
    tab['\f'] = 1;
    tab['\n'] = 1;
    tab['\r'] = 1;
    tab['\t'] = 1;
    tab['\v'] = 1;

    tab['\0'] = 1;  // Null byte is also a whitespace character

    return tab;
  }();

  static bool lex_is_space(uint8_t c) {
    return whitespace_table[static_cast<uint8_t>(c)];
  }
}  // namespace ncc::lex

enum class NumType {
  Decimal,
  DecimalExplicit,
  Hexadecimal,
  Binary,
  Octal,
  Floating,
};

static NCC_FORCE_INLINE bool validate_identifier(std::string_view id) {
  /*
   * This state machine checks if the identifier looks
   * like 'a::b::c::d_::e::f'.
   */

  int state = 0;

  for (char c : id) {
    switch (state) {
      case 0:
        if (std::isalnum(c) || c == '_') continue;
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

static NCC_FORCE_INLINE bool canonicalize_float(std::string &buf) {
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
  if (std::from_chars(buf.data() + e_pos + 1, buf.data() + buf.size(), exponent)
          .ec != std::errc()) [[unlikely]] {
    return false;
  }

  long double x = mantissa * std::pow(10.0, exponent);

  std::array<char, FLOATING_POINT_PRECISION> buffer;
  static_assert(FLOATING_POINT_PRECISION >= 10,
                "Floating point precision must be at least 10");

  if (std::snprintf(buffer.data(), buffer.size(), "%.*Lf",
                    (int)(FLOATING_POINT_PRECISION - 1), x) < 0) [[unlikely]] {
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

static NCC_FORCE_INLINE bool canonicalize_number(std::string &buf,
                                                 NumType type) {
  boost::uint128_type x = 0, i = 0;

  std::transform(buf.begin(), buf.end(), buf.begin(), ::tolower);
  std::erase(buf, '_');

  switch (type) {
    case NumType::Hexadecimal: {
      for (i = 2; i < buf.size(); ++i) {
        // Check for overflow
        if ((x >> 64) & 0xF000000000000000) [[unlikely]] {
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
    case NumType::Binary: {
      for (i = 2; i < buf.size(); ++i) {
        // Check for overflow
        if ((x >> 64) & 0x8000000000000000) [[unlikely]] {
          return false;
        }
        if (buf[i] != '0' && buf[i] != '1') [[unlikely]] {
          return false;
        }

        x = (x << 1) + (buf[i] - '0');
      }
      break;
    }
    case NumType::Octal: {
      for (i = 2; i < buf.size(); ++i) {
        // Check for overflow
        if ((x >> 64) & 0xE000000000000000) [[unlikely]] {
          return false;
        }
        if (buf[i] < '0' || buf[i] > '7') [[unlikely]] {
          return false;
        }

        x = (x << 3) + (buf[i] - '0');
      }
      break;
    }
    case NumType::DecimalExplicit: {
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
    case NumType::Decimal: {
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
    case NumType::Floating: {
      __builtin_unreachable();
    }
  }

  std::stringstream ss;
  if (x == 0) {
    ss << '0';
  }

  for (i = x; i; i /= 10) {
    ss << (char)('0' + i % 10);
  }

  buf.assign(ss.str());
  std::reverse(buf.begin(), buf.end());

  return true;
}

void Tokenizer::reset_state() { m_fifo = std::queue<char>(); }

enum class LexState {
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

// We use this layer of indirection to ensure that the compiler can have full
// optimization capabilities as if the functions has static linkage.
class Tokenizer::StaticImpl {
public:
  static NCC_FORCE_INLINE Token ParseIdentifier(Tokenizer &L, char c,
                                                std::string &buf,
                                                LocationID start_pos) {
    { /* Read in what is hopefully an identifier */
      int colon_state = 0;

      while (std::isalnum(c) || c == '_' || c == ':') {
        if (c != ':' && colon_state == 1) {
          if (!buf.ends_with("::")) {
            char tc = buf.back();
            buf.pop_back();
            L.m_fifo.push(tc);
            break;
          }
          colon_state = 0;
        } else if (c == ':') {
          colon_state = 1;
        }

        buf += c;
        c = L.nextc();
      }
    }

    /* Check for f-string */
    if (buf == "f" && c == '"') {
      L.m_fifo.push(c);
      return Token(KeyW, __FString, start_pos);
    }

    /* We overshot; this must be a punctor ':' */
    if (buf.size() > 0 && buf.back() == ':') {
      char tc = buf.back();
      buf.pop_back();
      L.m_fifo.push(tc);
    }
    L.m_fifo.push(c);

    { /* Determine if it's a keyword or an identifier */
      auto it = KeywordsMap.find(buf);
      if (it != KeywordsMap.end()) {
        return Token(KeyW, it->second, start_pos);
      }
    }

    { /* Check if it's an operator */
      auto it = word_operators.find(buf);
      if (it != word_operators.end()) {
        return Token(Oper, it->second, start_pos);
      }
    }

    /* Ensure it's a valid identifier */
    if (!validate_identifier(buf)) {
      L.reset_state();
      return Token::EndOfFile();
    }

    /* For compiler internal debugging */
    qcore_assert(buf != "__builtin_lexer_crash",
                 "The source code invoked a compiler panic API.");

    if (buf == "__builtin_lexer_abort") {
      return Token::EndOfFile();
    }

    /* Return the identifier */
    return Token(Name, string(buf), start_pos);
  };

  static NCC_FORCE_INLINE Token ParseString(Tokenizer &L, char c,
                                            std::string &buf,
                                            LocationID start_pos) {
    while (true) {
      while (c != buf[0]) {
        /* Normal character */
        if (c != '\\') {
          buf += c;
        } else {
          /* String escape sequences */
          c = L.nextc();
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
              char hex[2] = {L.nextc(), L.nextc()};
              if (!std::isxdigit(hex[0]) || !std::isxdigit(hex[1])) {
                L.reset_state();
                return Token::EndOfFile();
              }
              buf +=
                  (hextable[(uint8_t)hex[0]] << 4) | hextable[(uint8_t)hex[1]];
              break;
            }
            case 'u': {
              c = L.nextc();
              if (c != '{') {
                L.reset_state();
                return Token::EndOfFile();
              }

              std::string hex;

              while (true) {
                c = L.nextc();
                if (c == '}') {
                  break;
                }

                if (!std::isxdigit(c)) {
                  L.reset_state();
                  return Token::EndOfFile();
                }

                hex += c;
              }

              uint32_t codepoint;
              if (std::from_chars(hex.data(), hex.data() + hex.size(),
                                  codepoint, 16)
                      .ec != std::errc()) {
                L.reset_state();
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
                L.reset_state();
                return Token::EndOfFile();
              }

              break;
            }
            case 'o': {
              char oct[3] = {L.nextc(), L.nextc(), L.nextc()};
              int val;
              if (std::from_chars(oct, oct + 3, val, 8).ec != std::errc()) {
                L.reset_state();
                return Token::EndOfFile();
              }

              buf += val;
              break;
            }
            case 'b': {
              c = L.nextc();
              if (c != '{') {
                L.reset_state();
                return Token::EndOfFile();
              }

              std::string bin;

              while (true) {
                c = L.nextc();
                if (c == '}') {
                  break;
                }

                if ((c != '0' && c != '1') || bin.size() >= 64) {
                  L.reset_state();
                  return Token::EndOfFile();
                }

                bin += c;
              }

              uint64_t codepoint = 0;
              for (size_t i = 0; i < bin.size(); i++) {
                codepoint = (codepoint << 1) | (bin[i] - '0');
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

        c = L.nextc();
      }

      /* Skip over characters permitted in between multi-part strings */
      do {
        c = L.nextc();
      } while (lex_is_space(c) || c == '\\');

      /* Check for a multi-part string */
      if (c == buf[0]) {
        c = L.nextc();
        continue;
      }

      L.m_fifo.push(c);
      /* Character or string */
      if (buf.front() == '\'' && buf.size() == 2) {
        return Token(Char, string(std::string(1, buf[1])), start_pos);
      } else {
        return Token(Text, string(buf.substr(1, buf.size() - 1)), start_pos);
      }
    }
  }

  static NCC_FORCE_INLINE void ParseIntegerUnwind(Tokenizer &L, char c,
                                                  std::string &buf) {
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

    for (auto it = q.rbegin(); it != q.rend(); ++it) {
      L.m_fifo.push(*it);
    }
  }

  static NCC_FORCE_INLINE Token ParseInteger(Tokenizer &L, char c,
                                             std::string &buf,
                                             LocationID start_pos) {
    NumType type = NumType::Decimal;

    /* [0x, 0X, 0d, 0D, 0b, 0B, 0o, 0O] */
    if (buf.size() == 1 && buf[0] == '0') [[unlikely]] {
      switch (c) {
        case 'x':
        case 'X': {
          type = NumType::Hexadecimal;
          buf += c;
          c = L.nextc();
          break;
        }

        case 'b':
        case 'B': {
          type = NumType::Binary;
          buf += c;
          c = L.nextc();
          break;
        }

        case 'o':
        case 'O': {
          type = NumType::Octal;
          buf += c;
          c = L.nextc();
          break;
        }

        case 'd':
        case 'D': {
          type = NumType::DecimalExplicit;
          buf += c;
          c = L.nextc();
          break;
        }
      }
    }

    enum class FloatPart {
      MantissaPost,
      ExponentSign,
      ExponentPre,
      ExponentPost,
    } float_lit_state = FloatPart::MantissaPost;

    bool is_lexing = true;
    while (is_lexing) {
      /* Skip over the integer syntax formatting */
      if (c == '_') [[unlikely]] {
        while (lex_is_space(c) || c == '_' || c == '\\') [[unlikely]] {
          c = L.nextc();
        }
      } else if (lex_is_space(c)) [[unlikely]] {
        break;
      }

      switch (type) {
        case NumType::Decimal: {
          if (std::isdigit(c)) [[likely]] {
            buf += c;
          } else if (c == '.') {
            buf += c;
            type = NumType::Floating;
            float_lit_state = FloatPart::MantissaPost;
          } else if (c == 'e' || c == 'E') {
            buf += 'e';
            type = NumType::Floating;
            float_lit_state = FloatPart::ExponentSign;
          } else {
            ParseIntegerUnwind(L, c, buf);
            is_lexing = false;
          }
          break;
        }

        case NumType::DecimalExplicit: {
          if (std::isdigit(c)) [[likely]] {
            buf += c;
          } else {
            ParseIntegerUnwind(L, c, buf);
            is_lexing = false;
          }
          break;
        }

        case NumType::Hexadecimal: {
          if (std::isxdigit(c)) [[likely]] {
            buf += c;
          } else {
            ParseIntegerUnwind(L, c, buf);
            is_lexing = false;
          }
          break;
        }

        case NumType::Binary: {
          if (c == '0' || c == '1') [[likely]] {
            buf += c;
          } else {
            ParseIntegerUnwind(L, c, buf);
            is_lexing = false;
          }
          break;
        }

        case NumType::Octal: {
          if (c >= '0' && c <= '7') [[likely]] {
            buf += c;
          } else {
            ParseIntegerUnwind(L, c, buf);
            is_lexing = false;
          }
          break;
        }

        case NumType::Floating: {
          switch (float_lit_state) {
            case FloatPart::MantissaPost: {
              if (std::isdigit(c)) {
                buf += c;
              } else if (c == 'e' || c == 'E') {
                buf += 'e';
                float_lit_state = FloatPart::ExponentSign;
              } else {
                ParseIntegerUnwind(L, c, buf);
                is_lexing = false;
              }
              break;
            }

            case FloatPart::ExponentSign: {
              if (c == '-') {
                buf += c;
              } else if (c == '+') {
                float_lit_state = FloatPart::ExponentPre;
              } else if (std::isdigit(c)) {
                buf += c;
                float_lit_state = FloatPart::ExponentPre;
              } else {
                ParseIntegerUnwind(L, c, buf);
                is_lexing = false;
              }
              break;
            }

            case FloatPart::ExponentPre: {
              if (std::isdigit(c)) {
                buf += c;
              } else if (c == '.') {
                buf += c;
                float_lit_state = FloatPart::ExponentPost;
              } else {
                ParseIntegerUnwind(L, c, buf);
                is_lexing = false;
              }
              break;
            }

            case FloatPart::ExponentPost: {
              if (std::isdigit(c)) {
                buf += c;
              } else {
                ParseIntegerUnwind(L, c, buf);
                is_lexing = false;
              }
              break;
            }
          }

          break;
        }
      }

      if (is_lexing) [[likely]] {
        c = L.nextc();
      }
    }

    if (type == NumType::Floating) {
      if (canonicalize_float(buf)) {
        return Token(NumL, string(std::move(buf)), start_pos);
      }
    } else if (canonicalize_number(buf, type)) [[likely]] {
      return Token(IntL, string(std::move(buf)), start_pos);
    }

    L.reset_state();
    return Token::EndOfFile();
  }

  static NCC_FORCE_INLINE Token ParseCommentSingleLine(Tokenizer &L, char c,
                                                       std::string &buf,
                                                       LocationID start_pos) {
    while (c != '\n') [[likely]] {
      buf += c;
      c = L.nextc();
    }

    return Token(Note, string(std::move(buf)), start_pos);
  };

  static NCC_FORCE_INLINE Token ParseCommentMultiLine(Tokenizer &L, char c,
                                                      std::string &buf,
                                                      LocationID start_pos) {
    size_t level = 1;

    while (true) {
      if (c == '/') {
        char tmp = L.nextc();
        if (tmp == '*') {
          level++;
          buf += "/*";
        } else {
          buf += c;
          buf += tmp;
        }

        c = L.nextc();
      } else if (c == '*') {
        char tmp = L.nextc();
        if (tmp == '/') {
          level--;
          if (level == 0) {
            return Token(Note, string(std::move(buf)), start_pos);
          } else {
            buf += "*";
            buf += tmp;
          }
        } else {
          buf += c;
          buf += tmp;
        }
        c = L.nextc();
      } else {
        buf += c;
        c = L.nextc();
      }
    }
  }

  static NCC_FORCE_INLINE Token ParseSingleLineMacro(Tokenizer &L, char c,
                                                     std::string &buf,
                                                     LocationID start_pos) {
    while (std::isalnum(c) || c == '_' || c == ':') [[likely]] {
      buf += c;
      c = L.nextc();
    }

    L.m_fifo.push(c);

    return Token(Macr, string(std::move(buf)), start_pos);
  }

  static NCC_FORCE_INLINE Token ParseBlockMacro(Tokenizer &L, char c,
                                                std::string &buf,
                                                LocationID start_pos) {
    uint32_t state_parens = 1;

    while (true) {
      if (c == '(') {
        state_parens++;
      } else if (c == ')') {
        state_parens--;
      }

      if (state_parens == 0) {
        return Token(MacB, string(std::move(buf)), start_pos);
      }

      buf += c;

      c = L.nextc();
    }
  }

  static NCC_FORCE_INLINE bool ParseOther(Tokenizer &L, char c,
                                          std::string &buf,
                                          LocationID start_pos, LexState &state,
                                          Token &token) {
    /* Check if it's a punctor */
    if (buf.size() == 1) {
      auto it = PunctorsMap.find(buf);
      if (it != PunctorsMap.end()) {
        L.m_fifo.push(c);
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
      L.m_fifo.push(c);
      state = LexState::CommentSingleLine;
      return false;
    }

    bool found = false;
    while (true) {
      bool contains = false;
      if (operator_set.count(buf)) {
        contains = true;
        found = true;
      }

      if (contains) {
        buf += c;
        if (buf.size() > 4) { /* Handle infinite error case */
          L.reset_state();
          return false;
        }
        c = L.nextc();
      } else {
        break;
      }
    }

    if (!found) {
      L.reset_state();
      return false;
    }

    L.m_fifo.push(buf.back());
    L.m_fifo.push(c);
    token =
        Token(Oper, OperatorsMap.find(buf.substr(0, buf.size() - 1))->second,
              start_pos);

    return true;
  }
};

void Tokenizer::nextc_refill() {
  m_file.read(m_getc_buffer.data(), GETC_BUFFER_SIZE);
  auto gcount = m_file.gcount();

  // Fill extra buffer with '#' with is a comment character
  memset(m_getc_buffer.data() + gcount, '\n', GETC_BUFFER_SIZE - gcount);
  m_getc_buffer_pos = 0;

  if (gcount == 0) [[unlikely]] {
    if (m_eof) [[unlikely]] {
      m_env->set("this.file.eof.offset", std::to_string(m_offset));
      m_env->set("this.file.eof.line", std::to_string(m_line));
      m_env->set("this.file.eof.column", std::to_string(m_column));
      m_env->set("this.file.eof.filename", GetCurrentFilename());

      // Benchmarks show that this is the fastest way to signal EOF
      // for large files.
      throw ScannerEOF();
    }
    m_eof = true;
  }
}

NCC_FORCE_INLINE char Tokenizer::nextc() {
  if (m_getc_buffer_pos == GETC_BUFFER_SIZE) [[unlikely]] {
    nextc_refill();
  }

  auto c = m_getc_buffer[m_getc_buffer_pos++];

  m_offset += 1;
  m_line = m_line + (c == '\n');
  m_column = (c != '\n') * (m_column + 1);

  return c;
}

NCC_EXPORT Token Tokenizer::GetNext() {
  /**
   * **WARNING**: Do not just start editing this function without
   * having a holistic understanding of all code that depends on the lexer
   * (most of the pipeline).
   *
   * This function provides various undocumented invariant guarantees that
   * if broken will likely result in internal runtime corruption and other
   * undefined behavior.
   * */

  std::string buf;
  LocationID start_pos = 0;
  LexState state = LexState::Start;
  char c{};

  while (true) {
    { /* If the Lexer over-consumed, we will return the saved character */
      if (m_fifo.empty()) {
        c = nextc();
      } else {
        c = m_fifo.front();
        m_fifo.pop();
      }
    }

    switch (state) {
      case LexState::Start: {
        if (lex_is_space(c)) {
          continue;
        }

        start_pos = InternLocation(
            Location(m_offset - 1, m_line, m_column - 1, GetCurrentFilename()));

        if (std::isalpha(c) || c == '_') {
          /* Identifier or keyword or operator */

          buf += c, state = LexState::Identifier;
          continue;
        } else if (c == '/') {
          state = LexState::CommentStart; /* Comment or operator */
          continue;
        } else if (std::isdigit(c)) {
          buf += c, state = LexState::Integer;
          continue;
        } else if (c == '"' || c == '\'') {
          buf += c, state = LexState::String;
          continue;
        } else if (c == '@') {
          state = LexState::MacroStart;
          continue;
        } else {
          /* Operator or punctor or invalid */
          buf += c;
          state = LexState::Other;
          continue;
        }
      }
      case LexState::Identifier: {
        return StaticImpl::ParseIdentifier(*this, c, buf, start_pos);
      }
      case LexState::String: {
        return StaticImpl::ParseString(*this, c, buf, start_pos);
      }
      case LexState::Integer: {
        return StaticImpl::ParseInteger(*this, c, buf, start_pos);
      }
      case LexState::CommentStart: {
        if (c == '/') { /* Single line comment */
          state = LexState::CommentSingleLine;
          continue;
        } else if (c == '*') { /* Multi-line comment */
          state = LexState::CommentMultiLine;
          continue;
        } else { /* Divide operator */
          m_fifo.push(c);
          return Token(Oper, OpSlash, start_pos);
        }
      }
      case LexState::CommentSingleLine: {
        return StaticImpl::ParseCommentSingleLine(*this, c, buf, start_pos);
      }
      case LexState::CommentMultiLine: {
        return StaticImpl::ParseCommentMultiLine(*this, c, buf, start_pos);
      }
      case LexState::MacroStart: {
        if (c == '(') {
          state = LexState::BlockMacro;
          continue;
        } else {
          state = LexState::SingleLineMacro;
          buf += c;
          continue;
        }
        break;
      }
      case LexState::SingleLineMacro: {
        return StaticImpl::ParseSingleLineMacro(*this, c, buf, start_pos);
      }
      case LexState::BlockMacro: {
        return StaticImpl::ParseBlockMacro(*this, c, buf, start_pos);
      }
      case LexState::Other: {
        Token token;
        if (StaticImpl::ParseOther(*this, c, buf, start_pos, state, token)) {
          return token;
        }
        break;
      }
    }
  }
}

///============================================================================///

NCC_EXPORT const char *ncc::lex::qlex_ty_str(TokenType ty) {
  switch (ty) {
    case EofF:
      return "eof";
    case KeyW:
      return "key";
    case Oper:
      return "op";
    case Punc:
      return "sym";
    case Name:
      return "name";
    case IntL:
      return "int";
    case NumL:
      return "num";
    case Text:
      return "str";
    case Char:
      return "char";
    case MacB:
      return "macb";
    case Macr:
      return "macr";
    case Note:
      return "note";
  }

  qcore_panic("unreachable");
}

NCC_EXPORT std::ostream &ncc::lex::operator<<(std::ostream &os, TokenType ty) {
  switch (ty) {
    case EofF: {
      os << "EofF";
      break;
    }

    case KeyW: {
      os << "KeyW";
      break;
    }

    case Oper: {
      os << "Oper";
      break;
    }

    case Punc: {
      os << "Punc";
      break;
    }

    case Name: {
      os << "Name";
      break;
    }

    case IntL: {
      os << "IntL";
      break;
    }

    case NumL: {
      os << "NumL";
      break;
    }

    case Text: {
      os << "Text";
      break;
    }

    case Char: {
      os << "Char";
      break;
    }

    case MacB: {
      os << "MacB";
      break;
    }

    case Macr: {
      os << "Macr";
      break;
    }

    case Note: {
      os << "Note";
      break;
    }
  }

  return os;
}

static void escape_string(std::ostream &ss, std::string_view input) {
  ss << '"';

  for (char ch : input) {
    switch (ch) {
      case '"':
        ss << "\\\"";
        break;
      case '\\':
        ss << "\\\\";
        break;
      case '\b':
        ss << "\\b";
        break;
      case '\f':
        ss << "\\f";
        break;
      case '\n':
        ss << "\\n";
        break;
      case '\r':
        ss << "\\r";
        break;
      case '\t':
        ss << "\\t";
        break;
      case '\0':
        ss << "\\0";
        break;
      default:
        if (ch >= 32 && ch < 127) {
          ss << ch;
        } else {
          char hex[5];
          snprintf(hex, sizeof(hex), "\\x%02x", (int)(uint8_t)ch);
          ss << hex;
        }
        break;
    }
  }

  ss << '"';
}

NCC_EXPORT std::ostream &ncc::lex::operator<<(std::ostream &os, Token tok) {
  // Serialize the token so that the core logger system can use it

  os << "${T:{\"type\":" << (int)tok.get_type()
     << ",\"posid\":" << tok.get_start().GetId() << ",\"value\":";
  escape_string(os, tok.as_string());
  os << "}}";

  return os;
}

NCC_EXPORT
std::optional<std::vector<std::string>> Tokenizer::GetSourceWindow(
    Point start, Point end, char fillchar) {
  if (start.x > end.x || (start.x == end.x && start.y > end.y)) {
    qcore_print(QCORE_ERROR, "Invalid source window range");
    return std::nullopt;
  }

  m_file.clear();
  auto current_source_offset = m_file.tellg();
  if (!m_file) {
    qcore_print(QCORE_ERROR, "Failed to get the current file offset");
    return std::nullopt;
  }

  if (m_file.seekg(0, std::ios::beg)) {
    long line = 0, column = 0;

    bool spinning = true;
    while (spinning) {
      int ch = m_file.get();
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

          ch = m_file.get();
          if (ch == EOF) [[unlikely]] {
            spinning = false;
          }
        } while (spinning);

        if (!line_buf.empty()) {
          lines.push_back(line_buf);
        }

        size_t max_length = 0;
        for (const auto &l : lines) {
          max_length = std::max(max_length, l.size());
        }

        for (auto &l : lines) {
          if (l.size() < max_length) {
            l.append(max_length - l.size(), fillchar);
          }
        }

        m_file.seekg(current_source_offset);
        return lines;
      }
    }
  } else {
    qcore_print(QCORE_ERROR, "Failed to seek to the beginning of the file");
  }

  m_file.seekg(current_source_offset);

  return std::nullopt;
}
