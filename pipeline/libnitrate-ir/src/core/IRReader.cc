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
#include <nitrate-ir/IRReader.hh>

using namespace ncc::ir;

void IRReader::Str(std::string_view str) {
  /// TODO: Implement generic deserializer
  qcore_implement();
}

void IRReader::Uint(uint64_t val) {
  /// TODO: Implement generic deserializer
  qcore_implement();
}

void IRReader::Dbl(double val) {
  /// TODO: Implement generic deserializer
  qcore_implement();
}

void IRReader::Boolean(bool val) {
  /// TODO: Implement generic deserializer
  qcore_implement();
}

void IRReader::Null() {
  /// TODO: Implement generic deserializer
  qcore_implement();
}

void IRReader::BeginObj() {
  /// TODO: Implement generic deserializer
  qcore_implement();
}

void IRReader::EndObj() {
  /// TODO: Implement generic deserializer
  qcore_implement();
}

void IRReader::BeginArr(size_t max_size) {
  /// TODO: Implement generic deserializer
  qcore_implement();
}

void IRReader::EndArr() {
  /// TODO: Implement generic deserializer
  qcore_implement();
}
