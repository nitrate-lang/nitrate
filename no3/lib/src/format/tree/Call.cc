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

void CambrianFormatter::Visit(FlowPtr<Call> n) {
  PrintMultilineComments(n);

  auto wrap_threshold = 8ULL;

  n->GetFunc().Accept(*this);

  size_t argc = n->GetArgs().size();

  bool any_named = std::any_of(n->GetArgs().begin(), n->GetArgs().end(), [](const CallArg& arg) {
    auto name = arg.first;
    return std::isdigit(name->at(0)) == 0;
  });

  bool any_lambdas = std::any_of(n->GetArgs().begin(), n->GetArgs().end(),
                                 [](auto arg) { return std::get<1>(arg)->Is(QAST_FUNCTION); });

  bool is_wrapping = argc >= wrap_threshold || any_named || any_lambdas;

  if (is_wrapping) {
    m_line << "(";
    size_t m_line_size = m_line.Length();
    std::swap(m_indent, m_line_size);

    for (auto it = n->GetArgs().begin(); it != n->GetArgs().end(); ++it) {
      auto arg = *it;
      auto name = std::get<0>(arg);
      auto value = std::get<1>(arg);

      if (std::isdigit(name->at(0)) == 0) {
        m_line << name << ": ";
      }

      value.Accept(*this);

      if (it != n->GetArgs().end() - 1) {
        m_line << ", ";
      }

      if (it != n->GetArgs().end() - 1) {
        m_line << std::endl << GetIndent();
      }
    }

    std::swap(m_indent, m_line_size);
    m_line << ")";
  } else {
    m_line << "(";
    IterateExceptLast(
        n->GetArgs().begin(), n->GetArgs().end(),
        [&](auto arg, size_t) {
          auto name = std::get<0>(arg);
          auto value = std::get<1>(arg);

          if (!std::isdigit(name->at(0))) {
            m_line << name << ": ";
          }

          value.Accept(*this);
        },
        [&](let) { m_line << ", "; });
    m_line << ")";
  }
}

void CambrianFormatter::Visit(FlowPtr<TemplateCall> n) {
  PrintMultilineComments(n);

  n->GetFunc().Accept(*this);

  m_line << "{";
  IterateExceptLast(
      n->GetTemplateArgs().begin(), n->GetTemplateArgs().end(),
      [&](auto arg, size_t) {
        auto name = std::get<0>(arg);
        auto value = std::get<1>(arg);
        bool should_print_name = !std::isdigit(name->at(0));

        if (should_print_name) {
          m_line << name << ": ";
        }

        value.Accept(*this);
      },
      [&](let) { m_line << ", "; });
  m_line << "}";

  m_line << "(";
  IterateExceptLast(
      n->GetArgs().begin(), n->GetArgs().end(),
      [&](auto arg, size_t) {
        auto name = std::get<0>(arg);
        auto value = std::get<1>(arg);

        if (!std::isdigit(name->at(0))) {
          m_line << name << ": ";
        }

        value.Accept(*this);
      },
      [&](let) { m_line << ", "; });
  m_line << ")";
}
