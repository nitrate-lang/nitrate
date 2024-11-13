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

#define __NITRATE_LEXER_IMPL__

#include <nitrate-core/Error.h>
#include <nitrate-lexer/Lexer.h>
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
#include <nitrate-lexer/Base.hh>
#include <sstream>
#include <utility>

#include "LibMacro.h"
#include "nitrate-core/Env.h"
#include "nitrate-lexer/Token.h"

///============================================================================///

LIB_EXPORT qlex_t *qlex_direct(const char *src, size_t len, const char *filename, qcore_env_t env) {
  try {
    if (!filename) {
      filename = "<unknown>";
    }

    auto ss = std::make_shared<std::istringstream>(std::string(src, len));

    qlex_t *obj = qlex_new(ss, filename, env);
    if (!obj) {
      return nullptr;
    }

    return obj;
  } catch (std::bad_alloc &) {
    return nullptr;
  } catch (...) {
    return nullptr;
  }
}

LIB_EXPORT void qlex_free(qlex_t *obj) {
  try {
    delete obj;
  } catch (...) {
    qcore_panic("qlex_free: failed to free lexer");
  }
}

LIB_EXPORT void qlex_set_flags(qlex_t *obj, qlex_flags_t flags) { obj->m_flags = flags; }
LIB_EXPORT qlex_flags_t qlex_get_flags(qlex_t *obj) { return obj->m_flags; }

LIB_EXPORT void qlex_collect(qlex_t *obj, const qlex_tok_t *tok) { obj->collect_impl(tok); }

LIB_EXPORT void qlex_insert(qlex_t *obj, qlex_tok_t tok) { obj->push_impl(&tok); }
LIB_EXPORT const char *qlex_filename(qlex_t *obj) { return obj->m_filename; }

LIB_EXPORT uint32_t qlex_line(qlex_t *obj, uint32_t loc) {
  try {
    auto r = obj->loc2rowcol(loc);
    if (!r) {
      return UINT32_MAX;
    }

    return r->first;
  } catch (...) {
    qcore_panic("qlex_line: failed to get line number");
  }
}

LIB_EXPORT uint32_t qlex_col(qlex_t *obj, uint32_t loc) {
  try {
    auto r = obj->loc2rowcol(loc);
    if (!r) {
      return UINT32_MAX;
    }

    return r->second;
  } catch (...) {
    qcore_panic("qlex_col: failed to get column number");
  }
}

LIB_EXPORT uint32_t qlex_offset(qlex_t *obj, uint32_t base, uint32_t offset) {
  try {
    long curpos = 0;
    std::optional<uint32_t> seek_base_pos;
    uint8_t *buf = nullptr;
    std::streamsize bufsz = 0;
    uint32_t res = 0;

    if (!(seek_base_pos = base)) {
      return res;
    }

    if ((curpos = obj->m_file->tellg()) == -1) {
      return res;
    }

    obj->m_file->seekg(*seek_base_pos + offset, std::ios_base::beg);

    bufsz = offset;

    if ((buf = (uint8_t *)malloc(bufsz + 1)) == nullptr) {
      obj->m_file->seekg(curpos, std::ios_base::beg);
      return res;
    }

    if (!obj->m_file->read((char *)buf, bufsz)) {
      free(buf);
      obj->m_file->seekg(curpos, std::ios_base::beg);

      return res;
    }

    buf[bufsz] = '\0';
    obj->m_file->seekg(curpos, std::ios_base::beg);

    //===== AUTOMATA TO CALCULATE THE NEW ROW AND COLUMN =====//
    uint32_t row, col;

    if ((row = qlex_line(obj, base)) == UINT32_MAX) {
      free(buf);
      return res;
    }

    if ((col = qlex_col(obj, base)) == UINT32_MAX) {
      free(buf);
      return res;
    }

    for (std::streamsize i = 0; i < bufsz; i++) {
      if (buf[i] == '\n') {
        row++;
        col = 1;
      } else {
        col++;
      }
    }

    free(buf);

    return obj->save_loc(row, col, *seek_base_pos + offset);
  } catch (...) {
    qcore_panic("qlex_offset: failed to calculate offset");
  }
}

