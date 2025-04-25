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

    AST_sIF,       /* If statement */
    AST_sSWITCH,   /* Switch statement */
    AST_sCASE,     /* Case statement */
    AST_sRET,      /* Return statement */
    AST_sBRK,      /* Break statement */
    AST_sCONT,     /* Continue statement */
    AST_sWHILE,    /* While statement */
    AST_sFOR,      /* For statement */
    AST_sFOREACH,  /* Foreach statement */
    AST_sASM,      /* Inline assembly statement */
    AST_sTYPEDEF,  /* Type alias declaration */
    AST_sSTRUCT,   /* Struct definition */
    AST_sENUM,     /* Enum definition */
    AST_sSCOPE,    /* Namespace scope */
    AST_sBLOCK,    /* Block statement */
    AST_sEXPORT,   /* Export statement */
    AST_sVAR,      /* Variable declaration */
    AST_sFUNCTION, /* Function definition/declaration */

    ///======================================================================

    AST__TYPE_FIRST = AST_tINFER,
    AST__TYPE_LAST = AST_tFUNCTION,

    AST__STMT_FIRST = AST_sIF,
    AST__STMT_LAST = AST_sFUNCTION,

    AST__EXPR_FIRST = AST_DISCARDED,
    AST__EXPR_LAST = AST_sFUNCTION,

    AST__FIRST = AST_DISCARDED,
    AST__LAST = AST_sFUNCTION,
    AST__RANGE = AST__LAST - AST__FIRST,
  };

  auto operator<<(std::ostream& os, ASTNodeKind kind) -> std::ostream&;
}  // namespace ncc::parse

#endif
