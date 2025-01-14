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

#include <nitrate-lexer/Lexer.hh>
#include <nitrate-parser/EC.hh>

using namespace ncc;
using namespace ncc::lex;

static thread_local IScanner *g_current_scanner = nullptr;

void Parser_SetCurrentScanner(IScanner *scanner) {
  g_current_scanner = scanner;
}

static constexpr std::optional<std::string> unescape_string_slice(
    std::string_view &buf) {
  if (!buf.starts_with("\"")) [[unlikely]] {
    return std::nullopt;
  }

  buf.remove_prefix(1);

  std::string unescaped;

  while (!buf.empty()) {
    char c = buf[0];
    buf.remove_prefix(1);

    if (c == '\\') {
      if (buf.empty()) [[unlikely]] {
        return std::nullopt;
      }

      c = buf[0];
      buf.remove_prefix(1);

      switch (c) {
        case 'n':
          unescaped += '\n';
          break;
        case 't':
          unescaped += '\t';
          break;
        case 'r':
          unescaped += '\r';
          break;
        case '0':
          unescaped += '\0';
          break;
        case '\\':
          unescaped += '\\';
          break;
        case '\'':
          unescaped += '\'';
          break;
        case '\"':
          unescaped += '\"';
          break;
        case 'x': {
          constexpr std::array<uint8_t, 256> hextable = [] {
            std::array<uint8_t, 256> table = {};
            for (uint8_t i = 0; i < 10; i++) {
              table['0' + i] = i;
            }
            for (uint8_t i = 0; i < 6; i++) {
              table['a' + i] = 10 + i;
              table['A' + i] = 10 + i;
            }
            return table;
          }();

          if (buf.size() < 2) [[unlikely]] {
            return std::nullopt;
          }

          char hex[2] = {buf[0], buf[1]};
          if (!std::isxdigit(hex[0]) || !std::isxdigit(hex[1])) [[unlikely]] {
            return std::nullopt;
          }

          unescaped +=
              (hextable[(uint8_t)hex[0]] << 4) | hextable[(uint8_t)hex[1]];
          buf.remove_prefix(2);
          break;
        }
        default: {
          return std::nullopt;
        }
      }
    } else if (c == '\"') {
      break;
    } else {
      unescaped += c;
    }
  }

  return unescaped;
}

static std::optional<std::pair<Token, std::string>> find_and_decode_token(
    std::string_view buf) {
  std::string_view orig_buf = buf;

  auto pos = buf.find("${T:{\"type\":");
  if (pos == std::string_view::npos) [[unlikely]] {
    return std::nullopt;
  }
  const auto start_offset = pos;
  buf.remove_prefix(pos + 12);
  if (buf.empty()) [[unlikely]] {
    return std::nullopt;
  }

  int type = 0;
  while (std::isdigit(buf[0])) {
    type = type * 10 + (buf[0] - '0');
    buf.remove_prefix(1);

    if (buf.empty()) [[unlikely]] {
      return std::nullopt;
    }
  }

  if (!buf.starts_with(",\"posid\":")) [[unlikely]] {
    return std::nullopt;
  }

  buf.remove_prefix(9);
  if (buf.empty()) [[unlikely]] {
    return std::nullopt;
  }

  int posid = 0;
  while (std::isdigit(buf[0])) {
    posid = posid * 10 + (buf[0] - '0');
    buf.remove_prefix(1);

    if (buf.empty()) [[unlikely]] {
      return std::nullopt;
    }
  }

  if (!buf.starts_with(",\"value\":")) [[unlikely]] {
    return std::nullopt;
  }

  buf.remove_prefix(9);
  auto unescaped = unescape_string_slice(buf);

  if (!unescaped.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  if (!buf.starts_with("}}")) [[unlikely]] {
    return std::nullopt;
  }

  buf.remove_prefix(2);

  auto slice = std::string(orig_buf.substr(0, start_offset)) + std::string(buf);

  switch (type) {
    case EofF: {
      return {{Token::EndOfFile(), slice}};
    }

    case KeyW: {
      return {{Token(KeyW, LexicalKeywords.left.at(unescaped.value()),
                     LocationID(posid)),
               slice}};
    }

    case Oper: {
      return {{Token(Oper, LexicalOperators.left.at(unescaped.value()),
                     LocationID(posid)),
               slice}};
    }

    case Punc: {
      return {{Token(Punc, LexicalPunctors.left.at(unescaped.value()),
                     LocationID(posid)),
               slice}};
    }

    case Name:
    case IntL:
    case NumL:
    case Text:
    case Char:
    case MacB:
    case Macr:
    case Note: {
      return {{Token(static_cast<TokenType>(type), unescaped.value(),
                     LocationID(posid)),
               slice}};
    }

    default: {
      return std::nullopt;
    }
  }
}

NCC_EXPORT std::string ncc::parse::ec::Formatter(std::string_view message_raw,
                                                 Sev) {
  IScanner *rd = g_current_scanner;

  if (rd) {
    auto result_opt = find_and_decode_token(message_raw);
    Token tok;
    std::string message;
    if (result_opt.has_value()) {
      tok = result_opt.value().first;
      message = result_opt.value().second;
    } else {
      message = message_raw;
    }

    auto token_start = rd->Start(tok), token_end = rd->End(tok);
    auto start_filename = token_start.GetFilename();
    auto start_line = token_start.GetRow(), start_col = token_start.GetCol();
    auto end_line = token_end.GetRow();

    std::stringstream ss;
    ss << "[\x1b[0m\x1b[31;1mSyntax\x1b[0m\x1b[37;1m]: ";
    ss << (start_filename->empty() ? "?" : start_filename) << ":";
    ss << (start_line == QLEX_EOFF ? "?" : std::to_string(start_line + 1))
       << ":";
    ss << (start_col == QLEX_EOFF ? "?" : std::to_string(start_col + 1))
       << ":\x1b[0m ";
    ss << "\x1b[37;1m" << message << "\x1b[0m";

    if (start_line != QLEX_EOFF) {
      IScanner::Point start_pos(start_line == 0 ? 0 : start_line - 1, 0),
          end_pos;

      if (end_line != QLEX_EOFF) {
        end_pos = IScanner::Point(end_line + 1, -1);
      } else {
        end_pos = IScanner::Point(start_line + 1, -1);
      }

      if (auto window = rd->GetSourceWindow(start_pos, end_pos, ' ')) {
        ss << "\n";

        for (auto &line : window.value()) {
          ss << line << "\n";
        }

        for (uint32_t i = 0; i < start_col; i++) {
          ss << " ";
        }
        ss << "\x1b[32;1m^\x1b[0m";
      }
    }

    return ss.str();
  } else {
    return std::string(message_raw);
  }
}
