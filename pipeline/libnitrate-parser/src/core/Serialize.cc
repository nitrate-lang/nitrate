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
