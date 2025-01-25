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

#include <nitrate-core/Logger.hh>
#include <nitrate-core/Macro.hh>
#include <nitrate-ir/ToJson.hh>

using namespace ncc::ir;

static void EscapeString(std::ostream &os, const std::string_view &input) {
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

void IRJsonWriter::Delim() {
  if (!m_count.empty() && !m_comma.empty()) {
    if (m_count.top()++ > 0) {
      bool use_comma = m_comma.top() == true || (m_count.top() & 1) != 0;

      m_os << (use_comma ? "," : ":");
    }
  }
}

void IRJsonWriter::StrImpl(std::string_view str) {
  Delim();

  EscapeString(m_os, str);
}

void IRJsonWriter::UintImpl(uint64_t val) {
  Delim();

  m_os << val;
}

void IRJsonWriter::DoubleImpl(double val) {
  Delim();

  m_os << val;
}

void IRJsonWriter::BoolImpl(bool val) {
  Delim();

  m_os << (val ? "true" : "false");
}

void IRJsonWriter::NullImpl() {
  Delim();

  m_os << "null";
}

void IRJsonWriter::BeginObjImpl(size_t) {
  Delim();

  m_comma.push(false);
  m_count.push(0);
  m_os << "{";
}

void IRJsonWriter::EndObjImpl() {
  if (!m_count.empty() && !m_comma.empty()) {
    m_os << "}";
    m_count.pop();
    m_comma.pop();
  }
}

void IRJsonWriter::BeginArrImpl(size_t) {
  Delim();

  m_comma.push(true);
  m_count.push(0);
  m_os << "[";
}

void IRJsonWriter::EndArrImpl() {
  if (!m_count.empty() && !m_comma.empty()) {
    m_os << "]";
    m_count.pop();
    m_comma.pop();
  }
}
