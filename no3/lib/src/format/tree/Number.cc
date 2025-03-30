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

void CambrianFormatter::WriteFloatLiteralChunk(std::string_view float_str) {
  constexpr size_t kInsertSepEvery = 10;

  bool already_write_type_suffix = false;

  for (size_t i = 0; i < float_str.size(); i++) {
    bool underscore = false;

    if (!already_write_type_suffix && i != 0 && (i % (kInsertSepEvery)) == 0) {
      underscore = true;
    } else if (!already_write_type_suffix && (std::isdigit(float_str[i]) == 0) && float_str[i] != '.') {
      already_write_type_suffix = true;
      underscore = true;
    }

    if (underscore) {
      m_line << "_";
    }

    m_line << float_str[i];
  }
}

void CambrianFormatter::WriteFloatLiteral(std::string_view float_str) {
  constexpr size_t kMaxChunkSize = 50;

  if (float_str.empty()) {
    m_line << "";
  }

  size_t chunks_n = float_str.size() / kMaxChunkSize;
  size_t rem = float_str.size() % kMaxChunkSize;

  size_t m_line_size = m_line.Length();

  for (size_t i = 0; i < chunks_n; i++) {
    WriteFloatLiteralChunk(float_str.substr(i * kMaxChunkSize, kMaxChunkSize));

    if (rem > 0 || i < chunks_n - 1) {
      m_line << "_ \\" << '\n';
      if (m_line_size != 0U) {
        m_line << std::string(m_line_size, ' ');
      }
    }
  }

  if (rem > 0) {
    WriteFloatLiteralChunk(float_str.substr(chunks_n * kMaxChunkSize, rem));
  }
}

void CambrianFormatter::Visit(FlowPtr<Float> n) {
  PrintMultilineComments(n);

  WriteFloatLiteral(n->GetValue());
}

void CambrianFormatter::Visit(FlowPtr<Integer> n) {
  PrintMultilineComments(n);

  WriteFloatLiteral(n->GetValue());
}

void CambrianFormatter::Visit(FlowPtr<Boolean> n) {
  PrintMultilineComments(n);

  m_line << (n->GetValue() ? "true" : "false");
}

void CambrianFormatter::Visit(FlowPtr<parse::Null> n) {
  PrintMultilineComments(n);

  m_line << "null";
}
