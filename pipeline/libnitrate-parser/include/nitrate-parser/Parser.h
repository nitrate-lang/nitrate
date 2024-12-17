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

#ifndef __NITRATE_AST_PARSER_H__
#define __NITRATE_AST_PARSER_H__

#include <nitrate-core/Env.h>
#include <stdbool.h>

#include <nitrate-lexer/Lexer.hh>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct npar_t npar_t;
typedef struct npar_node_t npar_node_t;

/**
 * @brief Create a new parser instance from non-owning references to a lexer and
 * parser configuration.
 *
 * @param lexer Lexer stream object.
 * @param env The environment.
 *
 * @return A new parser instance or NULL if an error occurred.
 *
 * @note If `!lexer` NULL is returned.
 * @note The returned object must be freed with `npar_free`.
 * @note The returned instance does not contain internal locks.
 *
 * @note This function is thread safe.
 */
npar_t *npar_new(qlex_t *lexer, qcore_env_t env);

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
void npar_free(npar_t *parser);

/**
 * @brief Parse Nitrate code into a parse tree.
 *
 * @param parser The parser instance to use for parsing.
 * @param out The output parse tree.
 *
 * @return Returns true if no non-fatal parsing errors occurred, false
 * otherwise. A value of true, however, does not guarantee that the parse tree
 * is valid.
 *
 * @note If `!parser` or `!out`, false is returned.
 *
 * @note This function is thread safe.
 */
bool npar_do(npar_t *parser, npar_node_t **out);

/**
 * @brief Check if the parse tree is valid.
 *
 * @param parser The parser instance to use for parsing or NULL.
 * @param base The base node of the parse tree to check.
 *
 * @return True if the AST is valid, false otherwise.
 *
 * @note If base is NULL, false is returned.
 *
 * @note This function is thread safe.
 */
bool npar_check(npar_t *parser, const npar_node_t *base);

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
typedef void (*npar_dump_cb)(const char *msg, size_t len, uintptr_t data);

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
void npar_dumps(npar_t *parser, bool no_ansi, npar_dump_cb cb, uintptr_t data);

#ifdef __cplusplus
}
#endif

#endif  // __NITRATE_AST_PARSER_H__
