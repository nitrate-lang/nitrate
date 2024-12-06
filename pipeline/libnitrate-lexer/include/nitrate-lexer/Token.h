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

#ifndef __NITRATE_LEXER_TOKEN_H__
#define __NITRATE_LEXER_TOKEN_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

typedef enum qlex_ty_t {
  qEofF = 1, /* End of file */
  qErro,     /* Error, invalid token */
  qKeyW,     /* Keyword */
  qOper,     /* Operator */
  qPunc,     /* Punctuation */
  qName,     /* Identifier */
  qIntL,     /* Integer literal */
  qNumL,     /* Floating-point literal */
  qText,     /* String literal */
  qChar,     /* Character literal */
  qMacB,     /* Macro block */
  qMacr,     /* Macro call */
  qNote,     /* Comment */
} __attribute__((packed)) qlex_ty_t;

typedef enum qlex_punc_t {
  qPuncLPar, /* Left parenthesis */
  qPuncRPar, /* Right parenthesis */
  qPuncLBrk, /* Left bracket */
  qPuncRBrk, /* Right bracket */
  qPuncLCur, /* Left curly brace */
  qPuncRCur, /* Right curly brace */
  qPuncComa, /* Comma */
  qPuncColn, /* Colon */
  qPuncSemi, /* Semicolon */
} __attribute__((packed)) qlex_punc_t;

typedef enum qlex_op_t {
  qOpPlus,        /* '+':    Addition operator */
  qOpMinus,       /* '-':    Subtraction operator */
  qOpTimes,       /* '*':    Multiplication operator */
  qOpSlash,       /* '/':    Division operator */
  qOpPercent,     /* '%':    Modulus operator */
  qOpBitAnd,      /* '&':    Bitwise AND operator */
  qOpBitOr,       /* '|':    Bitwise OR operator */
  qOpBitXor,      /* '^':    Bitwise XOR operator */
  qOpBitNot,      /* '~':    Bitwise NOT operator */
  qOpLShift,      /* '<<':   Left shift operator */
  qOpRShift,      /* '>>':   Right shift operator */
  qOpROTL,        /* '<<<':  Rotate left operator */
  qOpROTR,        /* '>>>':  Rotate right operator */
  qOpLogicAnd,    /* '&&':   Logical AND operator */
  qOpLogicOr,     /* '||':   Logical OR operator */
  qOpLogicXor,    /* '^^':   Logical XOR operator */
  qOpLogicNot,    /* '!':    Logical NOT operator */
  qOpLT,          /* '<':    Less than operator */
  qOpGT,          /* '>':    Greater than operator */
  qOpLE,          /* '<=':   Less than or equal to operator */
  qOpGE,          /* '>=':   Greater than or equal to operator */
  qOpEq,          /* '==':   Equal to operator */
  qOpNE,          /* '!=':   Not equal to operator */
  qOpSet,         /* '=':    Assignment operator */
  qOpPlusSet,     /* '+=':   Addition assignment operator */
  qOpMinusSet,    /* '-=':   Subtraction assignment operator */
  qOpTimesSet,    /* '*=':   Multiplication assignment operator */
  qOpSlashSet,    /* '/=':   Division assignment operator */
  qOpPercentSet,  /* '%=':   Modulus assignment operator */
  qOpBitAndSet,   /* '&=':   Bitwise AND assignment operator */
  qOpBitOrSet,    /* '|=':   Bitwise OR assignment operator */
  qOpBitXorSet,   /* '^=':   Bitwise XOR assignment operator */
  qOpLogicAndSet, /* '&&=':  Logical AND assignment operator */
  qOpLogicOrSet,  /* '||=':  Logical OR assignment operator */
  qOpLogicXorSet, /* '^^=':  Logical XOR assignment operator */
  qOpLShiftSet,   /* '<<=':  Left shift assignment operator */
  qOpRShiftSet,   /* '>>=':  Right shift assignment operator */
  qOpROTLSet,     /* '<<<=': Rotate left assignment operator */
  qOpROTRSet,     /* '>>>=': Rotate right assignment operator */
  qOpInc,         /* '++':   Increment operator */
  qOpDec,         /* '--':   Decrement operator */
  qOpAs,          /* 'as':   Type cast operator */
  qOpBitcastAs,   /* 'bitcast_as': Bitcast operator */
  qOpIn,          /* 'in':         Generic membership operator */
  qOpOut,         /* 'out':        Output operator */
  qOpSizeof,      /* 'sizeof':     Size of operator */
  qOpBitsizeof,   /* 'bitsizeof':  Bit size of operator */
  qOpAlignof,     /* 'alignof':    Alignment of operator */
  qOpTypeof,      /* 'typeof':     Type of operator */
  qOpDot,         /* '.':          Dot operator */
  qOpRange,       /* '..':         Range operator */
  qOpEllipsis,    /* '...':        Ellipsis operator */
  qOpArrow,       /* '=>':         Arrow operator */
  qOpTernary,     /* '?':          Ternary operator */
} __attribute__((packed)) qlex_op_t;

