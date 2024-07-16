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

#ifndef __QUIXCC_TYPES_SYNTAXTREENODES_H__
#define __QUIXCC_TYPES_SYNTAXTREENODES_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <quixcc/core/Arena.h>
#include <quixcc/interface/Types.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

///=============================================================================
/// BEGIN: ABSTRACT SYNTAX TREE DATA TYPES
///=============================================================================

/**
 * @brief Quixcc abstract syntax tree node.
 */
typedef struct quixcc_ast_node_t quixcc_ast_node_t;

/**
 * @brief Quixcc abstract syntax tree node type.
 */
typedef enum quixcc_ast_ntype_t {
  QUIXCC_AST_NODE_STMT = 001,
  QUIXCC_AST_NODE_TYPE = 002,
  QUIXCC_AST_NODE_DECL = 003,
  QUIXCC_AST_NODE_EXPR = 004,
  QUIXCC_AST_NODE_CEXPR = 010,

  QUIXCC_AST_NODE_BINEXPR = 100,
  QUIXCC_AST_NODE_UNEXPR = 101,
  QUIXCC_AST_NODE_TEREXPR = 102,
  QUIXCC_AST_NODE_INT = 400,
  QUIXCC_AST_NODE_FLOAT = 401,
  QUIXCC_AST_NODE_STRING = 402,
  QUIXCC_AST_NODE_CHAR = 403,
  QUIXCC_AST_NODE_BOOL = 404,
  QUIXCC_AST_NODE_NULL = 405,
  QUIXCC_AST_NODE_UNDEF = 406,
  QUIXCC_AST_NODE_CALL = 116,
  QUIXCC_AST_NODE_LIST = 117,
  QUIXCC_AST_NODE_ASSOC = 118,
  QUIXCC_AST_NODE_FIELD = 119,
  QUIXCC_AST_NODE_INDEX = 120,
  QUIXCC_AST_NODE_SLICE = 121,
  QUIXCC_AST_NODE_FSTRING = 122,
  QUIXCC_AST_NODE_IDENT = 123,

  QUIXCC_AST_NODE_MUT_TY = 300,
  QUIXCC_AST_NODE_U1_TY = 301,
  QUIXCC_AST_NODE_U8_TY = 302,
  QUIXCC_AST_NODE_U16_TY = 303,
  QUIXCC_AST_NODE_U32_TY = 304,
  QUIXCC_AST_NODE_U64_TY = 305,
  QUIXCC_AST_NODE_U128_TY = 306,
  QUIXCC_AST_NODE_I8_TY = 307,
  QUIXCC_AST_NODE_I16_TY = 308,
  QUIXCC_AST_NODE_I32_TY = 309,
  QUIXCC_AST_NODE_I64_TY = 310,
  QUIXCC_AST_NODE_I128_TY = 311,
  QUIXCC_AST_NODE_F32_TY = 312,
  QUIXCC_AST_NODE_F64_TY = 313,
  QUIXCC_AST_NODE_VOID_TY = 314,
  QUIXCC_AST_NODE_PTR_TY = 315,
  QUIXCC_AST_NODE_OPAQUE_TY = 316,
  QUIXCC_AST_NODE_STRING_TY = 317,
  QUIXCC_AST_NODE_ENUM_TY = 318,
  QUIXCC_AST_NODE_STRUCT_TY = 319,
  QUIXCC_AST_NODE_GROUP_TY = 320,
  QUIXCC_AST_NODE_REGION_TY = 321,
  QUIXCC_AST_NODE_UNION_TY = 322,
  QUIXCC_AST_NODE_ARRAY_TY = 323,
  QUIXCC_AST_NODE_VECTOR_TY = 324,
  QUIXCC_AST_NODE_MAP_TY = 325,
  QUIXCC_AST_NODE_TUPLE_TY = 326,
  QUIXCC_AST_NODE_SET_TY = 327,
  QUIXCC_AST_NODE_RESULT_TY = 328,
  QUIXCC_AST_NODE_FN_TY = 329,
  QUIXCC_AST_NODE_UNRES_TY = 330,

  QUIXCC_AST_NODE_TYPEDEF = 500,
  QUIXCC_AST_NODE_FNDECL = 501,
  QUIXCC_AST_NODE_STRUCT = 502,
  QUIXCC_AST_NODE_REGION = 504,
  QUIXCC_AST_NODE_GROUP = 506,
  QUIXCC_AST_NODE_UNION = 508,
  QUIXCC_AST_NODE_ENUM = 510,
  QUIXCC_AST_NODE_FN = 512,
  QUIXCC_AST_NODE_SUBSYSTEM = 514,
  QUIXCC_AST_NODE_EXPORT = 515,
  QUIXCC_AST_NODE_COMPOSITE_FIELD = 516,

  QUIXCC_AST_NODE_BLOCK = 700,
  QUIXCC_AST_NODE_CONST = 701,
  QUIXCC_AST_NODE_VAR = 702,
  QUIXCC_AST_NODE_LET = 703,
  QUIXCC_AST_NODE_INLINE_ASM = 704,
  QUIXCC_AST_NODE_RETURN = 705,
  QUIXCC_AST_NODE_RETIF = 706,
  QUIXCC_AST_NODE_RETZ = 707,
  QUIXCC_AST_NODE_RETV = 708,
  QUIXCC_AST_NODE_BREAK = 709,
  QUIXCC_AST_NODE_CONTINUE = 710,
  QUIXCC_AST_NODE_IF = 711,
  QUIXCC_AST_NODE_WHILE = 712,
  QUIXCC_AST_NODE_FOR = 713,
  QUIXCC_AST_NODE_FORM = 714,
  QUIXCC_AST_NODE_FOREACH = 715,
  QUIXCC_AST_NODE_CASE = 716,
  QUIXCC_AST_NODE_SWITCH = 717,
} quixcc_ast_ntype_t;

