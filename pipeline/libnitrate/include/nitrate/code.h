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

#ifndef __LIBNITRATE_CODE_H__
#define __LIBNITRATE_CODE_H__

#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * @brief Generic Nitrate Toolchain tranformation function.                   *
 *                                                                            *
 ******************************************************************************
 *                                                                            *
 * @param in      Transform input                                             *
 * @param out     Transform output                                            *
 * @param options NULL-terminated array of options, or `NULL` for empty       *
 *                                                                            *
 * @return `true` if the transformation was successful, `false` otherwise.    *
 *                                                                            *
 ******************************************************************************
 *                                                                            *
 * @note The input and output streams are not closed by this function. It is  *
 * the caller's responsibility to close the streams. If either the input or   *
 * output stream is `NULL`, the function will return `false`.                 *
 *                                                                            *
 * @warning Some internal validation is performed on the `options` parameter. *
 * However, there is no guarantee that all permutations of options are        *
 * guaranteed to trigger well-defined deterministic transformations. If an    *
 * option is not recognized, the transformer will fail. See the documentation *
 * for more information on available options and their respective usage.      *
 *                                                                            *
 * @note Dependency initialization and deinitialization is handled            *
 * automatically. To increase performance for multiple calls, consider        *
 * manually initializing/deinitializing this library's dependencies.          *
 *                                                                            *
 * @note This function is thread-safe. However, it it may block in sometimes. *
 * Some components can limit the number of concurrent internal contexts that  *
 * may be created. Such internal operations will block until a slot is able   *
 * to be allocated.                                                           *
 *                                                                            *
 ******************************************************************************
 * @note This function is an ideal target for fuzz based testing              *
 *****************************************************************************/
auto NitPipeline(FILE *in, FILE *out, const char *const options[]) -> bool;

/******************************************************************************/

#ifdef __cplusplus
}
#endif

#endif  // __LIBNITRATE_CODE_H__
