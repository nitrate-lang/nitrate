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

#include <lsp/lang/format/Formatter.hh>

using namespace no3::lsp::fmt;
using namespace ncc::parse;

void CambrianFormatter::Visit(FlowPtr<parse::For> n) {
  PrintLineComments(n);

  m_line << "for (";

  if (n->GetInit().has_value()) {
    n->GetInit().value().Accept(*this);
    if (!n->GetInit().value()->IsStmt()) {
      m_line << ";";
    }
  } else {
    m_line << ";";
  }

  if (n->GetCond().has_value()) {
    m_line << " ";
    n->GetCond().value().Accept(*this);
  }
  m_line << ";";

  if (n->GetStep().has_value()) {
    m_line << " ";
    n->GetStep().value().Accept(*this);
  }

  m_line << ") ";
  n->GetBody().Accept(*this);

  m_line << ";";
}

void CambrianFormatter::Visit(FlowPtr<parse::Foreach> n) {
  PrintLineComments(n);

  m_line << "foreach (";
  if (n->GetIndex().empty()) {
    m_line << n->GetValue();
  } else {
    m_line << n->GetIndex() << ", " << n->GetValue();
  }

  m_line << " in ";
  n->GetExpr().Accept(*this);
  m_line << ") ";

  n->GetBody().Accept(*this);

  m_line << ";";
}
