////////////////////////////////////////////////////////////////////////////////
///                                                                          ///
///  ░▒▓██████▓▒░░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░ ░▒▓██████▓▒░  ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░        ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓██████▓▒░░▒▓█▓▒░      ░▒▓█▓▒░        ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░        ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ///
///  ░▒▓██████▓▒░ ░▒▓██████▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░ ░▒▓██████▓▒░  ///
///    ░▒▓█▓▒░                                                               ///
///     ░▒▓██▓▒░                                                             ///
///                                                                          ///
///   * QUIX LANG COMPILER - The official compiler for the Quix language.    ///
///   * Copyright (C) 2024 Wesley C. Jones                                   ///
///                                                                          ///
///   The QUIX Compiler Suite is free software; you can redistribute it or   ///
///   modify it under the terms of the GNU Lesser General Public             ///
///   License as published by the Free Software Foundation; either           ///
///   version 2.1 of the License, or (at your option) any later version.     ///
///                                                                          ///
///   The QUIX Compiler Suite is distributed in the hope that it will be     ///
///   useful, but WITHOUT ANY WARRANTY; without even the implied warranty of ///
///   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      ///
///   Lesser General Public License for more details.                        ///
///                                                                          ///
///   You should have received a copy of the GNU Lesser General Public       ///
///   License along with the QUIX Compiler Suite; if not, see                ///
///   <https://www.gnu.org/licenses/>.                                       ///
///                                                                          ///
////////////////////////////////////////////////////////////////////////////////

#define __QUIX_IMPL__

#include <quix-core/Error.h>
#include <quix-lexer/Lexer.h>
#include <string.h>

#include <array>
#include <boost/bimap.hpp>
#include <boost/unordered_map.hpp>
#include <cctype>
#include <charconv>
#include <cmath>
#include <csetjmp>
#include <cstdio>
#include <deque>
#include <iomanip>
#include <quix-lexer/Base.hh>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "LibMacro.h"

