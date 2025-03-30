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
#include <unordered_set>

using namespace ncc;
using namespace ncc::parse;
using namespace no3::format;

void CambrianFormatter::Visit(FlowPtr<Block> n) {
  PrintLineComments(n);

  bool is_root_block = !m_did_root;
  m_did_root = true;

  switch (n->GetSafety()) {
    case BlockMode::Safe: {
      m_line << "safe ";
      break;
    }

    case BlockMode::Unsafe: {
      m_line << "unsafe ";
      break;
    }

    case BlockMode::Unknown: {
      break;
    }
  }

  static const std::unordered_set<ASTNodeKind> extra_seperation = {
      AST_STRUCT,     AST_ENUM, AST_FUNCTION, AST_SCOPE, AST_EXPORT,  AST_BLOCK,

      AST_INLINE_ASM, AST_IF,   AST_WHILE,    AST_FOR,   AST_FOREACH, AST_SWITCH,
  };

  if (!is_root_block && n->GetStatements().empty()) {
    m_line << "{}";
    return;
  }

  if (!is_root_block) {
    m_line << "{" << '\n';
    m_indent += m_tabSize;
  }

  auto items = n->GetStatements();

  for (auto it = items.begin(); it != items.end(); ++it) {
    auto item = *it;

    m_line << GetIndent();
    item.Accept(*this);
    m_line << '\n';

    bool is_last_item = it == items.end() - 1;

    bool is_next_item_different = (it + 1 != items.end() && (*std::next(it))->GetKind() != item->GetKind());

    bool extra_newline = !is_last_item && (is_next_item_different || extra_seperation.contains(item->GetKind()));

    if (extra_newline) {
      m_line << '\n';
    }
  }

  if (!is_root_block) {
    m_indent -= m_tabSize;
    m_line << GetIndent() << "}";
  }
}
