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

#ifndef __NITRATE_IR_ENCODE_TOJSON_H__
#define __NITRATE_IR_ENCODE_TOJSON_H__

#include <nitrate-ir/IRWriter.hh>
#include <ostream>
#include <stack>

namespace ncc::ir {
  class NCC_EXPORT IrJsonWriter : public IrWriter {
    std::ostream& m_os;
    std::stack<bool> m_comma;
    std::stack<size_t> m_count;

    void Delim();

    void StrImpl(std::string_view str);
    void UintImpl(uint64_t val);
    void DoubleImpl(double val);
    void BoolImpl(bool val);
    void NullImpl();
    void BeginObjImpl(size_t pair_count);
    void EndObjImpl();
    void BeginArrImpl(size_t size);
    void EndArrImpl();

  public:
    IrJsonWriter(std::ostream& os, WriterSourceProvider rd = std::nullopt)
        : IrWriter(
              std::bind(&IrJsonWriter::str_impl, this, std::placeholders::_1),
              std::bind(&IrJsonWriter::uint_impl, this, std::placeholders::_1),
              std::bind(&IrJsonWriter::double_impl, this,
                        std::placeholders::_1),
              std::bind(&IrJsonWriter::bool_impl, this, std::placeholders::_1),
              std::bind(&IrJsonWriter::null_impl, this),
              std::bind(&IrJsonWriter::begin_obj_impl, this,
                        std::placeholders::_1),
              std::bind(&IrJsonWriter::end_obj_impl, this),
              std::bind(&IrJsonWriter::begin_arr_impl, this,
                        std::placeholders::_1),
              std::bind(&IrJsonWriter::end_arr_impl, this), rd),
          m_os(os) {
      m_comma.push(false);
      m_count.push(0);
    }
    virtual ~IrJsonWriter() = default;
  };
}  // namespace ncc::ir

#endif
