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

#ifndef __NITRATE_CODEGEN_CONFIG_H__
#define __NITRATE_CODEGEN_CONFIG_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#if defined(__cplusplus) && defined(__NITRATE_CODEGEN_IMPL__)
#include <initializer_list>
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct QcodeConfT QcodeConfT;

typedef enum qcode_key_t {
  QCK_UNKNOWN = 0,
  QCK_CRASHGUARD,
  QCV_FASTERROR,
} QcodeKeyT;

typedef enum qcode_val_t {
  QCV_UNKNOWN = 0,
  QCV_TRUE,
  QCV_FALSE,
  QCV_ON = QCV_TRUE,
  QCV_OFF = QCV_FALSE,
} QcodeValT;

///==========================================================================///

#if defined(__cplusplus) && defined(__NITRATE_CODEGEN_IMPL__)
}
typedef struct QcodeSettingT {
  qcode_key_t m_key;
  qcode_val_t m_value;

#if defined(__cplusplus) && defined(__NITRATE_CODEGEN_IMPL__)
  constexpr QcodeSettingT(const std::initializer_list<int> &list)
      : m_key(static_cast<qcode_key_t>(list.begin()[0])),
        m_value(static_cast<qcode_val_t>(list.begin()[1])) {}
#endif
} QcodeSettingT;

extern "C" {
#else

typedef struct QcodeSettingT {
  QcodeKeyT m_key;
  QcodeValT m_value;
} QcodeSettingT;

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
 * the `qcode_conf_free` function. Do NOT use the `free()` function to free
 * the configuration object directly.
 *
 * @note The default configuration values are specified in the documentation
 * and in the source code. See the `qcode_conf_getopts` function implementation
 * for more information.
 *
 * @note The exact values of the default configuration options are subject to
 * change in future versions of the library. A best effort is made to ensure
 * that the default values are reasonable and **SECURE** for the language
 * sandbox, with a stronger emphasis on security.
 *
 * @note This function is thread-safe.
 */
QcodeConfT *QcodeConfNew(bool use_defaults);

/**
 * @brief Free a configuration object.
 *
 * @param conf A pointer to the configuration object to free or NULL.
 *
 * @warning The caller for ensuring that the configuration object is not used
 * after it has been freed. This includes ensuring that no other threads or
 * instances have references to the configuration object.
 *
 * @note It is safe to pass NULL to this function.
 *
 * @note This function is thread-safe.
 */
void QcodeConfFree(QcodeConfT *conf);

/**
 * @brief Set a configuration option.
 *
 * @param conf Pointer to configuration context.
 * @param key Configuration option to set.
 * @param value Configuration value to set.
 *
 * @return True on success, false on failure.
 *
 * @note If the configuration change causes a known issue, the function will
 * return false and the configuration will not be changed. This check is not
 * guaranteed to catch all issues, but it is a best effort to prevent common
 * mistakes.
 *
 * @note This function is thread-safe.
 */
bool QcodeConfSetopt(QcodeConfT *conf, QcodeKeyT key, QcodeValT value);

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
bool QcodeConfGetopt(QcodeConfT *conf, QcodeKeyT key, QcodeValT *value);

/**
 * @brief Get readonly access to the configuration options.
 *
 * @param conf Pointer to configuration context.
 * @param count Pointer to store the number of configuration options. NULL is
 * not allowed.
 *
 * @return malloc'd array of configuration options on success OR NULL on
 * failure. User is responsible for freeing the array using `free()`.
 *
 * @note This function is thread-safe.
 */
QcodeSettingT *QcodeConfGetopts(QcodeConfT *conf, size_t *count);

/**
 * @brief Clear the configuration options.
 *
 * @param conf Pointer to configuration context.
 *
 * @note Will simply remove all configuration options from the configuration
 * object (including defaults).
 *
 * @note This function is thread-safe.
 */
void QcodeConfClear(QcodeConfT *conf);

#ifdef __cplusplus
}
#endif

#endif  // __NITRATE_CODEGEN_CONFIG_H__
