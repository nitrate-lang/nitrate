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

#ifndef __QUIXCC_PREPROCESSOR_ENGINE_API_H__
#define __QUIXCC_PREPROCESSOR_ENGINE_API_H__

#include <quixcc/types/All.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

///=============================================================================
/// BEGIN: QUIXCC QSYS REGISTRY
///=============================================================================

/// @brief Add a QSys Call to the preprocessor engine.
/// @param engine QUIX Job
/// @param num Unique number for the QSys Call
/// @param impl Implementation of the QSys Call
/// @return true if successful, false otherwise
/// @note This function is thread-safe.
bool quixcc_qsys_add(quixcc_cc_job_t* job, uint32_t num,
                     quixcc_qsys_impl_t impl);

/// @brief Remove a QSys Call from the preprocessor engine.
/// @param engine QUIX Job
/// @param num The unique number of the QSys Call
/// @return true if successful, false otherwise
/// @note This function is thread-safe.
bool quixcc_qsys_remove(quixcc_cc_job_t* job, uint32_t num);

/// @brief Check if a QSys Call exists in the preprocessor engine.
/// @param engine QUIX Job
/// @param num The unique number of the QSys Call
/// @return true if the QSys Call exists, false otherwise
/// @note This function is thread-safe.
bool quixcc_qsys_exists(quixcc_cc_job_t* job, uint32_t num);

/// @brief Get a QSys Call from the preprocessor engine.
/// @param engine QUIX Job
/// @param num The unique number of the QSys Call
/// @return The QSys Call implementation, or NULL if it does not exist
/// @note This function is thread-safe.
quixcc_qsys_impl_t quixcc_qsys_get(quixcc_cc_job_t* job, uint32_t num);

/// @brief List all QSys Calls in the preprocessor engine.
/// @param engine QUIX Job
/// @param count The number of QSys Calls
/// @return An array of QSys Call numbers
/// @note This function is thread-safe.
uint32_t* quixcc_qsys_list(quixcc_cc_job_t* job, uint32_t* count);

///=============================================================================
/// END: QUIXCC QSYS REGISTRY
///=============================================================================

///=============================================================================
/// BEGIN: QUIXCC ENGINE API
///=============================================================================

/// @brief Get job context from engine instance.
/// @param engine QUIX Engine
/// @return The job context (never NULL)
/// @note This function is thread-safe.
quixcc_cc_job_t* quixcc_engine_job(quixcc_engine_t* engine);

/// @brief Set the include path for the engine.
/// @param engine QUIX Engine
/// @param include_path Include directory path separated by colons ':'
/// @return true if successful, false otherwise
/// @note The `include_path` is a colon-separated list of directories.
/// @note The include path will be validated internally.
/// @note If the `include_path` is malformed, or any directory does not exist,
///       the function will return false.
/// @note This function is thread-safe.
bool quixcc_engine_include(quixcc_engine_t* engine, const char* include_path);

typedef enum quixcc_message_t {
  QUIXCC_MESSAGE_DEBUG = 0,
  QUIXCC_MESSAGE_INFO = 1,
  QUIXCC_MESSAGE_WARNING = 2,
  QUIXCC_MESSAGE_ERROR = 3,
  QUIXCC_MESSAGE_FAILED = 4,
  QUIXCC_MESSAGE_FATAL = 5,
} quixcc_message_t;

/// @brief Send a message to the engine.
/// @param engine QUIX Engine
/// @param mode Message type
/// @param format Printf-style format string
/// @param ... Printf-style arguments
bool quixcc_engine_message(quixcc_engine_t* engine, quixcc_message_t mode,
                           const char* format, ...);

bool quixcc_engine_emit(quixcc_engine_t* engine, quixcc_tok_t tok);

/// @brief Get string from expression
/// @param expr QUIX Expression
/// @param len Output length of the string
/// @return The string representation of the expression. NULL if failed.
/// @note The lifetime of the returned string is tied to the expression.
/// @note The string is NOT guaranteed to be null-terminated.
/// @warning The string may contain any byte within, including null bytes.
/// @note This function is thread-safe.
const char* quixcc_expr_to_string(quixcc_expr_t* expr, size_t* len);