#define QUIXCC_AST_NODE_COUNT 83

typedef enum quixcc_ast_ftype_t {
  QUIXCC_AST_FIELD_BOOL = 0,
  QUIXCC_AST_FIELD_CHAR = 1,
  QUIXCC_AST_FIELD_INT8 = 2,
  QUIXCC_AST_FIELD_INT16 = 3,
  QUIXCC_AST_FIELD_INT32 = 4,
  QUIXCC_AST_FIELD_INT64 = 5,
  QUIXCC_AST_FIELD_UINT8 = 6,
  QUIXCC_AST_FIELD_UINT16 = 7,
  QUIXCC_AST_FIELD_UINT32 = 8,
  QUIXCC_AST_FIELD_UINT64 = 9,
  QUIXCC_AST_FIELD_FLOAT = 10,
  QUIXCC_AST_FIELD_DOUBLE = 11,
  QUIXCC_AST_FIELD_QSTRING = 12,
  QUIXCC_AST_FIELD_QVEC = 13
} quixcc_ast_ftype_t;

///=============================================================================
/// END: ABSTRACT SYNTAX TREE DATA TYPES
///=============================================================================

///=============================================================================
/// BEGIN: ABSTRACT SYNTAX TREE PROPERTY ACCESSORS
///=============================================================================

/**
 * @brief Returns the type number of a node.
 *
 * @param node The node
 *
 * @return The type number of the node.
 *
 * @note This function is thread-safe.
 * @note This function is fast.
 * @note This function will always succeed for valid nodes.
 */
quixcc_ast_ntype_t quixcc_ast_typeof(const quixcc_ast_node_t *node);

/**
 * @brief Returns the current size of the node object in memory
 * 
 * @param node The node
 * 
 * @return The size of the node in memory (only the node itself, not it recursive memory usage)
 */
