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

#ifndef __QUIXCC_TYPES_TOKENS_H__
#define __QUIXCC_TYPES__H____QUIXCC_TYPES_TOKENS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef enum {
  QUIXCC_LEX_EOF = 0,
  QUIXCC_LEX_UNK = 1,
  QUIXCC_LEX_IDENT = 2,
  QUIXCC_LEX_KW = 3,
  QUIXCC_LEX_OP = 4,
  QUIXCC_LEX_PUNCT = 5,
  QUIXCC_LEX_INT = 6,
  QUIXCC_LEX_FLOAT = 7,
  QUIXCC_LEX_STR = 8,
  QUIXCC_LEX_CHAR = 9,
  QUIXCC_LEX_METABLK = 10,
  QUIXCC_LEX_METASEG = 11,
  QUIXCC_LEX_NOTE = 12,
} quixcc_lex_type_t;

typedef enum {
  QUIXCC_KW_SUBSYSTEM = 10,
  QUIXCC_KW_IMPORT = 11,
  QUIXCC_KW_PUB = 12,
  QUIXCC_KW_SEC = 13,
  QUIXCC_KW_PRO = 14,

  QUIXCC_KW_TYPE = 20,
  QUIXCC_KW_LET = 21,
  QUIXCC_KW_VAR = 22,
  QUIXCC_KW_CONST = 23,
  QUIXCC_KW_STATIC = 24,

  QUIXCC_KW_STRUCT = 40,
  QUIXCC_KW_REGION = 41,
  QUIXCC_KW_GROUP = 42,
  QUIXCC_KW_UNION = 43,
  QUIXCC_KW_OPAQUE = 44,
  QUIXCC_KW_ENUM = 45,
  QUIXCC_KW_FSTRING = 46,
  QUIXCC_KW_IMPL = 47,

  QUIXCC_KW_FN = 60,
  QUIXCC_KW_NOTHROW = 61,
  QUIXCC_KW_FOREIGN = 62,
  QUIXCC_KW_IMPURE = 63,
  QUIXCC_KW_TSAFE = 64,
  QUIXCC_KW_PURE = 65,
  QUIXCC_KW_QUASIPURE = 66,
  QUIXCC_KW_RETROPURE = 67,
  QUIXCC_KW_CRASHPOINT = 68,
  QUIXCC_KW_INLINE = 69,
  QUIXCC_KW_UNSAFE = 70,
  QUIXCC_KW_SAFE = 71,
  QUIXCC_KW_IN = 72,
  QUIXCC_KW_OUT = 73,
  QUIXCC_KW_REQ = 74,

  QUIXCC_KW_IF = 90,
  QUIXCC_KW_ELSE = 91,
  QUIXCC_KW_FOR = 92,
  QUIXCC_KW_WHILE = 93,
  QUIXCC_KW_DO = 94,
  QUIXCC_KW_SWITCH = 95,
  QUIXCC_KW_CASE = 96,
  QUIXCC_KW_DEFAULT = 97,
  QUIXCC_KW_BREAK = 98,
  QUIXCC_KW_CONTINUE = 99,
  QUIXCC_KW_RETURN = 100,
  QUIXCC_KW_RETIF = 101,
  QUIXCC_KW_RETZ = 102,
  QUIXCC_KW_RETV = 103,
  QUIXCC_KW_FORM = 104,
  QUIXCC_KW_FOREACH = 105,

  QUIXCC_KW_ASM = 130,

  QUIXCC_KW_VOID = 140,
  QUIXCC_KW_UNDEF = 141,
  QUIXCC_KW_NULL = 142,
  QUIXCC_KW_TRUE = 143,
  QUIXCC_KW_FALSE = 144,
} quixcc_lex_kw_t;

