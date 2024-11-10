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

#ifndef __NITRATE_QXIR_CONFIG_H__
#define __NITRATE_QXIR_CONFIG_H__

#include <nitrate-ir/TypeDecl.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#if defined(__cplusplus) && defined(__NITRATE_QXIR_IMPL__)
#include <initializer_list>
#endif

#ifdef __cplusplus
extern "C" {
#endif

///==========================================================================///

#if defined(__cplusplus) && defined(__NITRATE_QXIR_IMPL__)
}
typedef struct qxir_setting_t {
  qxir_key_t key;
  qxir_val_t value;

#if defined(__cplusplus) && defined(__NITRATE_QXIR_IMPL__)
  constexpr qxir_setting_t(const std::initializer_list<int> &list)
      : key(static_cast<qxir_key_t>(list.begin()[0])),
        value(static_cast<qxir_val_t>(list.begin()[1])) {}
#endif
} qxir_setting_t;

extern "C" {
#else

typedef struct qxir_setting_t {
  qxir_key_t key;
  qxir_val_t value;
} qxir_setting_t;

#endif

/**
 * @brief Create a new configuration object.
 *
 * @param use_defaults If true, the configuration object will be initialized
 * with default values. Otherwise, an empty configuration object will be
 * created.
 *
 * @return A pointer to the newly created configuration object on success OR
 * NULL on failure.
 *
 * @warning The caller is responsible for freeing the configuration object using
 * the `qxir_conf_free` function. Do NOT use the `free()` function to free
 * the configuration object directly.
 *
 * @note The default configuration values are specified in the documentation
 * and in the source code. See the `qxir_conf_getopts` function implementation
 * for more information.
 *
 * @note The exact values of the default configuration options are subject to
 * change in future versions of the library. A best effort is made to ensure that
 * the default values are reasonable and **SECURE** for the language sandbox, with a stronger
 * emphasis on security.
 *
 * @note This function is thread-safe.
 */
qxir_conf_t *qxir_conf_new(bool use_defaults);

/**
 * @brief Free a configuration object.
 *
 * @param conf A pointer to the configuration object to free or NULL.
 *
 *
 * @warning The caller for ensuring that the configuration object is not used
 * after it has been freed. This includes ensuring that no other threads or instances
 * have references to the configuration object.
 *
 * @note It is safe to pass NULL to this function.
 *
 * @note This function is thread-safe.
 */
void qxir_conf_free(qxir_conf_t *conf);

/**
 * @brief Set a configuration option.
 *
 * @param conf Pointer to configuration context.
 * @param key Configuration option to set.
 * @param value Configuration value to set.
 *
 * @return True on success, false on failure.
 *
 * @note If the configuration change causes a known issue, the function will return false and the
 * configuration will not be changed. This check is not guaranteed to catch all issues, but it is a
 * best effort to prevent common mistakes.
 *
 * @note This function is thread-safe.
 */
bool qxir_conf_setopt(qxir_conf_t *conf, qxir_key_t key, qxir_val_t value);

/**
 * @brief Get the value of a configuration option.
 *
 * @param conf Pointer to configuration context.
 * @param key Configuration option to get.
 * @param value Pointer to store the configuration value. NULL is allowed.
 *
 * @return True if the configuration option is set, false otherwise.
 *
 * @note This function is thread-safe.
 */
bool qxir_conf_getopt(qxir_conf_t *conf, qxir_key_t key, qxir_val_t *value);

/**
 * @brief Get readonly access to the configuration options.
 *
 * @param conf Pointer to configuration context.
 * @param count Pointer to store the number of configuration options. NULL is not allowed.
 *
 * @return malloc'd array of configuration options on success OR NULL on failure. User is
 * responsible for freeing the array using `free()`.
 *
 * @note This function is thread-safe.
 */
qxir_setting_t *qxir_conf_getopts(qxir_conf_t *conf, size_t *count);

/**
 * @brief Clear the configuration options.
 *
 * @param conf Pointer to configuration context.
 *
 * @note Will simply remove all configuration options from the configuration object (including
 * defaults).
 *
 * @note This function is thread-safe.
 */
void qxir_conf_clear(qxir_conf_t *conf);

/**
 * @brief Dump the configuration options to a stream.
 *
 * @param conf Pointer to configuration context.
 * @param stream File stream to write the configuration options to.
 * @param field_delim Delimiter to separate the key and value of each configuration option, or NULL
 * to use the default delimiter.
 * @param line_delim Delimiter to separate each configuration option, or NULL to use the default
 * delimiter.
 *
 * @return The number of bytes written to the stream on success OR 0 on failure.
 *
 * @note This function is thread-safe.
 */
size_t qxir_conf_dump(qxir_conf_t *conf, FILE *stream, const char *field_delim,
                      const char *line_delim);

#ifdef __cplusplus
}
#endif

#endif  // __NITRATE_QXIR_CONFIG_H__