LIB_EXPORT uint32_t qlex_spanx(qlex_t *obj, uint32_t start, uint32_t end,
                               void (*callback)(const char *, uint32_t, uintptr_t),
                               uintptr_t userdata) {
  try {
    std::optional<uint32_t> begoff, endoff;

    if (!(begoff = start)) {
      return UINT32_MAX;
    }

    if (!(endoff = end)) {
      return UINT32_MAX;
    }

    if (*endoff < *begoff) {
      return UINT32_MAX;
    }

    uint32_t span = *endoff - *begoff;

    long curpos = 0;
    uint8_t *buf = nullptr;
    std::streamsize bufsz = 0;

    if ((curpos = obj->m_file->tellg()) == -1) {
      return UINT32_MAX;
    }

    obj->m_file->seekg(*begoff, std::ios_base::beg);

    bufsz = span;

    if ((buf = (uint8_t *)malloc(bufsz + 1)) == nullptr) {
      obj->m_file->seekg(curpos, std::ios_base::beg);
      return UINT32_MAX;
    }

    if (!obj->m_file->read((char *)buf, bufsz)) {
      free(buf);
      obj->m_file->seekg(curpos, std::ios_base::beg);
      return UINT32_MAX;
    }

    buf[bufsz] = '\0';
    obj->m_file->seekg(curpos, std::ios_base::beg);

    callback((const char *)buf, bufsz, userdata);

    free(buf);
    return span;
  } catch (...) {
    qcore_panic("qlex_spanx: failed to calculate span");
  }
}

LIB_EXPORT void qlex_rect(qlex_t *obj, uint32_t x_0, uint32_t y_0, uint32_t x_1, uint32_t y_1,
                          char *out, size_t max_size, char fill) {
  try {
    // Bounds check rectangle
    if (x_0 > x_1 || y_0 > y_1) [[unlikely]] {
      qcore_panic("qlex_rect: invalid rectangle bounds");
    }

    // Calculate the size of the rectangle
    size_t width = x_1 - x_0;
    size_t height = y_1 - y_0;
    size_t buf_size = width * height;

    // Includes null terminator
    if (buf_size + 1 > max_size) [[unlikely]] {
      qcore_panic("qlex_rect: buffer too small");
    }

    memset(out, fill, buf_size);
    out[buf_size] = '\0';

    for (size_t i = 0; i < height; i++) {
      uint32_t start_off = 0, end_off = 10;

      uint32_t start = obj->save_loc(y_0 + i, x_0, start_off);
      uint32_t end = obj->save_loc(y_0 + i, x_1, end_off);

      qlex_spanx(
          obj, start, end,
          [](const char *str, uint32_t len, uintptr_t ptr) { memcpy((void *)ptr, str, len); },
          (uintptr_t)(out + i * width));
    }
  } catch (...) {
  }
}

LIB_EXPORT char *qlex_snippet(qlex_t *obj, qlex_tok_t tok, uint32_t *offset) {
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
      curpos = obj->m_file->tellg();
      if ((long)curpos == -1) {
        return nullptr;
      }
      seek_base_pos = tok_beg_offset < SNIPPET_SIZE / 2 ? 0 : tok_beg_offset - SNIPPET_SIZE / 2;

      obj->m_file->seekg(seek_base_pos, std::ios_base::beg);
    }

    { /* Read the snippet and calculate token offset */
      obj->m_file->read(snippet_buf, SNIPPET_SIZE);
      read = obj->m_file->gcount();
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
        obj->m_file->seekg(curpos, std::ios_base::beg);
        return output;
      }
    }

    obj->m_file->seekg(curpos, std::ios_base::beg);

    return nullptr;
  } catch (std::bad_alloc &) {
    return nullptr;
  } catch (...) {
    qcore_panic("qlex_snippet: failed to get snippet");
  }
}

LIB_EXPORT qlex_tok_t qlex_next(qlex_t *self) {
  try {
    qcore_env_t old = qcore_env_current();
    qcore_env_set_current(self->m_env);

    qlex_tok_t tok = self->next();

    qcore_env_set_current(old);

    return tok;
  } catch (...) {
    qcore_panic("qlex_next: failed to get next token");
  }
}