typedef enum qlex_key_t {
  qKScope,      /* 'scope' */
  qKImport,     /* 'import' */
  qKPub,        /* 'pub' */
  qKSec,        /* 'sec' */
  qKPro,        /* 'pro' */
  qKType,       /* 'type' */
  qKLet,        /* 'let' */
  qKVar,        /* 'var' */
  qKConst,      /* 'const' */
  qKStatic,     /* 'static' */
  qKStruct,     /* 'struct' */
  qKRegion,     /* 'region' */
  qKGroup,      /* 'group' */
  qKClass,      /* 'class' */
  qKUnion,      /* 'union' */
  qKOpaque,     /* 'opaque' */
  qKEnum,       /* 'enum' */
  qK__FString,  /* '__fstring' */
  qKFn,         /* 'fn' */
  qKNoexcept,   /* 'noexcept' */
  qKForeign,    /* 'foreign' */
  qKImpure,     /* 'impure' */
  qKTsafe,      /* 'tsafe' */
  qKPure,       /* 'pure' */
  qKQuasipure,  /* 'quasipure' */
  qKRetropure,  /* 'retropure' */
  qKCrashpoint, /* 'crashpoint' */
  qKInline,     /* 'inline' */
  qKUnsafe,     /* 'unsafe' */
  qKSafe,       /* 'safe' */
  qKPromise,    /* 'promise' */
  qKIf,         /* 'if' */
  qKElse,       /* 'else' */
  qKFor,        /* 'for' */
  qKWhile,      /* 'while' */
  qKDo,         /* 'do' */
  qKSwitch,     /* 'switch' */
  qKBreak,      /* 'break' */
  qKContinue,   /* 'continue' */
  qKReturn,     /* 'ret' */
  qKRetif,      /* 'retif' */
  qKForeach,    /* 'foreach' */
  qKTry,        /* 'try' */
  qKCatch,      /* 'catch' */
  qKThrow,      /* 'throw' */
  qK__Asm__,    /* '__asm__' */
  qKUndef,      /* 'undef' */
  qKNull,       /* 'null' */
  qKTrue,       /* 'true' */
  qKFalse,      /* 'false' */
} __attribute__((packed)) qlex_key_t;

struct qlex_t;
struct qlex_tok_t;

const char *qlex_str(struct qlex_t *lexer, const struct qlex_tok_t *tok,
                     size_t *len);

#ifdef __cplusplus
}
#endif

#if defined(__cplusplus) && defined(__NITRATE_LEXER_CPP__)

#include <string_view>
#include <type_traits>

#endif

