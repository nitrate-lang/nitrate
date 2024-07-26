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

#ifdef __cplusplus
extern "C" {
#endif

#include <quix-lexer/Lexer.h>
#include <quix-parser/Config.h>
#include <stdbool.h>

struct qparse_impl_t;
typedef struct qparse_impl_t qparse_impl_t;

typedef struct qparse_t {
  qparse_impl_t *impl; /* Parser implementation struct */
  qlex_t *lexer;       /* Polymporphic lexer */
  qparse_conf_t *conf; /* Parser configuration */
} qparse_t;

/**
 * @brief Create a new parser instance from non-owning references to a lexer and
 * parser configuration.
 *
 * @param lexer The lexer to use for parsing.
 * @param conf The parser configuration to use for parsing.
 *
 * @return A new parser instance.
 *
 * @note If `!lexer` or `!conf`, NULL is returned.
 *
 * @note The parser instance is owned by the caller and must be freed with
 * `qparse_free`.
 *
 * @note The lexer and configuration are not owned by the parser and must be
 * freed separately after the parser is freed.
 *
 * @note The parser instance isn't thread safe internally, but the functions
 * provided to operate on the parser instance are thread safe themselves. This means that there is
 * no internal locking in the parser context.
 * 
 * @note This function is thread safe.
 */
qparse_t *qparse_new(qlex_t *lexer, qparse_conf_t *conf);

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

#ifdef __cplusplus
}
#endif

#endif  // __QUIX_PARSER_PARSER_H__
