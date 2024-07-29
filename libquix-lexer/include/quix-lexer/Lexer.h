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

#ifndef __QUIX_LEXER_LEX_H__
#define __QUIX_LEXER_LEX_H__

#include <quix-lexer/Token.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

struct qlex_impl_t;
typedef struct qlex_impl_t qlex_impl_t;

struct qlex_t;
typedef qlex_tok_t (*qlex_get_fn)(struct qlex_t *self);
typedef void (*qlex_collect_fn)(struct qlex_t *self, const qlex_tok_t *tok);
typedef void (*qlex_free_fn)(struct qlex_t *self);
typedef void (*qlex_push_fn)(struct qlex_t *self, const qlex_tok_t *tok);

#define QLEX_FLAG_NONE 0
#define QLEX_NO_COMMENTS 0x01

typedef struct qlex_t {
  qlex_impl_t *impl;
  qlex_get_fn next;
  qlex_get_fn peek;
  qlex_push_fn push;
  qlex_collect_fn collect;
  qlex_free_fn destruct;
  qlex_tok_t cur;
  uint32_t flags;
} qlex_t;

/**
 * @brief Create a new lexer context.
 *
 * @param file A seekable file stream.
 *
 * @return New lexer context or NULL if an error occurred.
 * @note This function is thread-safe.
 */
qlex_t *qlex_new(FILE *file);

/**
 * @brief Destroy a lexer context.
 *
 * @param lexer Lexer context or NULL.
 * @note It is safe to pass NULL to this function.
 * @note This function is thread-safe.
 */
void qlex_free(qlex_t *lexer);

static inline void qlex_set_flags(qlex_t *lexer, uint32_t flags) { lexer->flags = flags; }
static inline uint32_t qlex_get_flags(qlex_t *lexer) { return lexer->flags; }

/**
 * @brief Calculate the size of a token.
 *
 * @param lexer Lexer context.
 * @param tok Token to calculate the size of.
 *
 * @return Size of the token in bytes. Returns 0 if the token is invalid.
 * @note This function is thread-safe.
 */
uint32_t qlex_tok_size(qlex_t *lexer, const qlex_tok_t *tok);

/**
 * @brief Write a token to a buffer.
 *
 * @param lexer Lexer context.
 * @param tok Token to write.
 * @param buf Buffer to write the token to.
 * @param size Size of the buffer.
 *
 * @return Number of bytes written. Rerturns 0 if the buffer is too small.
 * @note This function is thread-safe.
 * @warning Buffer WILL NOT be null-terminated.
 */
uint32_t qlex_tok_write(qlex_t *lexer, const qlex_tok_t *tok, char *buf, uint32_t size);

/**
 * @brief Send a signal to the lexer that the resources for the token will never be needed by the
 * client.
 *
 * @param lexer Lexer context.
 * @param tok Token to suggest for deallocation.
 *
 * @note This function is thread-safe.
 * @note This function merely suggests that the token can be deallocated. The lexer may choose to
 * ignore this suggestion.
 */
static inline void qlex_collect(qlex_t *lexer, const qlex_tok_t *tok) {
  lexer->collect(lexer, tok);
}

/**
 * @brief Lex the next token.
 *
 * @param lexer Lexer context.
 *
 * @return The next token.
 * @note This function is thread-safe.
 */
static inline qlex_tok_t qlex_next(qlex_t *lexer) { return lexer->next(lexer); }

/**
 * @brief Peek at the next token.
 *
 * @param lexer Lexer context.
 *
 * @return The next token.
 * @note This function is thread-safe.
 * @note Will not consume the token.
 */
static inline qlex_tok_t qlex_peek(qlex_t *lexer) { return lexer->peek(lexer); }

/**
 * @brief Push a token back into the lexer.
 *
 * @param lexer Lexer context.
 * @param tok Token to push back.
 * @note This function is thread-safe.
 */
static inline void qlex_push(qlex_t *lexer, qlex_tok_t tok) { lexer->push(lexer, &tok); }

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
 *
 * @return The internal string value for the token or empty string if this operation is applicable
 * for this token type.
 * @note This function is thread-safe.
 * @warning The lifetime shall exist for the duration of the lexer context.
 * @warning DO NOT MODIFY THE RETURNED STRING.
 */
const char *qlex_str(qlex_t *lexer, qlex_tok_t *tok);

const char *qlex_opstr(qlex_op_t op);
const char *qlex_kwstr(qlex_key_t kw);
const char *qlex_punctstr(qlex_punc_t punct);

void qlex_tok_fromstr(qlex_t *lexer, qlex_ty_t ty, const char *str, uint32_t src_idx, qlex_tok_t *out);

#ifdef __cplusplus
}
#endif

#endif  // __QUIX_LEXER_LEX_H__
