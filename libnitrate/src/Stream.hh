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

#pragma once

#include <queue>
#include <streambuf>
#include <vector>

struct quix_stream_t : public std::streambuf {
private:
  std::queue<FILE*> m_files;
  char ch;
  bool m_close_me;

public:
  quix_stream_t(FILE* f, bool auto_close) {
    m_files.push(f);
    m_close_me = auto_close;
  }
  quix_stream_t(std::vector<FILE*> stream_join) {
    for (auto f : stream_join) {
      m_files.push(f);
    }
    m_close_me = false;
  }
  virtual ~quix_stream_t() {
    if (m_close_me) {
      while (!m_files.empty()) {
        auto f = m_files.front();
        m_files.pop();
        fclose(f);
      }
    }
  }

  virtual int_type underflow() override {
    if (m_files.empty()) {
      return traits_type::eof();
    }

    int c;
    ch = c = fgetc(m_files.front());

    if (feof(m_files.front())) {
      m_files.pop();
      return underflow();
    } else if (ferror(m_files.front())) {
      return traits_type::eof();
    }

    setg(&ch, &ch, &ch + 1);

    return traits_type::to_int_type(ch);
  }

  virtual std::streamsize xsgetn(char* s, std::streamsize count) override {
    if (m_files.empty()) {
      return 0;
    }

    std::streamsize bytes_read = 0;

    while (bytes_read < count) {
      size_t n = fread(s + bytes_read, 1, count - bytes_read, m_files.front());
      bytes_read += n;

      if (feof(m_files.front())) {
        m_files.pop();
        return bytes_read + xsgetn(s + bytes_read, count - bytes_read);
      } else if (ferror(m_files.front())) {
        return 0;
      }
    }

    return bytes_read;
  }
};
