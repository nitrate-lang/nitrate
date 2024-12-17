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

#ifndef __NITRATE_LEXER_LEX_HH__
#define __NITRATE_LEXER_LEX_HH__

#include <nitrate-core/Env.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#include <nitrate-lexer/Token.hh>

typedef struct qlex_t qlex_t;

#define QLEX_FLAG_NONE 0
#define QLEX_NO_COMMENTS 0x01

typedef uint32_t qlex_flags_t;

void qlex_set_flags(qlex_t *lexer, qlex_flags_t flags);
qlex_flags_t qlex_get_flags(qlex_t *lexer);

/**
 * @brief Push token into the lexer.
 *
 * @param lexer Lexer context.
 * @param tok Token to push.
 * @note This function is thread-safe.
 * @note The next call to peek or next will return the pushed token.
 */
void qlex_insert(qlex_t *lexer, qlex_tok_t tok);

static inline uint32_t qlex_begin(const qlex_tok_t *tok) { return tok->start; }
uint32_t qlex_end(qlex_t *L, qlex_tok_t tok);

/**
 * @brief Return non-owning pointer to the filename associated with the lexer.
 *
 * @param lexer Lexer context.
 *
 * @return Non-owning pointer to the filename.
 */
const char *qlex_filename(qlex_t *lexer);

/**
 * @brief Get the line number of a location.
 *
 * @param lexer Lexer context.
 * @param loc Location.
 *
 * @return Line number or QLEX_EOFF on error.
 * @note This function is thread-safe.
 */
uint32_t qlex_line(qlex_t *lexer, uint32_t loc);

/**
 * @brief Get the column number of a location.
 *
 * @param lexer Lexer context.
 * @param loc Location.
 *
 * @return Column number or QLEX_EOFF on error.
 * @note This function is thread-safe.
 */
uint32_t qlex_col(qlex_t *lexer, uint32_t loc);

char *qlex_snippet(qlex_t *lexer, qlex_tok_t loc, uint32_t *offset);

void qlex_rect(qlex_t *lexer, uint32_t x_0, uint32_t y_0, uint32_t x_1,
               uint32_t y_1, char *out, size_t max_size, char fill);

/**
 * @brief Get the string representation of a token type.
 *
 * @param ty Token type.
 *
 * @return String representation of the token type.
 * @note This function is thread-safe.
 */
const char *qlex_ty_str(qlex_ty_t ty);

/**
 * @brief Test if two tokens are equal (not including the location).
 *
 * @param lexer Lexer context.
 * @param a First token.
 * @param b Second token.
 *
 * @return True if the tokens are equal, false otherwise.
 * @note This function is thread-safe.
 * @note The lexer context must have been the same used to
 * generate each of the respective tokens.
 */
bool qlex_eq(qlex_t *lexer, const qlex_tok_t *a, const qlex_tok_t *b);

/**
 * @brief Test if one token is less than another (not including the location).
 *
 * @param lexer Lexer context.
 * @param a First token.
 * @param b Second token.
 *
 * @return True if the first token is less than the second, false otherwise.
 * @note This function is thread-safe.
 * @note The lexer context must have been the same used to
 * generate each of the respective tokens.
 */
bool qlex_lt(qlex_t *lexer, const qlex_tok_t *a, const qlex_tok_t *b);

/**
 * @brief Get the internal string value for a token.
 *
 * @param lexer Lexer context.
 * @param tok Token.
 * @param len Pointer to store the length of the string.
 *
 * @return The internal string value for the token or empty string if this
 * operation is applicable for this token type.
 * @note This function is thread-safe.
 * @warning The lifetime shall exist for the duration of the lexer context.
 * @warning DO NOT MODIFY THE RETURNED STRING.
 * @warning The returned string is NULL-terminated, however, it may contain any
 * bytes within the data including NULL bytes.
 */
const char *qlex_str(qlex_t *lexer, const qlex_tok_t *tok, size_t *len);

const char *qlex_opstr(qlex_op_t op);
const char *qlex_kwstr(qlex_key_t kw);
const char *qlex_punctstr(qlex_punc_t punct);

void qlex_tok_fromstr(qlex_t *lexer, qlex_ty_t ty, const char *str,
                      qlex_tok_t *out);

#endif