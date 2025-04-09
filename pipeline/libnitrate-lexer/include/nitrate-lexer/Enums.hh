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

#ifndef __NITRATE_LEXER_ENUMS_HH__
#define __NITRATE_LEXER_ENUMS_HH__

#include <cstdint>

namespace ncc::lex {
  enum TokenType : uint8_t {
    EofF, /* End of file */
    KeyW, /* Keyword */
    Oper, /* Operator */
    Punc, /* Punctuation */
    Name, /* Identifier */
    IntL, /* Integer literal */
    NumL, /* Floating-point literal */
    Text, /* String literal */
    Char, /* Character literal */
    MacB, /* Macro block */
    Macr, /* Macro call */
    Note, /* Comment */
  };

  enum Punctor : uint8_t {
    PuncLPar,  /* '(':  Left parenthesis */
    PuncRPar,  /* ')':  Right parenthesis */
    PuncLBrk,  /* '[':  Left bracket */
    PuncRBrk,  /* ']':  Right bracket */
    PuncLCur,  /* '{':  Left curly brace */
    PuncRCur,  /* '}':  Right curly brace */
    PuncComa,  /* ',':  Comma */
    PuncColn,  /* ':':  Colon */
    PuncSemi,  /* ';':  Semicolon */
    PuncScope, /* '::': Scope resolution */
  };

  enum Operator : uint8_t {
    OpPlus,        /* '+':    Addition operator */
    OpMinus,       /* '-':    Subtraction operator */
    OpTimes,       /* '*':    Multiplication operator */
    OpSlash,       /* '/':    Division operator */
    OpPercent,     /* '%':    Modulus operator */
    OpBitAnd,      /* '&':    Bitwise AND operator */
    OpBitOr,       /* '|':    Bitwise OR operator */
    OpBitXor,      /* '^':    Bitwise XOR operator */
    OpBitNot,      /* '~':    Bitwise NOT operator */
    OpLShift,      /* '<<':   Left shift operator */
    OpRShift,      /* '>>':   Right shift operator */
    OpROTL,        /* '<<<':  Rotate left operator */
    OpROTR,        /* '>>>':  Rotate right operator */
    OpLogicAnd,    /* '&&':   Logical AND operator */
    OpLogicOr,     /* '||':   Logical OR operator */
    OpLogicXor,    /* '^^':   Logical XOR operator */
    OpLogicNot,    /* '!':    Logical NOT operator */
    OpLT,          /* '<':    Less than operator */
    OpGT,          /* '>':    Greater than operator */
    OpLE,          /* '<=':   Less than or equal to operator */
    OpGE,          /* '>=':   Greater than or equal to operator */
    OpEq,          /* '==':   Equal to operator */
    OpNE,          /* '!=':   Not equal to operator */
    OpSet,         /* '=':    Assignment operator */
    OpPlusSet,     /* '+=':   Addition assignment operator */
    OpMinusSet,    /* '-=':   Subtraction assignment operator */
    OpTimesSet,    /* '*=':   Multiplication assignment operator */
    OpSlashSet,    /* '/=':   Division assignment operator */
    OpPercentSet,  /* '%=':   Modulus assignment operator */
    OpBitAndSet,   /* '&=':   Bitwise AND assignment operator */
    OpBitOrSet,    /* '|=':   Bitwise OR assignment operator */
    OpBitXorSet,   /* '^=':   Bitwise XOR assignment operator */
    OpLogicAndSet, /* '&&=':  Logical AND assignment operator */
    OpLogicOrSet,  /* '||=':  Logical OR assignment operator */
    OpLogicXorSet, /* '^^=':  Logical XOR assignment operator */
    OpLShiftSet,   /* '<<=':  Left shift assignment operator */
    OpRShiftSet,   /* '>>=':  Right shift assignment operator */
    OpROTLSet,     /* '<<<=': Rotate left assignment operator */
    OpROTRSet,     /* '>>>=': Rotate right assignment operator */
    OpInc,         /* '++':   Increment operator */
    OpDec,         /* '--':   Decrement operator */
    OpAs,          /* 'as':   Type cast operator */
    OpBitcastAs,   /* 'bitcast_as': Bitcast operator */
    OpIn,          /* 'in':         Generic membership operator */
    OpOut,         /* 'out':        Output operator */
    OpSizeof,      /* 'sizeof':     Size of operator */
    OpBitsizeof,   /* 'bitsizeof':  Bit size of operator */
    OpAlignof,     /* 'alignof':    Alignment of operator */
    OpTypeof,      /* 'typeof':     Type of operator */
    OpComptime,    /* 'comptime':   Compile-time operator */
    OpDot,         /* '.':          Dot operator */
    OpRange,       /* '..':         Range operator */
    OpEllipsis,    /* '...':        Ellipsis operator */
    OpArrow,       /* '=>':         Arrow operator */
    OpTernary,     /* '?':          Ternary operator */
    Op_First = OpPlus,
    Op_Last = OpTernary,
  };

  enum Keyword : uint8_t {
    Scope,       /* 'scope' */
    Pub,         /* 'pub' */
    Sec,         /* 'sec' */
    Pro,         /* 'pro' */
    Import,      /* 'import' */
    Type,        /* 'type' */
    Let,         /* 'let' */
    Var,         /* 'var' */
    Const,       /* 'const' */
    Static,      /* 'static' */
    Struct,      /* 'struct' */
    Region,      /* 'region' */
    Group,       /* 'group' */
    Class,       /* 'class' */
    Union,       /* 'union' */
    Opaque,      /* 'opaque' */
    Enum,        /* 'enum' */
    __FString,   /* '__fstring' */
    Fn,          /* 'fn' */
    Safe,        /* 'safe' */
    Unsafe,      /* 'unsafe' */
    Pure,        /* 'pure' */
    Impure,      /* 'impure' */
    Quasi,       /* 'quasi' */
    Retro,       /* 'retro' */
    Inline,      /* 'inline' */
    Foreign,     /* 'foreign' */
    Promise,     /* 'promise' */
    If,          /* 'if' */
    Else,        /* 'else' */
    For,         /* 'for' */
    While,       /* 'while' */
    Do,          /* 'do' */
    Switch,      /* 'switch' */
    Break,       /* 'break' */
    Continue,    /* 'continue' */
    Return,      /* 'ret' */
    Foreach,     /* 'foreach' */
    Try,         /* 'try' */
    Catch,       /* 'catch' */
    Throw,       /* 'throw' */
    Async,       /* 'async' */
    Await,       /* 'await' */
    __Asm__,     /* '__asm__' */
    True,        /* 'true' */
    False,       /* 'false' */
    EscapeBlock, /* 'escape_block' */
    UnitAssert,  /* 'unit_assert' */
  };
}  // namespace ncc::lex

#endif
