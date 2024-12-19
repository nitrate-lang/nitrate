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

CPP_EXPORT std::optional<std::pair<uint32_t, uint32_t>> NCCLexer::loc2rowcol(
    uint32_t loc) {
  if (m_off2rc.left.find(loc) == m_off2rc.left.end()) [[unlikely]] {
    return std::nullopt;
  }

  return m_off2rc.left.at(loc);
}

CPP_EXPORT uint32_t NCCLexer::save_loc(uint32_t row, uint32_t col,
                                       uint32_t offset) {
  m_off2rc.insert({offset, {row, col}});
  return offset;
}

CPP_EXPORT uint32_t NCCLexer::cur_loc() {
  return save_loc(m_row, m_col, m_offset);
}

///============================================================================///

class GetCExcept {};

char NCCLexer::getc() {
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

NCCToken NCCLexer::step_buffer() {
  NCCToken tok;

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

CPP_EXPORT NCCToken NCCLexer::next() {
  NCCToken tok;

  if (m_next_tok.has_value()) {
    tok = m_next_tok.value();
    m_next_tok.reset();
  } else {
    /// TODO: Handle no_comments configuration
    bool no_comments = true;

    do {
      m_next_tok.reset();
      tok = step_buffer();

    } while (no_comments && tok.ty == qNote);
  }

  m_current_tok = tok;
  return tok;
}

CPP_EXPORT NCCToken NCCLexer::peek() {
  if (m_next_tok.has_value()) {
    m_current_tok = m_next_tok;
    return m_next_tok.value();
  }

  m_current_tok = (m_next_tok = next());
  return m_current_tok.value();
}

CPP_EXPORT NCCToken NCCLexer::current() {
  return m_current_tok.value_or(NCCToken());
}

///============================================================================///

CPP_EXPORT void NCCLexer::push_impl(const NCCToken *tok) {
  m_tok_buf.push_front(*tok);
  m_next_tok.reset();
}

using namespace ncc::lex;

CPP_EXPORT
std::unique_ptr<ncc::lex::ISourceFile> ncc::lex::SourceFileFromSeekableStream(
    std::istream &stream, std::string_view filename, size_t begin_offset) {
  /// TODO: Implement this function
  qcore_implement();
}

CPP_EXPORT NCCToken Tokenizer::Next() {
  /// TODO: Implement this function
  qcore_implement();
}
