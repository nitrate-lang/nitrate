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

#include <bit>
#include <nitrate-core/Logger.hh>
#include <nitrate-ir/ToMsgPack.hh>

using namespace ncc::ir::encode;

void IR_MsgPackWriter::str_impl(std::string_view str) {
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

void IR_MsgPackWriter::uint_impl(uint64_t x) {
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

void IR_MsgPackWriter::double_impl(double val) {
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

void IR_MsgPackWriter::bool_impl(bool val) { m_os.put(val ? 0xc3 : 0xc2); }

void IR_MsgPackWriter::null_impl() { m_os.put(0xc0); }

void IR_MsgPackWriter::begin_obj_impl(size_t pair_count) {
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

void IR_MsgPackWriter::end_obj_impl() {}

void IR_MsgPackWriter::begin_arr_impl(size_t size) {
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

void IR_MsgPackWriter::end_arr_impl() {}
