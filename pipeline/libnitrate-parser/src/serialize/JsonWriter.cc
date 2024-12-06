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

#include <nitrate-parser/Writer.hh>

using namespace npar;

void AST_JsonWriter::str_impl(std::string_view str) {
  (void)m_os;

  /// TODO: Implement support for JSON
  qcore_implement();
}

void AST_JsonWriter::uint_impl(uint64_t val) {
  /// TODO: Implement support for JSON
  qcore_implement();
}

void AST_JsonWriter::double_impl(double val) {
  /// TODO: Implement support for JSON
  qcore_implement();
}

void AST_JsonWriter::bool_impl(bool val) {
  /// TODO: Implement support for JSON
  qcore_implement();
}

void AST_JsonWriter::null_impl() {
  /// TODO: Implement support for JSON
  qcore_implement();
}

void AST_JsonWriter::begin_obj_impl() {
  /// TODO: Implement support for JSON
  qcore_implement();
}

void AST_JsonWriter::end_obj_impl() {
  /// TODO: Implement support for JSON
  qcore_implement();
}

void AST_JsonWriter::begin_arr_impl(size_t max_size) {
  /// TODO: Implement support for JSON
  qcore_implement();
}

void AST_JsonWriter::end_arr_impl() {
  /// TODO: Implement support for JSON
  qcore_implement();
}
