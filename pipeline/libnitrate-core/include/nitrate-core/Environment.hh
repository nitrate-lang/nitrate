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

#ifndef __NITRATE_CORE_ENV_H__
#define __NITRATE_CORE_ENV_H__

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uintptr_t qcore_env_t;

/**
 * @brief Create a new environment.
 * @return The environment handle otherwise a NOP if the environment already
 * exists.
 */
qcore_env_t qcore_env_create(qcore_env_t env);

/**
 * @brief Drop a reference to an environment.
 * @param env The environment handle.
 * @note The environment will be destroyed when the last reference is dropped.
 */
void qcore_env_destroy(qcore_env_t env);

/**
 * @brief Get the current environment.
 * @return The current environment handle.
 */
qcore_env_t qcore_env_current();

/**
 * @brief Set the current environment.
 * @param env The environment handle.
 */
void qcore_env_set_current(qcore_env_t env);

/**
 * @brief Get the value of an environment variable.
 * @param key The environment variable key.
 * @return The value of the environment variable, or NULL to unset the variable.
 * @note Strings will be cloned internally.
 */
void qcore_env_set(const char *key, const char *value);

#define qcore_env_unset(key) qcore_env_set(key, NULL)

/**
 * @brief Get the value of an environment variable.
 * @param key The environment variable key.
 * @return The value of the environment variable, or NULL if the variable is not
 * set.
 * @note Strings will be cloned internally.
 */
const char *qcore_env_get(const char *key);

#ifdef __cplusplus
}
#endif

#endif  // __NITRATE_CORE_ENV_H__
