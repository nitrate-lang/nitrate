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
    Plus,    /* '+': Addition */
    Minus,   /* '-': Subtraction */
    Times,   /* '*': Multiplication */
    Slash,   /* '/': Division */
    Percent, /* '%': Modulus */

    BitAnd,  /* '&':   Bitwise AND */
    BitOr,   /* '|':   Bitwise OR */
    BitXor,  /* '^':   Bitwise XOR */
    BitNot,  /* '~':   Bitwise NOT */
    BitShl,  /* '<<':  Bitwise left shift */
    BitShr,  /* '>>':  Bitwise right shift */
    BitRotl, /* '<<<': Bitwise rotate left */
    BitRotr, /* '>>>': Bitwise rotate right */

    LogicAnd, /* '&&': Logical AND */
    LogicOr,  /* '||': Logical OR */
    LogicXor, /* '^^': Logical XOR */
    LogicNot, /* '!':  Logical NOT */
    LogicLt,  /* '<':  Logical less than */
    LogicGt,  /* '>':  Logical greater than */
    LogicLe,  /* '<=': Logical less than or equal to */
    LogicGe,  /* '>=': Logical greater than or equal to */
    LogicEq,  /* '==': Logical equal to */
    LogicNe,  /* '!=': Logical not equal to */

    Set,         /* '=':    Assignment */
    SetPlus,     /* '+=':   Addition Assignment */
    SetMinus,    /* '-=':   Subtraction Assignment */
    SetTimes,    /* '*=':   Multiplication Assignment */
    SetSlash,    /* '/=':   Division Assignment */
    SetPercent,  /* '%=':   Modulus Assignment */
    SetBitAnd,   /* '&=':   Bitwise AND Assignment */
    SetBitOr,    /* '|=':   Bitwise OR Assignment */
    SetBitXor,   /* '^=':   Bitwise XOR Assignment */
    SetBitNot,   /* '~=':   Bitwise NOT Assignment */
    SetBitShl,   /* '<<=':  Bitwise left shift Assignment */
    SetBitShr,   /* '>>=':  Bitwise right shift Assignment */
    SetBitRotl,  /* '<<<=': Bitwise rotate left Assignment */
    SetBitRotr,  /* '>>>=': Bitwise rotate right Assignment */
    SetLogicAnd, /* '&&=':  Logical AND Assignment */
    SetLogicOr,  /* '||=':  Logical OR Assignment */
    SetLogicXor, /* '^^=':  Logical XOR Assignment */
    SetLogicNot, /* '!==':  Logical NOT Assignment */
    SetLogicLt,  /* '<==':  Logical less than Assignment */
    SetLogicGt,  /* '>==':  Logical greater than Assignment */
    SetLogicLe,  /* '<==':  Logical less than or equal to Assignment */
    SetLogicGe,  /* '>==':  Logical greater than or equal to Assignment */
    SetLogicEq,  /* '===':  Logical equal to Assignment */
    SetLogicNe,  /* '!==':  Logical not equal to Assignment */
    SetInc,      /* '++':   Increment */
    SetDec,      /* '--':   Decrement */

    As,        /* 'as':         Type cast */
    BitcastAs, /* 'bitcast_as': Bitcast */
    Sizeof,    /* 'sizeof':     Size of */
    Alignof,   /* 'alignof':    Alignment of */
    Typeof,    /* 'typeof':     Type of */

    In,       /* 'in' */
    Out,      /* 'out' */
    Dot,      /* '.':          Dot */
    Range,    /* '..':         Range */
    Ellipsis, /* '...':        Ellipsis */
    Scope,    /* '::':         Scope resolution */
    Question, /* '?':          Ternary operator (conditional) */
  };

  static inline const boost::bimap<Operator, std::string_view> OPERATOR_MAP = [] {
    boost::bimap<Operator, std::string_view> mapping;
    auto& map = mapping.left;

    map.insert({Operator::Plus, "+"});
    map.insert({Operator::Minus, "-"});
    map.insert({Operator::Times, "*"});
    map.insert({Operator::Slash, "/"});
    map.insert({Operator::Percent, "%"});

    map.insert({Operator::BitAnd, "&"});
    map.insert({Operator::BitOr, "|"});
    map.insert({Operator::BitXor, "^"});
    map.insert({Operator::BitNot, "~"});
    map.insert({Operator::BitShl, "<<"});
    map.insert({Operator::BitShr, ">>"});
    map.insert({Operator::BitRotl, "<<<"});
    map.insert({Operator::BitRotr, ">>>"});

    map.insert({Operator::LogicAnd, "&&"});
    map.insert({Operator::LogicOr, "||"});
    map.insert({Operator::LogicXor, "^^"});
    map.insert({Operator::LogicNot, "!"});
    map.insert({Operator::LogicLt, "<"});
    map.insert({Operator::LogicGt, ">"});
    map.insert({Operator::LogicLe, "<="});
    map.insert({Operator::LogicGe, ">="});
    map.insert({Operator::LogicEq, "=="});
    map.insert({Operator::LogicNe, "!="});

    map.insert({Operator::Set, "="});
    map.insert({Operator::SetPlus, "+="});
    map.insert({Operator::SetMinus, "-="});
    map.insert({Operator::SetTimes, "*="});
    map.insert({Operator::SetSlash, "/="});
    map.insert({Operator::SetPercent, "%="});
    map.insert({Operator::SetBitAnd, "&="});
    map.insert({Operator::SetBitOr, "|="});
    map.insert({Operator::SetBitXor, "^="});
    map.insert({Operator::SetBitNot, "~="});
    map.insert({Operator::SetBitShl, "<<="});
    map.insert({Operator::SetBitShr, ">>="});
    map.insert({Operator::SetBitRotl, "<<<="});
    map.insert({Operator::SetBitRotr, ">>>="});
    map.insert({Operator::SetLogicAnd, "&&="});
    map.insert({Operator::SetLogicOr, "||="});
    map.insert({Operator::SetLogicXor, "^^="});
    map.insert({Operator::SetLogicNot, "!=="});
    map.insert({Operator::SetLogicLt, "<=="});
    map.insert({Operator::SetLogicGt, ">=="});
    map.insert({Operator::SetLogicLe, "<=="});
    map.insert({Operator::SetLogicGe, ">=="});
    map.insert({Operator::SetLogicEq, "==="});
    map.insert({Operator::SetLogicNe, "!=="});
    map.insert({Operator::SetInc, "++"});
    map.insert({Operator::SetDec, "--"});

    map.insert({Operator::As, "as"});
    map.insert({Operator::BitcastAs, "bitcast_as"});
    map.insert({Operator::Sizeof, "sizeof"});
    map.insert({Operator::Alignof, "alignof"});
    map.insert({Operator::Typeof, "typeof"});

    map.insert({Operator::In, "in"});
    map.insert({Operator::Out, "out"});
    map.insert({Operator::Dot, "."});
    map.insert({Operator::Range, ".."});
    map.insert({Operator::Ellipsis, "..."});
    map.insert({Operator::Scope, "::"});
    map.insert({Operator::Question, "?"});

    return mapping;
  }();

  [[nodiscard]] inline auto is_operator(std::string_view str) -> bool {
    return OPERATOR_MAP.right.find(str) != OPERATOR_MAP.right.end();
  }

  [[nodiscard]] inline auto operator_from_string(std::string_view str) -> std::optional<Operator> {
    auto it = OPERATOR_MAP.right.find(str);
    if (it != OPERATOR_MAP.right.end()) {
      return it->second;
    }

    return std::nullopt;
  }

  [[nodiscard]] inline auto operator_to_string(Operator op) -> std::string_view { return OPERATOR_MAP.left.at(op); }
}  // namespace nitrate::compiler::lexer
