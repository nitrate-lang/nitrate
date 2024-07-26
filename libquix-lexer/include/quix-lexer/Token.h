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
  qEofF, /* End of file */
  qErro, /* Error, invalid token */
  qKeyW, /* Keyword */
  qOper, /* Operator */
  qPunc, /* Punctuation */
  qName, /* Identifier */
  qIntL, /* Integer literal */
  qNumL, /* Floating-point literal */
  qText, /* String literal */
  qChar, /* Character literal */
  qMacB, /* Macro block */
  qMacr, /* Macro call */
  qNote, /* Comment */
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
  qOpTernary,       /* '?:': Ternary operator */
  qOpArrow,         /* '=>': Arrow operator */
  qOpDot,           /* '.': Dot operator */
  qOpPlus,          /* '+': Addition operator */
  qOpMinus,         /* '-': Subtraction operator */
  qOpTimes,         /* '*': Multiplication operator */
  qOpSlash,         /* '/': Division operator */
  qOpPercent,       /* '%': Modulus operator */
  qOpBitAnd,        /* '&': Bitwise AND operator */
  qOpBitOr,         /* '|': Bitwise OR operator */
  qOpBitXor,        /* '^': Bitwise XOR operator */
  qOpBitNot,        /* '~': Bitwise NOT operator */
  qOpLogicAnd,      /* '&&': Logical AND operator */
  qOpLogicOr,       /* '||': Logical OR operator */
  qOpLogicXor,      /* '^^': Logical XOR operator */
  qOpLogicNot,      /* '!': Logical NOT operator */
  qOpLShift,        /* '<<': Left shift operator */
  qOpRShift,        /* '>>': Right shift operator */
  qOpROTR,          /* '>>>': Rotate right operator */
  qOpROTL,          /* '<<<': Rotate left operator */
  qOpInc,           /* '++': Increment operator */
  qOpDec,           /* '--': Decrement operator */
  qOpSet,           /* '=': Assignment operator */
  qOpPlusSet,       /* '+=': Addition assignment operator */
  qOpMinusSet,      /* '-=': Subtraction assignment operator */
  qOpTimesSet,      /* '*=': Multiplication assignment operator */
  qOpSlashSet,      /* '/=': Division assignment operator */
  qOpPercentSet,    /* '%=': Modulus assignment operator */
  qOpBitAndSet,     /* '&=': Bitwise AND assignment operator */
  qOpBitOrSet,      /* '|=': Bitwise OR assignment operator */
  qOpBitXorSet,     /* '^=': Bitwise XOR assignment operator */
  qOpLogicAndSet,   /* '&&=': Logical AND assignment operator */
  qOpLogicOrSet,    /* '||=': Logical OR assignment operator */
  qOpLogicXorSet,   /* '^^=': Logical XOR assignment operator */
  qOpLShiftSet,     /* '<<=': Left shift assignment operator */
  qOpRShiftSet,     /* '>>=': Right shift assignment operator */
  qOpROTRSet,       /* '>>>=': Rotate right assignment operator */
  qOpROTLSet,       /* '<<<=': Rotate left assignment operator */
  qOpLT,            /* '<': Less than operator */
  qOpGT,            /* '>': Greater than operator */
  qOpLE,            /* '<=': Less than or equal to operator */
  qOpGE,            /* '>=': Greater than or equal to operator */
  qOpEq,            /* '==': Equal to operator */
  qOpNE,            /* '!=': Not equal to operator */
  qOpAs,            /* 'as': Type cast operator */
  qOpIs,            /* 'is': Generic check operator */
  qOpIn,            /* 'in': Generic membership operator */
  qOpSizeof,        /* 'sizeof': Size of operator */
  qOpAlignof,       /* 'alignof': Alignment of operator */
  qOpTypeof,        /* 'typeof': Type of operator */
  qOpOffsetof,      /* 'offsetof': Offset of operator */
  qOpRange,         /* '..': Range operator */
  qOpEllipsis,      /* '...': Ellipsis operator */
  qOpSpaceship,     /* '<=>': Spaceship operator */
  qOpBitcastAs,     /* 'bitcast_as': Bitcast operator */
  qOpReinterpretAs, /* 'reinterpret_as': Reinterpret operator */
  qOpBitsizeof,     /* 'bitsizeof': Bit size of operator */
  qOpOut,           /* 'out': Output operator */
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
  qKUnion,      /* 'union' */
  qKOpaque,     /* 'opaque' */
  qKEnum,       /* 'enum' */
  qKFString,    /* 'fstring' */
  qKImpl,       /* 'impl' */
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
  qKReq,        /* 'req' */
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

typedef struct qlex_loc_t {
  uint32_t idx; /* Index into internal data structure */
} __attribute__((packed)) qlex_loc_t;

typedef struct qlex_tok_t {
  /* Token type */
  qlex_ty_t ty : 4;

  /* Location of the token in the source code. */
  /* The token size will be calculated as needed. */
  uint32_t src_idx : 24;

  /* Token data */
  union {
    qlex_punc_t punc;
    qlex_op_t op;
    qlex_key_t key;
    uint32_t str_idx;
  } v;

#if defined(__cplusplus) && defined(__QUIX_LEXER_IMPL__)
  qlex_tok_t() : ty(qEofF), src_idx(0) {}
  qlex_tok_t(qlex_ty_t ty, uint32_t src_idx, qlex_punc_t punc)
      : ty(ty), src_idx(src_idx), v{.punc = punc} {}
  qlex_tok_t(qlex_ty_t ty, uint32_t src_idx, qlex_op_t op)
      : ty(ty), src_idx(src_idx), v{.op = op} {}
  qlex_tok_t(qlex_ty_t ty, uint32_t src_idx, qlex_key_t key)
      : ty(ty), src_idx(src_idx), v{.key = key} {}
  qlex_tok_t(qlex_ty_t ty, uint32_t src_idx, uint32_t str_idx)
      : ty(ty), src_idx(src_idx), v{.str_idx = str_idx} {}
  static qlex_tok_t err(uint32_t src_idx) {
    return qlex_tok_t(qErro, src_idx, 0);
  }
  static qlex_tok_t eof(uint32_t src_idx) {
    return qlex_tok_t(qEofF, src_idx, 0);
  }
#endif
} __attribute__((packed)) qlex_tok_t;

#ifdef __cplusplus
}
#endif

#endif  // __QUIX_LEXER_TOKEN_H__
