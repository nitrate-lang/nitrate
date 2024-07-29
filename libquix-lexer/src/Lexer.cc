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

#include <quix-core/Arena.h>
#include <quix-core/Error.h>
#include <quix-lexer/Lexer.h>
#include <string.h>

#include <array>
#include <boost/bimap.hpp>
#include <cassert>
#include <cctype>
#include <charconv>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <functional>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <regex>
#include <stdexcept>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "LibMacro.h"

///============================================================================///
/// BEGIN: LEXICAL GRAMMAR CONSTRAINTS
#define MAX_NUMBER_SIZE 128
#define MAX_IDENTIFIER_SIZE 2048
#define FLOATING_POINT_PRECISION 128
/// END:   LEXICAL GRAMMAR CONSTRAINTS
///============================================================================///

///============================================================================///
/// BEGIN: PERFORMANCE HYPER PARAMETERS
#define GETC_BUFFER_SIZE 2048
#define TOKEN_BUF_SIZE 1024
/// END:   PERFORMANCE HYPER PARAMETERS
///============================================================================///

namespace qlex {
  template <typename L, typename R>
  boost::bimap<L, R> make_bimap(
      std::initializer_list<typename boost::bimap<L, R>::value_type> list) {
    return boost::bimap<L, R>(list.begin(), list.end());
  }

  ///============================================================================///
  /// BEGIN: LEXER LOOKUP TABLES
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
          {"union", qKUnion},
          {"opaque", qKOpaque},
          {"enum", qKEnum},
          {"fstring", qKFString},
          {"impl", qKImpl},
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
          {"req", qKReq},
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
          {"?:", qOpTernary},
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
          {"<=>", qOpSpaceship},
          {"bitcast_as", qOpBitcastAs},
          {"reinterpret_as", qOpReinterpretAs},
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
          {"reinterpret_as", qOpReinterpretAs},
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

  static thread_local qcore_arena_t g_lexer_arena;

  ///============================================================================///
  /// BEGIN: LEXER INTERNAL TYPES

  template <class T>
  struct Arena {
    typedef T value_type;

    Arena() = default;

    template <class U>
    constexpr Arena(const Arena<U> &) noexcept {}

    [[nodiscard]] T *allocate(std::size_t n) {
      return (T *)qcore_arena_alloc(&g_lexer_arena, n * sizeof(T));
    }

    void deallocate(T *p, std::size_t n) noexcept {
      (void)p;
      (void)n;
    }
  };

  template <class T, class U>
  bool operator==(const Arena<T> &, const Arena<U> &) {
    return true;
  }
  template <class T, class U>
  bool operator!=(const Arena<T> &, const Arena<U> &) {
    return false;
  }

  typedef std::basic_string<char, std::char_traits<char>, Arena<char>> ident_buf_t;
  typedef std::basic_string<char, std::char_traits<char>, Arena<char>> num_buf_t;
  /// END:   LEXER INTERNAL TYPES
  ///============================================================================///

}  // namespace qlex

namespace std {
  template <>
  struct hash<qlex::ident_buf_t> {
    std::size_t operator()(const qlex::ident_buf_t &str) const noexcept {
      return std::hash<std::string_view>()(str);
    }
  };
}  // namespace std

enum class NumType {
  Invalid,
  Decimal,
  DecimalExplicit,
  Hexadecimal,
  Binary,
  Octal,
  Floating,
};

static thread_local std::unordered_map<qlex::num_buf_t, NumType> num_cache;
static thread_local std::unordered_map<qlex::num_buf_t, qlex::num_buf_t> can_cache;

#if MEMORY_OVER_SPEED
class StringRetainer {
  std::unordered_map<uint8_t, std::vector<std::string>> m_strings;

#define BIN_ID(x) (x & 0x7)
#define INDEX_ID(x) (x >> 3)

#define CONV_ID(bin, idx) (((idx) << 3) | (bin))

  uint8_t size_to_bignum(size_t size) {
    if (size <= 2) return 0;
    if (size <= 4) return 1;
    if (size <= 6) return 2;
    if (size <= 8) return 3;
    if (size <= 10) return 4;
    if (size <= 16) return 5;
    if (size <= 128) return 6;
    return 7;
  }

public:
  StringRetainer() {
    for (uint8_t i = 0; i < 8; i++) {
      m_strings[i] = std::vector<std::string>();
    }
  }

