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

#include <string.h>

#include <array>
#include <boost/bimap.hpp>
#include <boost/unordered_map.hpp>
#include <cctype>
#include <cmath>
#include <csetjmp>
#include <cstdint>
#include <cstdio>
#include <deque>
#include <iomanip>
#include <nitrate-core/Logger.hh>
#include <nitrate-core/Macro.hh>
#include <nitrate-core/String.hh>
#include <nitrate-lexer/Lexer.hh>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

using namespace ncc::core;
using namespace ncc::lex;

///============================================================================///
/// BEGIN: LEXICAL GRAMMAR CONSTRAINTS
#define FLOATING_POINT_PRECISION 100
/// END:   LEXICAL GRAMMAR CONSTRAINTS
///============================================================================///

namespace qlex {
  ///============================================================================///
  /// BEGIN: LEXER LOOKUP TABLES
  template <typename L, typename R>
  boost::bimap<L, R> make_bimap(
      std::initializer_list<typename boost::bimap<L, R>::value_type> list) {
    return boost::bimap<L, R>(list.begin(), list.end());
  }

  static const boost::bimap<std::string_view, qlex_key_t> keywords = make_bimap<
      std::string_view, qlex_key_t>({
      {"scope", qKScope},     {"import", qKImport}, {"pub", qKPub},
      {"sec", qKSec},         {"pro", qKPro},       {"type", qKType},
      {"let", qKLet},         {"var", qKVar},       {"const", qKConst},
      {"static", qKStatic},   {"struct", qKStruct}, {"region", qKRegion},
      {"group", qKGroup},     {"class", qKClass},   {"union", qKUnion},
      {"opaque", qKOpaque},   {"enum", qKEnum},     {"__fstring", qK__FString},
      {"fn", qKFn},           {"unsafe", qKUnsafe}, {"safe", qKSafe},
      {"promise", qKPromise}, {"if", qKIf},         {"else", qKElse},
      {"for", qKFor},         {"while", qKWhile},   {"do", qKDo},
      {"switch", qKSwitch},   {"break", qKBreak},   {"continue", qKContinue},
      {"ret", qKReturn},      {"retif", qKRetif},   {"foreach", qKForeach},
      {"try", qKTry},         {"catch", qKCatch},   {"throw", qKThrow},
      {"async", qKAsync},     {"await", qKAwait},   {"__asm__", qK__Asm__},
      {"undef", qKUndef},     {"null", qKNull},     {"true", qKTrue},
      {"false", qKFalse},
  });

  static const boost::bimap<std::string_view, qlex_op_t> operators =
      make_bimap<std::string_view, qlex_op_t>({
          {"+", qOpPlus},
          {"-", qOpMinus},
          {"*", qOpTimes},
          {"/", qOpSlash},
          {"%", qOpPercent},
          {"&", qOpBitAnd},
          {"|", qOpBitOr},
          {"^", qOpBitXor},
          {"~", qOpBitNot},
          {"<<", qOpLShift},
          {">>", qOpRShift},
          {"<<<", qOpROTL},
          {">>>", qOpROTR},
          {"&&", qOpLogicAnd},
          {"||", qOpLogicOr},
          {"^^", qOpLogicXor},
          {"!", qOpLogicNot},
          {"<", qOpLT},
          {">", qOpGT},
          {"<=", qOpLE},
          {">=", qOpGE},
          {"==", qOpEq},
          {"!=", qOpNE},
          {"=", qOpSet},
          {"+=", qOpPlusSet},
          {"-=", qOpMinusSet},
          {"*=", qOpTimesSet},
          {"/=", qOpSlashSet},
          {"%=", qOpPercentSet},
          {"&=", qOpBitAndSet},
          {"|=", qOpBitOrSet},
          {"^=", qOpBitXorSet},
          {"&&=", qOpLogicAndSet},
          {"||=", qOpLogicOrSet},
          {"^^=", qOpLogicXorSet},
          {"<<=", qOpLShiftSet},
          {">>=", qOpRShiftSet},
          {"<<<=", qOpROTLSet},
          {">>>=", qOpROTRSet},
          {"++", qOpInc},
          {"--", qOpDec},
          {"as", qOpAs},
          {"bitcast_as", qOpBitcastAs},
          {"in", qOpIn},
          {"out", qOpOut},
          {"sizeof", qOpSizeof},
          {"bitsizeof", qOpBitsizeof},
          {"alignof", qOpAlignof},
          {"typeof", qOpTypeof},
          {".", qOpDot},
          {"..", qOpRange},
          {"...", qOpEllipsis},
          {"=>", qOpArrow},
          {"?", qOpTernary},
      });

