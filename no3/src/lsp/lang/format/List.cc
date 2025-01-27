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

void CambrianFormatter::Visit(FlowPtr<List> n) {
  PrintMultilineComments(n);

  auto wrap_threshold = 8ULL;

  if (n->GetItems().empty()) {
    m_line << "[]";
    return;
  }

  auto argc = n->GetItems().size();
  bool is_compressing =
      argc >= wrap_threshold &&
      std::all_of(n->GetItems().begin(), n->GetItems().end(),
                  [&](auto x) { return x->IsEq(n->GetItems().front()); });

  if (is_compressing) {
    m_line << "[";
    n->GetItems().front().Accept(*this);
    m_line << "; " << argc << "]";
  } else {
    static const std::unordered_set<npar_ty_t> extra_seperation = {
        QAST_TEREXPR, QAST_CALL, QAST_LIST,
        QAST_ASSOC,   QAST_SEQ,  QAST_TEMPL_CALL,
    };

    bool special_case =
        std::any_of(n->GetItems().begin(), n->GetItems().end(), [&](auto x) {
          return extra_seperation.contains(x->GetKind()) ||
                 x->IsStmtExpr(QAST_FUNCTION);
        });

    size_t break_at{};

    if (special_case) {
      break_at = 1;
    } else {
      break_at = argc <= wrap_threshold
                     ? wrap_threshold
                     : static_cast<size_t>(std::ceil(std::sqrt(argc)));
    }

    if (break_at == 1) {
      m_line << "[";

      m_line << std::endl;

      { /* Write list items */
        size_t the_indent = m_indent + m_tabSize;
        std::swap(m_indent, the_indent);

        for (size_t i = 0; i < n->GetItems().size(); i++) {
          m_line << GetIndent();
          auto item = n->GetItems()[i];
          item.Accept(*this);

          bool is_last = i == n->GetItems().size() - 1;
          if (!is_last) {
            m_line << ",";
          }

          m_line << std::endl;
        }

        std::swap(m_indent, the_indent);
      }

      m_line << GetIndent() << "]";
    } else {
      m_line << "[";

      bool is_assoc_map =
          std::all_of(n->GetItems().begin(), n->GetItems().end(),
                      [](auto x) { return x->Is(QAST_ASSOC); });

      { /* Write list items */
        size_t the_indent =
            is_assoc_map ? m_indent + m_tabSize : m_line.Length();
        std::swap(m_indent, the_indent);

        for (size_t i = 0; i < n->GetItems().size(); i++) {
          auto item = n->GetItems()[i];
          item.Accept(*this);

          bool is_last = i == n->GetItems().size() - 1;
          if (!is_last) {
            m_line << ",";
          }

          bool is_break = !is_last && i != 0 && (i + 1) % break_at == 0;

          if (is_break) {
            m_line << std::endl << GetIndent();
          } else if (!is_last) {
            m_line << " ";
          }
        }

        std::swap(m_indent, the_indent);
      }

      m_line << "]";
    }
  }
}

void CambrianFormatter::Visit(FlowPtr<Assoc> node) {
  PrintMultilineComments(node);

  const std::function<void(FlowPtr<Assoc>, bool)> format =
      [&](const FlowPtr<Assoc>& n, bool use_braces) {
        bool is_value_map = false;
        if (n->GetValue()->Is(QAST_LIST)) {
          auto* list = n->GetValue()->As<List>();
          is_value_map =
              list->GetItems().empty() ||
              std::all_of(list->GetItems().begin(), list->GetItems().end(),
                          [](auto x) { return x->Is(QAST_ASSOC); });
        }

        if (use_braces) {
          m_line << "{" << std::endl;
          m_indent += m_tabSize;
          m_line << GetIndent();
        }

        n->GetKey().Accept(*this);
        m_line << ": ";

        if (is_value_map) {
          auto* list = n->GetValue()->As<List>();

          if (list->GetItems().empty()) {
            m_line << "{}";
          } else {
            m_line << "{" << std::endl;
            m_indent += m_tabSize;

            for (auto it = list->GetItems().begin();
                 it != list->GetItems().end(); ++it) {
              m_line << GetIndent();

              format(it->As<Assoc>(), false);

              if (it != list->GetItems().end() - 1) {
                m_line << ",";
              }

              m_line << std::endl;
            }

            m_indent -= m_tabSize;
            m_line << GetIndent() << "}";
          }
        } else {
          n->GetValue().Accept(*this);
        }

        if (use_braces) {
          m_indent -= m_tabSize;
          m_line << std::endl << GetIndent() << "}";
        }
      };

  format(node, true);
}
