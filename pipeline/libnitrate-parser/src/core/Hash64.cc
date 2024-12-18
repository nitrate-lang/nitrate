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

#include <core/Hash.hh>
#include <nitrate-core/Logger.hh>
#include <nitrate-core/Macro.hh>

using namespace npar;

void AST_Hash64::str_impl(std::string_view str) {
  qcore_assert(!m_state.empty());
  m_state.top().second++;

  bool hash_name =
      m_state.top().first == true || (m_state.top().second & 1) != 0;

  if (hash_name) {
    m_sum += std::hash<std::string_view>{}(str);
  }
}

void AST_Hash64::uint_impl(uint64_t val) {
  qcore_assert(!m_state.empty());
  m_state.top().second++;

  m_sum += val;
}

void AST_Hash64::double_impl(double val) {
  qcore_assert(!m_state.empty());
  m_state.top().second++;

  m_sum += std::hash<double>{}(val);
}

void AST_Hash64::bool_impl(bool val) {
  qcore_assert(!m_state.empty());
  m_state.top().second++;

  m_sum += std::hash<bool>{}(val);
}

void AST_Hash64::null_impl() {
  qcore_assert(!m_state.empty());
  m_state.top().second++;

  m_sum += std::hash<std::nullptr_t>{}(nullptr);
}

void AST_Hash64::begin_obj_impl(size_t) {
  qcore_assert(!m_state.empty());
  m_state.top().second++;

  m_sum++;

  m_state.push({false, 0});
}

void AST_Hash64::end_obj_impl() {
  qcore_assert(!m_state.empty());
  m_state.pop();

  m_sum++;
}

void AST_Hash64::begin_arr_impl(size_t size) {
  qcore_assert(!m_state.empty());
  m_state.top().second++;

  m_sum += size;

  m_state.push({true, 0});
}

void AST_Hash64::end_arr_impl() {
  qcore_assert(!m_state.empty());
  m_state.pop();

  m_sum++;
}
