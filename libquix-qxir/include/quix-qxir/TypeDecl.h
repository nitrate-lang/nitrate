////////////////////////////////////////////////////////////////////////////////
///                                                                          ///
///  ░▒▓██████▓▒░░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░ ░▒▓██████▓▒░  ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░        ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓██████▓▒░░▒▓█▓▒░      ░▒▓█▓▒░        ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░        ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ///
///  ░▒▓██████▓▒░ ░▒▓██████▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░ ░▒▓██████▓▒░  ///
///    ░▒▓█▓▒░                                                               ///
///     ░▒▓██▓▒░                                                             ///
///                                                                          ///
///   * QUIX LANG COMPILER - The official compiler for the Quix language.    ///
///   * Copyright (C) 2024 Wesley C. Jones                                   ///
///                                                                          ///
///   The QUIX Compiler Suite is free software; you can redistribute it or   ///
///   modify it under the terms of the GNU Lesser General Public             ///
///   License as published by the Free Software Foundation; either           ///
///   version 2.1 of the License, or (at your option) any later version.     ///
///                                                                          ///
///   The QUIX Compiler Suite is distributed in the hope that it will be     ///
///   useful, but WITHOUT ANY WARRANTY; without even the implied warranty of ///
///   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      ///
///   Lesser General Public License for more details.                        ///
///                                                                          ///
///   You should have received a copy of the GNU Lesser General Public       ///
///   License along with the QUIX Compiler Suite; if not, see                ///
///   <https://www.gnu.org/licenses/>.                                       ///
///                                                                          ///
////////////////////////////////////////////////////////////////////////////////

#ifndef __QUIX_QXIR_TYPE_DECL_H__
#define __QUIX_QXIR_TYPE_DECL_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Quixcc abstract syntax tree node.
 */
typedef struct qxir_node_t qxir_node_t;

/**
 * @brief Quixcc QXIR module.
 */
typedef struct qmodule_t qmodule_t;

/**
 * @brief Quixcc abstract syntax tree node type.
 */
typedef enum qxir_ty_t {
  QIR_NODE_BINEXPR,
  QIR_NODE_UNEXPR,
  QIR_NODE_POST_UNEXPR,

  QIR_NODE_INT,
  QIR_NODE_FLOAT,
  QIR_NODE_STRING,
  QIR_NODE_LIST,

  QIR_NODE_ALLOC,
  QIR_NODE_CALL,
  QIR_NODE_SEQ,
  QIR_NODE_ASYNC,
  QIR_NODE_INDEX,
  QIR_NODE_IDENT,
  QIR_NODE_GLOBAL,
  QIR_NODE_RET,
  QIR_NODE_BRK,
  QIR_NODE_CONT,
  QIR_NODE_IF,
  QIR_NODE_WHILE,
  QIR_NODE_FOR,
  QIR_NODE_FORM,
  QIR_NODE_FOREACH,
  QIR_NODE_CASE,
  QIR_NODE_SWITCH,
  QIR_NODE_FN,
  QIR_NODE_ASM,

  QIR_NODE_U1_TY,
  QIR_NODE_U8_TY,
  QIR_NODE_U16_TY,
  QIR_NODE_U32_TY,
  QIR_NODE_U64_TY,
  QIR_NODE_U128_TY,
  QIR_NODE_I8_TY,
  QIR_NODE_I16_TY,
  QIR_NODE_I32_TY,
  QIR_NODE_I64_TY,
  QIR_NODE_I128_TY,
  QIR_NODE_F16_TY,
  QIR_NODE_F32_TY,
  QIR_NODE_F64_TY,
  QIR_NODE_F128_TY,
  QIR_NODE_VOID_TY,
  QIR_NODE_PTR_TY,
  QIR_NODE_OPAQUE_TY,
  QIR_NODE_STRING_TY,
  QIR_NODE_STRUCT_TY,
  QIR_NODE_UNION_TY,
  QIR_NODE_ARRAY_TY,
  QIR_NODE_LIST_TY,
  QIR_NODE_INTRIN_TY,
  QIR_NODE_FN_TY,

  QIR_NODE_TMP, /* Temp node; must be resolved with more information */
  QIR_NODE_BAD,
} qxir_ty_t;

typedef struct qxir_conf_t qxir_conf_t;

typedef enum qxir_key_t {
  QQK_UNKNOWN = 0,
  QQK_CRASHGUARD,
  QQV_FASTERROR,
} qxir_key_t;

typedef enum qxir_val_t {
  QQV_UNKNOWN = 0,
  QQV_TRUE,
  QQV_FALSE,
  QQV_ON = QQV_TRUE,
  QQV_OFF = QQV_FALSE,
} qxir_val_t;

#ifdef __cplusplus
}
#endif

#endif  // __QUIX_QXIR_TYPE_DECL_H__
