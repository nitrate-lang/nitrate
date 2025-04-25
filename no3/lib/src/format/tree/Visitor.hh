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

#pragma once

#include <memory>
#include <nitrate-parser/ASTVisitor.hh>

namespace no3::format {

  struct FormatterConfig {
    enum class CommentStyle : uint8_t {
      Multiline,
      Pythonic,
      CStyle,
      SwiggleArrow,
    };

    size_t m_indent_size = 2;
    size_t m_max_line_length = 128;
    size_t m_max_line_length_expr = 128;
    size_t m_max_line_length_stmt = 128;
    size_t m_max_line_length_type = 128;
    size_t m_max_line_length_comment = 128;
    CommentStyle m_comment_style = CommentStyle::CStyle;
    bool m_remove_multiline_comments = false;
    bool m_remove_single_line_comments = false;
    bool m_remove_doc_comments = false;
    bool m_remove_doc_comments_multiline = false;
    bool m_remove_doc_comments_singleline = false;

    static auto CanonicalSettings() -> FormatterConfig { return {}; }
  };

  class CanonicalFormatterFactory {
  public:
    static auto Create(std::ostream& out, bool& has_errors,
                       const FormatterConfig& config = FormatterConfig::CanonicalSettings())
        -> std::unique_ptr<ncc::parse::ASTVisitor>;
  };
}  // namespace no3::format