size_t quixcc_ast_sizeof(const quixcc_ast_node_t *node);

/**
 * @brief Translates a type number to a string representation.
 *
 * @param type The type number.
 *
 * @return The string representation of the type number.
 *
 * @note This function is thread-safe.
 * @note This function is fast.
 * @note This function will always succeed for valid type numbers.
 * @warning Do not free the returned string.
 * @note The function is pure, i.e. each call with the same argument will
 * return the same pointer to the same string.
 */
const char *quixcc_ast_ntype_name(quixcc_ast_ntype_t type);

/**
 * @brief Returns the number of child nodes contains as fields
 *        for a given node type.
 *
 * @param type The type number.
 *
 * @return The number of child nodes.
 *
 * @note The return value indicates how many walkable subfields a node directly
 * contains. So for a fictious example, a `BinExpr` could contain 3 members
 * (including the operator), but only 2 walkable subfields (the lhs and rhs). So
 * 2 would be returned in this case.
 * @note This function is thread-safe.
 */
uint16_t quixcc_ast_subcount(quixcc_ast_ntype_t type);

/**
 * @brief Returns the field type of a field within a node.
 *
 * @param node The node.
 * @param index The index of the field.
 *
 * @return The field type of the field within the node argument.
 *
 * @note If the node is invalid, the behavior is undefined.
 * @warning If the index is out of bounds for the given node, a panic will
 * occur.
 * @note This function is thread-safe.
 */
quixcc_ast_ftype_t quixcc_ast_field_type(quixcc_ast_node_t *node, uint16_t index);

/**
 * @brief Returns the name of a field within a node.
 *
 * @param node The node.
 * @param index The index of the field.
 *
 * @return The name of the field within the node argument.
 *
 * @note If the node is invalid, the behavior is undefined.
 * @warning If the index is out of bounds for the given node, a panic will
 * occur.
 * @note Do not free the returned string.
 * @note This function is thread-safe.
 */
const char *quixcc_ast_field_name(quixcc_ast_node_t *node, uint16_t index);

/**
 * @brief Returns the index of a field within a node.
 *
 * @param node The node.
 * @param name The name of the field.
 *
 * @return The index of the field within the node argument.
 *
 * @note If the node is invalid, the behavior is undefined.
 * @warning If the name is not found in the given node, a panic will occur.
 * @note This function is thread-safe.
 */
uint16_t quixcc_ast_field_index(quixcc_ast_node_t *node, const char *name);

/**
 * @brief Convenience for `quixcc_ast_subcount` for a pointer to a
 * node.
 *
 * @param node The node.
 *
 * @return The number of child nodes.
 *
 * @note This function is thread-safe.
 */
static inline uint16_t quixcc_ast_subcount_of(const quixcc_ast_node_t *node) {
  return quixcc_ast_subcount(quixcc_ast_typeof(node));
}

/**
 * @brief Returns a pointer to a child node within a node.
 *
 * @param node The node.
 * @param index The index of the child node.
 *
 * @return Pointer to the child node within the node argument.
 *
 * @note If the node is invalid, the behavior is undefined.
 * @warning If the index is out of bounds for the given node, a panic will
 * occur.
 * @note This function is thread-safe.
 * @note This function will never return `NULL`.
 */
quixcc_ast_node_t **quixcc_ast_get_child(quixcc_ast_node_t *node, uint16_t index);

/**
 * @brief Returns a pointer to a field within a node.
 *
 * @param node The node.
 * @param index The index of the field.
 * @param ftype The field type.
 *
 * @return Pointer to the field within the node argument.
 *
 * @note If the node is invalid, the behavior is undefined.
 * @warning If the index is out of bounds for the given node, a panic will
 * occur.
 * @warning If the field type is invalid for the given node, a panic will
 * occur.
 * @note This function is thread-safe.
 * @note This function will never return `NULL`.
 * @warning This function is dangerous when used directly. Use the type-safe
 * macros instead.
 */
