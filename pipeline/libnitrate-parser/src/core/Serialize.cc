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
#include <nitrate-parser/ASTWriter.hh>

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

void AST_JsonWriter::begin_obj_impl(size_t) {
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

void AST_JsonWriter::begin_arr_impl(size_t) {
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

///===========================================================================///

void AST_MsgPackWriter::str_impl(std::string_view str) {
  size_t sz = str.size();

  if (sz <= 31) {
    m_os.put(0b10100000 | sz);
  } else if (sz <= UINT8_MAX) {
    m_os.put(0xd9);
    m_os.put(sz);
  } else if (sz <= UINT16_MAX) {
    m_os.put(0xda);
    m_os.put((sz >> 8) & 0xff);
    m_os.put(sz & 0xff);
  } else if (sz <= UINT32_MAX) {
    m_os.put(0xdb);
    m_os.put((sz >> 24) & 0xff);
    m_os.put((sz >> 16) & 0xff);
    m_os.put((sz >> 8) & 0xff);
    m_os.put(sz & 0xff);
  }

  m_os.write(str.data(), sz);
}

void AST_MsgPackWriter::uint_impl(uint64_t x) {
  if (x <= INT8_MAX) {
    m_os.put(x & 0x7f);
  } else if (x <= UINT8_MAX) {
    m_os.put(0xcc);
    m_os.put(x);
  } else if (x <= UINT16_MAX) {
    m_os.put(0xcd);
    m_os.put((x >> 8) & 0xff);
    m_os.put(x & 0xff);
  } else if (x <= UINT32_MAX) {
    m_os.put(0xce);
    m_os.put((x >> 24) & 0xff);
    m_os.put((x >> 16) & 0xff);
    m_os.put((x >> 8) & 0xff);
    m_os.put(x & 0xff);
  } else {
    m_os.put(0xcf);
    m_os.put((x >> 56) & 0xff);
    m_os.put((x >> 48) & 0xff);
    m_os.put((x >> 40) & 0xff);
    m_os.put((x >> 32) & 0xff);
    m_os.put((x >> 24) & 0xff);
    m_os.put((x >> 16) & 0xff);
    m_os.put((x >> 8) & 0xff);
    m_os.put(x & 0xff);
  }
}

void AST_MsgPackWriter::double_impl(double val) {
  uint64_t raw = std::bit_cast<uint64_t>(val);

  m_os.put(0xcb);
  m_os.put((raw >> 56) & 0xff);
  m_os.put((raw >> 48) & 0xff);
  m_os.put((raw >> 40) & 0xff);
  m_os.put((raw >> 32) & 0xff);
  m_os.put((raw >> 24) & 0xff);
  m_os.put((raw >> 16) & 0xff);
  m_os.put((raw >> 8) & 0xff);
  m_os.put(raw & 0xff);
}

void AST_MsgPackWriter::bool_impl(bool val) { m_os.put(val ? 0xc3 : 0xc2); }

void AST_MsgPackWriter::null_impl() { m_os.put(0xc0); }

void AST_MsgPackWriter::begin_obj_impl(size_t pair_count) {
  if (pair_count <= 15) {
    m_os.put(0x80 | pair_count);
  } else if (pair_count <= UINT16_MAX) {
    m_os.put(0xde);
    m_os.put((pair_count >> 8) & 0xff);
    m_os.put(pair_count & 0xff);
  } else if (pair_count <= UINT32_MAX) {
    m_os.put(0xdf);
    m_os.put((pair_count >> 24) & 0xff);
    m_os.put((pair_count >> 16) & 0xff);
    m_os.put((pair_count >> 8) & 0xff);
    m_os.put(pair_count & 0xff);
  }
}

void AST_MsgPackWriter::end_obj_impl() {}

void AST_MsgPackWriter::begin_arr_impl(size_t size) {
  if (size <= 15) {
    m_os.put(0x90 | size);
  } else if (size <= UINT16_MAX) {
    m_os.put(0xdc);
    m_os.put((size >> 8) & 0xff);
    m_os.put(size & 0xff);
  } else if (size <= UINT32_MAX) {
    m_os.put(0xdd);
    m_os.put((size >> 24) & 0xff);
    m_os.put((size >> 16) & 0xff);
    m_os.put((size >> 8) & 0xff);
    m_os.put(size & 0xff);
  }
}

void AST_MsgPackWriter::end_arr_impl() {}
