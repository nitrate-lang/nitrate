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
#include <nlohmann/json.hpp>
#include <ostream>
#include <string_view>

using namespace ncc::parse;

void AstJsonWriter::Delim() {
  if (!m_count.empty() && !m_comma.empty()) {
    if (m_count.top()++ > 0) {
      bool use_comma = m_comma.top() || (m_count.top() & 1) != 0;

      m_os << (use_comma ? "," : ":");
    }
  }
}

void AstJsonWriter::StrImpl(std::string_view str) {
  Delim();

  m_os << nlohmann::json(str);
}

void AstJsonWriter::UintImpl(uint64_t val) {
  Delim();

  m_os << val;
}

void AstJsonWriter::DoubleImpl(double val) {
  Delim();

  m_os << val;
}

void AstJsonWriter::BoolImpl(bool val) {
  Delim();

  m_os << (val ? "true" : "false");
}

void AstJsonWriter::NullImpl() {
  Delim();

  m_os << "null";
}

void AstJsonWriter::BeginObjImpl(size_t) {
  Delim();

  m_comma.push(false);
  m_count.push(0);
  m_os << "{";
}

void AstJsonWriter::EndObjImpl() {
  if (!m_count.empty() && !m_comma.empty()) {
    m_os << "}";
    m_count.pop();
    m_comma.pop();
  }
}

void AstJsonWriter::BeginArrImpl(size_t) {
  Delim();

  m_comma.push(true);
  m_count.push(0);
  m_os << "[";
}

void AstJsonWriter::EndArrImpl() {
  if (!m_count.empty() && !m_comma.empty()) {
    m_os << "]";
    m_count.pop();
    m_comma.pop();
  }
}

///===========================================================================///

void AstMsgPackWriter::StrImpl(std::string_view str) {
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

void AstMsgPackWriter::UintImpl(uint64_t x) {
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

void AstMsgPackWriter::DoubleImpl(double x) {
  auto raw = std::bit_cast<uint64_t>(x);

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

void AstMsgPackWriter::BoolImpl(bool x) { m_os.put(x ? 0xc3 : 0xc2); }

void AstMsgPackWriter::NullImpl() { m_os.put(0xc0); }

void AstMsgPackWriter::BeginObjImpl(size_t pair_count) {
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

void AstMsgPackWriter::EndObjImpl() {}

void AstMsgPackWriter::BeginArrImpl(size_t size) {
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

void AstMsgPackWriter::EndArrImpl() {}
