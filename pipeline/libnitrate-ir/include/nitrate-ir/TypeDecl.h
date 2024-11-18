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
  QIR_NODE_BINEXPR,     /* Binary expression */
  QIR_NODE_UNEXPR,      /* Unary expression */
  QIR_NODE_POST_UNEXPR, /* Postfix unary expression */

  QIR_NODE_INT,   /* Integer literal */
  QIR_NODE_FLOAT, /* Floating-point literal */
  QIR_NODE_LIST,  /* List literal */

  QIR_NODE_CALL,   /* Function call */
  QIR_NODE_SEQ,    /* Sequence */
  QIR_NODE_INDEX,  /* Index / member access */
  QIR_NODE_IDENT,  /* Identifier */
  QIR_NODE_EXTERN, /* Linker visibility modifier */
  QIR_NODE_LOCAL,  /* Variable declaration */
  QIR_NODE_RET,    /* Return statement */
  QIR_NODE_BRK,    /* Break statement */
  QIR_NODE_CONT,   /* Continue statement */
  QIR_NODE_IF,     /* If statement */
  QIR_NODE_WHILE,  /* While loop */
  QIR_NODE_FOR,    /* For loop */
  QIR_NODE_CASE,   /* Case statement */
  QIR_NODE_SWITCH, /* Switch statement */
  QIR_NODE_FN,     /* Function definition */
  QIR_NODE_ASM,    /* Inline assembly */
  QIR_NODE_IGN,    /* No-op */

  QIR_NODE_U1_TY,     /* 1-bit unsigned integer (boolean) */
  QIR_NODE_U8_TY,     /* 8-bit unsigned integer */
  QIR_NODE_U16_TY,    /* 16-bit unsigned integer */
  QIR_NODE_U32_TY,    /* 32-bit unsigned integer */
  QIR_NODE_U64_TY,    /* 64-bit unsigned integer */
  QIR_NODE_U128_TY,   /* 128-bit unsigned integer */
  QIR_NODE_I8_TY,     /* 8-bit signed integer */
  QIR_NODE_I16_TY,    /* 16-bit signed integer */
  QIR_NODE_I32_TY,    /* 32-bit signed integer */
  QIR_NODE_I64_TY,    /* 64-bit signed integer */
  QIR_NODE_I128_TY,   /* 128-bit signed integer */
  QIR_NODE_F16_TY,    /* 16-bit floating-point */
  QIR_NODE_F32_TY,    /* 32-bit floating-point */
  QIR_NODE_F64_TY,    /* 64-bit floating-point */
  QIR_NODE_F128_TY,   /* 128-bit floating-point */
  QIR_NODE_VOID_TY,   /* Void type */
  QIR_NODE_PTR_TY,    /* Pointer type */
  QIR_NODE_OPAQUE_TY, /* Opaque type */
  QIR_NODE_STRUCT_TY, /* Struct type */
  QIR_NODE_UNION_TY,  /* Union type */
  QIR_NODE_ARRAY_TY,  /* Array type */
  QIR_NODE_FN_TY,     /* Function type */

  QIR_NODE_TMP, /* Temp node; must be resolved with more information */
} nr_ty_t;

#define QIR_NODE_COUNT 46

typedef struct nr_conf_t nr_conf_t;

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
