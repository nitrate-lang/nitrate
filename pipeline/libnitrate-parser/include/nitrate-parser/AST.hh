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
    QAST_DISCARDED, /* Deleted node */

    /*****************************************************************************
     * Expressions
     ****************************************************************************/
    AST_eBIN,        /* Binary expression */
    AST_eUNARY,      /* Unary expression */
    QAST_TEREXPR,    /* Ternary expression */
    AST_eINT,        /* Integer literal */
    AST_eFLOAT,      /* Floating-point literal */
    AST_eSTRING,     /* String literal */
    AST_eCHAR,       /* Character literal */
    AST_eBOOL,       /* Boolean literal */
    AST_eNULL,       /* Null literal */
    AST_eUNDEF,      /* Undefined expression */
    AST_eCALL,       /* Function call */
    QAST_LIST,       /* List expression */
    QAST_ASSOC,      /* Associative pair */
    QAST_INDEX,      /* Index access */
    QAST_SLICE,      /* Slice access */
    QAST_FSTRING,    /* Formatted string */
    QAST_IDENT,      /* Identifier */
    QAST_TEMPL_CALL, /* Template call */
    QAST_IMPORT,     /* Import expression */

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
    QAST_VOID,     /* Void type */
    QAST_INFER,    /* Inferred type */
    QAST_OPAQUE,   /* Opaque named type */
    QAST_NAMED,    /* Unresolved type name */
    QAST_REF,      /* Reference type */
    QAST_PTR,      /* Raw pointer type */
    QAST_ARRAY,    /* Basic array type */
    QAST_TUPLE,    /* Tuple type */
    QAST_TEMPLATE, /* Template type */
    QAST_FUNCTOR,  /* Function type */

    /*****************************************************************************
     * Statements
     ****************************************************************************/

    QAST_IF,         /* If statement */
    QAST_RETIF,      /* Return-if statement */
    QAST_SWITCH,     /* Switch statement */
    QAST_CASE,       /* Case statement */
    QAST_RETURN,     /* Return statement */
    QAST_BREAK,      /* Break statement */
    QAST_CONTINUE,   /* Continue statement */
    QAST_WHILE,      /* While statement */
    QAST_FOR,        /* For statement */
    QAST_FOREACH,    /* Foreach statement */
    QAST_INLINE_ASM, /* Inline assembly statement */
    QAST_TYPEDEF,    /* Type alias declaration */
    QAST_STRUCT,     /* Struct definition */
    QAST_ENUM,       /* Enum definition */
    QAST_SCOPE,      /* Namespace scope */
    QAST_BLOCK,      /* Block statement */
    QAST_EXPORT,     /* Export statement */
    QAST_VAR,        /* Variable declaration */
    QAST_FUNCTION,   /* Function definition */

    ///======================================================================

    QAST__TYPE_FIRST = AST_tU1,
    QAST__TYPE_LAST = QAST_FUNCTOR,

    QAST__STMT_FIRST = QAST_IF,
    QAST__STMT_LAST = QAST_FUNCTION,

    QAST__EXPR_FIRST = QAST_DISCARDED,
    QAST__EXPR_LAST = QAST_FUNCTION,

    QAST__FIRST = QAST_DISCARDED,
    QAST__LAST = QAST_FUNCTION,
    QAST__RANGE = QAST__LAST - QAST__FIRST,
  };

  auto operator<<(std::ostream& os, ASTNodeKind kind) -> std::ostream&;
}  // namespace ncc::parse

#endif
