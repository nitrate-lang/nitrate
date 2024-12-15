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

#ifndef __LIBNITRATE_STREAM_H__
#define __LIBNITRATE_STREAM_H__

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
nit_stream_t *nit_join(bool auto_close, size_t num, /* ... FILE* */...);

/// @brief Concatenate multiple streams into a single stream.
nit_stream_t *nit_joinv(bool auto_close, size_t num,
                        /* ... FILE* */ va_list va);

/// @brief Concatenate multiple streams into a single stream.
nit_stream_t *nit_njoin(bool auto_close, size_t num, FILE *const *streams);

///=============================================================================

static inline nit_stream_t *NIT_MEMOPEN(const void *data_ptr,
                                        size_t data_size) {
  return nit_from(fmemopen((void *)data_ptr, data_size, "rb"), true);
}

static inline nit_stream_t *NIT_OPEM_STREAM(char **buf_ptr, size_t *buf_size) {
  return nit_from(open_memstream(buf_ptr, buf_size), true);
}

/// @brief Predefined diagnostic callback that writes to `stdout`.
void nit_diag_stdout(const char *, void *);

/// @brief Predefined diagnostic callback that writes to `stderr`.
void nit_diag_stderr(const char *, void *);

#ifdef __cplusplus
}
#endif

#endif  // __LIBNITRATE_STREAM_H__
