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
#include <bit>
#include <boost/bimap.hpp>
#include <cctype>
#include <charconv>
#include <cmath>
#include <cstdio>
#include <iomanip>
#include <queue>
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
          {"volatile", qKVolatile},
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

  inline qlex_size retain(std::string_view str) {
    uint8_t bin_num = size_to_bignum(str.size());

    auto it = std::find(m_strings[bin_num].begin(), m_strings[bin_num].end(), str);

    if (it == m_strings[bin_num].end()) {
      m_strings[bin_num].push_back(std::string(str));
      return CONV_ID(bin_num, m_strings[bin_num].size() - 1);
    }

    return CONV_ID(bin_num, std::distance(m_strings[bin_num].begin(), it));
  }

  inline void release(qlex_size idx) {
    m_strings.at(BIN_ID(idx)).at(INDEX_ID(idx)).clear();
    m_strings.at(BIN_ID(idx)).at(INDEX_ID(idx)).shrink_to_fit();
  }

  inline std::string_view operator[](qlex_size idx) const {
    return m_strings.at(BIN_ID(idx)).at(INDEX_ID(idx));
  }
};
#else
class StringRetainer {
  std::vector<std::string> m_strings;

public:
  StringRetainer() = default;

  inline qlex_size retain(std::string_view str) {
    m_strings.push_back(std::string(str.data(), str.size()));
    return m_strings.size() - 1;
  }

  inline void release(qlex_size idx) { m_strings[idx].clear(); }

  inline std::string_view at(qlex_size idx) const { return m_strings.at(idx); }
};
#endif

struct clever_me_t {
  uint32_t rc_fmt : 1; /* Holds the row-column format? */
  uint32_t col : 10;   /* Column number (max 1024) */
  uint32_t row : 21;   /* Row number (max 2097152) */
} __attribute__((packed)) bits;

class qlex_impl_t final {
  StringRetainer m_holdings;

  std::array<qlex_tok_t, TOKEN_BUF_SIZE> m_tokens;
  size_t m_tok_pos;
  std::queue<qlex_tok_t> m_undo;
  std::unordered_map<qlex_size, clever_me_t> m_tag_to_loc;
  std::unordered_map<qlex_size, qlex_size> m_tag_to_off;

  std::deque<char> m_pushback;
  std::array<char, GETC_BUFFER_SIZE> m_buf;
  size_t m_bufpos;
  qlex_size m_row;
  qlex_size m_col;
  qlex_size m_offset;
  qlex_size m_locctr;
  FILE *m_file;

  bool m_is_owned;

  class GetCControlFlow {
  public:
    GetCControlFlow() = default;
  };

  void refill_buffer();
  qlex_tok_t do_automata() noexcept;
  void reset_state();

  char getc();

  qlex_loc_t loc() {
    clever_me_t bits;
    static_assert(sizeof(bits) == sizeof(qlex_size));

    if (m_row <= 2097152 || m_col <= 1024) {
      bits.rc_fmt = 1;
      bits.col = m_col;
      bits.row = m_row;

    } else {
      bits.rc_fmt = 0;
    }

    qlex_size tag = m_locctr++;
    m_tag_to_loc[tag] = bits;
    m_tag_to_off[tag] = m_offset;

    return {tag};
  }

public:
  qlex_impl_t(FILE *file, bool is_owned)
      : m_holdings(),
        m_tokens(),
        m_tok_pos(TOKEN_BUF_SIZE + 1),
        m_buf(),
        m_bufpos(GETC_BUFFER_SIZE + 1),
        m_row(1),
        m_col(1),
        m_offset(0),
        m_locctr(1),  // 0 means invalid location
        m_file(file),
        m_is_owned(is_owned) {
    if (fseek(file, 0, SEEK_SET) != 0) {
      qcore_panic("qlex_impl_t::LexerState: failed to seek to start of file");
    }
  }

  ~qlex_impl_t() {
    qcore_assert(m_file, "qlex_impl_t::~LexerState: file is NULL. Double free?");

    if (m_is_owned) {
      fclose(m_file);
    }
  }

