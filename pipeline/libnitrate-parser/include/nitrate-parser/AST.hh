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
    /*****************************************************************************
     * Expressions
     ****************************************************************************/
    QAST_BINEXPR,    /* Binary expression */
    QAST_UNEXPR,     /* Unary expression */
    QAST_TEREXPR,    /* Ternary expression */
    QAST_INT,        /* Integer literal */
    QAST_FLOAT,      /* Floating-point literal */
    QAST_STRING,     /* String literal */
    QAST_CHAR,       /* Character literal */
    QAST_BOOL,       /* Boolean literal */
    QAST_NULL,       /* Null literal */
    QAST_UNDEF,      /* Undefined expression */
    QAST_CALL,       /* Function call */
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

    QAST_U1,       /* 1-bit unsigned integer (boolean) */
    QAST_U8,       /* 8-bit unsigned integer */
    QAST_U16,      /* 16-bit unsigned integer */
    QAST_U32,      /* 32-bit unsigned integer */
    QAST_U64,      /* 64-bit unsigned integer */
    QAST_U128,     /* 128-bit unsigned integer */
    QAST_I8,       /* 8-bit signed integer */
    QAST_I16,      /* 16-bit signed integer */
    QAST_I32,      /* 32-bit signed integer */
    QAST_I64,      /* 64-bit signed integer */
    QAST_I128,     /* 128-bit signed integer */
    QAST_F16,      /* 16-bit floating-point number */
    QAST_F32,      /* 32-bit floating-point number */
    QAST_F64,      /* 64-bit floating-point number */
    QAST_F128,     /* 128-bit floating-point number */
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

    QAST__TYPE_FIRST = QAST_U1,
    QAST__TYPE_LAST = QAST_FUNCTOR,

    QAST__STMT_FIRST = QAST_IF,
    QAST__STMT_LAST = QAST_FUNCTION,

    QAST__EXPR_FIRST = QAST_BINEXPR,
    QAST__EXPR_LAST = QAST_FUNCTION,

    QAST__FIRST = QAST_BINEXPR,
    QAST__LAST = QAST_FUNCTION,
    QAST__RANGE = QAST__LAST - QAST__FIRST,
  };

  std::ostream& operator<<(std::ostream& os, ASTNodeKind kind);
}  // namespace ncc::parse

#endif
