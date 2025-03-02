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

void CambrianFormatter::Visit(FlowPtr<parse::Scope> n) {
  PrintLineComments(n);

  m_line << "scope ";

  if (n->GetName()) {
    m_line << n->GetName();
  }

  if (!n->GetDeps().empty()) {
    m_line << ": [";
    IterateExceptLast(
        n->GetDeps().begin(), n->GetDeps().end(), [&](auto dep, size_t) { m_line << dep; },
        [&](let) { m_line << ", "; });
    m_line << "]";
  }

  m_line << " ";
  n->GetBody().Accept(*this);
}

void CambrianFormatter::Visit(FlowPtr<Export> n) {
  PrintLineComments(n);

  m_line << n->GetVis();

  if (n->GetAbiName()) {
    m_line << " ";
    EscapeStringLiteral(n->GetAbiName());
  }

  if (!n->GetAttributes().empty()) {
    m_line << " [";
    IterateExceptLast(
        n->GetAttributes().begin(), n->GetAttributes().end(), [&](auto attr, size_t) { attr.Accept(*this); },
        [&](let) { m_line << ", "; });
    m_line << "]";
  }

  m_line << " ";

  n->GetBody()->Accept(*this);
}
