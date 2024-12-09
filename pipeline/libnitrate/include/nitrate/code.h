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

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/// @brief Nitrate stream type.
typedef struct nit_stream_t nit_stream_t;

/// @brief Close a Nitrate stream.
void nit_fclose(nit_stream_t *);

/// @brief Create a Nitrate stream from a C stream.
nit_stream_t *nit_from(FILE *, bool auto_close);

/// @brief Concatenate multiple streams into a single stream.
nit_stream_t *nit_join(size_t num, /* ... FILE* */...);

/// @brief Concatenate multiple streams into a single stream.
nit_stream_t *nit_joinv(size_t num, /* ... FILE* */ va_list va);

/// @brief Concatenate multiple streams into a single stream.
nit_stream_t *nit_njoin(size_t num, FILE **streams);

///=============================================================================

static inline nit_stream_t *NIT_MEMOPEN(const void *data_ptr,
                                        size_t data_size) {
  return nit_from(fmemopen((void *)data_ptr, data_size, "rb"), true);
}

static inline nit_stream_t *NIT_OPEM_STREAM(char **buf_ptr, size_t *buf_size) {
  return nit_from(open_memstream(buf_ptr, buf_size), true);
}

/// @brief Predefined diagnostic callback that writes to `stdout`.
void nit_diag_stdout(const char *, const char *, void *);

/// @brief Predefined diagnostic callback that writes to `stderr`.
void nit_diag_stderr(const char *, const char *, void *);

/// @brief Diagnostic callback function prototype.
typedef void (*nit_diag_cb)(const char *message, const char *by, void *opaque);

/******************************************************************************
 * @brief Generic Nitrate Toolchain tranformation function.                   *
 *                                                                            *
 ******************************************************************************
 *                                                                            *
 * @param in      Transform input, or `NULL` to indicate no input             *
 * @param out     Transform output, or `NULL` to discard output               *
 * @param diag    Diagnostic callback function, or `NULL` to discard          *
 * @param opaque  Context to pass to the diagnostic callback                  *
 * @param options NULL-terminated array of options, or `NULL` for empty       *
 *                                                                            *
 * @return `true` if the transformation was successful, `false` otherwise.    *
 *                                                                            *
 ******************************************************************************
 *                                                                            *
 * @note The input and output streams are not closed by this function. It is  *
 * the caller's responsibility to close the streams. The input and output     *
 * streams cannot be the same.                                                *
 *                                                                            *
 * @warning Some internal validation is performed on the `options` parameter. *
 * However, there is no guarantee that all permutations of options are        *
 * guaranteed to trigger well-defined deterministic transformations. If an    *
 * option is not recognized, the transformer will fail. See the documentation *
 * for more information on available options and their respective usage.      *
 *                                                                            *
 * @note Dependency initialization and deinitialization are handled           *
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
bool nit_cc(nit_stream_t *in, nit_stream_t *out, nit_diag_cb diag, void *opaque,
            const char *const options[]);

#ifdef __cplusplus
}
#endif

#endif  // __LIBNITRATE_CODE_H__
