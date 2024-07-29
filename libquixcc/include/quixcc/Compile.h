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

#ifndef __QUIXCC_COMPILE_H__
#define __QUIXCC_COMPILE_H__

#include <quixcc/interface/All.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Create a new compiler job with default settings.
 *
 * @return A pointer to the newly created compiler job.
 *
 * @note The caller is responsible for disposing of the job using
 *       `quixcc_cc_dispose()`.
 * @note This function is thread-safe.
 */
quixcc_cc_job_t *quixcc_cc_new();

/**
 * @brief Dispose a compiler job.
 *
 * Releases all associated resources and deallocates memory for the
 * specified compiler job.
 *
 * @param job The compiler job to be disposed.
 * @return true if the job was disposed successfully.
 *
 * @note The job is disposed, and ALL associated resources are released.
 * @note This function is thread-safe.
 * @note This function will return false if the job is locked.
 * @note If `!job`, this function is a no-op.
 */
bool quixcc_cc_dispose(quixcc_cc_job_t *job);

/**
 * @brief Set an option for a compiler job.
 *
 * @param job The compiler job.
 * @param option The option to add.
 * @param enabled Is the option enabled?
 *
 * @note This function is thread-safe.
 * @note The option string is copied internally.
 * @note It is okay to set the same option multiple times.
 *       The last setting will be used.
 * @note If `!job || !option`, this function is a no-op.
 * @note This function will block until the job is unlocked.
 */
void quixcc_cc_option(quixcc_cc_job_t *job, const char *option, bool enable);

/**
 * @brief Set the input stream for a compiler job.
 *
 * @param job The compiler job.
 * @param in The input stream.
 * @param filename The input filename (required for error messages).
 *
 * @note This function is thread-safe.
 * @note The filename string is copied internally.
 * @note If `!job || !in || !filename`, this function is a no-op.
 * @note The FILE handle for the input stream is owned by the caller.
 * @warning The caller must ensure that the input stream is open and
 *          readable for the ENTIRE duration of the job.
 * @note This function will block until the job is unlocked.
 */
void quixcc_cc_source(quixcc_cc_job_t *job, FILE *in, const char *filename);

/**
 * @brief Set the LLVM Target Triple for a compiler job.
 *
 * @param job The compiler job.
 * @param triple The LLVM Target Triple.
 * @return true if the triple was set successfully. false if the triple
 *         is invalid or unknown.
 * @warning An empty string is a special case and will use the Host Target
 *          Triple returned by `llvm::sys::getDefaultTargetTriple()`.
 * @note This function will validate the triple before setting it and will
 *       check if it is supported.
 * @note It is okay to set the triple multiple times. The last **VALID**
 *       triple will be used.
 * @note If `!job || !triple`, this function is a no-op.
 * @note This function is thread-safe.
 * @note This function will block until the job is unlocked.
 */
bool quixcc_cc_target(quixcc_cc_job_t *job, const char *llvm_triple);

/**
 * @brief Set the LLVM Target CPU for a compiler job.
 *
 * @param job The compiler job.
 * @param cpu The LLVM Target CPU.
 * @return true if the CPU was set successfully. false if the CPU
 *         is invalid or unknown.
 * @note It is okay to set the CPU multiple times. The last **VALID**
 *       CPU will be used.
 * @note This function is thread-safe.
 * @note If `!job || !cpu`, this function is a no-op.
 * @warning Currently, the CPU is not validated for correctness or backend
 *          support.
 * @note This function will block until the job is unlocked.
 */
bool quixcc_cc_cpu(quixcc_cc_job_t *job, const char *cpu);

/**
 * @brief Set the output stream for a compiler job.
 *
 * @param job The compiler job.
 * @param out The output stream.
 * @param[out] old_out The previous output stream.
 *
 * @note The FILE handle for the output stream is owned by the caller.
 * @note If `!job || !out`, this function is a no-op.
 * @note The caller must ensure that the output stream is open and
 *       writable for the ENTIRE duration of the job.
 * @note If `old_out` is not NULL, the previous output stream will be
 *       returned in it.
 * @note This function is thread-safe.
 * @note This function will block until the job is unlocked.
 */
void quixcc_cc_output(quixcc_cc_job_t *job, FILE *out, FILE **old_out);

/**
 * @brief Run a compiler job.
 *
 * @param job The compiler job.
 * @return true if the job completed without errors.
 *
 * @note This function is thread-safe.
 * @note Use `quixcc_status()` for a detailed result.
 * @note If `!job`, this function is a no-op.
 * @note This function will block until the job is unlocked.
 */
bool quixcc_cc_run(quixcc_cc_job_t *job);

/**
 * @brief Get the result of a compiler job.
 *
 * This function retrieves the result of the specified compiler job.
 *
 * @param job The compiler job.
 * @return The result of the job, or NULL.
 *
 * @note This function is thread-safe.
 * @note The result is owned by the job and should not be modified.
 * @note If the job is locked, this function will return NULL.
 * @note The result is valid until the job is disposed.
 * @note If `!job`, this function is a no-op.
 */
