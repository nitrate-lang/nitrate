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

#define __QUIX_LEXER_IMPL__

#include <quix-core/Error.h>
#include <quix-lexer/Lexer.h>
#include <string.h>

#include <boost/bimap.hpp>
#include <boost/unordered_map.hpp>
#include <cctype>
#include <cmath>
#include <csetjmp>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <quix-lexer/Base.hh>
#include <utility>

#include "LibMacro.h"
#include "quix-core/Env.h"
#include "quix-lexer/Token.h"

///============================================================================///

LIB_EXPORT qlex_t *qlex_direct(const char *src, size_t len, const char *filename, qcore_env_t env) {
  try {
    if (!filename) {
      filename = "<unknown>";
    }

    FILE *file = fmemopen((void *)src, len, "r");
    if (!file) {
      return nullptr;
    }

    qlex_t *obj = qlex_new(file, filename, env);
    if (!obj) {
      fclose(file);
      return nullptr;
    }

    obj->m_is_owned = true;

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

LIB_EXPORT qlex_size qlex_line(qlex_t *obj, qlex_loc_t loc) {
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

LIB_EXPORT qlex_size qlex_col(qlex_t *obj, qlex_loc_t loc) {
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

LIB_EXPORT qlex_loc_t qlex_offset(qlex_t *obj, qlex_loc_t base, qlex_size offset) {
  try {
    long curpos;
    std::optional<qlex_size> seek_base_pos;
    uint8_t *buf = nullptr;
    size_t bufsz;
    qlex_loc_t res{.tag = 0};

    if (!(seek_base_pos = obj->loc2offset(base))) {
      return res;
    }

    if ((curpos = ftell(obj->m_file)) == -1) {
      return res;
    }

    if (fseek(obj->m_file, *seek_base_pos + offset, SEEK_SET) != 0) {
      return res;
    }

    bufsz = offset;

    if ((buf = (uint8_t *)malloc(bufsz + 1)) == nullptr) {
      if (fseek(obj->m_file, curpos, SEEK_SET) != 0) {
        qcore_panic("qlex_offset: failed to restore file position");
      }
      return res;
    }

    if (fread(buf, 1, bufsz, obj->m_file) != bufsz) {
      free(buf);
      if (fseek(obj->m_file, curpos, SEEK_SET) != 0) {
        qcore_panic("qlex_offset: failed to restore file position");
      }
      return res;
    }

    buf[bufsz] = '\0';
    if (fseek(obj->m_file, curpos, SEEK_SET) != 0) {
      qcore_panic("qlex_offset: failed to restore file position");
    }

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

    for (size_t i = 0; i < bufsz; i++) {
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

LIB_EXPORT qlex_size qlex_span(qlex_t *obj, qlex_loc_t start, qlex_loc_t end) {
  try {
    std::optional<qlex_size> begoff, endoff;

    if (!(begoff = obj->loc2offset(start))) {
      return UINT32_MAX;
    }

    if (!(endoff = obj->loc2offset(end))) {
      return UINT32_MAX;
    }

    if (*endoff < *begoff) {
      return 0;
    }

    return *endoff - *begoff;
  } catch (...) {
    qcore_panic("qlex_span: failed to calculate span");
  }
}

LIB_EXPORT qlex_size qlex_spanx(qlex_t *obj, qlex_loc_t start, qlex_loc_t end,
                                void (*callback)(const char *, qlex_size, uintptr_t),
                                uintptr_t userdata) {
  try {
    std::optional<qlex_size> begoff, endoff;

    if (!(begoff = obj->loc2offset(start))) {
      return UINT32_MAX;
    }

    if (!(endoff = obj->loc2offset(end))) {
      return UINT32_MAX;
    }

    if (*endoff < *begoff) {
      return UINT32_MAX;
    }

    qlex_size span = *endoff - *begoff;

    long curpos;
    uint8_t *buf = nullptr;
    size_t bufsz;

    if ((curpos = ftell(obj->m_file)) == -1) {
      return UINT32_MAX;
    }

    if (fseek(obj->m_file, *begoff, SEEK_SET) != 0) {
      return UINT32_MAX;
    }

    bufsz = span;

    if ((buf = (uint8_t *)malloc(bufsz + 1)) == nullptr) {
      if (fseek(obj->m_file, curpos, SEEK_SET) != 0) {
        qcore_panic("qlex_spanx: failed to restore file position");
      }
      return UINT32_MAX;
    }

    if (fread(buf, 1, bufsz, obj->m_file) != bufsz) {
      free(buf);
      if (fseek(obj->m_file, curpos, SEEK_SET) != 0) {
        qcore_panic("qlex_spanx: failed to restore file position");
      }
      return UINT32_MAX;
    }

    buf[bufsz] = '\0';
    if (fseek(obj->m_file, curpos, SEEK_SET) != 0) {
      qcore_panic("qlex_spanx: failed to restore file position");
    }

    callback((const char *)buf, bufsz, userdata);

    free(buf);
    return span;
  } catch (...) {
    qcore_panic("qlex_spanx: failed to calculate span");
  }
}

LIB_EXPORT void qlex_rect(qlex_t *obj, qlex_size x_0, qlex_size y_0, qlex_size x_1, qlex_size y_1,
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

    // Fill background with spaces
    memset(out, fill, buf_size);
    out[buf_size] = '\0';

    // qlex_size base_off = 0;

    for (size_t i = 0; i < height; i++) {
      qlex_size start_off = 0, end_off = 10;

      /// TODO: Calculate the start and end offsets

      qlex_loc_t start = obj->save_loc(y_0 + i, x_0, start_off);
      qlex_loc_t end = obj->save_loc(y_0 + i, x_1, end_off);

      qlex_spanx(
          obj, start, end,
          [](const char *str, qlex_size len, uintptr_t ptr) { memcpy((void *)ptr, str, len); },
          (uintptr_t)(out + i * width));
    }

  } catch (...) {
  }
}

LIB_EXPORT char *qlex_snippet(qlex_t *obj, qlex_tok_t tok, qlex_size *offset) {
  try {
#define SNIPPET_SIZE 100

    qlex_size tok_beg_offset;
    char snippet_buf[SNIPPET_SIZE];
    size_t curpos, seek_base_pos, read;

    { /* Convert the location to an offset into the source */
      auto src_offset_opt = obj->loc2offset(tok.start);
      if (!src_offset_opt) {
        return nullptr; /* Return early if translation failed */
      }

      tok_beg_offset = *src_offset_opt - 1;
    }

    { /* Calculate offsets and seek to the correct position */
      curpos = ftell(obj->m_file);
      if ((long)curpos == -1) {
        return nullptr;
      }
      seek_base_pos = tok_beg_offset < SNIPPET_SIZE / 2 ? 0 : tok_beg_offset - SNIPPET_SIZE / 2;

      if (fseek(obj->m_file, seek_base_pos, SEEK_SET) != 0) {
        if (fseek(obj->m_file, curpos, SEEK_SET) != 0) {
          qcore_panic("qlex_snippet: failed to restore file position");
        }
        return nullptr;
      }
    }

    { /* Read the snippet and calculate token offset */
      read = fread(snippet_buf, 1, SNIPPET_SIZE, obj->m_file);
      memset(snippet_buf + read, 0, SNIPPET_SIZE - read);

      if (tok_beg_offset < SNIPPET_SIZE / 2) {
        *offset = tok_beg_offset;
      } else {
        *offset = SNIPPET_SIZE / 2;
      }
    }

    // Extract the line that contains the token
    qlex_size slice_start = 0;

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

        qlex_size slice_size = slice_end - slice_start;
        char *output = (char *)malloc(slice_size + 1);
        if (!output) {
          qcore_panic("qlex_snippet: failed to allocate memory");
        }
        memcpy(output, snippet_buf + slice_start, slice_size);
        output[slice_size] = '\0';
        *offset -= slice_start;
        if (fseek(obj->m_file, curpos, SEEK_SET) != 0) {
          qcore_panic("qlex_snippet: failed to restore file position");
        }
        return output;
      }
    }

    if (fseek(obj->m_file, curpos, SEEK_SET) != 0) {
      qcore_panic("qlex_snippet: failed to restore file position");
    }
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

CPP_EXPORT std::optional<qlex_size> qlex_t::loc2offset(qlex_loc_t loc) {
  if (m_tag_to_off.find(loc.tag) == m_tag_to_off.end()) [[unlikely]] {
    return std::nullopt;
  }

  return m_tag_to_off[loc.tag];
}

CPP_EXPORT std::optional<std::pair<qlex_size, qlex_size>> qlex_t::loc2rowcol(qlex_loc_t loc) {
  if (m_tag_to_loc.left.find(loc.tag) == m_tag_to_loc.left.end()) [[unlikely]] {
    return std::nullopt;
  }

  clever_me_t it = m_tag_to_loc.left.at(loc.tag);

  if (!it.rc_fmt) [[unlikely]] {
    return std::nullopt;
  }

  qlex_size row = it.row;
  qlex_size col = it.col;

  return std::make_pair(row, col);
}

CPP_EXPORT qlex_loc_t qlex_t::save_loc(qlex_size row, qlex_size col, qlex_size offset) {
  if (row <= 2097152 || col <= 1024) [[likely]] {
    clever_me_t bits;
    static_assert(sizeof(bits) == sizeof(qlex_size));

    bits.rc_fmt = 1;
    bits.col = col;
    bits.row = row;

    if (m_tag_to_loc.right.find(bits) != m_tag_to_loc.right.end()) {
      return {m_tag_to_loc.right.at(bits)};
    }

    qlex_size tag = m_locctr++;
    m_tag_to_loc.insert({tag, bits});
    m_tag_to_off[tag] = offset;

    return {tag};
  } else {
    return {0};
  }
}

CPP_EXPORT qlex_loc_t qlex_t::cur_loc() { return save_loc(m_row, m_col, m_offset); }

///============================================================================///

CPP_EXPORT std::string_view qlex_t::get_string(qlex_size idx) {
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

CPP_EXPORT qlex_size qlex_t::put_string(std::string_view str) {
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

CPP_EXPORT void qlex_t::release_string(qlex_size idx) {
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
    size_t read = fread(m_getc_buf.data(), 1, GETC_BUFFER_SIZE, m_file);

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

  if (!m_tok_buf.empty()) {
    tok = m_tok_buf.back();
    m_tok_buf.pop_back();
  } else {
    try {
      m_tok_buf.push_back(next_impl());
      tok = m_tok_buf.back();
      m_tok_buf.pop_back();
    } catch (GetCExcept &) {
      tok.ty = qEofF;
    }
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
  m_tok_buf.insert(m_tok_buf.begin(), *tok);
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