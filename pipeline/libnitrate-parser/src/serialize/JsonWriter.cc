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

#include <nitrate-core/Error.h>
#include <nitrate-core/Macro.h>

#include <nitrate-parser/Writer.hh>

using namespace npar;

static void escape_string(std::ostream &os, const std::string_view &input) {
  os << "\"";

  for (char ch : input) {
    switch (ch) {
      case '"':
        os << "\\\"";
        break;
      case '\\':
        os << "\\\\";
        break;
      case '\b':
        os << "\\b";
        break;
      case '\f':
        os << "\\f";
        break;
      case '\n':
        os << "\\n";
        break;
      case '\r':
        os << "\\r";
        break;
      case '\t':
        os << "\\t";
        break;
      case '\0':
        os << "\\0";
        break;
      default:
        if (ch >= 32 && ch < 127) {
          os << ch;
        } else {
          char hex[5];
          snprintf(hex, sizeof(hex), "\\x%02x", (int)(uint8_t)ch);
          os << hex;
        }
        break;
    }
  }

  os << "\"";
}

void AST_JsonWriter::delim() {
  qcore_assert(!m_count.empty() && !m_comma.empty());

  if (m_count.top()++ > 0) {
    bool use_comma = m_comma.top() == true || (m_count.top() & 1) != 0;

    m_os << (use_comma ? "," : ":");
  }
}

void AST_JsonWriter::str_impl(std::string_view str) {
  delim();

  escape_string(m_os, str);
}

void AST_JsonWriter::uint_impl(uint64_t val) {
  delim();

  m_os << val;
}

void AST_JsonWriter::double_impl(double val) {
  delim();

  m_os << val;
}

void AST_JsonWriter::bool_impl(bool val) {
  delim();

  m_os << (val ? "true" : "false");
}

void AST_JsonWriter::null_impl() {
  delim();

  m_os << "null";
}

void AST_JsonWriter::begin_obj_impl() {
  delim();

  m_comma.push(false);
  m_count.push(0);
  m_os << "{";
}

void AST_JsonWriter::end_obj_impl() {
  qcore_assert(!m_count.empty() && !m_comma.empty());

  m_os << "}";
  m_count.pop();
  m_comma.pop();
}

void AST_JsonWriter::begin_arr_impl(size_t max_size) {
  (void)max_size;
  delim();

  m_comma.push(true);
  m_count.push(0);
  m_os << "[";
}

void AST_JsonWriter::end_arr_impl() {
  qcore_assert(!m_count.empty() && !m_comma.empty());

  m_os << "]";
  m_count.pop();
  m_comma.pop();
}