void *quixcc_ast_get(const quixcc_ast_node_t *node, uint16_t index, quixcc_ast_ftype_t ftype);

#define QUIXCC_AST_GET_BOOL(__node, __index)                                                       \
  (*(bool *)quixcc_ast_get((__node), (__index), QUIXCC_AST_FIELD_BOOL))

#define QUIXCC_AST_GET_CHAR(__node, __index)                                                       \
  (*(char *)quixcc_ast_get((__node), (__index), QUIXCC_AST_FIELD_CHAR))

#define QUIXCC_AST_GET_INT8(__node, __index)                                                       \
  (*(int8_t *)quixcc_ast_get((__node), (__index), QUIXCC_AST_FIELD_INT8))

#define QUIXCC_AST_GET_INT16(__node, __index)                                                      \
  (*(int16_t *)quixcc_ast_get((__node), (__index), QUIXCC_AST_FIELD_INT16))

#define QUIXCC_AST_GET_INT32(__node, __index)                                                      \
  (*(int32_t *)quixcc_ast_get((__node), (__index), QUIXCC_AST_FIELD_INT32))

#define QUIXCC_AST_GET_INT64(__node, __index)                                                      \
  (*(int64_t *)quixcc_ast_get((__node), (__index), QUIXCC_AST_FIELD_INT64))

#define QUIXCC_AST_GET_UINT8(__node, __index)                                                      \
  (*(uint8_t *)quixcc_ast_get((__node), (__index), QUIXCC_AST_FIELD_UINT8))

#define QUIXCC_AST_GET_UINT16(__node, __index)                                                     \
  (*(uint16_t *)quixcc_ast_get((__node), (__index), QUIXCC_AST_FIELD_UINT16))

#define QUIXCC_AST_GET_UINT32(__node, __index)                                                     \
  (*(uint32_t *)quixcc_ast_get((__node), (__index), QUIXCC_AST_FIELD_UINT32))

#define QUIXCC_AST_GET_UINT64(__node, __index)                                                     \
  (*(uint64_t *)quixcc_ast_get((__node), (__index), QUIXCC_AST_FIELD_UINT64))

#define QUIXCC_AST_GET_FLOAT(__node, __index)                                                      \
  (*(float *)quixcc_ast_get((__node), (__index), QUIXCC_AST_FIELD_FLOAT))

#define QUIXCC_AST_GET_DOUBLE(__node, __index)                                                     \
  (*(double *)quixcc_ast_get((__node), (__index), QUIXCC_AST_FIELD_DOUBLE))

#define QUIXCC_AST_GET_QSTRING(__node, __index)                                                    \
  (*(quixcc_string_t *)quixcc_ast_get((__node), (__index), QUIXCC_AST_FIELD_QSTRING))

#define QUIXCC_AST_GET_QVEC(__node, __index)                                                       \
  (*(quixcc_vec_t *)quixcc_ast_get((__node), (__index), QUIXCC_AST_FIELD_QVEC))

///=============================================================================

#define QUIXCC_AST_REF_BOOL(__node, __index)                                                       \
  ((bool *)quixcc_ast_get((__node), (__index), QUIXCC_AST_FIELD_BOOL))

#define QUIXCC_AST_REF_CHAR(__node, __index)                                                       \
  ((char *)quixcc_ast_get((__node), (__index), QUIXCC_AST_FIELD_CHAR))

#define QUIXCC_AST_REF_INT8(__node, __index)                                                       \
  ((int8_t *)quixcc_ast_get((__node), (__index), QUIXCC_AST_FIELD_INT8))

#define QUIXCC_AST_REF_INT16(__node, __index)                                                      \
  ((int16_t *)quixcc_ast_get((__node), (__index), QUIXCC_AST_FIELD_INT16))

#define QUIXCC_AST_REF_INT32(__node, __index)                                                      \
  ((int32_t *)quixcc_ast_get((__node), (__index), QUIXCC_AST_FIELD_INT32))

