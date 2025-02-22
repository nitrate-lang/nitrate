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

#include <core/SyntaxDiagnostics.hh>
#include <nitrate-lexer/Scanner.hh>
#include <nlohmann/json.hpp>
#include <sstream>

using namespace ncc;
using namespace ncc::lex;

static thread_local IScanner *GCurrentScanner = nullptr;

void ParserSetCurrentScanner(IScanner *scanner) { GCurrentScanner = scanner; }

static std::optional<Token> ParseJsonToken(const nlohmann::json &j) {
  if (!j.is_object()) {
    return std::nullopt;
  }

  if (!j.contains("type") || !j.contains("pos")) {
    return std::nullopt;
  }

  const auto type = static_cast<TokenType>(j["type"].get<int>());

  LocationID start = j["pos"].is_null() ? LocationID() : LocationID(j["pos"].get<uint32_t>());

  return Token(type, TokenData::GetDefault(type), start);
}

static auto FindAndDecodeToken(std::string_view buf) -> std::optional<std::pair<Token, std::string>> {
  const auto pos = buf.find("$TOKEN{");
  if (pos == std::string_view::npos) {
    return std::nullopt;
  }

  std::string_view orig_buf = buf;

  buf.remove_prefix(pos + 7);
  if (buf.empty()) {
    return std::nullopt;
  }

  size_t payload_length = 0;
  while (!buf.empty() && std::isdigit(buf.front()) != 0) {
    payload_length = payload_length * 10 + (buf.front() - '0');
    buf.remove_prefix(1);
  }

  if (buf.size() < payload_length + 1) {
    return std::nullopt;
  }

  if (buf[payload_length] != '}') {
    return std::nullopt;
  }

  std::string_view json_payload = buf.substr(0, payload_length);

  nlohmann::json j = nlohmann::json::parse(json_payload, nullptr, false);
  if (j.is_discarded()) {
    return std::nullopt;
  }

  auto token_opt = ParseJsonToken(j);

  if (!token_opt.has_value()) {
    return std::nullopt;
  }

  return std::make_pair(token_opt.value(), std::string(orig_buf.substr(0, pos)) +
                                               std::string(orig_buf.substr(pos + 7 + (payload_length + 1) + 2)));
}

NCC_EXPORT auto ncc::parse::ec::Formatter(std::string_view msg, Sev sev) -> std::string {
  IScanner *rd = GCurrentScanner;

  if (rd != nullptr) {
    auto result_opt = FindAndDecodeToken(msg);
    Token tok;
    std::string message;
    if (result_opt.has_value()) {
      tok = result_opt.value().first;
      message = result_opt.value().second;
    } else {
      message = msg;
    }

    auto token_start = rd->Start(tok);
    auto token_end = rd->End(tok);
    auto start_filename = token_start.GetFilename();
    auto start_line = token_start.GetRow();
    auto start_col = token_start.GetCol();
    auto end_line = token_end.GetRow();

    std::stringstream ss;
    ss << "\x1b[0m\x1b[37;1m[\x1b[0m\x1b[31;1mParse\x1b[0m\x1b[37;1m]: " << sev << ":\x1b[0m ";
    bool any_source_location = !start_filename->empty() || start_line != kLexEof || start_col != kLexEof;

    if (any_source_location) {
      ss << (start_filename->empty() ? "?" : start_filename) << ":";
      ss << (start_line == kLexEof ? "?" : std::to_string(start_line + 1)) << ":";
      ss << (start_col == kLexEof ? "?" : std::to_string(start_col + 1)) << ":\x1b[0m ";
    }

    ss << "\x1b[37;1m" << message << "\x1b[0m";

    if (start_line != kLexEof) {
      IScanner::Point start_pos(start_line == 0 ? 0 : start_line - 1, 0);
      IScanner::Point end_pos;

      if (end_line != kLexEof) {
        end_pos = IScanner::Point(end_line + 1, -1);
      } else {
        end_pos = IScanner::Point(start_line + 1, -1);
      }

      if (auto window = rd->GetSourceWindow(start_pos, end_pos, ' ')) {
        ss << "\n";

        for (const auto &line : window.value()) {
          ss << line << "\n";
        }

        for (uint32_t i = 0; i < start_col; i++) {
          ss << " ";
        }
        ss << "\x1b[32;1m^\x1b[0m";
      }
    }

    return ss.str();
  }

  std::stringstream ss;
  ss << "\x1b[0m\x1b[37;1m[\x1b[0m\x1b[31;1mParse\x1b[0m\x1b[37;1m]: " << sev << ":\x1b[0m ";

  if (auto result = FindAndDecodeToken(msg)) {
    ss << result.value().second;
  } else {
    ss << msg;
  }

  return ss.str();
}
