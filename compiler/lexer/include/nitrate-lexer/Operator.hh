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
  enum class Operator : uint8_t {
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

    map.insert({Operator::OpPlus, "+"});
    map.insert({Operator::OpMinus, "-"});
    map.insert({Operator::OpTimes, "*"});
    map.insert({Operator::OpSlash, "/"});
    map.insert({Operator::OpPercent, "%"});

    map.insert({Operator::OpBitAnd, "&"});
    map.insert({Operator::OpBitOr, "|"});
    map.insert({Operator::OpBitXor, "^"});
    map.insert({Operator::OpBitNot, "~"});
    map.insert({Operator::OpBitShl, "<<"});
    map.insert({Operator::OpBitShr, ">>"});
    map.insert({Operator::OpBitRotl, "<<<"});
    map.insert({Operator::OpBitRotr, ">>>"});

    map.insert({Operator::OpLogicAnd, "&&"});
    map.insert({Operator::OpLogicOr, "||"});
    map.insert({Operator::OpLogicXor, "^^"});
    map.insert({Operator::OpLogicNot, "!"});
    map.insert({Operator::OpLogicLt, "<"});
    map.insert({Operator::OpLogicGt, ">"});
    map.insert({Operator::OpLogicLe, "<="});
    map.insert({Operator::OpLogicGe, ">="});
    map.insert({Operator::OpLogicEq, "=="});
    map.insert({Operator::OpLogicNe, "!="});

    map.insert({Operator::OpSet, "="});
    map.insert({Operator::OpSetPlus, "+="});
    map.insert({Operator::OpSetMinus, "-="});
    map.insert({Operator::OpSetTimes, "*="});
    map.insert({Operator::OpSetSlash, "/="});
    map.insert({Operator::OpSetPercent, "%="});
    map.insert({Operator::OpSetBitAnd, "&="});
    map.insert({Operator::OpSetBitOr, "|="});
    map.insert({Operator::OpSetBitXor, "^="});
    map.insert({Operator::OpSetBitNot, "~="});
    map.insert({Operator::OpSetBitShl, "<<="});
    map.insert({Operator::OpSetBitShr, ">>="});
    map.insert({Operator::OpSetBitRotl, "<<<="});
    map.insert({Operator::OpSetBitRotr, ">>>="});
    map.insert({Operator::OpSetLogicAnd, "&&="});
    map.insert({Operator::OpSetLogicOr, "||="});
    map.insert({Operator::OpSetLogicXor, "^^="});
    map.insert({Operator::OpSetLogicNot, "!=="});
    map.insert({Operator::OpSetLogicLt, "<=="});
    map.insert({Operator::OpSetLogicGt, ">=="});
    map.insert({Operator::OpSetLogicLe, "<=="});
    map.insert({Operator::OpSetLogicGe, ">=="});
    map.insert({Operator::OpSetLogicEq, "==="});
    map.insert({Operator::OpSetLogicNe, "!=="});
    map.insert({Operator::OpSetInc, "++"});
    map.insert({Operator::OpSetDec, "--"});

    map.insert({Operator::OpAs, "as"});
    map.insert({Operator::OpBitcastAs, "bitcast_as"});
    map.insert({Operator::OpSizeof, "sizeof"});
    map.insert({Operator::OpAlignof, "alignof"});
    map.insert({Operator::OpTypeof, "typeof"});

    map.insert({Operator::OpIn, "in"});
    map.insert({Operator::OpOut, "out"});
    map.insert({Operator::OpDot, "."});
    map.insert({Operator::OpRange, ".."});
    map.insert({Operator::OpEllipsis, "..."});

    return mapping;
  }();
}  // namespace nitrate::compiler::lexer