typedef struct qlex_tok_t {
  /* Token type */
  qlex_ty_t ty : 4;

  uint32_t start, end;

  /* Token data */
  union {
    qlex_punc_t punc;
    qlex_op_t op;
    qlex_key_t key;
    uint32_t str_idx;
  } __attribute__((packed)) v;

  uint64_t pad : 4;

#if defined(__cplusplus) && defined(__NITRATE_LEXER_CPP__)

  constexpr qlex_tok_t() : ty(qErro), start(0), end(0), v{.str_idx = 0} {}

  constexpr qlex_tok_t(qlex_ty_t ty, qlex_punc_t punc, uint32_t loc_beg = 0,
                       uint32_t loc_end = 0)
      : ty(ty), start(loc_beg), end(loc_end), v{.punc = punc} {}

  constexpr qlex_tok_t(qlex_ty_t ty, qlex_op_t op, uint32_t loc_beg = 0,
                       uint32_t loc_end = 0)
      : ty(ty), start(loc_beg), end(loc_end), v{.op = op} {}

  constexpr qlex_tok_t(qlex_ty_t ty, qlex_key_t key, uint32_t loc_beg = 0,
                       uint32_t loc_end = 0)
      : ty(ty), start(loc_beg), end(loc_end), v{.key = key} {}

  constexpr qlex_tok_t(qlex_ty_t ty, uint32_t str_idx, uint32_t loc_beg = 0,
                       uint32_t loc_end = 0)
      : ty(ty), start(loc_beg), end(loc_end), v{.str_idx = str_idx} {}

  constexpr static qlex_tok_t err(uint32_t loc_start, uint32_t loc_end) {
    return qlex_tok_t(qErro, 0, loc_start, loc_end);
  }
  constexpr static qlex_tok_t eof(uint32_t loc_start, uint32_t loc_end) {
    return qlex_tok_t(qEofF, 0, loc_start, loc_end);
  }

  template <typename T>
  constexpr T as() const {
    if constexpr (std::is_same_v<T, qlex_punc_t>) {
      return v.punc;
    } else if constexpr (std::is_same_v<T, qlex_key_t>) {
      return v.key;
    } else if constexpr (std::is_same_v<T, qlex_op_t>) {
      return v.op;
    }

    static_assert(std::is_same_v<T, T>, "Invalid type");
  }

  constexpr bool is(qlex_ty_t val) const { return ty == val; }

  constexpr bool operator==(const qlex_tok_t &rhs) const {
    if (ty != rhs.ty) return false;
    switch (ty) {
      case qEofF:
      case qErro:
        return true;
      case qPunc:
        return v.punc == rhs.v.punc;
      case qOper:
        return v.op == rhs.v.op;
      case qKeyW:
        return v.key == rhs.v.key;
      case qIntL:
      case qNumL:
      case qText:
      case qName:
      case qChar:
      case qMacB:
      case qMacr:
      case qNote:
        return v.str_idx == rhs.v.str_idx;
      default:
        __builtin_unreachable();
    }
  }

  template <auto V>
  constexpr bool is() const {
    if constexpr (std::is_same_v<decltype(V), qlex_key_t>) {
      return ty == qKeyW && as<qlex_key_t>() == V;
    } else if constexpr (std::is_same_v<decltype(V), qlex_punc_t>) {
      return ty == qPunc && as<qlex_punc_t>() == V;
    } else if constexpr (std::is_same_v<decltype(V), qlex_op_t>) {
      return ty == qOper && as<qlex_op_t>() == V;
    }
  }

  inline std::string_view as_string(qlex_t *lexer) const {
    size_t len;
    const char *s = qlex_str(lexer, this, &len);

    return std::string_view(s, len);
  }

  constexpr bool operator<(const qlex_tok_t &rhs) const {
    if (ty != rhs.ty) return ty < rhs.ty;
    switch (ty) {
      case qEofF:
      case qErro:
        return false;
      case qPunc:
        return v.punc < rhs.v.punc;
      case qOper:
        return v.op < rhs.v.op;
      case qKeyW:
        return v.key < rhs.v.key;
      case qIntL:
      case qNumL:
      case qText:
      case qName:
      case qChar:
      case qMacB:
      case qMacr:
      case qNote:
        return v.str_idx < rhs.v.str_idx;
      default:
        __builtin_unreachable();
    }
  }

#endif
} __attribute__((packed)) qlex_tok_t;

#define QLEX_TOK_SIZE (sizeof(qlex_tok_t))

#endif  // __NITRATE_LEXER_TOKEN_H__
