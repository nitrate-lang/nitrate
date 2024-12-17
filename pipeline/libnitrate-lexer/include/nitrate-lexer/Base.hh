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

#include <array>
#include <boost/bimap.hpp>
#include <boost/unordered_map.hpp>
#include <deque>
#include <limits>
#include <nitrate-core/Environment.hh>
#include <nitrate-core/Logger.hh>
#include <nitrate-lexer/Lexer.hh>
#include <nitrate-lexer/Token.hh>
#include <optional>

struct __attribute__((visibility("default"))) qlex_t {
private:
  static constexpr uint32_t GETC_BUFFER_SIZE = 256;

  struct srcloc_t {
    uint32_t rc_fmt : 1; /* Holds the row-column format? */
    uint32_t col : 10;   /* Column number (max 1024) */
    uint32_t row : 21;   /* Row number (max 2097152) */

    bool operator<(const srcloc_t &other) const {
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

  std::optional<qlex_tok_t> m_next_tok, m_current_tok;

  uint32_t m_row;
  uint32_t m_col;
  uint32_t m_offset;
  char m_last_ch;

  boost::bimap<uint32_t, std::pair<uint32_t, uint32_t>> m_off2rc;

public:
  std::string m_filename;
  std::istream &m_file;
  qlex_flags_t m_flags;
  qcore_env_t m_env;

  qlex_tok_t step_buffer();
  void reset_automata();
  char getc();

  ///============================================================================///

  virtual qlex_tok_t next_impl();

  virtual std::optional<std::pair<uint32_t, uint32_t>> loc2rowcol(
      uint32_t offset);
  virtual uint32_t save_loc(uint32_t row, uint32_t col, uint32_t offset);
  uint32_t cur_loc();

  ///============================================================================///

  qlex_tok_t next();
  qlex_tok_t peek();
  qlex_tok_t current();

  void push_impl(const qlex_tok_t *tok);

  ///============================================================================///

  qlex_t(std::istream &file, const char *filename, qcore_env_t env)
      : m_getc_pos(GETC_BUFFER_SIZE),
        m_next_tok({}),
        m_current_tok({}),
        m_row(1),
        m_col(0),
        m_offset(std::numeric_limits<uint32_t>::max()),
        m_last_ch(0),
        m_filename(filename ? filename : "<unknown>"),
        m_file(file),
        m_flags(0),
        m_env(env) {}
  virtual ~qlex_t() {}

  const std::string &filename() const { return m_filename; }
  qlex_flags_t flags() const { return m_flags; }
  qcore_env_t env() const { return m_env; }
};

#endif  // __NITRATE_LEXER_BASE_H__
