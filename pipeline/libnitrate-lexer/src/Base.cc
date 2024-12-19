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

#include <string.h>

#include <boost/bimap.hpp>
#include <boost/unordered_map.hpp>
#include <cctype>
#include <cmath>
#include <csetjmp>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <ios>
#include <nitrate-core/Logger.hh>
#include <nitrate-core/Macro.hh>
#include <nitrate-lexer/Base.hh>
#include <nitrate-lexer/Lexer.hh>
#include <utility>

CPP_EXPORT void qlex_set_flags(qlex_t *obj, qlex_flags_t flags) {
  obj->m_flags = flags;
}

CPP_EXPORT qlex_flags_t qlex_get_flags(qlex_t *obj) { return obj->flags(); }

CPP_EXPORT void qlex_insert(qlex_t *obj, qlex_tok_t tok) {
  obj->push_impl(&tok);
}

CPP_EXPORT uint32_t qlex_end(qlex_t *L, qlex_tok_t tok) {
  /// TODO: Implement this function
  return UINT32_MAX;
}

CPP_EXPORT const char *qlex_filename(qlex_t *obj) {
  return obj->filename().c_str();
}

CPP_EXPORT uint32_t qlex_line(qlex_t *obj, uint32_t loc) {
  try {
    auto r = obj->loc2rowcol(loc);
    if (!r) {
      return QLEX_EOFF;
    }

    return r->first;
  } catch (...) {
    qcore_panic("qlex_line: failed to get line number");
  }
}

CPP_EXPORT uint32_t qlex_col(qlex_t *obj, uint32_t loc) {
  try {
    auto r = obj->loc2rowcol(loc);
    if (!r) {
      return QLEX_EOFF;
    }

    return r->second;
  } catch (...) {
    qcore_panic("qlex_col: failed to get column number");
  }
}

CPP_EXPORT char *qlex_snippet(qlex_t *obj, qlex_tok_t tok, uint32_t *offset) {
  try {
#define SNIPPET_SIZE 100

    uint32_t tok_beg_offset;
    char snippet_buf[SNIPPET_SIZE];
    size_t curpos, seek_base_pos, read;

    { /* Convert the location to an offset into the source */
      auto src_offset_opt = tok.start;
      if (!src_offset_opt) {
        return nullptr; /* Return early if translation failed */
      }

      tok_beg_offset = src_offset_opt - 1;
    }

    { /* Calculate offsets and seek to the correct position */
      curpos = obj->m_file.tellg();
      if ((long)curpos == -1) {
        return nullptr;
      }
      seek_base_pos = tok_beg_offset < SNIPPET_SIZE / 2
                          ? 0
                          : tok_beg_offset - SNIPPET_SIZE / 2;

      obj->m_file.seekg(seek_base_pos, std::ios_base::beg);
    }

    { /* Read the snippet and calculate token offset */
      obj->m_file.read(snippet_buf, SNIPPET_SIZE);
      read = obj->m_file.gcount();
      memset(snippet_buf + read, 0, SNIPPET_SIZE - read);

      if (tok_beg_offset < SNIPPET_SIZE / 2) {
        *offset = tok_beg_offset;
      } else {
        *offset = SNIPPET_SIZE / 2;
      }
    }

    // Extract the line that contains the token
    uint32_t slice_start = 0;

    for (size_t i = 0; i < read; i++) {
      if (snippet_buf[i] == '\n') {
        slice_start = i + 1;
      } else if (i == *offset) { /* Danger ?? */
        size_t slice_end;
        for (slice_end = i; slice_end < read; slice_end++) {
          char ch = snippet_buf[slice_end];
          if (ch == '\n' || ch == 0) {
            break;
          }
        }

        uint32_t slice_size = slice_end - slice_start;
        char *output = (char *)malloc(slice_size + 1);
        if (!output) {
          qcore_panic("qlex_snippet: failed to allocate memory");
        }
        memcpy(output, snippet_buf + slice_start, slice_size);
        output[slice_size] = '\0';
        *offset -= slice_start;
        obj->m_file.seekg(curpos, std::ios_base::beg);
        return output;
      }
    }

    obj->m_file.seekg(curpos, std::ios_base::beg);

    return nullptr;
  } catch (std::bad_alloc &) {
    return nullptr;
  } catch (...) {
    qcore_panic("qlex_snippet: failed to get snippet");
  }
}

///============================================================================///

CPP_EXPORT std::optional<std::pair<uint32_t, uint32_t>> qlex_t::loc2rowcol(
    uint32_t loc) {
  if (m_off2rc.left.find(loc) == m_off2rc.left.end()) [[unlikely]] {
    return std::nullopt;
  }

  return m_off2rc.left.at(loc);
}

CPP_EXPORT uint32_t qlex_t::save_loc(uint32_t row, uint32_t col,
                                     uint32_t offset) {
  m_off2rc.insert({offset, {row, col}});
  return offset;
}

CPP_EXPORT uint32_t qlex_t::cur_loc() {
  return save_loc(m_row, m_col, m_offset);
}

///============================================================================///

class GetCExcept {};

char qlex_t::getc() {
  /* Refill the buffer if necessary */
  if (m_getc_pos == GETC_BUFFER_SIZE) [[unlikely]] {
    m_file.read(m_getc_buf.data(), GETC_BUFFER_SIZE);
    size_t read = m_file.gcount();

    if (read == 0) [[unlikely]] {
      throw GetCExcept();
    }

    memset(m_getc_buf.data() + read, '#', GETC_BUFFER_SIZE - read);
    m_getc_pos = 0;
  }

  { /* Update source location */
    if (m_last_ch == '\n') {
      m_row++;
      m_col = 1;
    } else {
      m_col++;
    }

    m_offset++;
  }

  return m_last_ch = m_getc_buf[m_getc_pos++];
}

qlex_tok_t qlex_t::step_buffer() {
  qlex_tok_t tok;

  if (m_tok_buf.empty()) {
    try {
      return next_impl();
    } catch (GetCExcept &) {
      tok.ty = qEofF;
    }
  } else {
    tok = m_tok_buf.front();
    m_tok_buf.pop_front();
  }
  return tok;
}

CPP_EXPORT qlex_tok_t qlex_t::next() {
  qlex_tok_t tok;

  if (m_next_tok.has_value()) {
    tok = m_next_tok.value();
    m_next_tok.reset();
  } else {
    do {
      m_next_tok.reset();
      tok = step_buffer();
    } while (m_flags & QLEX_NO_COMMENTS && tok.ty == qNote);
  }

  m_current_tok = tok;
  return tok;
}

CPP_EXPORT qlex_tok_t qlex_t::peek() {
  if (m_next_tok.has_value()) {
    m_current_tok = m_next_tok;
    return m_next_tok.value();
  }

  m_current_tok = (m_next_tok = next());
  return m_current_tok.value();
}

CPP_EXPORT qlex_tok_t qlex_t::current() {
  return m_current_tok.value_or(qlex_tok_t());
}

///============================================================================///

CPP_EXPORT void qlex_t::push_impl(const qlex_tok_t *tok) {
  m_tok_buf.push_front(*tok);
  m_next_tok.reset();
}
