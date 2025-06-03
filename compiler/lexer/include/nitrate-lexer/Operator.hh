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
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <boost/bimap.hpp>
#include <cstdint>

namespace nitrate::compiler::lexer {
  enum Operator : uint8_t {
    OpPlus,    /* '+': Addition */
    OpMinus,   /* '-': Subtraction */
    OpTimes,   /* '*': Multiplication */
    OpSlash,   /* '/': Division */
    OpPercent, /* '%': Modulus */

    OpBitAnd,  /* '&':   Bitwise AND */
    OpBitOr,   /* '|':   Bitwise OR */
    OpBitXor,  /* '^':   Bitwise XOR */
    OpBitNot,  /* '~':   Bitwise NOT */
    OpBitShl,  /* '<<':  Bitwise left shift */
    OpBitShr,  /* '>>':  Bitwise right shift */
    OpBitRotl, /* '<<<': Bitwise rotate left */
    OpBitRotr, /* '>>>': Bitwise rotate right */

    OpLogicAnd, /* '&&': Logical AND */
    OpLogicOr,  /* '||': Logical OR */
    OpLogicXor, /* '^^': Logical XOR */
    OpLogicNot, /* '!':  Logical NOT */
    OpLogicLt,  /* '<':  Logical less than */
    OpLogicGt,  /* '>':  Logical greater than */
    OpLogicLe,  /* '<=': Logical less than or equal to */
    OpLogicGe,  /* '>=': Logical greater than or equal to */
    OpLogicEq,  /* '==': Logical equal to */
    OpLogicNe,  /* '!=': Logical not equal to */

    OpSet,         /* '=':    Assignment */
    OpSetPlus,     /* '+=':   Addition Assignment */
    OpSetMinus,    /* '-=':   Subtraction Assignment */
    OpSetTimes,    /* '*=':   Multiplication Assignment */
    OpSetSlash,    /* '/=':   Division Assignment */
    OpSetPercent,  /* '%=':   Modulus Assignment */
    OpSetBitAnd,   /* '&=':   Bitwise AND Assignment */
    OpSetBitOr,    /* '|=':   Bitwise OR Assignment */
    OpSetBitXor,   /* '^=':   Bitwise XOR Assignment */
    OpSetBitNot,   /* '~=':   Bitwise NOT Assignment */
    OpSetBitShl,   /* '<<=':  Bitwise left shift Assignment */
    OpSetBitShr,   /* '>>=':  Bitwise right shift Assignment */
    OpSetBitRotl,  /* '<<<=': Bitwise rotate left Assignment */
    OpSetBitRotr,  /* '>>>=': Bitwise rotate right Assignment */
    OpSetLogicAnd, /* '&&=':  Logical AND Assignment */
    OpSetLogicOr,  /* '||=':  Logical OR Assignment */
    OpSetLogicXor, /* '^^=':  Logical XOR Assignment */
    OpSetLogicNot, /* '!==':  Logical NOT Assignment */
    OpSetLogicLt,  /* '<==':  Logical less than Assignment */
    OpSetLogicGt,  /* '>==':  Logical greater than Assignment */
    OpSetLogicLe,  /* '<==':  Logical less than or equal to Assignment */
    OpSetLogicGe,  /* '>==':  Logical greater than or equal to Assignment */
    OpSetLogicEq,  /* '===':  Logical equal to Assignment */
    OpSetLogicNe,  /* '!==':  Logical not equal to Assignment */
    OpSetInc,      /* '++':   Increment */
    OpSetDec,      /* '--':   Decrement */

    OpAs,        /* 'as':         Type cast */
    OpBitcastAs, /* 'bitcast_as': Bitcast */
    OpSizeof,    /* 'sizeof':     Size of */
    OpAlignof,   /* 'alignof':    Alignment of */
    OpTypeof,    /* 'typeof':     Type of */

    OpIn,       /* 'in' */
    OpOut,      /* 'out' */
    OpDot,      /* '.':          Dot */
    OpRange,    /* '..':         Range */
    OpEllipsis, /* '...':        Ellipsis */
  };

  static inline const boost::bimap<Operator, std::string_view> OPERATOR_MAP = [] {
    boost::bimap<Operator, std::string_view> mapping;
    auto& map = mapping.left;

    map[OpPlus] = "+";
    map[OpMinus] = "-";
    map[OpTimes] = "*";
    map[OpSlash] = "/";
    map[OpPercent] = "%";

    map[OpBitAnd] = "&";
    map[OpBitOr] = "|";
    map[OpBitXor] = "^";
    map[OpBitNot] = "~";
    map[OpBitShl] = "<<";
    map[OpBitShr] = ">>";
    map[OpBitRotl] = "<<<";
    map[OpBitRotr] = ">>>";

    map[OpLogicAnd] = "&&";
    map[OpLogicOr] = "||";
    map[OpLogicXor] = "^^";
    map[OpLogicNot] = "!";
    map[OpLogicLt] = "<";
    map[OpLogicGt] = ">";
    map[OpLogicLe] = "<=";
    map[OpLogicGe] = ">=";
    map[OpLogicEq] = "==";
    map[OpLogicNe] = "!=";

    map[OpSet] = "=";
    map[OpSetPlus] = "+=";
    map[OpSetMinus] = "-=";
    map[OpSetTimes] = "*=";
    map[OpSetSlash] = "/=";
    map[OpSetPercent] = "%=";
    map[OpSetBitAnd] = "&=";
    map[OpSetBitOr] = "|=";
    map[OpSetBitXor] = "^=";
    map[OpSetBitNot] = "~=";
    map[OpSetBitShl] = "<<=";
    map[OpSetBitShr] = ">>=";
    map[OpSetBitRotl] = "<<<=";
    map[OpSetBitRotr] = ">>>=";
    map[OpSetLogicAnd] = "&&=";
    map[OpSetLogicOr] = "||=";
    map[OpSetLogicXor] = "^^=";
    map[OpSetLogicNot] = "!==";
    map[OpSetLogicLt] = "<==";
    map[OpSetLogicGt] = ">==";
    map[OpSetLogicLe] = "<==";
    map[OpSetLogicGe] = ">==";
    map[OpSetLogicEq] = "===";
    map[OpSetLogicNe] = "!==";
    map[OpSetInc] = "++";
    map[OpSetDec] = "--";

    map[OpAs] = "as";
    map[OpBitcastAs] = "bitcast_as";
    map[OpSizeof] = "sizeof";
    map[OpAlignof] = "alignof";
    map[OpTypeof] = "typeof";

    map[OpIn] = "in";
    map[OpOut] = "out";
    map[OpDot] = ".";
    map[OpRange] = "..";
    map[OpEllipsis] = "...";

    return mapping;
  }();
}  // namespace nitrate::compiler::lexer
