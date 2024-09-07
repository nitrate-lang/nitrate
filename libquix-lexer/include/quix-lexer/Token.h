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

#ifndef __QUIX_LEXER_TOKEN_H__
#define __QUIX_LEXER_TOKEN_H__

#ifdef __cplusplus
extern "C" {
#endif

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
  qPuncLPar = 1, /* Left parenthesis */
  qPuncRPar,     /* Right parenthesis */
  qPuncLBrk,     /* Left bracket */
  qPuncRBrk,     /* Right bracket */
  qPuncLCur,     /* Left curly brace */
  qPuncRCur,     /* Right curly brace */
  qPuncComa,     /* Comma */
  qPuncColn,     /* Colon */
  qPuncSemi,     /* Semicolon */
} __attribute__((packed)) qlex_punc_t;

typedef enum qlex_op_t {
  qOpUnknown = 0, /* Unknown operator */
  qOpTernary = 1, /* '?:': Ternary operator */
  qOpArrow,       /* '=>': Arrow operator */
  qOpDot,         /* '.': Dot operator */
  qOpPlus,        /* '+': Addition operator */
  qOpMinus,       /* '-': Subtraction operator */
  qOpTimes,       /* '*': Multiplication operator */
  qOpSlash,       /* '/': Division operator */
  qOpPercent,     /* '%': Modulus operator */
  qOpBitAnd,      /* '&': Bitwise AND operator */
  qOpBitOr,       /* '|': Bitwise OR operator */
  qOpBitXor,      /* '^': Bitwise XOR operator */
  qOpBitNot,      /* '~': Bitwise NOT operator */
  qOpLogicAnd,    /* '&&': Logical AND operator */
  qOpLogicOr,     /* '||': Logical OR operator */
  qOpLogicXor,    /* '^^': Logical XOR operator */
  qOpLogicNot,    /* '!': Logical NOT operator */
  qOpLShift,      /* '<<': Left shift operator */
  qOpRShift,      /* '>>': Right shift operator */
  qOpROTR,        /* '>>>': Rotate right operator */
  qOpROTL,        /* '<<<': Rotate left operator */
  qOpInc,         /* '++': Increment operator */
  qOpDec,         /* '--': Decrement operator */
  qOpSet,         /* '=': Assignment operator */
  qOpPlusSet,     /* '+=': Addition assignment operator */
  qOpMinusSet,    /* '-=': Subtraction assignment operator */
  qOpTimesSet,    /* '*=': Multiplication assignment operator */
  qOpSlashSet,    /* '/=': Division assignment operator */
  qOpPercentSet,  /* '%=': Modulus assignment operator */
  qOpBitAndSet,   /* '&=': Bitwise AND assignment operator */
  qOpBitOrSet,    /* '|=': Bitwise OR assignment operator */
  qOpBitXorSet,   /* '^=': Bitwise XOR assignment operator */
  qOpLogicAndSet, /* '&&=': Logical AND assignment operator */
  qOpLogicOrSet,  /* '||=': Logical OR assignment operator */
  qOpLogicXorSet, /* '^^=': Logical XOR assignment operator */
  qOpLShiftSet,   /* '<<=': Left shift assignment operator */
  qOpRShiftSet,   /* '>>=': Right shift assignment operator */
  qOpROTRSet,     /* '>>>=': Rotate right assignment operator */
  qOpROTLSet,     /* '<<<=': Rotate left assignment operator */
  qOpLT,          /* '<': Less than operator */
  qOpGT,          /* '>': Greater than operator */
  qOpLE,          /* '<=': Less than or equal to operator */
  qOpGE,          /* '>=': Greater than or equal to operator */
  qOpEq,          /* '==': Equal to operator */
  qOpNE,          /* '!=': Not equal to operator */
  qOpAs,          /* 'as': Type cast operator */
  qOpIs,          /* 'is': Generic check operator */
  qOpIn,          /* 'in': Generic membership operator */
  qOpSizeof,      /* 'sizeof': Size of operator */
  qOpAlignof,     /* 'alignof': Alignment of operator */
  qOpTypeof,      /* 'typeof': Type of operator */
  qOpOffsetof,    /* 'offsetof': Offset of operator */
  qOpRange,       /* '..': Range operator */
  qOpEllipsis,    /* '...': Ellipsis operator */
  qOpBitcastAs,   /* 'bitcast_as': Bitcast operator */
  qOpBitsizeof,   /* 'bitsizeof': Bit size of operator */
  qOpOut,         /* 'out': Output operator */
} __attribute__((packed)) qlex_op_t;

typedef enum qlex_key_t {
  qKSubsystem,  /* 'subsystem' */
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
  qKFString,    /* 'fstring' */
  qKWith,       /* 'with' */
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
  qKVolatile,   /* 'volatile' */
  qKPromise,    /* 'promise' */
  qKIf,         /* 'if' */
  qKElse,       /* 'else' */
  qKFor,        /* 'for' */
  qKWhile,      /* 'while' */
  qKDo,         /* 'do' */
  qKSwitch,     /* 'switch' */
  qKCase,       /* 'case' */
  qKDefault,    /* 'default' */
  qKBreak,      /* 'break' */
  qKContinue,   /* 'continue' */
  qKReturn,     /* 'ret' */
  qKRetif,      /* 'retif' */
  qKRetz,       /* 'retz' */
  qKRetv,       /* 'retv' */
  qKForm,       /* 'form' */
  qKForeach,    /* 'foreach' */
  qK__Asm__,    /* '__asm__' */
  qKVoid,       /* 'void' */
  qKUndef,      /* 'undef' */
  qKNull,       /* 'null' */
  qKTrue,       /* 'true' */
  qKFalse,      /* 'false' */
} __attribute__((packed)) qlex_key_t;