  inline uint32_t retain(std::string_view str) {
    uint8_t bin_num = size_to_bignum(str.size());

    auto it = std::find(m_strings[bin_num].begin(), m_strings[bin_num].end(), str);

    if (it == m_strings[bin_num].end()) {
      m_strings[bin_num].push_back(std::string(str));
      return CONV_ID(bin_num, m_strings[bin_num].size() - 1);
    }

    return CONV_ID(bin_num, std::distance(m_strings[bin_num].begin(), it));
  }

  inline void release(uint32_t idx) {
    m_strings.at(BIN_ID(idx)).at(INDEX_ID(idx)).clear();
    m_strings.at(BIN_ID(idx)).at(INDEX_ID(idx)).shrink_to_fit();
  }

  inline std::string_view operator[](uint32_t idx) const {
    return m_strings.at(BIN_ID(idx)).at(INDEX_ID(idx));
  }
};
#else
class StringRetainer {
  std::vector<std::string> m_strings;

public:
  StringRetainer() = default;

  inline uint32_t retain(std::string_view str) {
    m_strings.push_back(std::string(str));
    return m_strings.size() - 1;
  }

  inline void release(uint32_t idx) { m_strings[idx].clear(); }

  inline std::string_view operator[](uint32_t idx) const { return m_strings[idx]; }
};
#endif

class qlex_impl_t final {
  StringRetainer m_holdings;

  std::array<qlex_tok_t, TOKEN_BUF_SIZE> m_tokens;
  size_t m_tok_pos;
  std::vector<qlex_tok_t> m_undo;

  std::deque<char> m_pushback;
  std::array<char, GETC_BUFFER_SIZE> m_buf;
  size_t m_bufpos;
  uint32_t m_row;
  uint32_t m_col;
  uint32_t m_offset;
  FILE *m_file;

  class GetCControlFlow {
  public:
    GetCControlFlow() = default;
  };

  void refill_buffer();
  qlex_tok_t do_automata() noexcept;
  void reset_state();

  char getc();

  inline uint32_t off() const { return m_offset; }
  inline uint32_t row() const { return m_row; }
  inline uint32_t col() const { return m_col; }

public:
  qlex_impl_t(FILE *file)
      : m_holdings(),
        m_tokens(),
        m_tok_pos(TOKEN_BUF_SIZE + 1),
        m_buf(),
        m_bufpos(GETC_BUFFER_SIZE + 1),
        m_row(1),
        m_col(1),
        m_offset(0),
        m_file(file) {
    if (fseek(file, 0, SEEK_SET) != 0) {
      qcore_panic("qlex_impl_t::LexerState: failed to seek to start of file");
    }
  }

  inline StringRetainer &Strings() { return m_holdings; }
  inline const StringRetainer &Strings() const { return m_holdings; }

  qlex_tok_t next();

  void undo(qlex_tok_t tok) { m_undo.push_back(tok); }
  uint32_t save_userstring(std::string_view str) { return m_holdings.retain(str); }
};

static qlex_tok_t _impl_next(qlex_t *self) {
  qlex_tok_t tok = qlex_peek(self);
  self->cur.ty = qErro;
  return tok;
}

static qlex_tok_t _impl_peek(qlex_t *self) {
  if (self->cur.ty != qErro) {
    return self->cur;
  }

  self->cur = self->impl->next();

  if (self->flags & QLEX_NO_COMMENTS) {
    while (self->cur.ty == qNote) {
      self->cur = self->impl->next();
    }
  }

  return self->cur;
}

static void _impl_push(qlex_t *self, const qlex_tok_t *tok) { self->impl->undo(*tok); }

static void _impl_collect(qlex_t *self, const qlex_tok_t *tok) {
  (void)self;
  (void)tok;
}

static void _impl_destruct(qlex_t *self) {
  delete self->impl;
  self->impl = nullptr;
}