LIB_EXPORT qlex_tok_t qlex_peek(qlex_t *self) {
  try {
    qcore_env_t old = qcore_env_current();
    qcore_env_set_current(self->m_env);

    qlex_tok_t tok = self->peek();

    qcore_env_set_current(old);

    return tok;
  } catch (...) {
    qcore_panic("qlex_peek: failed to peek next token");
  }
}

///============================================================================///

CPP_EXPORT std::optional<std::pair<uint32_t, uint32_t>> qlex_t::loc2rowcol(uint32_t loc) {
  if (m_off2rc.left.find(loc) == m_off2rc.left.end()) [[unlikely]] {
    return std::nullopt;
  }

  return m_off2rc.left.at(loc);
}

CPP_EXPORT uint32_t qlex_t::save_loc(uint32_t row, uint32_t col, uint32_t offset) {
  m_off2rc.insert({offset, {row, col}});
  return offset;
}

CPP_EXPORT uint32_t qlex_t::cur_loc() { return save_loc(m_row, m_col, m_offset); }

///============================================================================///

CPP_EXPORT std::string_view qlex_t::get_string(uint32_t idx) {
#if MEMORY_OVER_SPEED == 1
  if (auto it = m_strings->first.left.find(idx); it != m_strings->first.left.end()) [[likely]] {
    return it->second;
  }
#else
  if (auto it = m_strings->first.find(idx); it != m_strings->first.end()) [[likely]] {
    return it->second;
  }
#endif

  return "";
}

CPP_EXPORT uint32_t qlex_t::put_string(std::string_view str) {
#if MEMORY_OVER_SPEED == 1
  if (auto it = m_strings->first.right.find(str); it != m_strings->first.right.end()) {
    return it->second;
  }

  m_strings->first.insert({m_strings->second, std::string(str)});
  return m_strings->second++;
#else
  if (str.empty()) [[unlikely]] {
    return UINT32_MAX;
  }

  (*m_strings).first[m_strings->second] = std::string(str);
  return m_strings->second++;
#endif
}

CPP_EXPORT void qlex_t::release_string(uint32_t idx) {
#if MEMORY_OVER_SPEED == 1

#else
  if (auto it = m_strings->first.find(idx); it != m_strings->first.end()) [[likely]] {
    m_strings->first.erase(it);
  }
#endif
}

CPP_EXPORT void qlex_t::replace_interner(StringInterner new_interner) { m_strings = new_interner; }

///============================================================================///

class GetCExcept {};

char qlex_t::getc() {
  /* Refill the buffer if necessary */
  if (m_getc_pos == GETC_BUFFER_SIZE) [[unlikely]] {
    m_file->read(m_getc_buf.data(), GETC_BUFFER_SIZE);
    size_t read = m_file->gcount();

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

  if (m_next_tok.ty != qErro) {
    tok = m_next_tok;
    m_next_tok.ty = qErro;
  } else {
    do {
      m_next_tok.ty = qErro;
      tok = step_buffer();
    } while (m_flags & QLEX_NO_COMMENTS && tok.ty == qNote);
  }

  return tok;
}

CPP_EXPORT qlex_tok_t qlex_t::peek() {
  if (m_next_tok.ty != qErro) {
    return m_next_tok;
  }

  return m_next_tok = next();
}

///============================================================================///

CPP_EXPORT void qlex_t::push_impl(const qlex_tok_t *tok) {
  m_tok_buf.push_front(*tok);
  m_next_tok.ty = qErro;
}

CPP_EXPORT void qlex_t::collect_impl(const qlex_tok_t *tok) {
  switch (tok->ty) {
    case qEofF:
    case qErro:
    case qKeyW:
    case qOper:
    case qPunc:
      break;
    case qName:
    case qIntL:
    case qNumL:
    case qText:
    case qChar:
    case qMacB:
    case qMacr:
    case qNote:
      release_string(tok->v.str_idx);
      break;
  }
}