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

#ifndef __QUIX_QXIR_QXIR_H__
#define __QUIX_QXIR_QXIR_H__

#include <quix-core/Arena.h>
#include <quix-parser/Node.h>
#include <quix-qxir/Config.h>
#include <quix-qxir/Node.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

struct qxir_impl_t;
typedef struct qxir_impl_t qxir_impl_t;

typedef struct qxir_t {
  qxir_impl_t *impl;   /* QXIR implementation struct */
  qxir_conf_t *conf;   /* QXIR configuration */
  qparse_node_t *root; /* The root node of the parse tree */
  bool failed;         /* Whether the qxir failed (ie syntax errors) */
} qxir_t;

/**
 * @brief Create a new qxir instance from non-owning references to a lexer and
 * qxir configuration.
 *
 * @param root The root node of the parse tree.
 * @param conf QXIR configuration object.
 *
 * @return A new qxir instance or NULL if an error occurred.
 *
 * @note If `!root` or `!conf`, NULL is returned.
 * @note The returned object must be freed with `qxir_free`.
 * @note The root object and configuration object are not owned by the returned qxir object.
 * @note The returned instance does not contain internal locks.
 *
 * @note This function is thread safe.
 */
qxir_t *qxir_new(qparse_node_t *root, qxir_conf_t *conf);

/**
 * @brief Free a qxir instance.
 *
 * @param qxir The qxir instance to free (may be NULL).
 *
 * @note If `!qxir`, this function is a no-op.
 *
 * @note This function will not free the lexer or configuration associated with
 * the it. The caller is responsible for freeing the lexer and configuration
 * separately.
 * @note This function is thread safe.
 */
void qxir_free(qxir_t *qxir);

/**
 * @brief Serialize a qxir tree to a string.
 *
 * @param _node The root node of the qxir tree.
 * @param minify Whether to minify the output.
 * @param indent The number of spaces to indent the output.
 * @param arena The arena to allocate memory from.
 * @param outlen The length of the output string.
 *
 * @return The serialized qxir tree as a string.
 *
 * @note This function is thread safe.
 */
char *qxir_repr(const qxir_node_t *_node, bool minify, size_t indent, qcore_arena_t *arena,
                size_t *outlen);

/**
 * @brief Serialize a qxir tree to a binary representation.
 *
 * @param node The root node of the qxir tree.
 * @param compress Whether to compress the output.
 * @param arena The arena to allocate memory from.
 * @param out The output buffer.
 * @param outlen The length of the output buffer.
 *
 * @note This function is thread safe.
 */
void qxir_brepr(const qxir_node_t *node, bool compress, qcore_arena_t *arena, uint8_t **out,
                size_t *outlen);

/**
 * @brief Parse QUIX code into a qxir tree.
 *
 * @param qxir The qxir instance to use for parsing.
 * @param arena The arena to allocate memory from.
 * @param out The output qxir tree.
 *
 * @return Returns true if no non-fatal parsing errors occurred, false otherwise. A value of true,
 * however, does not guarantee that the qxir tree is valid.
 *
 * @note If `!qxir`, `!arena`, or `!out`, false is returned.
 *
 * @note The `out` node is allocated from the arena and is therefore
 * bound to the arena's lifetime.
 *
 * @note This function is thread safe.
 */
bool qxir_do(qxir_t *qxir, qcore_arena_t *arena, qxir_node_t **out);

/**
 * @brief Parse QUIX code into a qxir tree and dump it to a file.
 *
 * @param qxir The qxir instance to use for parsing.
 * @param out The file to dump the qxir tree to.
 * @param x0 Unused.
 * @param x1 Unused.
 *
 * @return Returns true if no non-fatal parsing errors occurred, false otherwise. A value of true,
 * however, does not guarantee that the qxir tree is valid.
 *
 * @note If `!qxir` or `!out`, false is returned.
 *
 * @note This function is thread safe.
 */
bool qxir_and_dump(qxir_t *qxir, FILE *out, void *x0, void *x1);

/**
 * @brief Check if the qxir tree is valid.
 *
 * @param qxir The qxir instance to use for parsing.
 * @param base The base node of the qxir tree to check.
 *
 * @return Returns true if the qxir tree is valid, false otherwise. A value of true, however, does
 * not indicate that the qxir tree is free of semantic errors. This just ensures it doesn't contain
 * NULL nodes and other illegal constructs (basic checks).
 *
 * @note If `!qxir`, `!base` false is returned.
 *
 * @note This function is thread safe.
 */
bool qxir_check(qxir_t *qxir, const qxir_node_t *base);

/**
 * @brief A callback function to facilitate the communication reports generated
 * by the qxir.
 *
 * @param msg The message to report.
 * @param len The length of the message.
 * @param data The user data to pass to the callback.
 *
 * @note This function is thread safe.
 */
typedef void (*qxir_dump_cb)(const char *msg, size_t len, uintptr_t data);

/**
 * @brief Dump the qxir's reports to a callback function.
 *
 * @param qxir The qxir instance to dump reports from.
 * @param cb The callback function to pass reports to.
 * @param data An arbitrary pointer to pass to every callback function.
 *
 * @note If `!qxir` or `!cb`, this function is a no-op.
 *
 * @note This function is thread safe.
 */
void qxir_dumps(qxir_t *qxir, bool no_ansi, qxir_dump_cb cb, uintptr_t data);

#ifdef __cplusplus
}
#endif

#endif  // __QUIX_QXIR_QXIR_H__