LIB_EXPORT qlex_t *qlex_new(FILE *file) {
  qlex_t *lexer = new qlex_t;
  lexer->impl = new qlex_impl_t(file);
  lexer->next = _impl_next;
  lexer->peek = _impl_peek;
  lexer->push = _impl_push;
  lexer->collect = _impl_collect;
  lexer->destruct = _impl_destruct;
  lexer->cur.ty = qErro;
  lexer->flags = QLEX_FLAG_NONE;
  return lexer;
}

LIB_EXPORT void qlex_free(qlex_t *lexer) {
  if (!lexer) return;

  lexer->destruct(lexer);

  delete lexer;
}

LIB_EXPORT uint32_t qlex_tok_size(qlex_t *lexer, const qlex_tok_t *tok) {
  uint32_t ret;

  switch (tok->ty) {
    case qEofF:
    case qErro:
      ret = 0;
      break;
    case qKeyW:
      ret = qlex::keywords.right.at(tok->v.key).size();
      break;
    case qOper:
      ret = qlex::operators.right.at(tok->v.op).size();
      break;
    case qPunc:
      ret = qlex::punctuation.right.at(tok->v.punc).size();
      break;
    case qName:
    case qIntL:
    case qNumL:
    case qText:
    case qChar:
    case qMacB:
    case qMacr:
    case qNote: {
      try {
        ret = lexer->impl->Strings()[tok->v.str_idx].size();
      } catch (...) {
        ret = 0;
      }
      break;
    }
  }

  return ret;
}

LIB_EXPORT uint32_t qlex_tok_write(qlex_t *lexer, const qlex_tok_t *tok, char *buf, uint32_t size) {
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
      try {
        if ((ret = lexer->impl->Strings()[tok->v.str_idx].size()) <= size) {
          memcpy(buf, lexer->impl->Strings()[tok->v.str_idx].data(), ret);
        } else {
          ret = 0;
        }
      } catch (...) {
        ret = 0;
      }
      break;
    }
  }

  return ret;
}

LIB_EXPORT const char *qlex_ty_str(qlex_ty_t ty) {
  switch (ty) {
    case qEofF:
      return "EOF";
    case qErro:
      return "ERROR";
    case qKeyW:
      return "KEYWORD";
    case qOper:
      return "OPERATOR";
    case qPunc:
      return "PUNCTUATION";
    case qName:
      return "IDENTIFIER";
    case qIntL:
      return "INTEGER";
    case qNumL:
      return "NUMBER";
    case qText:
      return "STRING";
    case qChar:
      return "CHARACTER";
    case qMacB:
      return "MACRO_BLOCK";
    case qMacr:
      return "MACRO";
    case qNote:
      return "COMMENT";
  }
}

LIB_EXPORT bool qlex_eq(qlex_t *lexer, const qlex_tok_t *a, const qlex_tok_t *b) {
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
    case qNote:
      return lexer->impl->Strings()[a->v.str_idx] == lexer->impl->Strings()[b->v.str_idx];
  }
}

LIB_EXPORT bool qlex_lt(qlex_t *lexer, const qlex_tok_t *a, const qlex_tok_t *b) {
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
      return lexer->impl->Strings()[a->v.str_idx] < lexer->impl->Strings()[b->v.str_idx];
  }
}

LIB_EXPORT const char *qlex_str(qlex_t *lexer, qlex_tok_t *tok) {
  switch (tok->ty) {
    case qEofF:
    case qErro:
    case qKeyW:
    case qOper:
    case qPunc:
      return "";
    case qName:
    case qIntL:
    case qNumL:
    case qText:
    case qChar:
    case qMacB:
    case qMacr:
    case qNote:
      return lexer->impl->Strings()[tok->v.str_idx].data();
  }
}

LIB_EXPORT const char *qlex_opstr(qlex_op_t op) { return qlex::operators.right.at(op).data(); }

LIB_EXPORT const char *qlex_kwstr(qlex_key_t kw) { return qlex::keywords.right.at(kw).data(); }

LIB_EXPORT const char *qlex_punctstr(qlex_punc_t punct) {
  return qlex::punctuation.right.at(punct).data();
}