  const StringRetainer &Strings() const { return m_holdings; }

  qlex_tok_t next();

  void undo(qlex_tok_t tok) { m_undo.push(tok); }
  qlex_size save_userstring(std::string_view str) { return m_holdings.retain(str); }

  std::optional<qlex_size> loc2offset(qlex_loc_t loc) {
    if (!m_tag_to_off.contains(loc.tag)) [[unlikely]] {
      return std::nullopt;
    }

    return m_tag_to_off[loc.tag];
  }

  std::optional<std::pair<qlex_size, qlex_size>> loc2rowcol(qlex_loc_t loc) {
    if (!m_tag_to_loc.contains(loc.tag)) [[unlikely]] {
      return std::nullopt;
    }

    clever_me_t it = m_tag_to_loc[loc.tag];

    if (!it.rc_fmt) [[unlikely]] {
      return std::nullopt;
    }

    qlex_size row = it.row;
    qlex_size col = it.col;

    return std::make_pair(row, col);
  }

  qlex_loc_t makeLoc(qlex_size row, qlex_size col, qlex_size offset) {
    clever_me_t bits;
    static_assert(sizeof(bits) == sizeof(qlex_size));

    if (row <= 2097152 || col <= 1024) {
      bits.rc_fmt = 1;
      bits.col = col;
      bits.row = row;

    } else {
      bits.rc_fmt = 0;
    }

    qlex_size tag = m_locctr++;
    m_tag_to_loc[tag] = bits;
    m_tag_to_off[tag] = offset;

    return {tag};
  }

  FILE *file() { return m_file; }
};

///============================================================================///

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

static void _impl_push(qlex_t *self, const qlex_tok_t *tok) {
  self->impl->undo(*tok);
  self->cur.ty = qErro;
}

static void _impl_collect(qlex_t *self, const qlex_tok_t *tok) {
  (void)self;
  (void)tok;
}

static void _impl_destruct(qlex_t *self) {
  delete self->impl;
  self->impl = nullptr;
}

///============================================================================///

LIB_EXPORT qlex_t *qlex_new(FILE *file, const char *filename) {
  try {
    qcore_assert(file, "qlex_new: file is NULL");

    if (!filename) {
      filename = "<unknown>";
    }

    qlex_t *lexer = new qlex_t;
    lexer->impl = new qlex_impl_t(file, false);
    lexer->next = _impl_next;
    lexer->peek = _impl_peek;
    lexer->push = _impl_push;
    lexer->collect = _impl_collect;
    lexer->destruct = _impl_destruct;
    lexer->cur.ty = qErro;
    lexer->flags = QLEX_FLAG_NONE;
    lexer->filename = filename;

    return lexer;
  } catch (std::bad_alloc &) {
    return nullptr;
  } catch (...) {
    qcore_panic("qlex_new: failed to create lexer");
  }
}

LIB_EXPORT qlex_t *qlex_istream__libextra(qlex_cxx_std_istream_t istream, const char *filename) {
  qcore_implement(__func__);
  /// TODO: Implement this function
  (void)istream;
  (void)filename;
}

LIB_EXPORT qlex_t *qlex_direct(const char *src, size_t len, const char *filename) {
  try {
    if (!filename) {
      filename = "<unknown>";
    }

    FILE *file = fmemopen((void *)src, len, "r");
    if (!file) {
      return nullptr;
    }

    qlex_t *lexer = new qlex_t;
    lexer->impl = new qlex_impl_t(file, true);
    lexer->next = _impl_next;
    lexer->peek = _impl_peek;
    lexer->push = _impl_push;
    lexer->collect = _impl_collect;
    lexer->destruct = _impl_destruct;
    lexer->cur.ty = qErro;
    lexer->flags = QLEX_FLAG_NONE;
    lexer->filename = filename;

    return lexer;
  } catch (std::bad_alloc &) {
    return nullptr;
  } catch (...) {
    return nullptr;
  }
}

