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

#ifndef __NITRATE_PARSER_CORE_HASH_H__
#define __NITRATE_PARSER_CORE_HASH_H__

#include <cassert>
#include <cstdint>
#include <nitrate-parser/ASTWriter.hh>

namespace npar {
  class CPP_EXPORT AST_Hash64 : public AST_Writer {
    std::stack<std::pair<bool, size_t>> m_state;
    uint64_t m_sum;

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
    AST_Hash64()
        : AST_Writer(
              std::bind(&AST_Hash64::str_impl, this, std::placeholders::_1),
              std::bind(&AST_Hash64::uint_impl, this, std::placeholders::_1),
              std::bind(&AST_Hash64::double_impl, this, std::placeholders::_1),
              std::bind(&AST_Hash64::bool_impl, this, std::placeholders::_1),
              std::bind(&AST_Hash64::null_impl, this),
              std::bind(&AST_Hash64::begin_obj_impl, this,
                        std::placeholders::_1),
              std::bind(&AST_Hash64::end_obj_impl, this),
              std::bind(&AST_Hash64::begin_arr_impl, this,
                        std::placeholders::_1),
              std::bind(&AST_Hash64::end_arr_impl, this)),
          m_sum(0) {
      m_state.push({false, 0});
    }
    virtual ~AST_Hash64() = default;

    uint64_t get() const { return m_sum; }
  };
}  // namespace npar

#endif
