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

#include <format/tree/Visitor.hh>

using namespace ncc;
using namespace ncc::parse;
using namespace no3::format;

void QuasiCanonicalFormatter::Visit(FlowPtr<parse::While> n) {
  PrintLineComments(n);

  m_line << "while ";
  n->GetCond().Accept(*this);
  m_line << " ";
  n->GetBody().Accept(*this);

  m_line << ";";
}

void QuasiCanonicalFormatter::Visit(FlowPtr<parse::Return> n) {
  PrintLineComments(n);

  if (n->GetValue().has_value()) {
    m_line << "ret ";
    n->GetValue().value().Accept(*this);
    m_line << ";";
  } else {
    m_line << "ret;";
  }
}

void QuasiCanonicalFormatter::Visit(FlowPtr<parse::If> n) {
  PrintLineComments(n);

  m_line << "if ";
  n->GetCond().Accept(*this);
  m_line << " ";
  n->GetThen().Accept(*this);

  if (n->GetElse()) {
    m_line << " else ";
    n->GetElse().value().Accept(*this);
  }
}

void QuasiCanonicalFormatter::Visit(FlowPtr<parse::Break> n) {
  PrintLineComments(n);

  m_line << "break;";
}

void QuasiCanonicalFormatter::Visit(FlowPtr<parse::Continue> n) {
  PrintLineComments(n);

  m_line << "continue;";
}
