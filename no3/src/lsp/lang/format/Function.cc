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

void CambrianFormatter::Visit(FlowPtr<Function> n) {
  PrintLineComments(n);

  m_line << "fn";

  if (!n->GetAttributes().empty()) {
    m_line << " [";
    IterateExceptLast(
        n->GetAttributes().begin(), n->GetAttributes().end(), [&](auto attr, size_t) { attr.Accept(*this); },
        [&](let) { m_line << ", "; });
    m_line << "]";
  }

  switch (n->GetPurity()) {
    case Purity::Impure: {
      break;
    }

    case Purity::Impure_TSafe: {
      m_line << " tsafe";
      break;
    }

    case Purity::Pure: {
      m_line << " pure";
      break;
    }

    case Purity::Quasi: {
      m_line << " quasi";
      break;
    }

    case Purity::Retro: {
      m_line << " retro";
      break;
    }
  }

  if (!n->GetCaptures().empty()) {
    m_line << " [";
    IterateExceptLast(
        n->GetCaptures().begin(), n->GetCaptures().end(),
        [&](auto cap, size_t) {
          if (cap.second) {
            m_line << "&";
          }
          m_line << cap.first;
        },
        [&](let) { m_line << ", "; });
    m_line << "]";
  }

  m_line << " " << n->GetName();

  if (n->GetTemplateParams()) {
    m_line << "<";
    IterateExceptLast(
        n->GetTemplateParams().value().begin(), n->GetTemplateParams().value().end(),
        [&](auto param, size_t) {
          m_line << std::get<0>(param);

          if (let type = std::get<1>(param)) {
            if (type->GetKind() != QAST_INFER) {
              m_line << ": ";
              type->Accept(*this);
            }
          }

          if (auto val = std::get<2>(param)) {
            m_line << " = ";
            val.value().Accept(*this);
          }
        },
        [&](let) { m_line << ", "; });
    m_line << ">";
  }

  m_line << "(";
  IterateExceptLast(
      n->GetParams().begin(), n->GetParams().end(),
      [&](auto param, size_t) {
        m_line << std::get<0>(param);

        if (let type = std::get<1>(param)) {
          if (type->GetKind() != QAST_INFER) {
            m_line << ": ";
            type->Accept(*this);
          }
        }

        if (auto def = std::get<2>(param)) {
          m_line << " = ";
          def.value().Accept(*this);
        }
      },
      [&](let) { m_line << ", "; });

  if (n->IsVariadic()) {
    if (!n->GetParams().empty()) {
      m_line << ", ";
    }
    m_line << "...";
  }
  m_line << ")";

  { /* Return type */
    auto return_type = n->GetReturn();

    if (!return_type->Is(QAST_INFER)) {
      m_line << ": ";
      return_type.Accept(*this);
    }
  }

  if (n->IsDeclaration()) {
    m_line << ";";
  } else {
    m_line << " ";
    WrapStmtBody(n->GetBody().value(), 10, true);
  }
}

void CambrianFormatter::Visit(FlowPtr<FuncTy> n) {
  PrintMultilineComments(n);

  m_line << "fn";

  if (!n->GetAttributes().empty()) {
    m_line << "[";
    IterateExceptLast(
        n->GetAttributes().begin(), n->GetAttributes().end(), [&](auto attr, size_t) { attr.Accept(*this); },
        [&](let) { m_line << ", "; });
    m_line << "] ";
  }

  switch (n->GetPurity()) {
    case Purity::Impure: {
      break;
    }

    case Purity::Impure_TSafe: {
      m_line << " tsafe";
      break;
    }

    case Purity::Pure: {
      m_line << " pure";
      break;
    }

    case Purity::Quasi: {
      m_line << " quasi";
      break;
    }

    case Purity::Retro: {
      m_line << " retro";
      break;
    }
  }

  m_line << "(";
  IterateExceptLast(
      n->GetParams().begin(), n->GetParams().end(),
      [&](auto param, size_t) {
        m_line << std::get<0>(param);

        if (auto type = std::get<1>(param); type->GetKind() != QAST_INFER) {
          m_line << ": ";
          type.Accept(*this);
        }

        if (auto def = std::get<2>(param)) {
          m_line << " = ";
          def.value().Accept(*this);
        }
      },
      [&](let) { m_line << ", "; });
  if (n->IsVariadic()) {
    if (!n->GetParams().empty()) {
      m_line << ", ";
    }
    m_line << "...";
  }
  m_line << ")";

  m_line << ": ";
  n->GetReturn().Accept(*this);
}
