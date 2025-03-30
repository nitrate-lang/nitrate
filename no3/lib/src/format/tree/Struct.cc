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

void CambrianFormatter::Visit(FlowPtr<parse::Struct> n) {
  PrintLineComments(n);

  switch (n->GetCompositeType()) {
    case CompositeType::Region: {
      m_line << "region ";
      break;
    }

    case CompositeType::Struct: {
      m_line << "struct ";
      break;
    }

    case CompositeType::Group: {
      m_line << "group ";
      break;
    }

    case CompositeType::Class: {
      m_line << "class ";
      break;
    }

    case CompositeType::Union: {
      m_line << "union ";
      break;
    }
  }

  if (!n->GetAttributes().empty()) {
    const auto wrap_threshold = 2ULL;

    const auto& attrs = n->GetAttributes();
    const auto break_at = attrs.size() > wrap_threshold ? wrap_threshold : attrs.size();

    m_line << "[";
    auto m_line_size = m_line.Length();
    for (size_t i = 0; i < attrs.size(); i++) {
      auto attr = attrs[i];
      attr.Accept(*this);

      if (i != attrs.size() - 1) {
        m_line << ", ";
      }

      if (i != attrs.size() - 1 && (i + 1) % break_at == 0) {
        m_line << '\n' << std::string(m_line_size, ' ');
      }
    }

    m_line << "]" << '\n' << std::string(m_line_size, ' ');
  }

  m_line << n->GetName();
  if (n->GetTemplateParams().has_value()) {
    m_line << "<";
    IterateExceptLast(
        n->GetTemplateParams().value().begin(), n->GetTemplateParams().value().end(),
        [&](auto param, size_t) {
          m_line << std::get<0>(param);
          if (auto type = std::get<1>(param); type->GetKind() != AST_tINFER) {
            m_line << ": ";
            type.Accept(*this);
          }
          if (auto val = std::get<2>(param)) {
            m_line << " = ";
            val.value().Accept(*this);
          }
        },
        [&](let) { m_line << ", "; });
    m_line << ">";
  }

  if (!n->GetNames().empty()) {
    m_line << ": ";

    const auto wrap_threshold = 2ULL;

    const auto& names = n->GetNames();
    const auto break_at = names.size() > wrap_threshold ? wrap_threshold : names.size();

    m_line << "[";
    auto m_line_size = m_line.Length();
    for (size_t i = 0; i < names.size(); i++) {
      m_line << names[i];

      if (i != names.size() - 1) {
        m_line << ", ";
      }

      if (i != names.size() - 1 && (i + 1) % break_at == 0) {
        m_line << '\n' << std::string(m_line_size, ' ');
      }
    }

    m_line << "]";
  }

  bool is_empty = n->GetFields().empty() && n->GetMethods().empty();

  if (is_empty) {
    m_line << ";";
    return;
  }

  m_line << " {" << '\n';
  m_indent += m_tabSize;

  auto fields_count = n->GetFields();
  auto methods_count = n->GetMethods();

  std::for_each(n->GetFields().begin(), n->GetFields().end(), [&](auto field) {
    m_line << GetIndent() << field.GetVis() << " ";

    m_line << field.GetName() << ": ";
    field.GetType().Accept(*this);

    if (field.GetValue().has_value()) {
      m_line << " = ";
      field.GetValue().value().Accept(*this);
    }

    m_line << "," << '\n';
  });

  if (!fields_count.empty() && !methods_count.empty()) {
    m_line << '\n';
  }

  std::for_each(n->GetMethods().begin(), n->GetMethods().end(), [&](auto method) {
    m_line << GetIndent() << method.m_vis << " ";
    method.m_func.Accept(*this);
    m_line << '\n';
  });

  m_indent -= m_tabSize;
  m_line << GetIndent() << "}";
}

void CambrianFormatter::Visit(FlowPtr<TupleTy> n) {
  /* If the number of fields exceeds the threshold, arange fields into a
   * matrix of row size ceil(sqrt(n)). */
  PrintMultilineComments(n);

  auto wrap_threshold = 8ULL;

  m_line << "(";

  auto items = n->GetItems();
  auto m_line_size = m_line.Length();
  auto break_at =
      items.size() <= wrap_threshold ? wrap_threshold : static_cast<size_t>(std::ceil(std::sqrt(items.size())));

  for (size_t i = 0; i < items.size(); i++) {
    if (i != 0 && i % break_at == 0) {
      m_line << '\n' << std::string(m_line_size, ' ');
    }

    auto item = items[i];
    item.Accept(*this);

    if (i != items.size() - 1) {
      m_line << ", ";
    }
  }
  m_line << ")";

  FormatTypeMetadata(n);
}
