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

#ifndef __NITRATE_LEXER_TOKEN_HH__
#define __NITRATE_LEXER_TOKEN_HH__

#include <cstdint>
#include <nitrate-core/Logger.hh>
#include <nitrate-core/String.hh>
#include <string_view>
#include <type_traits>

namespace ncc::lex {
  typedef enum qlex_ty_t {
    qEofF = 1, /* End of file */
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
    qKScope,     /* 'scope' */
    qKImport,    /* 'import' */
    qKPub,       /* 'pub' */
    qKSec,       /* 'sec' */
    qKPro,       /* 'pro' */
    qKType,      /* 'type' */
    qKLet,       /* 'let' */
    qKVar,       /* 'var' */
    qKConst,     /* 'const' */
    qKStatic,    /* 'static' */
    qKStruct,    /* 'struct' */
    qKRegion,    /* 'region' */
    qKGroup,     /* 'group' */
    qKClass,     /* 'class' */
    qKUnion,     /* 'union' */
    qKOpaque,    /* 'opaque' */
    qKEnum,      /* 'enum' */
    qK__FString, /* '__fstring' */
    qKFn,        /* 'fn' */
    qKUnsafe,    /* 'unsafe' */
    qKSafe,      /* 'safe' */
    qKPromise,   /* 'promise' */
    qKIf,        /* 'if' */
    qKElse,      /* 'else' */
    qKFor,       /* 'for' */
    qKWhile,     /* 'while' */
    qKDo,        /* 'do' */
    qKSwitch,    /* 'switch' */
    qKBreak,     /* 'break' */
    qKContinue,  /* 'continue' */
    qKReturn,    /* 'ret' */
    qKRetif,     /* 'retif' */
    qKForeach,   /* 'foreach' */
    qKTry,       /* 'try' */
    qKCatch,     /* 'catch' */
    qKThrow,     /* 'throw' */
    qKAsync,     /* 'async' */
    qKAwait,     /* 'await' */
    qK__Asm__,   /* '__asm__' */
    qKUndef,     /* 'undef' */
    qKNull,      /* 'null' */
    qKTrue,      /* 'true' */
    qKFalse,     /* 'false' */
  } __attribute__((packed)) qlex_key_t;

  constexpr size_t QLEX_EOFF = UINT32_MAX;
  constexpr size_t QLEX_NOFILE = 16777215;

  class Location {
    uint32_t m_offset : 32 = 0;
    uint32_t m_fileid : 24 = 0;

  public:
    constexpr Location(uint32_t offset = QLEX_EOFF,
                       uint32_t fileid = QLEX_NOFILE)
        : m_offset(offset), m_fileid(fileid) {}

    constexpr uint32_t GetOffset() const { return m_offset; }
    constexpr uint32_t GetFileId() const { return m_fileid; }
  } __attribute__((packed));

  union TokenData {
    qlex_punc_t punc;
    qlex_op_t op;
    qlex_key_t key;
    ncc::core::str_alias str;

    constexpr TokenData(qlex_punc_t punc) : punc(punc) {}
    constexpr TokenData(qlex_op_t op) : op(op) {}
    constexpr TokenData(qlex_key_t key) : key(key) {}
    constexpr TokenData(ncc::core::str_alias str) : str(str) {}
  } __attribute__((packed));

  std::string_view to_string(qlex_ty_t, TokenData);

  template <class LocationTracker>
  class TokenBase {
    LocationTracker m_start = 0;
    uint64_t pad : 4 = 0;
    qlex_ty_t m_type : 4;

  public:
    TokenData v;

    template <class T = qlex_op_t>
    constexpr TokenBase(qlex_ty_t ty = qEofF, T val = qOpPlus,
                        LocationTracker _start = LocationTracker())
        : m_start(_start), m_type(ty), v{val} {}

    constexpr static TokenBase EndOfFile() { return TokenBase(); }

    constexpr bool is(qlex_ty_t val) const { return m_type == val; }

    constexpr bool operator==(const TokenBase &rhs) const {
      if (m_type != rhs.m_type) return false;
      switch (m_type) {
        case qEofF:
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
          return v.str == rhs.v.str;
      }
    }

    template <auto V>
    constexpr bool is() const {
      if constexpr (std::is_same_v<decltype(V), qlex_key_t>) {
        return m_type == qKeyW && v.key == V;
      } else if constexpr (std::is_same_v<decltype(V), qlex_punc_t>) {
        return m_type == qPunc && v.punc == V;
      } else if constexpr (std::is_same_v<decltype(V), qlex_op_t>) {
        return m_type == qOper && v.op == V;
      }
    }

    std::string_view as_string() const { return to_string(m_type, v); }

    qlex_key_t as_key() const { return v.key; }
    qlex_op_t as_op() const { return v.op; }
    qlex_punc_t as_punc() const { return v.punc; }

    LocationTracker get_start() const { return m_start; }

    qlex_ty_t get_type() const { return m_type; }

    constexpr bool operator<(const TokenBase &rhs) const {
      if (m_type != rhs.m_type) {
        return m_type < rhs.m_type;
      }

      switch (m_type) {
        case qEofF:
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
          return v.str < rhs.v.str;
      }
    }
  } __attribute__((packed));

  using Token = TokenBase<uint32_t>;

  constexpr auto QLEX_TOK_SIZE = sizeof(Token);
}  // namespace ncc::lex

#endif  // __NITRATE_LEXER_TOKEN_H__
