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

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct quixcc_engine_t quixcc_engine_t;

typedef struct quixcc_expr_t quixcc_expr_t;

enum quixcc_msg_level_t {
  QUIXCC_RAW = 0,
  QUIXCC_DEBUG = 1,
  QUIXCC_SUCCESS = 2,
  QUIXCC_INFO = 3,
  QUIXCC_WARN = 4,
  QUIXCC_ERROR = 5,
  QUIXCC_FATAL = 6
};

struct quixcc_msg_t {
  uint64_t line;
  uint64_t column;
  const char *message;
  enum quixcc_msg_level_t m_level;
};

typedef struct quixcc_status_t {
  struct quixcc_msg_t **m_messages;
  uint32_t m_count;
  bool m_success;
} quixcc_status_t;

typedef struct quixcc_cc_job_t quixcc_cc_job_t;

typedef enum {
  QUIXCC_LEXCONF_IGN_COM = 1 << 0,
} quixcc_lexer_config_t;

typedef bool (*quixcc_qsys_impl_t)(quixcc_engine_t *engine, uint32_t num, quixcc_expr_t **expr,
                                   uint32_t argc);

typedef enum quixcc_iterm_t {
  /* Any order is acceptable */
  QUIXCC_TRAV_ANY = 0,

  /* Depth-first pre-order traversal */
  QUIXCC_TRAV_DFPRE = 1,

  /* Depth-first post-order traversal */
  QUIXCC_TRAV_DFPOST = 2,

  /* Breadth-first pre-order traversal */
  QUIXCC_TRAV_BFPRE = 3,

  /* Breadth-first post-order traversal */
  QUIXCC_TRAV_BFPOST = 4,
} quixcc_iterm_t;

typedef struct quixcc_string_t {
  char *m_data;
  size_t m_len;
} quixcc_string_t;

typedef struct quixcc_vec_t {
  void *m_data;
  uint32_t m_len;
  uint32_t m_cap;
  uint32_t m_elem_size;
} quixcc_vec_t;

#ifdef __cplusplus
}
#endif

#endif // __QUIXCC_TYPES_H__
