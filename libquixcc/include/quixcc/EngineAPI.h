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

#ifndef __QUIXCC_TYPES_H__
#define __QUIXCC_TYPES_H__

#include <quixcc/Types.h>
#include <stdbool.h>
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
bool quixcc_qsys_add(quixcc_job_t* job, uint32_t num, quixcc_qsys_impl_t impl);

/// @brief Remove a QSys Call from the preprocessor engine.
/// @param engine QUIX Job
/// @param num The unique number of the QSys Call
/// @return true if successful, false otherwise
/// @note This function is thread-safe.
bool quixcc_qsys_remove(quixcc_job_t* job, uint32_t num);

/// @brief Check if a QSys Call exists in the preprocessor engine.
/// @param engine QUIX Job
/// @param num The unique number of the QSys Call
/// @return true if the QSys Call exists, false otherwise
/// @note This function is thread-safe.
bool quixcc_qsys_exists(quixcc_job_t* job, uint32_t num);

/// @brief Get a QSys Call from the preprocessor engine.
/// @param engine QUIX Job
/// @param num The unique number of the QSys Call
/// @return The QSys Call implementation, or NULL if it does not exist
/// @note This function is thread-safe.
quixcc_qsys_impl_t quixcc_qsys_get(quixcc_job_t* job, uint32_t num);

/// @brief List all QSys Calls in the preprocessor engine.
/// @param engine QUIX Job
/// @param count The number of QSys Calls
/// @return An array of QSys Call numbers
/// @note This function is thread-safe.
uint32_t* quixcc_qsys_list(quixcc_job_t* job, uint32_t* count);

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
quixcc_job_t* quixcc_engine_job(quixcc_engine_t* engine);

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

enum quixcc_message_t {
  QUIXCC_MESSAGE_DEBUG = 0,
  QUIXCC_MESSAGE_INFO = 1,
  QUIXCC_MESSAGE_WARNING = 2,
  QUIXCC_MESSAGE_ERROR = 3,
  QUIXCC_MESSAGE_FAILED = 4,
  QUIXCC_MESSAGE_FATAL = 5,
};

/// @brief Send a message to the engine.
/// @param engine QUIX Engine
/// @param mode Message type
/// @param message The message to send
bool quixcc_engine_message(quixcc_engine_t* engine, quixcc_message_t mode,
                           const char* message);

bool quixcc_engine_emit(quixcc_engine_t* engine, quixcc_tok_t tok);

///=============================================================================
/// END: QUIXCC ENGINE API
///=============================================================================

#ifdef __cplusplus
}
#endif

#endif  // __QUIXCC_TYPES_H__
