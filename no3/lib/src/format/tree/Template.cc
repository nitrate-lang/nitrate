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
using namespace ncc::lex;

void QuasiCanonicalFormatter::Visit(FlowPtr<TemplateType> n) {
  PrintMultilineComments(n);

  bool is_optional =
      n->GetTemplate()->GetKind() == AST_tNAMED && n->GetTemplate()->As<NamedTy>()->GetName() == "__builtin_result";

  bool is_vector =
      n->GetTemplate()->GetKind() == AST_tNAMED && n->GetTemplate()->As<NamedTy>()->GetName() == "__builtin_vec";

  bool is_map =
      n->GetTemplate()->GetKind() == AST_tNAMED && n->GetTemplate()->As<NamedTy>()->GetName() == "__builtin_umap";

  bool is_set =
      n->GetTemplate()->GetKind() == AST_tNAMED && n->GetTemplate()->As<NamedTy>()->GetName() == "__builtin_uset";

  bool is_comptime = n->GetTemplate()->GetKind() == AST_tNAMED &&
                     n->GetTemplate()->As<NamedTy>()->GetName() == "__builtin_meta" && n->GetArgs().size() == 1 &&
                     n->GetArgs().front().second->Is(AST_eUNARY) &&
                     n->GetArgs().front().second.template As<Unary>()->GetOp() == OpComptime;

  size_t argc = n->GetArgs().size();
  if (is_optional && argc == 1) {
    n->GetArgs().front().second->Accept(*this);
    m_line << "?";
  } else if (is_vector && argc == 1) {
    m_line << "[";
    n->GetArgs().front().second->Accept(*this);
    m_line << "]";
  } else if (is_map && argc == 2) {
    m_line << "[";
    n->GetArgs().front().second->Accept(*this);
    m_line << "->";
    n->GetArgs().back().second->Accept(*this);
    m_line << "]";
  } else if (is_set && argc == 1) {
    m_line << "{";
    n->GetArgs().front().second->Accept(*this);
    m_line << "}";
  } else if (is_comptime) {
    m_line << "comptime(";
    n->GetArgs().front().second.template As<Unary>()->GetRHS().Accept(*this);
    m_line << ")";
  } else {
    n->GetTemplate().Accept(*this);

    m_line << "<";
    IterateExceptLast(
        n->GetArgs().begin(), n->GetArgs().end(),
        [&](auto arg, size_t) {
          if (!std::isdigit(arg.first->at(0))) {
            m_line << arg.first << ": ";
          }

          arg.second->Accept(*this);
        },
        [&](let) { m_line << ", "; });
    m_line << ">";
  }

  FormatTypeMetadata(n);
}
