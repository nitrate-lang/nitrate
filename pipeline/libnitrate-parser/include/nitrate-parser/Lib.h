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

#ifndef __NITRATE_PARSER_LIB_H__
#define __NITRATE_PARSER_LIB_H__

#include <nitrate-parser/Node.h>
#include <nitrate-parser/Parser.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the library.
 *
 * @return true if the library was initialized successfully.
 * @note This function is thread-safe.
 * @note The library is reference counted, so it is safe to call this function
 * multiple times. Each time will not reinitialize the library, but will
 * increment the reference count.
 */
bool qparse_lib_init();

/**
 * @brief Deinitialize the library.
 *
 * @note This function is thread-safe.
 * @note The library is reference counted, so it is safe to call this function
 * multiple times. Each time will not deinitialize the library, but when
 * the reference count reaches zero, the library will be deinitialized.
 */
void qparse_lib_deinit();

/**
 * @brief Get the version of the library.
 *
 * @return The version string of the library.
 * @warning Don't free the returned string.
 * @note This function is thread-safe.
 * @note This function is safe to call before initialization and after
 * deinitialization.
 */
const char* qparse_lib_version();

/**
 * @brief Get the last error message from the current thread.
 *
 * @return The last error message from the current thread.
 * @warning Don't free the returned string.
 * @note This function is thread-safe.
 * @note This function is safe to call before initialization and after
 * deinitialization.
 */
const char* qparse_strerror();

#ifdef __cplusplus
}
#endif

#endif  // __NITRATE_PARSER_LIB_H__
