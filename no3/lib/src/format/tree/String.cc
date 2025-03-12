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

auto CambrianFormatter::EscapeCharLiteral(char ch) const -> std::string {
  if ((std::isspace(ch) == 0) && (std::isprint(ch) == 0)) {
    const char* tab = "0123456789abcdef";
    uint8_t uch = ch;
    std::array<char, 6> enc = {'\'', '\\', 'x', 0, 0, '\''};
    enc[3] = tab[uch >> 4];
    enc[4] = tab[uch & 0xF];
    return {enc.data(), enc.size()};
  }

  switch (ch) {
    case '\n':
      return "'\\n'";
    case '\t':
      return "'\\t'";
    case '\r':
      return "'\\r'";
    case '\v':
      return "'\\v'";
    case '\f':
      return "'\\f'";
    case '\b':
      return "'\\b'";
    case '\a':
      return "'\\a'";
    case '\\':
      return "'\\\\'";
    case '\'':
      return "'\\''";
    default:
      return "'" + std::string(1, ch) + "'";
  }
}

auto CambrianFormatter::EscapeStringLiteralChunk(std::string_view str) const -> std::string {
  std::stringstream ss;

  for (char ch : str) {
    switch (ch) {
      case '\n':
        ss << "\\n";
        break;
      case '\t':
        ss << "\\t";
        break;
      case '\r':
        ss << "\\r";
        break;
      case '\v':
        ss << "\\v";
        break;
      case '\f':
        ss << "\\f";
        break;
      case '\b':
        ss << "\\b";
        break;
      case '\a':
        ss << "\\a";
        break;
      case '\\':
        ss << "\\\\";
        break;
      case '"':
        ss << "\\\"";
        break;
      default:
        ss << ch;
        break;
    }
  }

  return ss.str();
}

void CambrianFormatter::EscapeStringLiteral(std::string_view str, bool put_quotes) {
  constexpr size_t kMaxChunkSize = 60;

  if (str.empty()) {
    if (put_quotes) {
      m_line << "\"\"";
    }
    return;
  }

  auto chunks_n = str.size() / kMaxChunkSize;
  auto rem = str.size() % kMaxChunkSize;
  auto m_line_size = m_line.Length();

  if (chunks_n != 0U) {
    std::vector<std::string> chunks(chunks_n);

    for (size_t i = 0; i < chunks_n; i++) {
      chunks[i] = "\"" + EscapeStringLiteralChunk(str.substr(i * kMaxChunkSize, kMaxChunkSize)) + "\"";
    }

    auto max_segment_size =
        std::max_element(chunks.begin(), chunks.end(), [](auto a, auto b) { return a.size() < b.size(); })->size();

    for (size_t i = 0; i < chunks.size(); ++i) {
      if (i != 0 && (m_line_size != 0U)) {
        m_line << std::string(m_line_size, ' ');
      }

      m_line << chunks[i];

      auto rpad = (max_segment_size - chunks[i].size());
      if (rpad != 0U) {
        m_line << std::string(rpad, ' ');
      }

      if (rem > 0 || i < chunks_n - 1) {
        m_line << " \\" << std::endl;
      }
    }
  }

  if (rem > 0) {
    if ((m_line_size != 0U) && chunks_n > 0) {
      m_line << std::string(m_line_size, ' ');
    }

    if (chunks_n > 0 || put_quotes) {
      m_line << "\"";
    }

    m_line << EscapeStringLiteralChunk(str.substr(chunks_n * kMaxChunkSize, rem));

    if (chunks_n > 0 || put_quotes) {
      m_line << "\"";
    }
  }
}

void CambrianFormatter::Visit(FlowPtr<parse::String> n) {
  PrintMultilineComments(n);

  EscapeStringLiteral(n->GetValue());
}

void CambrianFormatter::Visit(FlowPtr<Character> n) {
  PrintMultilineComments(n);

  m_line << EscapeCharLiteral(n->GetValue());
}

void CambrianFormatter::Visit(FlowPtr<FString> n) {
  PrintMultilineComments(n);

  m_line << "f\"";
  for (auto part : n->GetItems()) {
    if (std::holds_alternative<ncc::string>(part)) {
      EscapeStringLiteral(*std::get<ncc::string>(part), false);
    } else {
      m_line << "{";
      std::get<FlowPtr<Expr>>(part).Accept(*this);
      m_line << "}";
    }
  }
  m_line << "\"";
}
