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

#ifndef __NITRATE_AST_KIND_H__
#define __NITRATE_AST_KIND_H__

#include <cstdint>
#include <iosfwd>

namespace ncc::parse {
  enum ASTNodeKind : uint8_t {
    AST_DISCARDED, /* Deleted node */

    /*****************************************************************************
     * Expressions
     ****************************************************************************/
    AST_eBIN,           /* Binary expression */
    AST_eUNARY,         /* Unary expression */
    AST_eINT,           /* Integer literal */
    AST_eFLOAT,         /* Floating-point literal */
    AST_eSTRING,        /* String literal */
    AST_eCHAR,          /* Character literal */
    AST_eBOOL,          /* Boolean literal */
    AST_eNULL,          /* Null literal */
    AST_eUNDEF,         /* Undefined expression */
    AST_eCALL,          /* Function call */
    AST_eLIST,          /* List expression */
    AST_ePAIR,          /* Associative pair */
    AST_eINDEX,         /* Index access */
    AST_eSLICE,         /* Slice access */
    AST_eFSTRING,       /* Formatted string */
    AST_eIDENT,         /* Identifier */
    AST_eTEMPLATE_CALL, /* Template call */
    AST_eIMPORT,        /* Import expression */

    /*****************************************************************************
     * Types
     ****************************************************************************/

    AST_tU1,       /* 1-bit unsigned integer (boolean) */
    AST_tU8,       /* 8-bit unsigned integer */
    AST_tU16,      /* 16-bit unsigned integer */
    AST_tU32,      /* 32-bit unsigned integer */
    AST_tU64,      /* 64-bit unsigned integer */
    AST_tU128,     /* 128-bit unsigned integer */
    AST_tI8,       /* 8-bit signed integer */
    AST_tI16,      /* 16-bit signed integer */
    AST_tI32,      /* 32-bit signed integer */
    AST_tI64,      /* 64-bit signed integer */
    AST_tI128,     /* 128-bit signed integer */
    AST_tF16,      /* 16-bit floating-point number */
    AST_tF32,      /* 32-bit floating-point number */
    AST_tF64,      /* 64-bit floating-point number */
    AST_tF128,     /* 128-bit floating-point number */
    AST_tVOID,     /* Void type */
    AST_tINFER,    /* Inferred type */
    AST_tOPAQUE,   /* Opaque named type */
    AST_tNAMED,    /* Unresolved type name */
    AST_tREF,      /* Reference type */
    AST_tPTR,      /* Raw pointer type */
    AST_tARRAY,    /* Basic array type */
    AST_tTUPLE,    /* Tuple type */
    AST_tTEMPLATE, /* Template type */
    AST_tFUNCTION, /* Function type */

    /*****************************************************************************
     * Statements
     ****************************************************************************/

    AST_IF,         /* If statement */
    AST_RETIF,      /* Return-if statement */
    AST_SWITCH,     /* Switch statement */
    AST_CASE,       /* Case statement */
    AST_RETURN,     /* Return statement */
    AST_BREAK,      /* Break statement */
    AST_CONTINUE,   /* Continue statement */
    AST_WHILE,      /* While statement */
    AST_FOR,        /* For statement */
    AST_FOREACH,    /* Foreach statement */
    AST_INLINE_ASM, /* Inline assembly statement */
    AST_TYPEDEF,    /* Type alias declaration */
    AST_STRUCT,     /* Struct definition */
    AST_ENUM,       /* Enum definition */
    AST_SCOPE,      /* Namespace scope */
    AST_BLOCK,      /* Block statement */
    AST_EXPORT,     /* Export statement */
    AST_VAR,        /* Variable declaration */
    AST_FUNCTION,   /* Function definition */

    ///======================================================================

    AST__TYPE_FIRST = AST_tU1,
    AST__TYPE_LAST = AST_tFUNCTION,

    AST__STMT_FIRST = AST_IF,
    AST__STMT_LAST = AST_FUNCTION,

    AST__EXPR_FIRST = AST_DISCARDED,
    AST__EXPR_LAST = AST_FUNCTION,

    AST__FIRST = AST_DISCARDED,
    AST__LAST = AST_FUNCTION,
    AST__RANGE = AST__LAST - AST__FIRST,
  };

  auto operator<<(std::ostream& os, ASTNodeKind kind) -> std::ostream&;
}  // namespace ncc::parse

#endif
