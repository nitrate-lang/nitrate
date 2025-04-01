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
#include <ios>
#include <lsp/core/Server.hh>
#include <lsp/core/ThreadPool.hh>
#include <lsp/core/protocol/LSP.hh>
#include <lsp/route/RoutesList.hh>
#include <nitrate-core/Assert.hh>
#include <nitrate-core/Logger.hh>

using namespace ncc;
using namespace no3::lsp::core;
using namespace no3::lsp::message;

class LSPServer::PImpl {
  class RequestScheduler {
    std::optional<ThreadPool> m_thread_pool;
    std::iostream& m_io;
    std::mutex& m_io_lock;
    std::atomic<bool> m_exit_requested = false;

  public:
    RequestScheduler(std::iostream& io, std::mutex& io_lock) : m_io(io), m_io_lock(io_lock) {}
    ~RequestScheduler() = default;

    [[nodiscard]] bool IsExitRequested() const { return m_exit_requested; }
    void Schedule(RequestMessage request);
  };

public:
  State m_state = State::Suspended;
  std::mutex m_state_mutex;
  std::iostream& m_io;
  std::mutex m_io_mutex;
  RequestScheduler m_request_scheduler;

  [[nodiscard]] auto ReadRequest() -> std::optional<RequestMessage>;

  PImpl(std::iostream& io) : m_io(io), m_request_scheduler(io, m_io_mutex) {}
};

LSPServer::LSPServer(std::iostream& io) : m_pimpl(std::make_unique<PImpl>(io)) {}

LSPServer::~LSPServer() = default;

auto LSPServer::Start() -> bool {
  qcore_assert(m_pimpl != nullptr);

  Log << Trace << "LSPServer: Start() called";

  {
    std::lock_guard lock(m_pimpl->m_state_mutex);
    Log << Trace << "LSPServer: Start(): State::Suspended -> State::Running";
    m_pimpl->m_state = State::Running;
  }

  constexpr size_t kMaxFailedRequestCount = 10;
  size_t sucessive_failed_request_count = 0;

  while (true) {
    std::lock_guard lock(m_pimpl->m_state_mutex);

    switch (auto current_state = m_pimpl->m_state) {
      case State::Suspended: {
        // Minimize CPU usage while waiting for the server to be resumed
        std::this_thread::sleep_for(std::chrono::milliseconds(32));
        break;
      }

      case State::Running: {
        auto request = m_pimpl->ReadRequest();
        if (!request.has_value()) [[unlikely]] {
          sucessive_failed_request_count++;
          Log << "LSPServer: Start(): ReadRequest() failed";

          if (sucessive_failed_request_count > kMaxFailedRequestCount) {
            Log << "LSPServer: Start(): Too many successive invalid requests (max: " << kMaxFailedRequestCount
                << "). Exiting.";
            Log << Trace << "LSPServer: Start(): State::Running -> State::Exited";
            m_pimpl->m_state = State::Exited;
            break;
          }

          break;
        }

        sucessive_failed_request_count = 0;

        auto& scheduler = m_pimpl->m_request_scheduler;
        scheduler.Schedule(std::move(request.value()));

        if (scheduler.IsExitRequested()) [[unlikely]] {
          Log << Trace << "LSPServer: Start(): Exit requested";
          Log << Trace << "LSPServer: Start(): State::Running -> State::Exited";
          m_pimpl->m_state = State::Exited;
          break;
        }

        break;
      }

      case State::Exited: {
        return true;
      }
    }
  }
}

auto LSPServer::Suspend() -> bool {
  qcore_assert(m_pimpl != nullptr);
  std::lock_guard lock(m_pimpl->m_state_mutex);

  Log << Trace << "LSPServer: Suspend() called";

  switch (auto current_state = m_pimpl->m_state) {
    case State::Suspended: {
      Log << Trace << "LSPServer: Suspend(): State::Suspended -> State::Suspended";
      return true;
    }

    case State::Running: {
      Log << Trace << "LSPServer: Suspend(): State::Running -> State::Suspended";
      m_pimpl->m_state = State::Suspended;
      return true;
    }

    case State::Exited: {
      Log << Trace << "LSPServer: Suspend(): State::Exited -> State::Exited";
      // Already exited, can not suspend
      return false;
    }
  }
}

auto LSPServer::Resume() -> bool {
  qcore_assert(m_pimpl != nullptr);
  std::lock_guard lock(m_pimpl->m_state_mutex);

  Log << Trace << "LSPServer: Resume() called";

  switch (auto current_state = m_pimpl->m_state) {
    case State::Suspended: {
      Log << Trace << "LSPServer: Resume(): State::Suspended -> State::Running";
      m_pimpl->m_state = State::Running;
      return true;
    }

    case State::Running: {
      Log << Trace << "LSPServer: Resume(): State::Running -> State::Running";
      return true;
    }

    case State::Exited: {
      Log << Trace << "LSPServer: Resume(): State::Exited -> State::Exited";
      // Already exited, can not resume
      return false;
    }
  }
}

auto LSPServer::Stop() -> bool {
  qcore_assert(m_pimpl != nullptr);
  std::lock_guard lock(m_pimpl->m_state_mutex);

  Log << Trace << "LSPServer: Stop() called";

  switch (auto current_state = m_pimpl->m_state) {
    case State::Suspended: {
      Log << Trace << "LSPServer: Stop(): State::Suspended -> State::Exited";
      m_pimpl->m_state = State::Exited;
      return true;
    }

    case State::Running: {
      Log << Trace << "LSPServer: Stop(): State::Running -> State::Exited";
      m_pimpl->m_state = State::Exited;
      return true;
    }

    case State::Exited: {
      Log << Trace << "LSPServer: Stop(): State::Exited -> State::Exited";
      return true;
    }
  }
}