#define QUIXCC_AST_REF_INT64(__node, __index)                                                      \
  ((int64_t *)quixcc_ast_get((__node), (__index), QUIXCC_AST_FIELD_INT64))

#define QUIXCC_AST_REF_UINT8(__node, __index)                                                      \
  ((uint8_t *)quixcc_ast_get((__node), (__index), QUIXCC_AST_FIELD_UINT8))

#define QUIXCC_AST_REF_UINT16(__node, __index)                                                     \
  ((uint16_t *)quixcc_ast_get((__node), (__index), QUIXCC_AST_FIELD_UINT16))

#define QUIXCC_AST_REF_UINT32(__node, __index)                                                     \
  ((uint32_t *)quixcc_ast_get((__node), (__index), QUIXCC_AST_FIELD_UINT32))

#define QUIXCC_AST_REF_UINT64(__node, __index)                                                     \
  ((uint64_t *)quixcc_ast_get((__node), (__index), QUIXCC_AST_FIELD_UINT64))

#define QUIXCC_AST_REF_FLOAT(__node, __index)                                                      \
  ((float *)quixcc_ast_get((__node), (__index), QUIXCC_AST_FIELD_FLOAT))

#define QUIXCC_AST_REF_DOUBLE(__node, __index)                                                     \
  ((double *)quixcc_ast_get((__node), (__index), QUIXCC_AST_FIELD_DOUBLE))

#define QUIXCC_AST_REF_QSTRING(__node, __index)                                                    \
  ((quixcc_string_t *)quixcc_ast_get((__node), (__index), QUIXCC_AST_FIELD_QSTRING))

#define QUIXCC_AST_REF_QVEC(__node, __index)                                                       \
  ((quixcc_vec_t *)quixcc_ast_get((__node), (__index), QUIXCC_AST_FIELD_QVEC))

///=============================================================================
/// END: ABSTRACT SYNTAX TREE PROPERTY ACCESSORS
///=============================================================================

///=============================================================================
/// BEGIN: ABSTRACT SYNTAX TREE ITERATION
///=============================================================================

/**
 * @brief Quixcc abstract syntax tree node.
 *
 * @param node The node. The legality of mutablity is context-dependent.
 * @param ex An arbitrary context dependent thing or `NULL`.
 *
 * @return Interpretation is context-dependent.
 *
 * @note Thread safety is required.
 */
typedef int (*quixcc_ast_visit_t)(quixcc_ast_node_t **node, void *ex);

/**
 * @brief Callback predicate on the Abstract Syntax Tree.
 *
 * @param node The node.
 * @param ex This parameter is context-dependent.
 *
 * @return `1` if true, `0` otherwise.
 *
 * @note Thread safety is required.
 */
typedef quixcc_ast_visit_t quixcc_ast_pred_t;

/**
 * @brief Iterates over the children of any node.
 *
 * @param node The node to iterate over.
 * @param order The order of traversal.
 * @param cb The callback to call for each node.
 *
 * @note This function is thread-safe.
 * @note A panic will occur if base is NULL.
 * @note A panic will occur if cb is NULL.
 */
void quixcc_ast_walk(quixcc_ast_node_t *base, quixcc_iterm_t order, quixcc_ast_visit_t cb);

/**
 * @brief Iterates over the children of any node.
 *
 * @param base The node to iterate over.
 * @param order The order of traversal.
 * @param cb The callback to call for each node.
 * @param walk_if The predicate to determine if a node should be walked.
 *
 * @note This function is thread-safe.
 * @note A panic will occur if base is NULL.
 * @note A panic will occur if cb is NULL.
 * @note `quixcc_ast_walk_ex` is equivalent to `quixcc_ast_walk` if `walk_if` is
 * `NULL`.
 */
