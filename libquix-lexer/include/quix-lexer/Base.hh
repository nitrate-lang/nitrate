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

#ifndef __QUIX_LEXER_BASE_H__
#define __QUIX_LEXER_BASE_H__

#include <quix-core/Error.h>
#include <quix-lexer/Lexer.h>
#include <quix-lexer/Token.h>

#include <array>
#include <boost/bimap.hpp>
#include <boost/unordered_map.hpp>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <queue>

#ifndef MEMORY_OVER_SPEED
#include <unordered_map>
#endif

struct qlex_t {
protected:
  ///============================================================================///
  /// BEGIN: PERFORMANCE HYPER PARAMETERS
  static constexpr qlex_size GETC_BUFFER_SIZE = 64;
  static constexpr qlex_size TOKEN_BUF_SIZE = 1;
  /// END:   PERFORMANCE HYPER PARAMETERS
  ///============================================================================///

  struct clever_me_t {
    uint32_t rc_fmt : 1; /* Holds the row-column format? */
    uint32_t col : 10;   /* Column number (max 1024) */
    uint32_t row : 21;   /* Row number (max 2097152) */
  } __attribute__((packed));

  qlex_size m_getc_pos;
  std::array<char, GETC_BUFFER_SIZE> m_getc_buf;

  qlex_size m_tok_pos;
  std::array<qlex_tok_t, TOKEN_BUF_SIZE> m_tok_buf;
  std::queue<qlex_tok_t> m_undo;

  qlex_tok_t m_cur;

  qlex_size m_row;
  qlex_size m_col;
  qlex_size m_offset;

#ifdef MEMORY_OVER_SPEED
  typedef std::shared_ptr<std::pair<boost::bimap<qlex_size, std::string>, qlex_size>>
      StringInterner;
#else
  typedef std::shared_ptr<std::pair<std::unordered_map<qlex_size, std::string>, qlex_size>>
      StringInterner;
#endif

  boost::unordered_map<qlex_size, clever_me_t> m_tag_to_loc;
  boost::unordered_map<qlex_size, qlex_size> m_tag_to_off;

  qlex_size m_locctr;

  qlex_tok_t step_buffer();
  void refill_buffer();

public:
  StringInterner m_strings;

  qlex_flags_t m_flags;

  const char *m_filename;
  FILE *m_file;
  bool m_is_owned;

  ///============================================================================///

  virtual qlex_tok_t next_impl() = 0;

  virtual std::optional<qlex_size> loc2offset(qlex_loc_t loc);
  virtual std::optional<std::pair<qlex_size, qlex_size>> loc2rowcol(qlex_loc_t loc);
  virtual qlex_loc_t save_loc(qlex_size row, qlex_size col, qlex_size offset);
  qlex_loc_t cur_loc();

  ///============================================================================///

  void push_impl(const qlex_tok_t *tok);
  void collect_impl(const qlex_tok_t *tok);

  std::string_view get_string(qlex_size idx);
  qlex_size put_string(std::string_view str);
  void release_string(qlex_size idx);
  virtual void replace_interner(StringInterner new_interner);

  char getc();
  qlex_tok_t next();
  qlex_tok_t peek();

  ///============================================================================///

  qlex_t(FILE *file, const char *filename, bool is_owned)
      : m_getc_pos(GETC_BUFFER_SIZE),
        m_tok_pos(TOKEN_BUF_SIZE),
        m_cur({}),
        m_row(1),
        m_col(1),
        m_offset(0),
        m_tag_to_loc({}),
        m_tag_to_off({}),
        m_locctr(1),
        m_strings(std::make_shared<decltype(m_strings)::element_type>()),
        m_flags(0),
        m_filename(filename),
        m_file(file),
        m_is_owned(is_owned) {
    if (!m_filename) {
      m_filename = "<unknown>";
    }

    m_cur.ty = qErro;
  }
  virtual ~qlex_t() = default;
};

#endif  // __QUIX_LEXER_BASE_H__
