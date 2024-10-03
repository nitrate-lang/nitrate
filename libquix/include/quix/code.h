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

#ifndef __LIBQUIX_CODE_H__
#define __LIBQUIX_CODE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

typedef void (*quix_diag_cb)(const char *message, const char *by, uint64_t userdata);

/**
 * @brief Generic interface to transformation utilities provided by the Quix Toolchain.
 *
 * @param source Input pipe to read data from.
 * @param output The output pipe to write data to.
 * @param diag_cb [Nullable] Callback function to report diagnostics.
 * @param userdata User data to pass to the diagnostic callback.
 * @param options [Nullable] NULL terminated array of options to pass to the transformer.
 *
 * @return `true` if the transformation was successful, `false` otherwise.
 *
 * @note The `source` can not be the same pipe as the `output`. This exclusion is not checking
 * internally.
 * @note The `diag_cb` function is optional. If provided, it will be called to report diagnostics.
 * The `userdata` parameter will be passed to the callback function on each invocation. `userdata`
 * can be used to create closures. `userdata` can be any value, it is not checked internally.
 *
 * @note The `options` parameter is optional. If provided, it will be passed to the transformer
 * otherwise an empty array will be passed.
 * @warning Don't forget the that the last element of the `options` array must be `NULL`. This is
 * how the function knows when to stop reading the array.
 *
 * @note The `options` array is validated internally to the fullest extent possible by the
 * appropriate subsystem. However, it is not guaranteed that all possible options will trigger well
 * defined transformations. The transformer may ignore some options or may not be able to handle
 * them. See the documentation for each component for more information on what options are
 * available. This function is mostly just a wrapper around various internal subsystems and
 * therefore has limited knowledge of the actual semantics of the options.
 *
 * @warning This function is thread-safe. However, it it may block other threads in some cases. Some
 * components can the number of concurrent runs on them which will result in an error being returned
 * for an otherwise valid call. This is not a bug, but a feature to maximize the efficiency of the
 * system.
 */
bool quix_cc(FILE *source, FILE *output, quix_diag_cb diag_cb, uint64_t userdata,
             const char *const options[]);

/**
 * @brief Deinitialize the Quix library.
 *
 * @note This function should be called when the Quix library is no longer needed.
 * @note This library is reference counted internally, so it is safe to call this function multiple
 * times. When the reference count actually reaches zero, the library will be deinitialized along
 * with any *external* components that were initialized by the library. If the library is not
 * initialized, this function will do nothing.
 *
 * @note To check if the library is initialized, use the `quix_detail_is_init` variable. Beware
 * that this variable is not atomic, therefore may not be safe in a multithreaded environment.
 *
 * @note This function is thread-safe.
 */
void quix_deinit(void);

/**
 * @brief Predefined diagnostic callback that writes to `stdout`.
 */
void quix_diag_stdout(const char *, const char *, uint64_t);

/**
 * @brief Predefined diagnostic callback that writes to `stderr`.
 */
void quix_diag_stderr(const char *, const char *, uint64_t);

#ifdef LIBQUIX_INTERNAL
extern bool quix_detail_is_init;
bool quix_lib_init();
#endif

#ifdef __cplusplus
}
#endif

#endif  // __LIBQUIX_CODE_H__