void quixcc_ast_walk_ex(quixcc_ast_node_t *base, quixcc_iterm_t order, quixcc_ast_visit_t cb,
                        quixcc_ast_pred_t walk_if);

///=============================================================================
/// END: ABSTRACT SYNTAX TREE ITERATION
///=============================================================================

///=============================================================================
/// BEGIN: ABSTRACT SYNTAX TREE NODE SERIALIZATION
///=============================================================================

/**
 * @brief Serializes an abstract syntax tree node to a string in the QAST
 * format.
 *
 * @param node The abstract syntax tree node.
 * @param minify Whether to minify the output.
 * @param arena The arena to allocate memory from (optional).
 * @param[out] outlen The length of the returned string.
 *
 * @return String representation of the abstract syntax tree node.
 *
 * @note This function is thread-safe.
 * @note If `arena` is `NULL`, the function will return a malloc()'ed string.
 * @note If `outlen` is not `NULL`, the length of the returned string will be
 * stored in the variable pointed to by `outlen`. Otherwise, it will be
 * ignored.
 * @note The output string will end with a null terminator. The NULL terminator
 * is not included in the length.
 */
char *quixcc_ast_repr(quixcc_ast_node_t *node, bool minify, size_t indent, quixcc_arena_t *arena,
                      size_t *outlen);

/**
 * @brief Serializes an abstract syntax tree node to a binary representation.
 *
 * @param node The abstract syntax tree node.
 * @param compress Whether to compress the output.
 * @param arena The arena to allocate memory from (optional).
 * @param[out] out The binary representation of the abstract syntax tree node.
 * @param[out] outlen The length of the binary representation.
 *
 * @note This function is thread-safe.
 * @note If `arena` is `NULL`, the function will provide a malloc()'ed buffer.
 * @note If `out` is not `NULL`, the binary representation will be stored in the
 * variable pointed to by `out`. Otherwise, it will panic.
 * @note The output will not be null-terminated.
 *
 * @warning The output is not human readable, it is a binary representation,
 * and is not safe to print.
 */
void quixcc_ast_brepr(quixcc_ast_node_t *node, bool compress, quixcc_arena_t *arena, uint8_t **out,
                      size_t *outlen);

/**
 * @brief Deserializes an abstract syntax tree node from either a string or
 * binary representation.
 *
 * @param repr The serialized abstract syntax tree node.
 * @param len The length of the repr.
 * @param arena The arena to allocate memory from
 * @param[out] out The deserialized abstract syntax tree node.
 *
 * @return `true` if the deserialization was successful, `false` otherwise.
 *
 * @note This function is thread-safe.
 * @note If the deserialization fails, the function will return `false` and
 * `out` will be `NULL`.
 * @note If the deserialization is successful, the function will return `true`
 * and `out` will be a pointer to the deserialized abstract syntax tree node.
 */
bool quixcc_ast_from(const uint8_t *repr, size_t len, quixcc_arena_t *arena,
                     quixcc_ast_node_t **out);

///=============================================================================
/// END: ABSTRACT SYNTAX TREE NODE SERIALIZATION
///=============================================================================

///=============================================================================
/// BEGIN: ABSTRACT SYNTAX TREE NODE MEMORY MANAGEMENT
///=============================================================================

/**
 * @brief Allocates an abstract syntax tree node.
 *
 * @param type The type of the node.
 * @param arena The arena to allocate memory from.
 *
 * @return The allocated abstract syntax tree node.
 *
 * @note This function is thread-safe.
 * @note If the allocation fails, the function will panic.
 */
quixcc_ast_node_t *quixcc_ast_alloc(quixcc_ast_ntype_t type, quixcc_arena_t *arena);

///=============================================================================
/// END: ABSTRACT SYNTAX TREE NODE MEMORY MANAGEMENT
///=============================================================================

#ifdef __cplusplus
}
#endif

#endif // __QUIXCC_TYPES_SYNTAXTREENODES_H__