typedef enum {
  QUIXCC_PUNCT_OPEN_PAREN = 1,
  QUIXCC_PUNCT_CLOSE_PAREN = 2,
  QUIXCC_PUNCT_OPEN_BRACE = 3,
  QUIXCC_PUNCT_CLOSE_BRACE = 4,
  QUIXCC_PUNCT_OPEN_BRACKET = 5,
  QUIXCC_PUNCT_CLOSE_BRACKET = 6,
  QUIXCC_PUNCT_COMMA = 8,
  QUIXCC_PUNCT_COLON = 9,
  QUIXCC_PUNCT_SEMICOLON = 10,
} quixcc_lex_punct_t;

typedef enum {
  QUIXCC_OP_AT = 1,
  QUIXCC_OP_TERNARY = 2,
  QUIXCC_OP_ARROW = 3,
  QUIXCC_OP_DOT = 4,

  QUIXCC_OP_PLUS = 10,
  QUIXCC_OP_MINUS = 11,
  QUIXCC_OP_MUL = 12,
  QUIXCC_OP_DIV = 13,
  QUIXCC_OP_MOD = 14,

  QUIXCC_OP_BIT_AND = 20,
  QUIXCC_OP_BIT_OR = 21,
  QUIXCC_OP_BIT_XOR = 22,
  QUIXCC_OP_BIT_NOT = 23,
  QUIXCC_OP_SHL = 24,
  QUIXCC_OP_SHR = 25,
  QUIXCC_OP_ROTR = 26,
  QUIXCC_OP_ROTL = 27,

  QUIXCC_OP_INC = 30,
  QUIXCC_OP_DEC = 31,
  QUIXCC_OP_ASSIGN = 32,
  QUIXCC_OP_PLUS_ASSIGN = 33,
  QUIXCC_OP_MINUS_ASSIGN = 34,
  QUIXCC_OP_MUL_ASSIGN = 35,
  QUIXCC_OP_DIV_ASSIGN = 36,
  QUIXCC_OP_MOD_ASSIGN = 37,
  QUIXCC_OP_BIT_OR_ASSIGN = 38,
  QUIXCC_OP_BIT_AND_ASSIGN = 39,
  QUIXCC_OP_BIT_XOR_ASSIGN = 40,
  QUIXCC_OP_XOR_ASSIGN = 41,
  QUIXCC_OP_OR_ASSIGN = 42,
  QUIXCC_OP_AND_ASSIGN = 43,
  QUIXCC_OP_SHL_ASSIGN = 44,
  QUIXCC_OP_SHR_ASSIGN = 45,

  QUIXCC_OP_NOT = 50,
  QUIXCC_OP_AND = 51,
  QUIXCC_OP_OR = 52,
  QUIXCC_OP_XOR = 53,

  QUIXCC_OP_LT = 60,
  QUIXCC_OP_GT = 61,
  QUIXCC_OP_LE = 62,
  QUIXCC_OP_GE = 63,
  QUIXCC_OP_EQ = 64,
  QUIXCC_OP_NE = 65,

  QUIXCC_OP_AS = 80,
  QUIXCC_OP_IS = 81,
  QUIXCC_OP_IN = 82,
  QUIXCC_OP_NOTIN = 83,
  QUIXCC_OP_SIZEOF = 84,
  QUIXCC_OP_ALIGNOF = 85,
  QUIXCC_OP_TYPEOF = 86,
  QUIXCC_OP_OFFSETOF = 87,
  QUIXCC_OP_RANGE = 88,
  QUIXCC_OP_ELLIPSIS = 89,
  QUIXCC_OP_SPACESHIP = 90,
} quixcc_lex_op_t;

typedef struct quixcc_lex_loc_t {
  uint32_t line;
  uint32_t column : 24;
  const char* file;
} __attribute__((packed)) quixcc_lex_loc_t;

typedef struct quixcc_tok_t {
  quixcc_lex_loc_t loc;
  union {
    quixcc_lex_op_t op;
    quixcc_lex_punct_t punct;
    quixcc_lex_kw_t kw;
    const char* data;
  } val;
  quixcc_lex_type_t ty : 8;
} __attribute__((packed)) quixcc_tok_t;

#define QUIXCC_TOK_SIZE sizeof(quixcc_tok_t)

#ifdef __cplusplus
}
#endif

#endif  // __QUIXCC_TYPES_TOKENS_H__