const quixcc_status_t *quixcc_cc_status(quixcc_cc_job_t *job);

/**
 * @brief Perform a one-shot compilation.
 *
 * This function compiles input from an input stream, processes it
 * according to specified options, and writes the output to the
 * provided output stream.
 *
 * @param in The input stream containing the source code to be compiled.
 * @param out The output stream where the compiled output will be written.
 * @param options An array of compiler options, terminated by a NULL
 *                element.
 * @return NULL if the compilation is successful, otherwise a string array
 *         containing status messages.
 *
 * @note This function is thread-safe.
 * @note It is the caller's responsibility to free the returned array
 *       if it is not NULL.
 * @warning Non-error messages will be discarded.
 * @note Compilation option parsing is handled internally.
 * @warning Ensure that the options array is properly NULL-terminated.
 * @note The FILE handles for input and output streams are owned by the
 *       caller.
 * @warning The caller must ensure that the input stream is open and
 *          readable for the ENTIRE duration of the compilation.
 * @warning The caller must ensure that the output stream is open and
 *          writable for the ENTIRE duration of the compilation.
 * @note If `!in || !out`, this function is a no-op and returns NULL.
 */
char **quixcc_cc_compile(FILE *in, FILE *out, const char *options[]);

///=============================================================================
/// BEGIN: LANGUAGE STUFF
///=============================================================================

/**
 * @brief Demangle a mangled symbol name into a pretty name.
 *
 * @param mangled The mangled symbol name to be demangled.
 * @return A malloc'd pretty name or NULL if the symbol could not be demangled.
 *
 * @note This function is thread-safe.
 * @note If `!mangled`, this function is a no-op and returns NULL.
 */
char *quixcc_cc_demangle(const char *mangled);

/// @brief Set the lexer configuration for a compiler job.
/// @param job The compiler job.
/// @param config The lexer configuration.
/// @note This function is thread-safe.
void quixcc_cc_lexconf(quixcc_cc_job_t *job, quixcc_lexer_config_t config);

/// @brief Get the next token from the lexer.
/// @param job The compiler job.
/// @return The next token from the lexer.
/// @warning This function is not thread-safe on the same job context, but is
/// thread-safe across different job contexts.
quixcc_tok_t quixcc_cc_next(quixcc_cc_job_t *job);

/// @brief Peek at the next token from the lexer.
/// @param job The compiler job.
/// @return The next token from the lexer.
/// @warning This function is not thread-safe on the same job context, but is
/// thread-safe across different job contexts.
quixcc_tok_t quixcc_cc_peek(quixcc_cc_job_t *job);

/// @brief Compare two tokens for equality.
/// @param a The first token.
/// @param b The second token.
/// @return true if the tokens are equal, false otherwise.
/// @note This function is thread-safe.
bool quixcc_cc_tokeq(const quixcc_tok_t a, const quixcc_tok_t b);

/// @brief Check if a token is valid (no error occurred).
/// @param tok The token to check.
/// @return true if the token is valid, false otherwise.
/// @note This function is thread-safe.
static inline bool quixcc_cc_lex_ok(const quixcc_tok_t *tok) { return tok->ty > QUIXCC_LEX_UNK; }

/// @brief Check if a token is of a specific type.
/// @param tok The token to check.
/// @param ty The type to check against.
/// @return true if the token is of the specified type, false otherwise.
/// @note This function is thread-safe.
static inline bool quixcc_cc_lex_is(const quixcc_tok_t *tok, quixcc_lex_type_t ty) {
  return tok->ty == ty;
}

/// @brief Signal to the job that a token's internal string memory (if any) is
/// no longer needed.
/// @param job The compiler job.
/// @param tok The token to release.
/// @note This function is not thread-safe on the same job context, but is
/// thread-safe across different job contexts.
void quixcc_cc_tok_release(quixcc_cc_job_t *job, quixcc_tok_t *tok);

/// @brief Get raw string representation of a token.
/// @param job The compiler job.
/// @param tok The token to serialize.
/// @param buf The buffer to write the string representation to.
/// @param len The length of the buffer.
/// @note This function is thread-safe.
size_t quixcc_cc_tok_serialize(quixcc_cc_job_t *job, const quixcc_tok_t *tok, char *buf,
                               size_t len);

/// @brief Get the human-readable string representation of a token.
/// @param job The compiler job.
/// @param tok The token to serialize.
/// @param buf The buffer to write the string representation to.
/// @param len The length of the buffer.
/// @return The number of characters written to the buffer.
/// @note This function is thread-safe.
size_t quixcc_cc_tok_humanize(quixcc_cc_job_t *job, const quixcc_tok_t *tok, char *buf, size_t len);

///=============================================================================
/// END: LANGUAGE STUFF
///=============================================================================

#ifdef __cplusplus
}
#endif

#endif  // __QUIXCC_COMPILE_H__
