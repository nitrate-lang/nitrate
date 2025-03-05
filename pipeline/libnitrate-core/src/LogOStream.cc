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

#include <nitrate-core/LogOStream.hh>
#include <nitrate-core/Logger.hh>
#include <nitrate-core/Macro.hh>
#include <streambuf>

using namespace ncc;

static LogOStream LogStreamBuffer;
NCC_EXPORT thread_local std::ostream ncc::clog(&LogStreamBuffer);

std::streambuf::int_type LogOStream::overflow(int_type c) {
  if (c == '\n') {
    Log << Raw << m_buffer;
    m_buffer.clear();
  } else {
    m_buffer.push_back(c);
  }

  return c;
}

auto LogOStream::xsputn(const char *s, std::streamsize count) -> std::streamsize {
  m_buffer.append(s, count);

  for (size_t pos = m_buffer.find('\n'); pos != std::string::npos; pos = m_buffer.find('\n', pos)) {
    Log << Raw << m_buffer.substr(0, pos);
    m_buffer.erase(0, pos + 1);
  }

  return count;
}

int LogOStream::sync() {
  if (!m_buffer.empty()) {
    Log << Raw << m_buffer;
    m_buffer.clear();
  }

  return 0;
}
