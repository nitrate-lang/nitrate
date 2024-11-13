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

#ifndef __NITRATE_LEXER_LEX_H__
#define __NITRATE_LEXER_LEX_H__

#include <nitrate-core/Env.h>
#include <nitrate-lexer/Token.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

typedef struct qlex_t qlex_t;

#define QLEX_FLAG_NONE 0
#define QLEX_NO_COMMENTS 0x01

typedef uint32_t qlex_flags_t;

#ifdef __cplusplus

#include <istream>
#include <memory>

/**
 * @brief Create a new lexer context.
 *
 * @param file A seekable file stream.
 * @param filename Name of the file or NULL for default.
 * @param env Parent environment.
 *
 * @return New lexer context or NULL if an error occurred.
 * @note The lifetime of the file stream and environment must exceed the lifetime of the lexer.
 * @note This function is thread-safe.
 */
qlex_t *qlex_new(std::shared_ptr<std::istream> file, const char *filename, qcore_env_t env);

#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Create a new lexer context from a string.
 *
 * @param src Source code.
 * @param len Length of the source code.
 * @param filename Name of the file or NULL for default.
 * @param env Parent environment.
 *
 * @return New lexer context or NULL if an error occurred.
 * @note This function is thread-safe.
 * @note The lifetime of the file stream and environment must exceed the lifetime of the lexer.
 * @warning The source code pointer must be valid for the duration of the lexer context.
 */
qlex_t *qlex_direct(const char *src, size_t len, const char *filename, qcore_env_t env);

/**
 * @brief Destroy a lexer context.
 *
 * @param lexer Lexer context or NULL.
 * @note It is safe to pass NULL to this function.
 * @note This function is thread-safe.
 */
void qlex_free(qlex_t *lexer);

void qlex_set_flags(qlex_t *lexer, qlex_flags_t flags);
qlex_flags_t qlex_get_flags(qlex_t *lexer);

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
 * @return Number of bytes written. Rerturns 0 if the buffer is too small or the token is invalid.
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
void qlex_collect(qlex_t *lexer, const qlex_tok_t *tok);

/**
 * @brief Lex the next token.
 *
 * @param lexer Lexer context.
 *
 * @return The next token.
 * @note This function is thread-safe.
 */
qlex_tok_t qlex_next(qlex_t *lexer);

/**
 * @brief Peek at the next token.
 *
 * @param lexer Lexer context.
 *
 * @return The next token.
 * @note This function is thread-safe.
 * @note Will not consume the token.
 */
qlex_tok_t qlex_peek(qlex_t *lexer);

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
static inline uint32_t qlex_end(const qlex_tok_t *tok) { return tok->end; }

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
 * @return Line number or UINT32_MAX on error.
 * @note This function is thread-safe.
 */
uint32_t qlex_line(qlex_t *lexer, uint32_t loc);

/**
 * @brief Get the column number of a location.
 *
 * @param lexer Lexer context.
 * @param loc Location.
 *
 * @return Column number or UINT32_MAX on error.
 * @note This function is thread-safe.
 */
uint32_t qlex_col(qlex_t *lexer, uint32_t loc);

char *qlex_snippet(qlex_t *lexer, qlex_tok_t loc, uint32_t *offset);

/**
 * @brief Calculate offset in source file from location structure.
 *
 * @param lexer Lexer context.
 * @param base Base location.
 * @param offset Offset in bytes. Negative values are allowed.
 *
 * @return New location structure. Tag member will be 0 on error.
 * @note This function is thread-safe.
 */
uint32_t qlex_offset(qlex_t *lexer, uint32_t base, uint32_t offset);

/**
 * @brief Get the number of bytes between two locations.
 *
 * @param lexer Lexer context.
 * @param start Start location.
 * @param end End location.
 *
 * @return Number of bytes between the two locations or UINT32_MAX on error.
 * @note This function is thread-safe.
 */
uint32_t qlex_span(qlex_t *lexer, uint32_t start, uint32_t end);

/**
 * @brief Get the number of bytes between two locations.
 *
 * @param lexer Lexer context.
 * @param start Start location.
 * @param end End location.
 * @param callback Callback function to call with the slice of the source in the span.
 * @param userdata Userdata to pass to the callback.
 *
 * @return Number of bytes between the two locations or UINT32_MAX on error.
 * @note This function is thread-safe.
 * @note If the callback is ever called this function is guaranteed to not return UINT32_MAX.
 *       Otherwise, the return value is UINT32_MAX.
 */
uint32_t qlex_spanx(qlex_t *lexer, uint32_t start, uint32_t end,
                    void (*callback)(const char *, uint32_t, uintptr_t), uintptr_t userdata);

void qlex_rect(qlex_t *lexer, uint32_t x_0, uint32_t y_0, uint32_t x_1, uint32_t y_1, char *out,
               size_t max_size, char fill);

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
 * @return The internal string value for the token or empty string if this operation is applicable
 * for this token type.
 * @note This function is thread-safe.
 * @warning The lifetime shall exist for the duration of the lexer context.
 * @warning DO NOT MODIFY THE RETURNED STRING.
 * @warning The returned string is NULL-terminated, however, it may contain any bytes within the
 * data including NULL bytes.
 */
const char *qlex_str(qlex_t *lexer, qlex_tok_t *tok, size_t *len);

const char *qlex_opstr(qlex_op_t op);
const char *qlex_kwstr(qlex_key_t kw);
const char *qlex_punctstr(qlex_punc_t punct);

void qlex_tok_fromstr(qlex_t *lexer, qlex_ty_t ty, const char *str, qlex_tok_t *out);

#ifdef __cplusplus
}
#endif

#endif  // __NITRATE_LEXER_LEX_H__