LIB_EXPORT void qlex_free(qlex_t *lexer) {
  try {
    if (!lexer) {
      return;
    }

    lexer->destruct(lexer);

    delete lexer;
  } catch (...) {
    qcore_panic("qlex_free: failed to free lexer");
  }
}

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
        if ((ret = lexer->impl->Strings().at(tok->v.str_idx).size()) <= size) {
          memcpy(buf, lexer->impl->Strings().at(tok->v.str_idx).data(), ret);
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
      case qNote:
        return lexer->impl->Strings().at(a->v.str_idx) == lexer->impl->Strings().at(b->v.str_idx);
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
        return lexer->impl->Strings().at(a->v.str_idx) < lexer->impl->Strings().at(b->v.str_idx);
    }
  } catch (std::out_of_range &) {
    return false;
  } catch (...) {
    qcore_panic("qlex_lt: invalid token");
  }

  __builtin_unreachable();
}

LIB_EXPORT const char *qlex_str(qlex_t *lexer, qlex_tok_t *tok, size_t *len) {
  qcore_assert(tok && len, "qlex_str: tok or len is NULL");

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
        *len = lexer->impl->Strings().at(tok->v.str_idx).size();
        return lexer->impl->Strings().at(tok->v.str_idx).data();
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

LIB_EXPORT void qlex_tok_fromstr(qlex_t *lexer, qlex_ty_t ty, const char *str, qlex_size src_idx,
                                 qlex_tok_t *out) {
  try {
    if (src_idx != 0) {
      /// TODO:
      qcore_panic("qlex_tok_fromstr: source index is not supported");
    }

    out->ty = ty;

    out->start.tag = 0;
    out->start.tag = 0;

    out->v.str_idx = lexer->impl->save_userstring(str);
  } catch (std::bad_alloc &) {
    qcore_panic("qlex_tok_fromstr: failed to create token: out of memory");
  } catch (...) {
    qcore_panic("qlex_tok_fromstr: failed to create token");
  }
}

LIB_EXPORT const char *qlex_filename(qlex_t *lexer) { return lexer->filename; }

LIB_EXPORT qlex_size qlex_line(qlex_t *lexer, qlex_loc_t loc) {
  try {
    auto r = lexer->impl->loc2rowcol(loc);
    if (!r) {
      return UINT32_MAX;
    }

    return r->first;
  } catch (...) {
    qcore_panic("qlex_line: failed to get line number");
  }
}

LIB_EXPORT qlex_size qlex_col(qlex_t *lexer, qlex_loc_t loc) {
  try {
    auto r = lexer->impl->loc2rowcol(loc);
    if (!r) {
      return UINT32_MAX;
    }

    return r->second;
  } catch (...) {
    qcore_panic("qlex_col: failed to get column number");
  }
}

LIB_EXPORT char *qlex_snippet(qlex_t *lexer, qlex_tok_t tok, qlex_size *offset) {
  try {
#define SNIPPET_SIZE 100

    qlex_size tok_beg_offset;
    char snippet_buf[SNIPPET_SIZE];
    qlex_size tok_size = qlex_tok_size(lexer, &tok);
    size_t curpos, seek_base_pos, read;

    { /* Convert the location to an offset into the source */
      auto src_offset_opt = lexer->impl->loc2offset(tok.start);
      if (!src_offset_opt) {
        return nullptr; /* Return early if translation failed */
      }

      tok_beg_offset = *src_offset_opt - tok_size - 1;
    }

    { /* Calculate offsets and seek to the correct position */
      curpos = ftell(lexer->impl->file());
      seek_base_pos = tok_beg_offset < SNIPPET_SIZE / 2 ? 0 : tok_beg_offset - SNIPPET_SIZE / 2;

      if (fseek(lexer->impl->file(), seek_base_pos, SEEK_SET) != 0) {
        fseek(lexer->impl->file(), curpos, SEEK_SET);
        return nullptr;
      }
    }

    { /* Read the snippet and calculate token offset */
      read = fread(snippet_buf, 1, SNIPPET_SIZE, lexer->impl->file());

      if (tok_beg_offset < SNIPPET_SIZE / 2) {
        *offset = tok_beg_offset;
      } else {
        *offset = SNIPPET_SIZE / 2;
      }
    }

    // Extract the line that contains the token
    qlex_size lstart = 0;

    for (size_t i = 0; i < read; i++) {
      if (snippet_buf[i] == '\n') {
        lstart = i + 1;
      } else if (i == *offset) { /* Danger ?? */
        qlex_size count = (i - lstart) + tok_size;
        char *output = (char *)malloc(count + 1);
        memcpy(output, snippet_buf + lstart, count);
        output[count] = '\0';
        *offset -= lstart;
        fseek(lexer->impl->file(), curpos, SEEK_SET);
        return output;
      }
    }

    fseek(lexer->impl->file(), curpos, SEEK_SET);
    return nullptr;
  } catch (std::bad_alloc &) {
    return nullptr;
  } catch (...) {
    qcore_panic("qlex_snippet: failed to get snippet");
  }
}

LIB_EXPORT qlex_loc_t qlex_offset(qlex_t *lexer, qlex_loc_t base, qlex_size offset) {
  try {
    long curpos;
    std::optional<qlex_size> seek_base_pos;
    uint8_t *buf = nullptr;
    size_t bufsz;

    if (!(seek_base_pos = lexer->impl->loc2offset(base))) {
      return base;
    }

    if ((curpos = ftell(lexer->impl->file())) == -1) {
      return base;
    }

    if (fseek(lexer->impl->file(), *seek_base_pos + offset, SEEK_SET) != 0) {
      return base;
    }

    bufsz = offset;

    if ((buf = (uint8_t *)malloc(bufsz + 1)) == nullptr) {
      fseek(lexer->impl->file(), curpos, SEEK_SET);
      return base;
    }

    if (fread(buf, 1, bufsz, lexer->impl->file()) != bufsz) {
      free(buf);
      fseek(lexer->impl->file(), curpos, SEEK_SET);
      return base;
    }

    buf[bufsz] = '\0';
    fseek(lexer->impl->file(), curpos, SEEK_SET);

    //===== AUTOMATA TO CALCULATE THE NEW ROW AND COLUMN =====//
    uint32_t row, col;

    if ((row = qlex_line(lexer, base)) == UINT32_MAX) {
      free(buf);
      return base;
    }

    if ((col = qlex_col(lexer, base)) == UINT32_MAX) {
      free(buf);
      return base;
    }

    for (size_t i = 0; i < bufsz; i++) {
      if (buf[i] == '\n') {
        row++;
        col = 1;
      } else {
        col++;
      }
    }

    free(buf);

    return lexer->impl->makeLoc(row, col, *seek_base_pos + offset);
  } catch (...) {
    qcore_panic("qlex_offset: failed to calculate offset");
  }
}

LIB_EXPORT qlex_size qlex_span(qlex_t *lexer, qlex_loc_t start, qlex_loc_t end) {
  try {
    std::optional<qlex_size> begoff, endoff;

    if (!(begoff = lexer->impl->loc2offset(start))) {
      return UINT32_MAX;
    }

    if (!(endoff = lexer->impl->loc2offset(end))) {
      return UINT32_MAX;
    }

    return *endoff - *begoff;
  } catch (...) {
    qcore_panic("qlex_span: failed to calculate span");
  }
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
    qlex_tok_t tok = m_undo.front();
    m_undo.pop();
    return tok;
  }

  if (m_tok_pos >= TOKEN_BUF_SIZE) [[unlikely]] {
    refill_buffer();
    m_tok_pos = 0;
  }

  return m_tokens[m_tok_pos++];
}