  static const boost::bimap<std::string_view, qlex_op_t> word_operators =
      make_bimap<std::string_view, qlex_op_t>({
          {"as", qOpAs},
          {"in", qOpIn},
          {"sizeof", qOpSizeof},
          {"alignof", qOpAlignof},
          {"typeof", qOpTypeof},
          {"bitcast_as", qOpBitcastAs},
          {"bitsizeof", qOpBitsizeof},
          {"out", qOpOut},
      });

  static const boost::bimap<std::string_view, qlex_punc_t> punctuation =
      make_bimap<std::string_view, qlex_punc_t>({
          {"(", qPuncLPar},
          {")", qPuncRPar},
          {"[", qPuncLBrk},
          {"]", qPuncRBrk},
          {"{", qPuncLCur},
          {"}", qPuncRCur},
          {",", qPuncComa},
          {":", qPuncColn},
          {";", qPuncSemi},
      });

  static constexpr std::array<uint8_t, 256> hextable = []() {
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
  /// END:   LEXER LOOKUP TABLES
  ///============================================================================///
}  // namespace qlex

static bool lex_is_space(uint8_t c) {
  return qlex::whitespace_table[static_cast<uint8_t>(c)];
}

enum class NumType {
  Decimal,
  DecimalExplicit,
  Hexadecimal,
  Binary,
  Octal,
  Floating,
};

static thread_local boost::unordered_map<std::string, std::string> can_cache;

///============================================================================///

static bool validate_identifier(std::string_view id) {
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

static bool canonicalize_float(std::string_view input, std::string &norm) {
  long double mantissa = 0, exponent = 0, x = 0;
  size_t e_pos = 0;

  if ((e_pos = input.find('e')) == std::string::npos) {
    norm = input.data();
    return true;
  }

  try {
    mantissa = std::stod(std::string(input.substr(0, e_pos)));
    exponent = std::stod(input.substr(e_pos + 1).data());
    x = mantissa * std::pow(10.0, exponent);
  } catch (...) {
    return false;
  }

  std::stringstream ss;
  ss << std::fixed << std::setprecision(FLOATING_POINT_PRECISION) << x;
  std::string str = ss.str();

  if (str.find('.') != std::string::npos) {
    while (!str.empty() && str.back() == '0') {
      str.pop_back();
    }

    if (str.back() == '.') {
      str.pop_back();
    }
  }

  norm = str;

  return true;
}

static bool canonicalize_number(std::string &number, std::string &norm,
                                NumType type) {
  if (can_cache.size() > 4096) {
    can_cache = {};
  } else if (can_cache.find(number) != can_cache.end()) {
    return norm = can_cache[number], true;
  }

  typedef unsigned int uint128_t __attribute__((mode(TI)));

  uint128_t x = 0, i = 0;

  std::transform(number.begin(), number.end(), number.begin(), ::tolower);
  std::erase(number, '_');

  switch (type) {
    case NumType::Hexadecimal: {
      for (i = 2; i < number.size(); ++i) {
        // Check for overflow
        if ((x >> 64) & 0xF000000000000000) {
          return false;
        }

        if (number[i] >= '0' && number[i] <= '9') {
          x = (x << 4) + (number[i] - '0');
        } else if (number[i] >= 'a' && number[i] <= 'f') {
          x = (x << 4) + (number[i] - 'a' + 10);
        } else {
          return false;
        }
      }
      break;
    }
    case NumType::Binary: {
      for (i = 2; i < number.size(); ++i) {
        // Check for overflow
        if ((x >> 64) & 0x8000000000000000) {
          return false;
        }
        if (number[i] != '0' && number[i] != '1') {
          return false;
        }

        x = (x << 1) + (number[i] - '0');
      }
      break;
    }
    case NumType::Octal: {
      for (i = 2; i < number.size(); ++i) {
        // Check for overflow
        if ((x >> 64) & 0xE000000000000000) {
          return false;
        }
        if (number[i] < '0' || number[i] > '7') {
          return false;
        }

        x = (x << 3) + (number[i] - '0');
      }
      break;
    }
    case NumType::DecimalExplicit: {
      for (i = 2; i < number.size(); ++i) {
        if (number[i] < '0' || number[i] > '9') {
          return false;
        }

        // check for overflow
        auto tmp = x;
        x = (x * 10) + (number[i] - '0');

        if (x < tmp) {
          return false;
        }
      }
      break;
    }
    case NumType::Decimal: {
      for (i = 0; i < number.size(); ++i) {
        if (number[i] < '0' || number[i] > '9') {
          return false;
        }

        // check for overflow
        auto tmp = x;
        x = (x * 10) + (number[i] - '0');
        if (x < tmp) {
          return false;
        }
      }
      break;
    }
    case NumType::Floating: {
      qcore_panic("unreachable");
    }
  }

  std::stringstream ss;
  if (x == 0) {
    ss << '0';
  }

  for (i = x; i; i /= 10) {
    ss << (char)('0' + i % 10);
  }

  std::string s = ss.str();
  std::reverse(s.begin(), s.end());

  return can_cache[number] = (norm = s), true;
}

void Tokenizer::reset_state() { m_pushback.clear(); }

char Tokenizer::nextc() {
  auto c = m_file.get();

  if (m_file.eof()) {
    throw ScannerEOF();
  }

  uint32_t line = GetCurrentLine(), col = GetCurrentColumn(),
           offset = GetCurrentOffset();

  if (c == '\n') {
    line++;
    col = 1;
  } else {
    col++;
  }

  offset++;

  UpdateLocation(line, col, offset, GetCurrentFilename());

  return c;
}

CPP_EXPORT Token Tokenizer::GetNext() {
  /**
   * **WARNING**: Do not just start editing this function without
   * having a holistic understanding of all code that depends on the lexer
   * (most of the pipeline).
   *
   * This function provides various undocumented invariant guarantees that
   * if broken will likely result in internal runtime corruption and other
   * undefined behavior.
   * */

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

  std::string buf;

  LexState state = LexState::Start;
  uint32_t state_parens = 0;
  char c = 0;
  bool eof_is_error = true;
  uint32_t start_pos{};

  try {
    while (true) {
      { /* If the Lexer over-consumed, we will return the saved character */
        if (m_pushback.empty()) {
          eof_is_error = false;
          c = nextc();
          eof_is_error = true;
          (void)eof_is_error;
        } else {
          c = m_pushback.front();
          m_pushback.pop_front();
        }
      }

      switch (state) {
        case LexState::Start: {
          if (lex_is_space(c)) {
            continue;
          }

          start_pos = GetCurrentOffset();

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
          { /* Read in what is hopefully an identifier */
            int colon_state = 0;

            while (std::isalnum(c) || c == '_' || c == ':') {
              if (c != ':' && colon_state == 1) {
                if (!buf.ends_with("::")) {
                  char tc = buf.back();
                  buf.pop_back();
                  m_pushback.push_back(tc);
                  break;
                }
                colon_state = 0;
              } else if (c == ':') {
                colon_state = 1;
              }

              buf += c;
              c = nextc();
            }
          }

          /* Check for f-string */
          if (buf == "f" && c == '"') {
            m_pushback.push_back(c);
            return Token(qKeyW, qK__FString, start_pos);
          }

          /* We overshot; this must be a punctor ':' */
          if (buf.size() > 0 && buf.back() == ':') {
            char tc = buf.back();
            buf.pop_back();
            m_pushback.push_back(tc);
          }
          m_pushback.push_back(c);

          { /* Determine if it's a keyword or an identifier */
            auto it = qlex::keywords.left.find(buf);
            if (it != qlex::keywords.left.end()) {
              return Token(qKeyW, it->second, start_pos);
            }
          }

          { /* Check if it's an operator */
            auto it = qlex::word_operators.left.find(buf);
            if (it != qlex::word_operators.left.end()) {
              return Token(qOper, it->second, start_pos);
            }
          }

          /* Ensure it's a valid identifier */
          if (!validate_identifier(buf)) {
            goto error_0;
          }

          /* For compiler internal debugging */
          qcore_assert(buf != "__builtin_lexer_crash",
                       "The source code invoked a compiler panic API.");

          if (buf == "__builtin_lexer_abort") {
            return Token::eof(start_pos);
          }

          /* Return the identifier */
          return Token(qName, intern(buf), start_pos);
        }
        case LexState::Integer: {
          NumType type = NumType::Decimal;

          /* [0x, 0X, 0d, 0D, 0b, 0B, 0o, 0O] */
          if (buf.size() == 1 && buf[0] == '0') {
            switch (c) {
              case 'x':
              case 'X':
                type = NumType::Hexadecimal;
                buf += c;
                c = nextc();
                break;
              case 'b':
              case 'B':
                type = NumType::Binary;
                buf += c;
                c = nextc();
                break;
              case 'o':
              case 'O':
                type = NumType::Octal;
                buf += c;
                c = nextc();
                break;
              case 'd':
              case 'D':
                type = NumType::DecimalExplicit;
                buf += c;
                c = nextc();
                break;
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
            if (c == '_') {
              while (lex_is_space(c) || c == '_' || c == '\\') {
                c = nextc();
              }
            } else if (lex_is_space(c)) {
              is_lexing = false;
              break;
            }

            switch (type) {
              case NumType::Decimal: {
                if (std::isdigit(c)) {
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
                  m_pushback.push_back(c);
                  is_lexing = false;
                }
                break;
              }

              case NumType::DecimalExplicit: {
                if (std::isdigit(c)) {
                  buf += c;
                } else {
                  m_pushback.push_back(c);
                  is_lexing = false;
                }
                break;
              }

              case NumType::Hexadecimal: {
                if (std::isxdigit(c)) {
                  buf += c;
                } else {
                  m_pushback.push_back(c);
                  is_lexing = false;
                }
                break;
              }

              case NumType::Binary: {
                if (c == '0' || c == '1') {
                  buf += c;
                } else {
                  m_pushback.push_back(c);
                  is_lexing = false;
                }
                break;
              }

              case NumType::Octal: {
                if (c >= '0' && c <= '7') {
                  buf += c;
                } else {
                  m_pushback.push_back(c);
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
                      m_pushback.push_back(c);
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
                      m_pushback.push_back(c);
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
                      m_pushback.push_back(c);
                      is_lexing = false;
                    }
                    break;
                  }

                  case FloatPart::ExponentPost: {
                    if (std::isdigit(c)) {
                      buf += c;
                    } else {
                      m_pushback.push_back(c);
                      is_lexing = false;
                    }
                    break;
                  }
                }

                break;
              }
            }

            if (is_lexing) {
              c = nextc();
            }
          }

          std::string norm;
          if (type == NumType::Floating) {
            if (canonicalize_float(buf, norm)) {
              return Token(qNumL, intern(std::move(norm)), start_pos);
            }
          } else if (canonicalize_number(buf, norm, type)) {
            return Token(qIntL, intern(std::move(norm)), start_pos);
          }

          /* Invalid number */
          goto error_0;
        }
        case LexState::CommentStart: {
          if (c == '/') { /* Single line comment */
            state = LexState::CommentSingleLine;
            continue;
          } else if (c == '*') { /* Multi-line comment */
            state = LexState::CommentMultiLine;
            continue;
          } else { /* Divide operator */
            m_pushback.push_back(c);
            return Token(qOper, qOpSlash, start_pos);
          }
        }
        case LexState::CommentSingleLine: {
          while (c != '\n') {
            buf += c;
            c = nextc();
          }

          return Token(qNote, intern(std::move(buf)), start_pos);
        }
        case LexState::CommentMultiLine: {
          size_t level = 1;

          while (true) {
            if (c == '/') {
              char tmp = nextc();
              if (tmp == '*') {
                level++;
                buf += "/*";
              } else {
                buf += c;
                buf += tmp;
              }

              c = nextc();
            } else if (c == '*') {
              char tmp = nextc();
              if (tmp == '/') {
                level--;
                if (level == 0) {
                  return Token(qNote, intern(std::move(buf)), start_pos);
                } else {
                  buf += "*";
                  buf += tmp;
                }
              } else {
                buf += c;
                buf += tmp;
              }
              c = nextc();
            } else {
              buf += c;
              c = nextc();
            }
          }

          continue;
        }
        case LexState::String: {
          if (c != buf[0]) {
            /* Normal character */
            if (c != '\\') {
              buf += c;
              continue;
            }

            /* String escape sequences */
            c = nextc();
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
                char hex[2] = {nextc(), nextc()};
                if (!std::isxdigit(hex[0]) || !std::isxdigit(hex[1])) {
                  goto error_0;
                }
                buf += (qlex::hextable[(uint8_t)hex[0]] << 4) |
                       qlex::hextable[(uint8_t)hex[1]];
                break;
              }
              case 'u': {
                c = nextc();
                if (c != '{') {
                  goto error_0;
                }

                std::string hex;

                while (true) {
                  c = nextc();
                  if (c == '}') {
                    break;
                  }

                  if (!std::isxdigit(c)) {
                    goto error_0;
                  }

                  hex += c;
                }

                uint32_t codepoint;
                try {
                  codepoint = std::stoi(hex, nullptr, 16);
                } catch (...) {
                  goto error_0;
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
                  goto error_0;
                }

                break;
              }
              case 'o': {
                char oct[4] = {nextc(), nextc(), nextc(), 0};
                try {
                  buf += std::stoi(oct, nullptr, 8);
                  break;
                } catch (...) {
                  goto error_0;
                }
              }
              case 'b': {
                c = nextc();
                if (c != '{') {
                  goto error_0;
                }

                std::string bin;

                while (true) {
                  c = nextc();
                  if (c == '}') {
                    break;
                  }

                  if ((c != '0' && c != '1') || bin.size() >= 64) {
                    goto error_0;
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
            continue;
          } else {
            do {
              c = nextc();
            } while (lex_is_space(c) || c == '\\');

            if (c == buf[0]) {
              continue;
            } else {
              m_pushback.push_back(c);
              /* Character or string */
              if (buf.front() == '\'' && buf.size() == 2) {
                return Token(qChar, intern(std::string(1, buf[1])), start_pos);
              } else {
                return Token(qText, intern(buf.substr(1, buf.size() - 1)),
                             start_pos);
              }
            }
          }
        }
        case LexState::MacroStart: {
          /*
           * Macros start with '@' and can be either single-line or block
           * macros. Block macros are enclosed in parentheses. Single-line
           * macros end with a newline character or a special cases
           */
          if (c == '(') {
            state = LexState::BlockMacro, state_parens = 1;
            continue;
          } else {
            state = LexState::SingleLineMacro, state_parens = 0;
            buf += c;
            continue;
          }
          break;
        }
        case LexState::SingleLineMacro: {
          /*
          Format:
              ... @macro_name  ...
          */

          while (std::isalnum(c) || c == '_' || c == ':') {
            buf += c;
            c = nextc();
          }

          m_pushback.push_back(c);

          return Token(qMacr, intern(std::move(buf)), start_pos);
        }
        case LexState::BlockMacro: {
          while (true) {
            if (c == '(') {
              state_parens++;
            } else if (c == ')') {
              state_parens--;
            }

            if (state_parens == 0) {
              return Token(qMacB, intern(std::move(buf)), start_pos);
            }

            buf += c;

            c = nextc();
          }
          continue;
        }
        case LexState::Other: {
          /* Check if it's a punctor */
          if (buf.size() == 1) {
            auto it = qlex::punctuation.left.find(buf);
            if (it != qlex::punctuation.left.end()) {
              m_pushback.push_back(c);
              return Token(qPunc, it->second, start_pos);
            }
          }

          /* Special case for a comment */
          if ((buf[0] == '~' && c == '>')) {
            buf.clear();
            state = LexState::CommentSingleLine;
            continue;
          }

          /* Special case for a comment */
          if (buf[0] == '#') {
            buf.clear();
            m_pushback.push_back(c);
            state = LexState::CommentSingleLine;
            continue;
          }

          bool found = false;
          while (true) {
            bool contains = false;
            if (std::any_of(
                    qlex::operators.begin(), qlex::operators.end(),
                    [&](const auto &pair) { return pair.left == buf; })) {
              contains = true;
              found = true;
            }

            if (contains) {
              buf += c;
              if (buf.size() > 4) { /* Handle infinite error case */
                goto error_0;
              }
              c = nextc();
            } else {
              break;
            }
          }

          if (!found) {
            goto error_0;
          }

          m_pushback.push_back(buf.back());
          m_pushback.push_back(c);
          return Token(qOper,
                       qlex::operators.left.at(buf.substr(0, buf.size() - 1)),
                       start_pos);
        }
      }
    }
    goto error_0;
  } catch (std::exception &e) { /* This should never happen */
    qcore_panicf("The lexer has a bug: %s", e.what());
  }

error_0: { /* Reset the lexer and return error token */
  reset_state();

  return Token::eof(start_pos);
}
}

///============================================================================///

CPP_EXPORT const char *ncc::lex::qlex_ty_str(qlex_ty_t ty) {
  switch (ty) {
    case qEofF:
      return "eof";
    case qKeyW:
      return "key";
    case qOper:
      return "op";
    case qPunc:
      return "sym";
    case qName:
      return "name";
    case qIntL:
      return "int";
    case qNumL:
      return "num";
    case qText:
      return "str";
    case qChar:
      return "char";
    case qMacB:
      return "macb";
    case qMacr:
      return "macr";
    case qNote:
      return "note";
  }

  qcore_panic("unreachable");
}

CPP_EXPORT void ncc::lex::qlex_tok_fromstr(ncc::lex::IScanner *, qlex_ty_t ty,
                                           const char *str, Token *out) {
  try {
    out->ty = ty;
    out->start = 0;

    switch (ty) {
      case qEofF: {
        break;
      }

      case qKeyW: {
        auto find = qlex::keywords.left.find(str);
        if (find == qlex::keywords.left.end()) [[unlikely]] {
          out->ty = qEofF;
        } else {
          out->v.key = find->second;
        }
        break;
      }

      case qOper: {
        auto find = qlex::operators.left.find(str);
        if (find == qlex::operators.left.end()) [[unlikely]] {
          out->ty = qEofF;
        } else {
          out->v.op = find->second;
        }
        break;
      }

      case qPunc: {
        auto find = qlex::punctuation.left.find(str);
        if (find == qlex::punctuation.left.end()) [[unlikely]] {
          out->ty = qEofF;
        } else {
          out->v.punc = find->second;
        }
        break;
      }

      case qName: {
        out->v.str_idx = intern(str);
        break;
      }

      case qIntL: {
        out->v.str_idx = intern(str);
        break;
      }

      case qNumL: {
        out->v.str_idx = intern(str);
        break;
      }

      case qText: {
        out->v.str_idx = intern(str);
        break;
      }

      case qChar: {
        out->v.str_idx = intern(str);
        break;
      }

      case qMacB: {
        out->v.str_idx = intern(str);
        break;
      }

      case qMacr: {
        out->v.str_idx = intern(str);
        break;
      }

      case qNote: {
        out->v.str_idx = intern(str);
        break;
      }
    }

  } catch (std::bad_alloc &) {
    qcore_panic("qlex_tok_fromstr: failed to create token: out of memory");
  } catch (...) {
    qcore_panic("qlex_tok_fromstr: failed to create token");
  }
}

CPP_EXPORT std::ostream &ncc::lex::operator<<(std::ostream &os, qlex_ty_t ty) {
  switch (ty) {
    case qEofF: {
      os << "qEofF";
      break;
    }

    case qKeyW: {
      os << "qKeyW";
      break;
    }

    case qOper: {
      os << "qOper";
      break;
    }

    case qPunc: {
      os << "qPunc";
      break;
    }

    case qName: {
      os << "qName";
      break;
    }

    case qIntL: {
      os << "qIntL";
      break;
    }

    case qNumL: {
      os << "qNumL";
      break;
    }

    case qText: {
      os << "qText";
      break;
    }

    case qChar: {
      os << "qChar";
      break;
    }

    case qMacB: {
      os << "qMacB";
      break;
    }

    case qMacr: {
      os << "qMacr";
      break;
    }

    case qNote: {
      os << "qNote";
      break;
    }
  }

  return os;
}

CPP_EXPORT std::ostream &ncc::lex::operator<<(std::ostream &os, Token tok) {
  os << tok.as_string();
  return os;
}

CPP_EXPORT const char *ncc::lex::op_repr(qlex_op_t op) {
  return qlex::operators.right.at(op).data();
}

CPP_EXPORT const char *ncc::lex::kw_repr(qlex_key_t kw) {
  return qlex::keywords.right.at(kw).data();
}

CPP_EXPORT const char *ncc::lex::punct_repr(qlex_punc_t punct) {
  return qlex::punctuation.right.at(punct).data();
}