LIB_EXPORT void qlex_tok_fromstr(qlex_t *lexer, qlex_ty_t ty, const char *str, uint32_t src_idx,
                                 qlex_tok_t *out) {
  out->ty = ty;
  out->loc.idx = src_idx;
  out->v.str_idx = lexer->impl->save_userstring(str);
}

///============================================================================///

char qlex_impl_t::getc() {
  size_t read;

  /* Refill the buffer if necessary */
  if (m_bufpos >= GETC_BUFFER_SIZE) [[unlikely]] {
    read = fread(m_buf.data(), 1, GETC_BUFFER_SIZE, m_file);

    if (read == 0) [[unlikely]] {
      throw GetCControlFlow();
    }

    memset(m_buf.data() + read, '\n', GETC_BUFFER_SIZE - read);
    m_bufpos = 0;
  }

  char c = m_buf[m_bufpos++];

  /* Update the row and column */
  if (c == '\n') {
    m_row++;
    m_col = 1;
  } else {
    m_col++;
  }

  m_offset++;

  return c;
}

qlex_tok_t qlex_impl_t::next() {
  if (!m_undo.empty()) {
    qlex_tok_t tok = m_undo.back();
    m_undo.pop_back();
    return tok;
  }

  if (m_tok_pos >= TOKEN_BUF_SIZE) [[unlikely]] {
    refill_buffer();
    m_tok_pos = 0;
  }

  return m_tokens[m_tok_pos++];
}

void qlex_impl_t::refill_buffer() {
  qcore_arena_t my_arena;
  qcore_arena_open(&my_arena);

  std::swap(my_arena, qlex::g_lexer_arena);

  for (size_t i = 0; i < TOKEN_BUF_SIZE; i++) {
    m_tokens[i] = do_automata();

    if (m_tokens[i].ty == qEofF) [[unlikely]] {
      qlex_tok_t eof{};
      eof.ty = qEofF;

      std::fill(m_tokens.begin() + i, m_tokens.end(), eof);
      break;
    }
  }

  /* Must clear to prevent leaks */
  num_cache.clear();
  can_cache.clear();

  std::swap(my_arena, qlex::g_lexer_arena);

  qcore_arena_close(&my_arena);
}

