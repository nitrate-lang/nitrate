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

#ifndef __NITRATE_NR_TYPE_DECL_H__
#define __NITRATE_NR_TYPE_DECL_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Nitrate abstract syntax tree node.
 */
typedef struct nr_node_t nr_node_t;

/**
 * @brief Nitrate NR module.
 */
typedef struct qmodule_t qmodule_t;

/**
 * @brief Nitrate abstract syntax tree node type.
 */
typedef enum nr_ty_t {
  NR_NODE_BINEXPR,     /* Binary expression */
  NR_NODE_UNEXPR,      /* Unary expression */
  NR_NODE_POST_UNEXPR, /* Postfix unary expression */

  NR_NODE_INT,   /* Integer literal */
  NR_NODE_FLOAT, /* Floating-point literal */
  NR_NODE_LIST,  /* List literal */

  NR_NODE_CALL,   /* Function call */
  NR_NODE_SEQ,    /* Sequence */
  NR_NODE_INDEX,  /* Index / member access */
  NR_NODE_IDENT,  /* Identifier */
  NR_NODE_EXTERN, /* Linker visibility modifier */
  NR_NODE_LOCAL,  /* Variable declaration */
  NR_NODE_RET,    /* Return statement */
  NR_NODE_BRK,    /* Break statement */
  NR_NODE_CONT,   /* Continue statement */
  NR_NODE_IF,     /* If statement */
  NR_NODE_WHILE,  /* While loop */
  NR_NODE_FOR,    /* For loop */
  NR_NODE_CASE,   /* Case statement */
  NR_NODE_SWITCH, /* Switch statement */
  NR_NODE_FN,     /* Function definition */
  NR_NODE_ASM,    /* Inline assembly */
  NR_NODE_IGN,    /* No-op */

  NR_NODE_U1_TY,     /* 1-bit unsigned integer (boolean) */
  NR_NODE_U8_TY,     /* 8-bit unsigned integer */
  NR_NODE_U16_TY,    /* 16-bit unsigned integer */
  NR_NODE_U32_TY,    /* 32-bit unsigned integer */
  NR_NODE_U64_TY,    /* 64-bit unsigned integer */
  NR_NODE_U128_TY,   /* 128-bit unsigned integer */
  NR_NODE_I8_TY,     /* 8-bit signed integer */
  NR_NODE_I16_TY,    /* 16-bit signed integer */
  NR_NODE_I32_TY,    /* 32-bit signed integer */
  NR_NODE_I64_TY,    /* 64-bit signed integer */
  NR_NODE_I128_TY,   /* 128-bit signed integer */
  NR_NODE_F16_TY,    /* 16-bit floating-point */
  NR_NODE_F32_TY,    /* 32-bit floating-point */
  NR_NODE_F64_TY,    /* 64-bit floating-point */
  NR_NODE_F128_TY,   /* 128-bit floating-point */
  NR_NODE_VOID_TY,   /* Void type */
  NR_NODE_PTR_TY,    /* Pointer type */
  NR_NODE_OPAQUE_TY, /* Opaque type */
  NR_NODE_STRUCT_TY, /* Struct type */
  NR_NODE_UNION_TY,  /* Union type */
  NR_NODE_ARRAY_TY,  /* Array type */
  NR_NODE_FN_TY,     /* Function type */
  NR_NODE_CONST_TY,  /* Constant wrapper type */

  NR_NODE_TMP, /* Temp node; must be resolved with more information */

  NR_NODE_FIRST = NR_NODE_BINEXPR,
  NR_NODE_LAST = NR_NODE_TMP,
} nr_ty_t;

#define NR_NODE_COUNT (NR_NODE_LAST - NR_NODE_FIRST + 1)

typedef enum nr_key_t {
  QQK_UNKNOWN = 0,
  QQK_CRASHGUARD,
  QQV_FASTERROR,
} nr_key_t;

typedef enum nr_val_t {
  QQV_UNKNOWN = 0,
  QQV_TRUE,
  QQV_FALSE,
  QQV_ON = QQV_TRUE,
  QQV_OFF = QQV_FALSE,
} nr_val_t;

#ifdef __cplusplus
}
#endif

#endif  // __NITRATE_NR_TYPE_DECL_H__