auto LSPServer::GetState() const -> State {
  qcore_assert(m_pimpl != nullptr);
  std::lock_guard lock(m_pimpl->m_state_mutex);

  return m_pimpl->m_state;
}

///======================================================================================================================

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

  Log << Trace << "LSPServer: ParseHttpHeader() called";

  std::string line;
  std::getline(in, line);
  if (!in) [[unlikely]] {
    Log << "LSPServer: ParseHttpHeader(): Failed to read line";
    return std::nullopt;
  }

  if (line.ends_with("\r")) {
    line = line.substr(0, line.size() - 1);
  }

  if (line.empty()) [[unlikely]] {
    end_of_headers = true;
    return std::nullopt;
  }

  auto pos = line.find(':');
  if (pos == std::string::npos) [[unlikely]] {
    Log << "LSPServer: ParseHttpHeader(): Invalid header format";
    return std::nullopt;
  }

  auto key = HttpHeaderStripWhitespace(line.substr(0, pos));
  auto val = HttpHeaderStripWhitespace(line.substr(pos + 1));

  Log << Trace << "LSPServer: ParseHttpHeader(): Result: (\"" << key << "\", \"" << val << "\")";

  return std::make_pair(key, val);
}

struct HttpMessage {
  std::unordered_map<std::string, std::string> m_headers;
  std::string m_content;
};

static auto ParseHttpMessage(std::istream& in) -> std::optional<HttpMessage> {
  std::unordered_map<std::string, std::string> headers;

  while (true) {
    bool end_of_headers = false;
    auto header = ParseHttpHeader(in, end_of_headers);
    if (end_of_headers) {
      break;
    }

    if (!header.has_value()) [[unlikely]] {
      Log << "LSPServer: ParseHttpMessage(): Failed to parse HTTP header";
      return std::nullopt;
    }

    headers[header->first] = header->second;
  }

  if (headers.empty()) [[unlikely]] {
    Log << "LSPServer: ParseHttpMessage(): No headers found";
    return std::nullopt;
  }

  if (!headers.contains("Content-Length")) [[unlikely]] {
    Log << "LSPServer: ParseHttpMessage(): Missing 'Content-Length' header";
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
    Log << "LSPServer: ParseHttpMessage(): Invalid 'Content-Length' header value: \"" << content_length << "\"";
    return std::nullopt;
  }

  Log << Debug << "LSPServer: ParseHttpMessage(): Content-Length: " << content_length_value;

  std::string content;
  content.resize(content_length_value);
  if (!in.read(content.data(), content_length_value)) [[unlikely]] {
    Log << "LSPServer: ParseHttpMessage(): Failed to read content";
    return std::nullopt;
  }

  if (in.gcount() != content_length_value) [[unlikely]] {
    Log << "LSPServer: ParseHttpMessage(): Read content size mismatch";
    return std::nullopt;
  }

  Log << Trace << "LSPServer: ParseHttpMessage(): Content: " << content;

  return HttpMessage{std::move(headers), std::move(content)};
}

static auto QuickJsonRPCMessageCheck(const nlohmann::json& json_rpc) -> bool {
  if (!json_rpc.contains("jsonrpc")) [[unlikely]] {
    Log << "LSPServer: ValidateJsonRPCMessage(): Missing 'jsonrpc' field";
    return false;
  }

  if (!json_rpc["jsonrpc"].is_string()) [[unlikely]] {
    Log << "LSPServer: ValidateJsonRPCMessage(): 'jsonrpc' field is not a string";
    return false;
  }

  if (json_rpc["jsonrpc"] != "2.0") [[unlikely]] {
    Log << "LSPServer: ValidateJsonRPCMessage(): 'jsonrpc' field is not '2.0'";
    return false;
  }

  if (!json_rpc.contains("method")) [[unlikely]] {
    Log << "LSPServer: ValidateJsonRPCMessage(): Missing 'method' field";
    return false;
  }

  if (!json_rpc["method"].is_string()) [[unlikely]] {
    Log << "LSPServer: ValidateJsonRPCMessage(): 'method' field is not a string";
    return false;
  }

  return true;
}

auto LSPServer::PImpl::ReadRequest() -> std::optional<RequestMessage> {
  std::lock_guard lock(m_io_mutex);
  if (m_io.eof()) [[unlikely]] {
    Log << Warning << "LSPServer: ReadRequest(): EOF reached";
    return std::nullopt;
  }

  if (!m_io) [[unlikely]] {
    Log << "LSPServer: ReadRequest(): Bad stream";
    return std::nullopt;
  }

  auto http_message = ParseHttpMessage(m_io);
  if (!http_message.has_value()) [[unlikely]] {
    Log << "LSPServer: ReadRequest(): Failed to parse HTTP message";
    return std::nullopt;
  }

  const auto& message = http_message.value();

  auto json_rpc = nlohmann::json::parse(message.m_content, nullptr, false);
  if (json_rpc.is_discarded()) [[unlikely]] {
    Log << "LSPServer: ReadRequest(): Failed to parse JSON-RPC message";
    return std::nullopt;
  }

  if (!QuickJsonRPCMessageCheck(json_rpc)) [[unlikely]] {
    Log << "LSPServer: ReadRequest(): Invalid LSP JSON-RPC message";
    return std::nullopt;
  }

  /// TODO: Parse the request from the input stream
  return std::nullopt;
}

void LSPServer::PImpl::RequestScheduler::Schedule(RequestMessage request) {
  if (!m_thread_pool.has_value()) [[unlikely]] {
    m_thread_pool.emplace();
    m_thread_pool->Start();
  }

  (void)request;
  (void)m_io;
  (void)m_io_lock;

  /// TODO: Handle the request
}