static bool validate_identifier(std::string_view id) {
  /*
   * This state machine checks if the identifier looks
   * like 'a::b::c::d_::e::f'.
   */

  int state = 0;

  for (const auto &c : id) {
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
  if (num_cache.contains(input)) return num_cache[input];

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
        // static const auto regexpFloat =
        //     std::regex("^([0-9]+(\\.[0-9]+)?)?(e[+-]?([0-9]+(\\.?[0-9]+)?)+)*$");

        // if (std::regex_match(input.data(), regexpFloat)) {
        //   return num_cache[input] = NumType::Floating;
        // }
        try {
          // std::stod(std::string(input));
          // stod and check if it succeeds

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

  if ((e_pos = input.find('e')) == std::string::npos) return input.data();

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
  if (can_cache.contains(number)) {
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

void qlex_impl_t::reset_state() { m_pushback.clear(); }

qlex_tok_t qlex_impl_t::do_automata() noexcept {
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
  char c;
  bool eof_is_error = true;

  try {
    while (true) {
      { /* If the Lexer over-consumed, we will return the saved character */
        if (m_pushback.empty()) {
          eof_is_error = false;
          c = getc();
          eof_is_error = true;
        } else {
          c = m_pushback.front();
          m_pushback.pop_front();
        }
      }

      switch (state) {
        case LexState::Start: {
          if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
            continue;
          } else if (std::isalpha(c) || c == '_') /* Identifier or keyword or operator */
          {
            buf += c, state = LexState::Identifier;
            continue;
          } else if (c == '/') /* Comment or operator */
          {
            state = LexState::CommentStart;
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
          } else /* Operator or punctor or invalid */
          {
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
            return qlex_tok_t(qKeyW, qKFString, off());
          }

          /* We overshot; this must be a punctor ':' */
          if (ibuf.size() > 0 && ibuf.back() == ':') {
            char tc = ibuf.back();
            ibuf.pop_back();
            m_pushback.push_back(tc);
          }
          m_pushback.push_back(c);

          /* Determine if it's a keyword or an identifier */
          for (const auto &[left, right] : qlex::keywords) {
            if (ibuf == left) {
              return qlex_tok_t(qKeyW, right, off());
            }
          }

          /* Check if it's an operator */
          for (const auto &[left, right] : qlex::word_operators) {
            if (ibuf == left) {
              return qlex_tok_t(qOper, right, off());
            }
          }

          /* Ensure it's a valid identifier */
          if (!validate_identifier(ibuf)) {
            goto error_0;
          }

          /* Return the identifier */
          return qlex_tok_t(qName, m_holdings.retain(ibuf), off());
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
              return qlex_tok_t(qNumL, m_holdings.retain(std::move(norm)), off());
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
            return qlex_tok_t(qIntL, m_holdings.retain(std::move(norm)), off());
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
            return qlex_tok_t(qOper, qOpSlash, off());
          }
        }
        case LexState::CommentSingleLine: {
          while (c != '\n') {
            buf += c;
            c = getc();
          }

          return qlex_tok_t(qNote, m_holdings.retain(std::move(buf)), off());
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
                  return qlex_tok_t(qNote, m_holdings.retain(std::move(buf)), off());
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
                char hex[4] = {getc(), getc(), getc(), getc()};
                uint32_t codepoint = 0;
                codepoint |= qlex::hextable[(uint8_t)hex[0]] << 12;
                codepoint |= qlex::hextable[(uint8_t)hex[1]] << 8;
                codepoint |= qlex::hextable[(uint8_t)hex[2]] << 4;
                codepoint |= qlex::hextable[(uint8_t)hex[3]];
                buf += codepoint;
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
              default:
                buf += c;
                break;
            }
            continue;
          }

          /* Character or string */
          if (buf.front() == '\'' && buf.size() == 2) {
            return qlex_tok_t(qChar, m_holdings.retain(std::string(1, buf[1])), off());
          } else {
            return qlex_tok_t(qText, m_holdings.retain(buf.substr(1, buf.size() - 1)), off());
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
              ... @macro_name(arg1, arg2, arg3, ...) ...
          */

          while (true) {
            if (c == '(') {
              state_parens++;
            } else if (c == ')') {
              state_parens--;

              if (state_parens == 0) {
                buf += ')';
                return qlex_tok_t(qMacr, m_holdings.retain(std::move(buf)), off());
              }
            }

            if (c == '\n') {
              return qlex_tok_t(qMacr, m_holdings.retain(std::move(buf)), off());
            }

            buf += c;

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
              return qlex_tok_t(qMacB, m_holdings.retain(std::move(buf)), off());
            }

            buf += c;

            c = getc();
          }
          continue;
        }
        case LexState::Other: {
          /* Check if it's a punctor */
          if (buf.size() == 1) {
            for (const auto &[left, right] : qlex::punctuation) {
              if (left == buf) {
                m_pushback.push_back(c);
                return qlex_tok_t(qPunc, right, off());
              }
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
            buf += c;
            state = LexState::CommentSingleLine;
            continue;
          }

          bool found = false;
          while (true) {
            bool contains = false;
            for (const auto &[left, right] : qlex::operators) {
              if (left == buf) {
                contains = true;
                found = true;
                break;
              }
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
          return qlex_tok_t(qOper, qlex::operators.left.at(buf.substr(0, buf.size() - 1)), off());
        }
      }
    }
    goto error_0;
  } catch (GetCControlFlow &) { /* This is just faster than checking for EOF everywhere */
    if (eof_is_error) {
      goto error_0;
    } else {
      reset_state();
      return qlex_tok_t::eof(off());
    }
  } catch (std::exception &e) { /* This should never happen */
    qcore_panicf("qlex_impl_t::do_automata: %s. The lexer has a bug.", e.what());
  } catch (...) { /* This should never happen */
    qcore_panic("qlex_impl_t::do_automata: unknown error. The lexer has a bug.");
  }

error_0: { /* Reset the lexer and return error token */
  reset_state();

  return qlex_tok_t::err(off());
}
}