typedef uint32_t qlex_size;

#if defined(__cplusplus) && defined(__QUIX_IMPL__)
}

#include <stdexcept>
#include <string>
#include <type_traits>

struct qlex_t;
struct qlex_tok_t;

typedef struct qlex_loc_t {
  uint32_t tag : 24;
} __attribute__((packed)) qlex_loc_t;

extern "C" const char *qlex_str(struct qlex_t *lexer, struct qlex_tok_t *tok, size_t *len);

typedef struct qlex_tok_t final {
  /* Token type */
  qlex_ty_t ty : 4;

  /* Location of the token in the source code. */
  /* The token size will be calculated as needed. */
  qlex_loc_t start, end;

  /* Token data */
  union {
    qlex_punc_t punc;
    qlex_op_t op;
    qlex_key_t key;
    qlex_size str_idx;
  } __attribute__((packed)) v;

  uint64_t pad : 4;

  qlex_tok_t() : ty(qEofF), start({}), end({}), v{.str_idx = 0} {}

  qlex_tok_t(qlex_ty_t ty, qlex_punc_t punc, qlex_loc_t loc_beg = {}, qlex_loc_t loc_end = {})
      : ty(ty), start(loc_beg), end(loc_end), v{.punc = punc} {}

  qlex_tok_t(qlex_ty_t ty, qlex_op_t op, qlex_loc_t loc_beg = {}, qlex_loc_t loc_end = {})
      : ty(ty), start(loc_beg), end(loc_end), v{.op = op} {}

  qlex_tok_t(qlex_ty_t ty, qlex_key_t key, qlex_loc_t loc_beg = {}, qlex_loc_t loc_end = {})
      : ty(ty), start(loc_beg), end(loc_end), v{.key = key} {}

  qlex_tok_t(qlex_ty_t ty, qlex_size str_idx, qlex_loc_t loc_beg = {}, qlex_loc_t loc_end = {})
      : ty(ty), start(loc_beg), end(loc_end), v{.str_idx = str_idx} {}

  static qlex_tok_t err(qlex_loc_t loc_start, qlex_loc_t loc_end) {
    return qlex_tok_t(qErro, 0, loc_start, loc_end);
  }
  static qlex_tok_t eof(qlex_loc_t loc_start, qlex_loc_t loc_end) {
    return qlex_tok_t(qEofF, 0, loc_start, loc_end);
  }

  template <typename T>
  T as() const {
    if constexpr (std::is_same_v<T, qlex_punc_t>) {
      return v.punc;
    } else if constexpr (std::is_same_v<T, qlex_key_t>) {
      return v.key;
    } else if constexpr (std::is_same_v<T, qlex_op_t>) {
      return v.op;
    }

    static_assert(std::is_same_v<T, T>, "Invalid type");
  }

  inline bool is(qlex_ty_t val) const { return ty == val; }

  bool operator==(const qlex_tok_t &rhs) = delete;

  template <auto V>
  bool is() const {
    if constexpr (std::is_same_v<decltype(V), qlex_key_t>) {
      return ty == qKeyW && as<qlex_key_t>() == V;
    } else if constexpr (std::is_same_v<decltype(V), qlex_punc_t>) {
      return ty == qPunc && as<qlex_punc_t>() == V;
    } else if constexpr (std::is_same_v<decltype(V), qlex_op_t>) {
      return ty == qOper && as<qlex_op_t>() == V;
    }
  }

  inline std::string as_string(qlex_t *lexer) {
    size_t len;
    const char *s = qlex_str(lexer, this, &len);

    return std::string(s, len);
  }

  bool operator<(const qlex_tok_t &rhs) const {
    if (ty != rhs.ty) return ty < rhs.ty;
    switch (ty) {
      case qPunc:
        return v.punc < rhs.v.punc;
      case qOper:
        return v.op < rhs.v.op;
      case qKeyW:
        return v.key < rhs.v.key;
      case qIntL:
      case qNumL:
      case qText:
      case qChar:
        return v.str_idx < rhs.v.str_idx;
      default:
        return false;
    }
  }
} __attribute__((packed)) qlex_tok_t;
extern "C" {
#else

typedef struct qlex_loc_t {
  uint32_t tag : 24;
} __attribute__((packed)) qlex_loc_t;

typedef struct qlex_tok_t {
  /* Token type */
  qlex_ty_t ty : 4;

  /* Location of the token in the source code. */
  /* The token size will be calculated as needed. */
  qlex_loc_t start, end;

  /* Token data */
  union {
    qlex_punc_t punc;
    qlex_op_t op;
    qlex_key_t key;
    qlex_size str_idx;
  } __attribute__((packed)) v;

  uint64_t pad : 4;
} __attribute__((packed)) qlex_tok_t;
#endif

#define QLEX_TOK_SIZE (sizeof(qlex_tok_t))

#ifdef __cplusplus
}
#endif

#endif  // __QUIX_LEXER_TOKEN_H__
