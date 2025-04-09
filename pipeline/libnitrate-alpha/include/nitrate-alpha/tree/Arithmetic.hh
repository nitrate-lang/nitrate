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

#pragma once

#include <cstdint>

namespace ncc::alpha::tree {
  enum class Op : uint8_t {
    Add,        /* Addition */
    Sub,        /* Subtraction */
    Mul,        /* Multiplication */
    sDiv,       /* Signed division */
    uDiv,       /* Unsigned division */
    sMod,       /* Signed modulus */
    uMod,       /* Unsigned modulus */
    BitAnd,     /* Bitwise AND */
    BitOr,      /* Bitwise OR */
    BitXor,     /* Bitwise XOR */
    BitNot,     /* Bitwise NOT */
    LShift,     /* Left shift */
    sRShift,    /* Signed shift */
    uRShift,    /* Unsigned right shift */
    LRotate,    /* Rotate left */
    RRotate,    /* Rotate right */
    LogicAnd,   /* Logical AND */
    LogicOr,    /* Logical OR */
    LogicXor,   /* Logical XOR */
    LogicNot,   /* Logical NOT */
    LT,         /* Less than */
    GT,         /* Greater than */
    LE,         /* Less than or equal to */
    GE,         /* Greater than or equal to */
    Eq,         /* Equal to */
    NE,         /* Not equal to */
    Set,        /* Assignment */
    BitcastAs,  /* Bitcast */
    Bitsizeof,  /* Bit size of */
    Bitalignof, /* Bit alignment of */
  };
}
