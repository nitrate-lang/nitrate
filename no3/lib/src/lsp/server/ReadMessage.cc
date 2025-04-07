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

#include <charconv>
#include <lsp/protocol/Notification.hh>
#include <lsp/protocol/Request.hh>
#include <lsp/server/Server.hh>
#include <nitrate-core/Assert.hh>
#include <nitrate-core/Logger.hh>

using namespace ncc;
using namespace no3::lsp::core;
using namespace no3::lsp::message;

struct HttpMessage {
  std::unordered_map<std::string, std::string> m_headers;
  std::string m_content;
};

static std::string HttpHeaderStripWhitespace(std::string value) {
  auto start = value.find_first_not_of(" \t");
  if (start != std::string::npos) {
    value.erase(0, start);
  }

  auto end = value.find_last_not_of(" \t");
  if (end != std::string::npos) {
    value.erase(end + 1);
  }

  return value;
}

static auto ParseHttpHeader(std::istream& in,
                            bool& end_of_headers) -> std::optional<std::pair<std::string, std::string>> {
  end_of_headers = false;

  std::string line;
  std::getline(in, line);
  if (!in) [[unlikely]] {
    Log << "ParseHttpHeader(): Failed to read line";
    return std::nullopt;
  }

  if (line.ends_with("\r")) {
    line = line.substr(0, line.size() - 1);
  }

  if (line.empty()) [[unlikely]] {
    Log << Trace << "ParseHttpHeader(): End of headers";
    end_of_headers = true;
    return std::nullopt;
  }

  auto pos = line.find(':');
  if (pos == std::string::npos) [[unlikely]] {
    Log << "ParseHttpHeader(): Invalid header format";
    return std::nullopt;
  }

  auto key = HttpHeaderStripWhitespace(line.substr(0, pos));
  auto val = HttpHeaderStripWhitespace(line.substr(pos + 1));

  Log << Trace << "ParseHttpHeader(): Result: (\"" << key << "\", \"" << val << "\")";

  return std::make_pair(key, val);
}

static auto ParseHttpMessage(std::istream& in) -> std::optional<HttpMessage> {
  std::unordered_map<std::string, std::string> headers;

  while (true) {
    bool end_of_headers = false;
    auto header = ParseHttpHeader(in, end_of_headers);
    if (end_of_headers) {
      break;
    }

    if (!header.has_value()) [[unlikely]] {
      Log << "ParseHttpMessage(): Failed to parse HTTP header";
      return std::nullopt;
    }

    headers[header->first] = header->second;
  }

  if (headers.empty()) [[unlikely]] {
    Log << "ParseHttpMessage(): No headers found";
    return std::nullopt;
  }

  if (!headers.contains("Content-Length")) [[unlikely]] {
    Log << "ParseHttpMessage(): Missing 'Content-Length' header";
    return std::nullopt;
  }

  if (!headers.contains("Content-Type")) [[unlikely]] {
    constexpr auto kDefaultContentType = "application/vscode-jsonrpc; charset=utf-8";
    headers["Content-Type"] = kDefaultContentType;
  }

  const auto& content_length = headers.at("Content-Length");
  std::streamsize content_length_value = 0;

  if (std::from_chars(content_length.data(), content_length.data() + content_length.size(), content_length_value).ec !=
      std::errc()) {
    Log << "ParseHttpMessage(): Invalid 'Content-Length' header value: \"" << content_length << "\"";
    return std::nullopt;
  }

  Log << Trace << "ParseHttpMessage(): Content-Length: " << content_length_value;

  std::string content;
  content.resize(content_length_value);
  if (!in.read(content.data(), content_length_value)) [[unlikely]] {
    Log << "ParseHttpMessage(): Failed to read content";
    return std::nullopt;
  }

  if (in.gcount() != content_length_value) [[unlikely]] {
    Log << "ParseHttpMessage(): Read content size mismatch";
    return std::nullopt;
  }

  Log << Trace << "ParseHttpMessage(): Content: " << content;

  return HttpMessage{std::move(headers), std::move(content)};
}

static auto QuickJsonRPCMessageCheck(const nlohmann::json& json_rpc) -> bool {
  if (!json_rpc.contains("jsonrpc")) [[unlikely]] {
    Log << "QuickJsonRPCMessageCheck(): Missing 'jsonrpc' field";
    return false;
  }

  if (!json_rpc["jsonrpc"].is_string()) [[unlikely]] {
    Log << "QuickJsonRPCMessageCheck(): 'jsonrpc' field is not a string";
    return false;
  }

  if (json_rpc["jsonrpc"] != "2.0") [[unlikely]] {
    Log << "QuickJsonRPCMessageCheck(): 'jsonrpc' field is not '2.0'";
    return false;
  }

  if (!json_rpc.contains("method")) [[unlikely]] {
    Log << "QuickJsonRPCMessageCheck(): Missing 'method' field";
    return false;
  }

  if (!json_rpc["method"].is_string()) [[unlikely]] {
    Log << "QuickJsonRPCMessageCheck(): 'method' field is not a string";
    return false;
  }

  if (json_rpc.contains("id")) {
    if (!json_rpc["id"].is_string() && !json_rpc["id"].is_number_integer()) [[unlikely]] {
      Log << "QuickJsonRPCMessageCheck(): 'id' field is not a string or integer";
      return false;
    }
  }

  return true;
}

static auto ConvertRPCMessageToLSPMessage(nlohmann::json json_rpc) -> std::unique_ptr<Message> {
  auto method = json_rpc["method"].get<std::string>();
  auto params = json_rpc.contains("params") ? std::move(json_rpc["params"]) : nlohmann::json{};

  if (bool is_notification = !json_rpc.contains("id")) {
    return std::make_unique<NotifyMessage>(std::move(method), std::move(params));
  }

  auto id = std::move(json_rpc["id"]);
  if (id.is_number_integer()) {
    return std::make_unique<RequestMessage>(std::move(method), id.get<int64_t>(), std::move(params));
  }

  if (id.is_string()) {
    return std::make_unique<RequestMessage>(std::move(method), id.get<std::string>(), std::move(params));
  }

  qcore_panic("unreachable");
}

auto Server::ReadRequest(std::istream& in, std::mutex& in_lock) -> std::optional<std::unique_ptr<Message>> {
  std::lock_guard lock(in_lock);
  if (in.eof()) [[unlikely]] {
    Log << "ReadRequest(): EOF reached";
    return std::nullopt;
  }

  if (!in) [[unlikely]] {
    Log << "ReadRequest(): Bad stream";
    return std::nullopt;
  }

  auto http_message = ParseHttpMessage(in);
  if (!http_message.has_value()) [[unlikely]] {
    Log << "ReadRequest(): Failed to parse HTTP message";
    return std::nullopt;
  }

  const auto& message = http_message.value();

  auto json_rpc = nlohmann::json::parse(message.m_content, nullptr, false);
  if (json_rpc.is_discarded()) [[unlikely]] {
    Log << "ReadRequest(): Failed to parse JSON-RPC message";
    return std::nullopt;
  }

  if (!QuickJsonRPCMessageCheck(json_rpc)) [[unlikely]] {
    Log << "ReadRequest(): Invalid LSP JSON-RPC message";
    return std::nullopt;
  }

  return ConvertRPCMessageToLSPMessage(std::move(json_rpc));
}
