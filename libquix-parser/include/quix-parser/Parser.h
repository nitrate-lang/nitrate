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

#ifndef __QUIX_PARSER_PARSER_H__
#define __QUIX_PARSER_PARSER_H__

#include <nitrate-core/Env.h>
#include <nitrate-lexer/Lexer.h>
#include <quix-parser/Config.h>
#include <quix-parser/Node.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct qparse_t qparse_t;

/**
 * @brief Create a new parser instance from non-owning references to a lexer and
 * parser configuration.
 *
 * @param lexer Lexer stream object.
 * @param conf Parser configuration object.
 * @param env The environment.
 *
 * @return A new parser instance or NULL if an error occurred.
 *
 * @note If `!lexer` or `!conf`, NULL is returned.
 * @note The returned object must be freed with `qparse_free`.
 * @note The lexer object and configuration object are not owned by the returned parser object.
 * @note The returned instance does not contain internal locks.
 *
 * @note This function is thread safe.
 */
qparse_t *qparse_new(qlex_t *lexer, qparse_conf_t *conf, qcore_env_t env);

/**
 * @brief Free a parser instance.
 *
 * @param parser The parser instance to free (may be NULL).
 *
 * @note If `!parser`, this function is a no-op.
 *
 * @note This function will not free the lexer or configuration associated with
 * the it. The caller is responsible for freeing the lexer and configuration
 * separately.
 * @note This function is thread safe.
 */
void qparse_free(qparse_t *parser);

/**
 * @brief Serialize a parse tree to a string.
 *
 * @param _node The root node of the parse tree.
 * @param minify Whether to minify the output.
 * @param indent The number of spaces to indent the output.
 * @param outlen The length of the output string.
 *
 * @return The serialized parse tree as a string.
 *
 * @note This function is thread safe.
 */
char *qparse_repr(const qparse_node_t *_node, bool minify, size_t indent, size_t *outlen);

/**
 * @brief Serialize a parse tree to a binary representation.
 *
 * @param node The root node of the parse tree.
 * @param compress Whether to compress the output.
 * @param out The output buffer.
 * @param outlen The length of the output buffer.
 *
 * @note This function is thread safe.
 */
void qparse_brepr(const qparse_node_t *node, bool compress, uint8_t **out, size_t *outlen);

/**
 * @brief Parse QUIX code into a parse tree.
 *
 * @param parser The parser instance to use for parsing.
 * @param out The output parse tree.
 *
 * @return Returns true if no non-fatal parsing errors occurred, false otherwise. A value of true,
 * however, does not guarantee that the parse tree is valid.
 *
 * @note If `!parser` or `!out`, false is returned.
 *
 * @note This function is thread safe.
 */
bool qparse_do(qparse_t *parser, qparse_node_t **out);

/**
 * @brief Parse QUIX code into a parse tree and dump it to a file.
 *
 * @param parser The parser instance to use for parsing.
 * @param out The file to dump the parse tree to.
 * @param x0 Unused.
 * @param x1 Unused.
 *
 * @return Returns true if no non-fatal parsing errors occurred, false otherwise. A value of true,
 * however, does not guarantee that the parse tree is valid.
 *
 * @note If `!parser` or `!out`, false is returned.
 *
 * @note This function is thread safe.
 */
bool qparse_and_dump(qparse_t *parser, FILE *out, void *x0, void *x1);

/**
 * @brief Check if the parse tree is valid.
 *
 * @param parser The parser instance to use for parsing.
 * @param base The base node of the parse tree to check.
 *
 * @return Returns true if the parse tree is valid, false otherwise. A value of true, however, does
 * not indicate that the parse tree is free of semantic errors. This just ensures it doesn't contain
 * NULL nodes and other illegal constructs (basic checks).
 *
 * @note If `!parser`, `!base` false is returned.
 *
 * @note This function is thread safe.
 */
bool qparse_check(qparse_t *parser, const qparse_node_t *base);

/**
 * @brief A callback function to facilitate the communication reports generated
 * by the parser.
 *
 * @param msg The message to report.
 * @param len The length of the message.
 * @param data The user data to pass to the callback.
 *
 * @note This function is thread safe.
 */
typedef void (*qparse_dump_cb)(const char *msg, size_t len, uintptr_t data);

/**
 * @brief Dump the parser's reports to a callback function.
 *
 * @param parser The parser instance to dump reports from.
 * @param cb The callback function to pass reports to.
 * @param data An arbitrary pointer to pass to every callback function.
 *
 * @note If `!parser` or `!cb`, this function is a no-op.
 *
 * @note This function is thread safe.
 */
void qparse_dumps(qparse_t *parser, bool no_ansi, qparse_dump_cb cb, uintptr_t data);

#ifdef __cplusplus
}
#endif

#endif  // __QUIX_PARSER_PARSER_H__