///============================================================================///
/// BEGIN: LEXICAL GRAMMAR CONSTRAINTS
#define FLOATING_POINT_PRECISION 128
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

  static const boost::bimap<std::string_view, qlex_key_t> keywords =
      make_bimap<std::string_view, qlex_key_t>({
          {"subsystem", qKSubsystem},
          {"import", qKImport},
          {"pub", qKPub},
          {"sec", qKSec},
          {"pro", qKPro},
          {"type", qKType},
          {"let", qKLet},
          {"var", qKVar},
          {"const", qKConst},
          {"static", qKStatic},
          {"struct", qKStruct},
          {"region", qKRegion},
          {"group", qKGroup},
          {"class", qKClass},
          {"union", qKUnion},
          {"opaque", qKOpaque},
          {"enum", qKEnum},
          {"fstring", qKFString},
          {"with", qKWith},
          {"fn", qKFn},
          {"noexcept", qKNoexcept},
          {"foreign", qKForeign},
          {"impure", qKImpure},
          {"tsafe", qKTsafe},
          {"pure", qKPure},
          {"quasipure", qKQuasipure},
          {"retropure", qKRetropure},
          {"crashpoint", qKCrashpoint},
          {"inline", qKInline},
          {"unsafe", qKUnsafe},
          {"safe", qKSafe},
          {"volatile", qKVolatile},
          {"promise", qKPromise},
          {"if", qKIf},
          {"else", qKElse},
          {"for", qKFor},
          {"while", qKWhile},
          {"do", qKDo},
          {"switch", qKSwitch},
          {"case", qKCase},
          {"default", qKDefault},
          {"break", qKBreak},
          {"continue", qKContinue},
          {"ret", qKReturn},
          {"retif", qKRetif},
          {"retz", qKRetz},
          {"retv", qKRetv},
          {"form", qKForm},
          {"foreach", qKForeach},
          {"__asm__", qK__Asm__},
          {"void", qKVoid},
          {"undef", qKUndef},
          {"null", qKNull},
          {"true", qKTrue},
          {"false", qKFalse},
      });

  static const boost::bimap<std::string_view, qlex_op_t> operators =
      make_bimap<std::string_view, qlex_op_t>({
          {"?", qOpTernary},
          {"=>", qOpArrow},
          {".", qOpDot},
          {"+", qOpPlus},
          {"-", qOpMinus},
          {"*", qOpTimes},
          {"/", qOpSlash},
          {"%", qOpPercent},
          {"&", qOpBitAnd},
          {"|", qOpBitOr},
          {"^", qOpBitXor},
          {"~", qOpBitNot},
          {"&&", qOpLogicAnd},
          {"||", qOpLogicOr},
          {"^^", qOpLogicXor},
          {"!", qOpLogicNot},
          {"<<", qOpLShift},
          {">>", qOpRShift},
          {">>>", qOpROTR},
          {"<<<", qOpROTL},
          {"++", qOpInc},
          {"--", qOpDec},
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
          {">>>=", qOpROTRSet},
          {"<<<=", qOpROTLSet},
          {"<", qOpLT},
          {">", qOpGT},
          {"<=", qOpLE},
          {">=", qOpGE},
          {"==", qOpEq},
          {"!=", qOpNE},
          {"as", qOpAs},
          {"is", qOpIs},
          {"in", qOpIn},
          {"sizeof", qOpSizeof},
          {"alignof", qOpAlignof},
          {"typeof", qOpTypeof},
          {"offsetof", qOpOffsetof},
          {"..", qOpRange},
          {"...", qOpEllipsis},
          {"bitcast_as", qOpBitcastAs},
          {"bitsizeof", qOpBitsizeof},
          {"out", qOpOut},
      });

  static const boost::bimap<std::string_view, qlex_op_t> word_operators =
      make_bimap<std::string_view, qlex_op_t>({
          {"as", qOpAs},
          {"is", qOpIs},
          {"in", qOpIn},
          {"sizeof", qOpSizeof},
          {"alignof", qOpAlignof},
          {"typeof", qOpTypeof},
          {"offsetof", qOpOffsetof},
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

  // Precomputed lookup table for hex char to byte conversion
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
  /// END:   LEXER LOOKUP TABLES
  ///============================================================================///

  ///============================================================================///
  /// BEGIN: LEXER INTERNAL TYPES
  typedef std::string ident_buf_t;
  typedef std::string num_buf_t;
  /// END:   LEXER INTERNAL TYPES
  ///============================================================================///

}  // namespace qlex

enum class NumType {
  Invalid,
  Decimal,
  DecimalExplicit,
  Hexadecimal,
  Binary,
  Octal,
  Floating,
};

static thread_local boost::unordered_map<qlex::num_buf_t, NumType> num_cache;
static thread_local boost::unordered_map<qlex::num_buf_t, qlex::num_buf_t> can_cache;

///============================================================================///

LIB_EXPORT qlex_t *qlex_new(FILE *file, const char *filename, qcore_env_t env) {
  try {
    return new qlex_t(file, filename, false, env);
  } catch (std::bad_alloc &) {
    return nullptr;
  } catch (...) {
    qcore_panic("qlex_new: failed to create lexer");
  }
}

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

static NumType check_number_literal_type(qlex::num_buf_t &input) {
  if (num_cache.size() > 4096) {
    num_cache = {};
  } else if (num_cache.find(input) != num_cache.end()) {
    return num_cache[input];
  }

  if (input.empty()) return num_cache[input] = NumType::Invalid;

  /* Check if it's a single digit */
  if (input.size() < 3) {
    if (std::isdigit(input[0]))
      return num_cache[input] = NumType::Decimal;
    else
      return num_cache[input] = NumType::Invalid;
  }

  std::transform(input.begin(), input.end(), input.begin(), ::tolower);
  std::erase(input, '_');

  std::string prefix;
  prefix.push_back(input[0]);
  prefix.push_back(input[1]);

  size_t i;

  if (prefix == "0x") {
    for (i = 2; i < input.size(); i++)
      if (!((input[i] >= '0' && input[i] <= '9') || (input[i] >= 'a' && input[i] <= 'f')))
        return num_cache[input] = NumType::Invalid;

    return num_cache[input] = NumType::Hexadecimal;
  } else if (prefix == "0b") {
    for (i = 2; i < input.size(); i++)
      if (!(input[i] == '0' || input[i] == '1')) return num_cache[input] = NumType::Invalid;

    return num_cache[input] = NumType::Binary;
  } else if (prefix == "0o") {
    for (i = 2; i < input.size(); i++)
      if (!(input[i] >= '0' && input[i] <= '7')) return num_cache[input] = NumType::Invalid;

    return num_cache[input] = NumType::Octal;
  } else if (prefix == "0d") {
    for (i = 2; i < input.size(); i++)
      if (!(input[i] >= '0' && input[i] <= '9')) return num_cache[input] = NumType::Invalid;

    return num_cache[input] = NumType::DecimalExplicit;
  } else {
    for (i = 0; i < input.size(); i++) {
      if (!(input[i] >= '0' && input[i] <= '9')) {
        try {
          double x;
          auto r = std::from_chars(input.data(), input.data() + input.size(), x);

          if (r.ec == std::errc::invalid_argument || r.ec == std::errc::result_out_of_range) {
            return num_cache[input] = NumType::Invalid;
          }

          return num_cache[input] = NumType::Floating;
        } catch (...) {
          return num_cache[input] = NumType::Invalid;
        }
      }
    }

    return num_cache[input] = NumType::Decimal;
  }
}

static bool canonicalize_float(std::string_view input, std::string &norm) {
  double mantissa = 0, exponent = 0, x = 0;
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
  ss << std::setprecision(FLOATING_POINT_PRECISION) << x;
  norm = ss.str();

  return true;
}

static bool canonicalize_number(qlex::num_buf_t &number, std::string &norm, NumType type) {
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
    default:
      break;
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

void qlex_t::reset_automata() { m_pushback.clear(); }

void qlex_t::eof_callback() {}

qlex_tok_t qlex_t::next_impl() {
  /// TODO: Correctly handle token source locations

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
  qlex_size state_parens = 0;
  char c;
  bool eof_is_error = true;
  qlex_loc_t start_pos{};

  try {
    while (true) {
      { /* If the Lexer over-consumed, we will return the saved character */
        if (m_pushback.empty()) {
          eof_is_error = false;
          c = getc();
          eof_is_error = true;
          (void)eof_is_error;
        } else {
          c = m_pushback.front();
          m_pushback.pop_front();
        }
      }

      if (c == EOF) {
        return qlex_tok_t::eof(cur_loc(), cur_loc());
      }

      switch (state) {
        case LexState::Start: {
          if (std::isspace(c)) {
            continue;
          } else if (std::isalpha(c) || c == '_') {
            /* Identifier or keyword or operator */
            start_pos = cur_loc();
            buf += c, state = LexState::Identifier;
            continue;
          } else if (c == '/') {
            start_pos = cur_loc();
            state = LexState::CommentStart; /* Comment or operator */
            continue;
          } else if (std::isdigit(c)) {
            start_pos = cur_loc();
            buf += c, state = LexState::Integer;
            continue;
          } else if (c == '"' || c == '\'') {
            start_pos = cur_loc();
            buf += c, state = LexState::String;
            continue;
          } else if (c == '@') {
            start_pos = cur_loc();
            state = LexState::MacroStart;
            continue;
          } else {
            /* Operator or punctor or invalid */
            start_pos = cur_loc();
            buf += c;
            state = LexState::Other;
            continue;
          }
        }
        case LexState::Identifier: {
          qlex::ident_buf_t ibuf;
          ibuf += buf[0];

          { /* Read in what is hopefully an identifier */
            int colon_state = 0;

            while (std::isalnum(c) || c == '_' || c == ':') {
              if (c != ':' && colon_state == 1) {
                if (!ibuf.ends_with("::")) {
                  char tc = ibuf.back();
                  ibuf.pop_back();
                  m_pushback.push_back(tc);
                  break;
                }
                colon_state = 0;
              } else if (c == ':') {
                colon_state = 1;
              }

              ibuf += c;
              c = getc();
            }
          }

          /* Check for f-string */
          if (ibuf == "f" && c == '"') {
            m_pushback.push_back(c);
            return qlex_tok_t(qKeyW, qKFString, start_pos, cur_loc());
          }

          /* We overshot; this must be a punctor ':' */
          if (ibuf.size() > 0 && ibuf.back() == ':') {
            char tc = ibuf.back();
            ibuf.pop_back();
            m_pushback.push_back(tc);
          }
          m_pushback.push_back(c);

          { /* Determine if it's a keyword or an identifier */
            auto it = qlex::keywords.left.find(ibuf);
            if (it != qlex::keywords.left.end()) {
              return qlex_tok_t(qKeyW, it->second, start_pos, cur_loc());
            }
          }

          { /* Check if it's an operator */
            auto it = qlex::word_operators.left.find(ibuf);
            if (it != qlex::word_operators.left.end()) {
              return qlex_tok_t(qOper, it->second, start_pos, cur_loc());
            }
          }

          /* Ensure it's a valid identifier */
          if (!validate_identifier(ibuf)) {
            goto error_0;
          }

          /* Return the identifier */
          return qlex_tok_t(qName, put_string(ibuf), start_pos, cur_loc());
        }
        case LexState::Integer: {
          qlex::num_buf_t nbuf;

          nbuf += buf[0];
          { /* Read in what is hopefully an integer */
            while (true) {
              if (!(std::isxdigit(c) || c == '_' || c == '-' || c == '.' || c == 'x' || c == 'b' ||
                    c == 'd' || c == 'o' || c == 'e' || c == '.')) {
                break;
              }

              if (c == '_') {
                do {
                  c = getc();
                } while (std::isspace(c) || c == '_' || c == '\\');
              }

              nbuf += c;
              c = getc();
            }
          }

          { /* Handle lexical ambiguity */
            std::vector<char> items;

            bool cutting = true;
            while (cutting) {
              if (nbuf.empty()) {
                break;
              }

              char last = nbuf.back();

              switch (last) {
                case '_':
                case '.':
                case '-':
                  items.push_back(last);
                  nbuf.pop_back();
                  break;
                default:
                  cutting = false;
                  break;
              }
            }

            for (auto it = items.rbegin(); it != items.rend(); ++it) {
              m_pushback.push_back(*it);
            }
            m_pushback.push_back(c);
          }

          /* Check if it's a floating point number */
          NumType type;

          std::string norm;
          if ((type = check_number_literal_type(nbuf)) == NumType::Floating) {
            if (canonicalize_float(nbuf, norm)) {
              return qlex_tok_t(qNumL, put_string(std::move(norm)), start_pos, cur_loc());
            } else {
              goto error_0;
            }
          }

          /* Check if it's a valid number */
          if (type == NumType::Invalid) {
            goto error_0;
          }

          /* Canonicalize the number */
          if (canonicalize_number(nbuf, norm, type)) {
            return qlex_tok_t(qIntL, put_string(std::move(norm)), start_pos, cur_loc());
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
            return qlex_tok_t(qOper, qOpSlash, start_pos, cur_loc());
          }
        }
        case LexState::CommentSingleLine: {
          while (c != '\n') {
            buf += c;
            c = getc();
          }

          return qlex_tok_t(qNote, put_string(std::move(buf)), start_pos, cur_loc());
        }
        case LexState::CommentMultiLine: {
          size_t level = 1;

          while (true) {
            if (c == '/') {
              char tmp = getc();
              if (tmp == '*') {
                level++;
                buf += "/*";
              } else {
                buf += c;
                buf += tmp;
              }

              c = getc();
            } else if (c == '*') {
              char tmp = getc();
              if (tmp == '/') {
                level--;
                if (level == 0) {
                  return qlex_tok_t(qNote, put_string(std::move(buf)), start_pos, cur_loc());
                } else {
                  buf += "*";
                  buf += tmp;
                }
              } else {
                buf += c;
                buf += tmp;
              }
              c = getc();
            } else {
              buf += c;
              c = getc();
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
            c = getc();
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
                char hex[2] = {getc(), getc()};
                if (!std::isxdigit(hex[0]) || !std::isxdigit(hex[1])) {
                  goto error_0;
                }
                buf += (qlex::hextable[(uint8_t)hex[0]] << 4) | qlex::hextable[(uint8_t)hex[1]];
                break;
              }
              case 'u': {
                c = getc();
                if (c != '{') {
                  goto error_0;
                }

                std::string hex;

                while (true) {
                  c = getc();
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
                char oct[4] = {getc(), getc(), getc(), 0};
                try {
                  buf += std::stoi(oct, nullptr, 8);
                  break;
                } catch (...) {
                  goto error_0;
                }
              }
              case 'b': {
                c = getc();
                if (c != '{') {
                  goto error_0;
                }

                std::string bin;

                while (true) {
                  c = getc();
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
              c = getc();
            } while (std::isspace(c) || c == '\\');

            if (c == buf[0]) {
              continue;
            } else {
              m_pushback.push_back(c);
              /* Character or string */
              if (buf.front() == '\'' && buf.size() == 2) {
                return qlex_tok_t(qChar, put_string(std::string(1, buf[1])), start_pos, cur_loc());
              } else {
                return qlex_tok_t(qText, put_string(buf.substr(1, buf.size() - 1)), start_pos,
                                  cur_loc());
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

          while (std::isalnum(c) || c == '_') {
            buf += c;
            c = getc();
          }

          while (true) {
            if (c == '(') {
              state_parens++;
            } else if (c == ')') {
              state_parens--;
            }

            buf += c;

            if (state_parens == 0) {
              return qlex_tok_t(qMacr, put_string(std::move(buf)), start_pos, cur_loc());
            }

            c = getc();
          }

          continue;
        }
        case LexState::BlockMacro: {
          while (true) {
            if (c == '(') {
              state_parens++;
            } else if (c == ')') {
              state_parens--;
            }

            if (state_parens == 0) {
              return qlex_tok_t(qMacB, put_string(std::move(buf)), start_pos, cur_loc());
            }

            buf += c;

            c = getc();
          }
          continue;
        }
        case LexState::Other: {
          /* Check if it's a punctor */
          if (buf.size() == 1) {
            auto it = qlex::punctuation.left.find(buf);
            if (it != qlex::punctuation.left.end()) {
              m_pushback.push_back(c);
              return qlex_tok_t(qPunc, it->second, start_pos, cur_loc());
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
            if (std::any_of(qlex::operators.begin(), qlex::operators.end(),
                            [&](const auto &pair) { return pair.left == buf; })) {
              contains = true;
              found = true;
            }

            if (contains) {
              buf += c;
              if (buf.size() > 4) { /* Handle infinite error case */
                goto error_0;
              }
              c = getc();
            } else {
              break;
            }
          }

          if (!found) {
            goto error_0;
          }

          m_pushback.push_back(buf.back());
          m_pushback.push_back(c);
          return qlex_tok_t(qOper, qlex::operators.left.at(buf.substr(0, buf.size() - 1)),
                            start_pos, cur_loc());
        }
      }
    }
    goto error_0;
  } catch (std::exception &e) { /* This should never happen */
    qcore_panicf("qlex_t::do_automata: %s. The lexer has a bug.", e.what());
  }

error_0: { /* Reset the lexer and return error token */
  reset_automata();

  return qlex_tok_t::err(cur_loc(), cur_loc());
}
}

///============================================================================///

LIB_EXPORT qlex_size qlex_tok_size(qlex_t *lexer, const qlex_tok_t *tok) {
  try {
    switch (tok->ty) {
      case qEofF:
      case qErro:
        return 0;
      case qKeyW:
        return qlex::keywords.right.at(tok->v.key).size();
      case qOper:
        return qlex::operators.right.at(tok->v.op).size();
      case qPunc:
        return qlex::punctuation.right.at(tok->v.punc).size();
      case qName:
      case qIntL:
      case qNumL:
      case qText:
      case qChar:
      case qMacB:
      case qMacr:
      case qNote:
        return qlex_span(lexer, qlex_begin(tok), qlex_end(tok));
    }
  } catch (std::out_of_range &) {
    return 0;
  } catch (...) {
    qcore_panic("qlex_tok_size: invalid token");
  }

  __builtin_unreachable();
}

LIB_EXPORT qlex_size qlex_tok_write(qlex_t *lexer, const qlex_tok_t *tok, char *buf,
                                    qlex_size size) {
  try {
    size_t ret;

    switch (tok->ty) {
      case qEofF:
      case qErro:
        ret = 0;
        break;
      case qKeyW: {
        if ((ret = qlex::keywords.right.at(tok->v.key).size()) <= size) {
          memcpy(buf, qlex::keywords.right.at(tok->v.key).data(), ret);
        } else {
          ret = 0;
        }
        break;
      }
      case qOper: {
        if ((ret = qlex::operators.right.at(tok->v.op).size()) <= size) {
          memcpy(buf, qlex::operators.right.at(tok->v.op).data(), ret);
        } else {
          ret = 0;
        }
        break;
      }
      case qPunc: {
        if ((ret = qlex::punctuation.right.at(tok->v.punc).size()) <= size) {
          memcpy(buf, qlex::punctuation.right.at(tok->v.punc).data(), ret);
        } else {
          ret = 0;
        }
        break;
      }
      case qName:
      case qIntL:
      case qNumL:
      case qText:
      case qChar:
      case qMacB:
      case qMacr:
      case qNote: {
        auto sv = lexer->get_string(tok->v.str_idx);
        if ((ret = sv.size()) <= size) {
          memcpy(buf, sv.data(), ret);
        } else {
          ret = 0;
        }
        break;
      }
    }

    return ret;
  } catch (std::out_of_range &) {
    return 0;
  } catch (...) {
    qcore_panic("qlex_tok_write: invalid token");
  }

  __builtin_unreachable();
}

LIB_EXPORT const char *qlex_ty_str(qlex_ty_t ty) {
  switch (ty) {
    case qEofF:
      return "eof";
    case qErro:
      return "err";
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

  __builtin_unreachable();
}

LIB_EXPORT bool qlex_eq(qlex_t *lexer, const qlex_tok_t *a, const qlex_tok_t *b) {
  try {
    if (a->ty != b->ty) return false;

    switch (a->ty) {
      case qEofF:
      case qErro:
        return true;
      case qKeyW:
        return a->v.key == b->v.key;
      case qOper:
        return a->v.op == b->v.op;
      case qPunc:
        return a->v.punc == b->v.punc;
      case qName:
      case qIntL:
      case qNumL:
      case qText:
      case qChar:
      case qMacB:
      case qMacr:
      case qNote: {
        return lexer->get_string(a->v.str_idx) == lexer->get_string(b->v.str_idx);
      }
    }
  } catch (std::out_of_range &) {
    return false;
  } catch (...) {
    qcore_panic("qlex_eq: invalid token");
  }

  __builtin_unreachable();
}

LIB_EXPORT bool qlex_lt(qlex_t *lexer, const qlex_tok_t *a, const qlex_tok_t *b) {
  try {
    if (a->ty != b->ty) return a->ty < b->ty;

    switch (a->ty) {
      case qEofF:
      case qErro:
        return false;
      case qKeyW:
        return a->v.key < b->v.key;
      case qOper:
        return a->v.op < b->v.op;
      case qPunc:
        return a->v.punc < b->v.punc;
      case qName:
      case qIntL:
      case qNumL:
      case qText:
      case qChar:
      case qMacB:
      case qMacr:
      case qNote:
        return lexer->get_string(a->v.str_idx) < lexer->get_string(b->v.str_idx);
    }
  } catch (std::out_of_range &) {
    return false;
  } catch (...) {
    qcore_panic("qlex_lt: invalid token");
  }

  __builtin_unreachable();
}

LIB_EXPORT const char *qlex_str(qlex_t *lexer, qlex_tok_t *tok, size_t *len) {
  size_t _len;
  if (!len) len = &_len;

  try {
    switch (tok->ty) {
      case qEofF:
      case qErro:
      case qKeyW:
      case qOper:
      case qPunc:
        *len = 0;
        return "";
      case qName:
      case qIntL:
      case qNumL:
      case qText:
      case qChar:
      case qMacB:
      case qMacr:
      case qNote:
        auto x = lexer->get_string(tok->v.str_idx);
        *len = x.size();
        return x.data();
    }
  } catch (std::out_of_range &) {
    *len = 0;
    return "";
  } catch (...) {
    qcore_panic("qlex_str: invalid token");
  }

  __builtin_unreachable();
}

LIB_EXPORT const char *qlex_opstr(qlex_op_t op) {
  try {
    return qlex::operators.right.at(op).data();
  } catch (...) {
    qcore_panic("qlex_opstr: invalid operator");
  }
}

LIB_EXPORT const char *qlex_kwstr(qlex_key_t kw) {
  try {
    return qlex::keywords.right.at(kw).data();
  } catch (...) {
    qcore_panic("qlex_kwstr: invalid keyword");
  }
}

LIB_EXPORT const char *qlex_punctstr(qlex_punc_t punct) {
  try {
    return qlex::punctuation.right.at(punct).data();
  } catch (...) {
    qcore_panic("qlex_punctstr: invalid punctuation");
  }
}

LIB_EXPORT void qlex_tok_fromstr(qlex_t *lexer, qlex_ty_t ty, const char *str, qlex_tok_t *out) {
  try {
    out->ty = ty;

    out->start.tag = 0;
    out->start.tag = 0;

    out->v.str_idx = lexer->put_string(str);
  } catch (std::bad_alloc &) {
    qcore_panic("qlex_tok_fromstr: failed to create token: out of memory");
  } catch (...) {
    qcore_panic("qlex_tok_fromstr: failed to create token");
  }
}
