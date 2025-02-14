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
#include <unordered_set>

using namespace no3::lsp::fmt;
using namespace ncc::parse;

void CambrianFormatter::Visit(FlowPtr<Block> n) {
  PrintLineComments(n);

  bool is_root_block = !m_did_root;
  m_did_root = true;

  switch (n->GetSafety()) {
    case SafetyMode::Safe: {
      m_line << "safe ";
      break;
    }

    case SafetyMode::Unsafe: {
      m_line << "unsafe ";
      break;
    }

    case SafetyMode::Unknown: {
      break;
    }
  }

  static const std::unordered_set<npar_ty_t> extra_seperation = {
      QAST_STRUCT,     QAST_ENUM, QAST_FUNCTION, QAST_SCOPE, QAST_EXPORT,  QAST_BLOCK,

      QAST_INLINE_ASM, QAST_IF,   QAST_WHILE,    QAST_FOR,   QAST_FOREACH, QAST_SWITCH,
  };

  if (!is_root_block && n->GetStatements().empty()) {
    m_line << "{}";
    return;
  }

  if (!is_root_block) {
    m_line << "{" << std::endl;
    m_indent += m_tabSize;
  }

  auto items = n->GetStatements();

  for (auto it = items.begin(); it != items.end(); ++it) {
    auto item = *it;

    m_line << GetIndent();
    item.Accept(*this);
    m_line << std::endl;

    bool is_last_item = it == items.end() - 1;

    bool is_next_item_different = (it + 1 != items.end() && (*std::next(it))->GetKind() != item->GetKind());

    bool extra_newline = !is_last_item && (is_next_item_different || extra_seperation.contains(item->GetKind()));

    if (extra_newline) {
      m_line << std::endl;
    }
  }

  if (!is_root_block) {
    m_indent -= m_tabSize;
    m_line << GetIndent() << "}";
  }
}