/// @brief Get int64 from expression
/// @param expr QUIX Expression
/// @param out Output int64_t
/// @return true if successful, false otherwise
/// @note This function is thread-safe.
bool quixcc_expr_to_int64(quixcc_expr_t* expr, int64_t* out);

/// @brief Create a new token
/// @param engine QUIX Engine
/// @param ty Token type
/// @param str C-string body of the token
/// @return A new token
/// @warning The token is managed by the engine and must not be mutated.
/// @note The data is copied internally.
quixcc_tok_t quixcc_tok_new(quixcc_engine_t* engine, quixcc_lex_type_t ty,
                            const char* str);

/// @brief Create a new token with a length
/// @param engine QUIX Engine
/// @param ty Token type
/// @param str Arbitrary buffer of the token body
/// @param len Length of the buffer
/// @return A new token
/// @warning The token is managed by the engine and must not be mutated.
/// @note The data is copied internally.
quixcc_tok_t quixcc_tok_new_ex(quixcc_engine_t* engine, quixcc_lex_type_t ty,
                               const char* str, size_t len);

/// @brief Create a new keyword token
/// @param kw Keyword
/// @return A new token
quixcc_tok_t quixcc_tok_new_kw(quixcc_lex_kw_t kw);

/// @brief Create a new operator token
/// @param op Operator
/// @return A new token
quixcc_tok_t quixcc_tok_new_op(quixcc_lex_op_t op);

/// @brief Create a new punctuation token
/// @param punct Punctuation
/// @return A new token
quixcc_tok_t quixcc_tok_new_punct(quixcc_lex_punct_t punct);

#define quixcc_tok_new_ident(engine, str) \
  quixcc_tok_new(engine, QUIXCC_LEX_IDENT, str)

///=============================================================================
/// END: QUIXCC ENGINE API
///=============================================================================

///=============================================================================
/// BEGIN: MACROS
///=============================================================================
#define QSYS_DECL(name) \
  bool name(quixcc_engine_t*, uint32_t, quixcc_expr_t**, uint32_t)

#define QSYS_ARGASSERT(name, nargs)                                        \
  if (c != nargs) {                                                        \
    quixcc_engine_message(e, QUIXCC_MESSAGE_ERROR,                         \
                          "QSys Call \"%s\" expects %d arguments, got %d", \
                          #name, nargs, c);                                \
    return false;                                                          \
  }

#define QSYS_ARG_STRING(_qname, _varname, _idx)                           \
  if (c <= _idx) {                                                        \
    quixcc_engine_message(e, QUIXCC_MESSAGE_FATAL,                        \
                          "%s: Argument %d OUT OF RANGE", #_qname, _idx); \
    return false;                                                         \
  }                                                                       \
  size_t _varname##_len = 0;                                              \
  const char* _varname = quixcc_expr_to_string(v[_idx], &_varname##_len); \
  if (_varname == NULL) {                                                 \
    quixcc_engine_message(                                                \
        e, QUIXCC_MESSAGE_ERROR,                                          \
        "%s: Failed to convert argument %d to string \"" #_varname "\"",  \
        #_qname, _idx);                                                   \
    return false;                                                         \
  }

#define QSYS_ARG_INT64(_qname, _varname, _idx)                            \
  if (c <= _idx) {                                                        \
    quixcc_engine_message(e, QUIXCC_MESSAGE_FATAL,                        \
                          "%s: Argument %d OUT OF RANGE", #_qname, _idx); \
    return false;                                                         \
  }                                                                       \
  int64_t _varname = 0;                                                   \
  if (!quixcc_expr_to_int64(v[_idx], &_varname)) {                        \
    quixcc_engine_message(                                                \
        e, QUIXCC_MESSAGE_ERROR,                                          \
        "%s: Failed to convert argument %d to int64_t \"" #_varname "\"", \
        #_qname, _idx);                                                   \
    return false;                                                         \
  }

///=============================================================================
/// END: MACROS
///=============================================================================

#ifdef __cplusplus
}
#endif

#endif  // __QUIXCC_TYPES_H__
