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

#ifndef __NITRATE_IR_ENCODE_TOMSGPACK_H__
#define __NITRATE_IR_ENCODE_TOMSGPACK_H__

#include <nitrate-ir/encode/Serialize.hh>
#include <ostream>

namespace ncc::ir::encode {
  class NCC_EXPORT IR_MsgPackWriter : public IR_Writer {
    std::ostream& m_os;

    void str_impl(std::string_view str);
    void uint_impl(uint64_t val);
    void double_impl(double val);
    void bool_impl(bool val);
    void null_impl();
    void begin_obj_impl(size_t pair_count);
    void end_obj_impl();
    void begin_arr_impl(size_t size);
    void end_arr_impl();

  public:
    IR_MsgPackWriter(std::ostream& os, WriterSourceProvider rd = std::nullopt)
        : IR_Writer(std::bind(&IR_MsgPackWriter::str_impl, this,
                              std::placeholders::_1),
                    std::bind(&IR_MsgPackWriter::uint_impl, this,
                              std::placeholders::_1),
                    std::bind(&IR_MsgPackWriter::double_impl, this,
                              std::placeholders::_1),
                    std::bind(&IR_MsgPackWriter::bool_impl, this,
                              std::placeholders::_1),
                    std::bind(&IR_MsgPackWriter::null_impl, this),
                    std::bind(&IR_MsgPackWriter::begin_obj_impl, this,
                              std::placeholders::_1),
                    std::bind(&IR_MsgPackWriter::end_obj_impl, this),
                    std::bind(&IR_MsgPackWriter::begin_arr_impl, this,
                              std::placeholders::_1),
                    std::bind(&IR_MsgPackWriter::end_arr_impl, this), rd),
          m_os(os) {}
    virtual ~IR_MsgPackWriter() = default;
  };
}  // namespace ncc::ir::encode

#endif