void qlex_impl_t::refill_buffer() {
  for (size_t i = 0; i < TOKEN_BUF_SIZE; i++) {
    m_tokens[i] = do_automata();

    if (m_tokens[i].ty == qEofF) [[unlikely]] {
      qlex_tok_t eof{};
      eof.ty = qEofF;

      if (i > 0) {
        eof.start = m_tokens[i - 1].start;
        eof.end = m_tokens[i - 1].end;
      }

      std::fill(m_tokens.begin() + i, m_tokens.end(), eof);
      break;
    }
  }

  /* Must clear to prevent leaks */
  if (num_cache.size() > 1000) {
    num_cache.clear();
  }

  if (can_cache.size() > 1000) {
    can_cache.clear();
  }
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
        } else {
          c = m_pushback.front();
          m_pushback.pop_front();
        }
      }

      switch (state) {
        case LexState::Start: {
          if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
            continue;
          } else if (std::isalpha(c) || c == '_') {
            /* Identifier or keyword or operator */
            start_pos = loc();
            buf += c, state = LexState::Identifier;
            continue;
          } else if (c == '/') {
            start_pos = loc();
            state = LexState::CommentStart; /* Comment or operator */
            continue;
          } else if (std::isdigit(c)) {
            start_pos = loc();
            buf += c, state = LexState::Integer;
            continue;
          } else if (c == '"' || c == '\'') {
            start_pos = loc();
            buf += c, state = LexState::String;
            continue;
          } else if (c == '@') {
            start_pos = loc();
            state = LexState::MacroStart;
            continue;
          } else {
            /* Operator or punctor or invalid */
            start_pos = loc();
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
            return qlex_tok_t(qKeyW, qKFString, start_pos, loc());
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
              return qlex_tok_t(qKeyW, right, start_pos, loc());
            }
          }

          /* Check if it's an operator */
          for (const auto &[left, right] : qlex::word_operators) {
            if (ibuf == left) {
              return qlex_tok_t(qOper, right, start_pos, loc());
            }
          }

          /* Ensure it's a valid identifier */
          if (!validate_identifier(ibuf)) {
            goto error_0;
          }

          /* Return the identifier */
          return qlex_tok_t(qName, m_holdings.retain(ibuf), start_pos, loc());
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
              return qlex_tok_t(qNumL, m_holdings.retain(std::move(norm)), start_pos, loc());
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
            return qlex_tok_t(qIntL, m_holdings.retain(std::move(norm)), start_pos, loc());
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
            return qlex_tok_t(qOper, qOpSlash, start_pos, loc());
          }
        }
        case LexState::CommentSingleLine: {
          while (c != '\n') {
            buf += c;
            c = getc();
          }

          return qlex_tok_t(qNote, m_holdings.retain(std::move(buf)), start_pos, loc());
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
                  return qlex_tok_t(qNote, m_holdings.retain(std::move(buf)), start_pos, loc());
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
                return qlex_tok_t(qChar, m_holdings.retain(std::string(1, buf[1])), start_pos,
                                  loc());
              } else {
                return qlex_tok_t(qText, m_holdings.retain(buf.substr(1, buf.size() - 1)),
                                  start_pos, loc());
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
              ... @macro_name(arg1, arg2, arg3, ...) ...
          */

          while (true) {
            if (c == '(') {
              state_parens++;
            } else if (c == ')') {
              state_parens--;

              if (state_parens == 0) {
                buf += ')';
                return qlex_tok_t(qMacr, m_holdings.retain(std::move(buf)), start_pos, loc());
              }
            }

            if (c == '\n') {
              return qlex_tok_t(qMacr, m_holdings.retain(std::move(buf)), start_pos, loc());
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
              return qlex_tok_t(qMacB, m_holdings.retain(std::move(buf)), start_pos, loc());
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
                return qlex_tok_t(qPunc, right, start_pos, loc());
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
          return qlex_tok_t(qOper, qlex::operators.left.at(buf.substr(0, buf.size() - 1)),
                            start_pos, loc());
        }
      }
    }
    goto error_0;
  } catch (GetCControlFlow &) { /* This is just faster than checking for EOF everywhere */
    if (eof_is_error) {
      goto error_0;
    } else {
      reset_state();
      return qlex_tok_t::eof(loc(), loc());
    }
  } catch (std::exception &e) { /* This should never happen */
    qcore_panicf("qlex_impl_t::do_automata: %s. The lexer has a bug.", e.what());
  } catch (...) { /* This should never happen */
    qcore_panic("qlex_impl_t::do_automata: unknown error. The lexer has a bug.");
  }

error_0: { /* Reset the lexer and return error token */
  reset_state();

  return qlex_tok_t::err(loc(), loc());
}
}
