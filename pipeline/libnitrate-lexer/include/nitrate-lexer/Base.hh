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

#ifndef __NITRATE_LEXER_BASE_H__
#define __NITRATE_LEXER_BASE_H__

#include <nitrate-core/Env.h>
#include <nitrate-core/Error.h>
#include <nitrate-lexer/Lexer.h>
#include <nitrate-lexer/Token.h>

#include <array>
#include <boost/bimap.hpp>
#include <boost/unordered_map.hpp>
#include <deque>
#include <limits>
#include <memory>
#include <optional>
#include <string>
#include <string_view>

#if MEMORY_OVER_SPEED == 1
#include <unordered_map>
#endif

struct __attribute__((visibility("default"))) qlex_t {
private:
  ///============================================================================///
  /// BEGIN: PERFORMANCE HYPER PARAMETERS
  static constexpr uint32_t GETC_BUFFER_SIZE = 4096;
  /// END:   PERFORMANCE HYPER PARAMETERS
  ///============================================================================///

  struct clever_me_t {
    uint32_t rc_fmt : 1; /* Holds the row-column format? */
    uint32_t col : 10;   /* Column number (max 1024) */
    uint32_t row : 21;   /* Row number (max 2097152) */

    bool operator<(const clever_me_t &other) const {
      if (row != other.row) {
        return row < other.row;
      } else {
        return col < other.col;
      }
    }
  } __attribute__((packed));

  uint32_t m_getc_pos;
  std::array<char, GETC_BUFFER_SIZE> m_getc_buf;

  std::deque<qlex_tok_t> m_tok_buf;
  std::deque<char> m_pushback;

  qlex_tok_t m_next_tok, m_current_tok;

  uint32_t m_row;
  uint32_t m_col;
  uint32_t m_offset;
  char m_last_ch;

#if MEMORY_OVER_SPEED == 1
  typedef std::shared_ptr<
      std::pair<boost::bimap<uint32_t, std::string>, uint32_t>>
      StringInterner;
#else
  typedef std::shared_ptr<
      std::pair<std::unordered_map<uint32_t, std::string>, uint32_t>>
      StringInterner;
#endif

  boost::bimap<uint32_t, std::pair<uint32_t, uint32_t>> m_off2rc;

  size_t m_locctr;

private:
  qlex_tok_t step_buffer();
  void reset_automata();
  char getc();

public:
  StringInterner m_strings;
  qcore_env_t m_env;

  qlex_flags_t m_flags;

  const char *m_filename;
  std::shared_ptr<std::istream> m_file;

  ///============================================================================///

  virtual qlex_tok_t next_impl();

  virtual std::optional<std::pair<uint32_t, uint32_t>> loc2rowcol(uint32_t loc);
  virtual uint32_t save_loc(uint32_t row, uint32_t col, uint32_t offset);
  uint32_t cur_loc();

  ///============================================================================///

  std::string_view get_string(uint32_t idx);
  uint32_t put_string(std::string_view str);
  void release_string(uint32_t idx);
  virtual void replace_interner(StringInterner new_interner);

  qlex_tok_t next();
  qlex_tok_t peek();
  qlex_tok_t current();

  void push_impl(const qlex_tok_t *tok);
  void collect_impl(const qlex_tok_t *tok);

  ///============================================================================///

  qlex_t(std::shared_ptr<std::istream> file, const char *filename,
         qcore_env_t env)
      : m_getc_pos(GETC_BUFFER_SIZE),
        m_next_tok({}),
        m_current_tok({}),
        m_row(1),
        m_col(0),
        m_offset(std::numeric_limits<uint32_t>::max()),
        m_last_ch(0),
        m_locctr(1),
        m_strings(std::make_shared<decltype(m_strings)::element_type>()),
        m_env(env),
        m_flags(0),
        m_filename(filename),
        m_file(file) {
    if (!m_filename) {
      m_filename = "<unknown>";
    }

    m_next_tok.ty = qErro;
  }
  virtual ~qlex_t() {}
};

#endif  // __NITRATE_LEXER_BASE_H__
