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

#include <lsp/core/LSPContext.hh>
#include <lsp/core/protocol/LogTrace.hh>
#include <nitrate-core/Assert.hh>
#include <nitrate-core/Logger.hh>

using namespace ncc;
using namespace no3::lsp::core;
using namespace no3::lsp::message;
using namespace no3::lsp::protocol;

static void BoostFlyweightInit() {
  static std::once_flag flyweight_init_flag;
  std::call_once(flyweight_init_flag, []() {
    Log << Trace << "LSPContext: BoostFlyweightInit(): Initializing boost flyweight";

    FlyString::init();
    FlyByteString::init();
  });
}

void LSPContext::FlushLogTraceQueue() {
  while (true) {
    const auto message = [&]() -> std::optional<std::string> {
      std::lock_guard lock(m_log_trace_lock);

      if (m_log_trace_queue.empty()) {
        return std::nullopt;
      }

      auto res = std::move(m_log_trace_queue.front());
      m_log_trace_queue.pop();

      return res;
    }();

    if (!message.has_value()) [[unlikely]] {
      break;
    }

    auto trace_message = LogTraceNotification(message.value());
    SendMessage(trace_message, false);
  }
}

auto LSPContext::ExecuteLSPRequest(const message::RequestMessage& message) -> message::ResponseMessage {
  auto method = message.GetMethod();
  auto response = message.GetResponseObject();
  const auto log_prefix = "LSPContext::ExecuteLSPRequest(\"" + std::string(method) + "\"): ";
  const auto may_ignore = method.starts_with("$/");
  if (may_ignore) {
    method.remove_prefix(2);
  }

  if (const auto is_initialize_request = method == "initialize"; m_is_lsp_initialized || is_initialize_request) {
    const auto route_it = LSP_REQUEST_MAP.find(method);
    if (route_it != LSP_REQUEST_MAP.end()) {
      (*this.*route_it->second)(message, response);
    } else [[unlikely]] {
      if (may_ignore) [[likely]] {
        Log << Debug << log_prefix << "Ignoring request";
      } else {
        Log << log_prefix << "No route found, ignoring request";
      }

      response.SetStatusCode(StatusCode::MethodNotFound);
    }
  } else [[unlikely]] {
    response.SetStatusCode(StatusCode::ServerNotInitialized);
    Log << Warning << log_prefix << "LSP not initialized, ignoring request";
  }

  return response;
}

void LSPContext::ExecuteLSPNotification(const message::NotifyMessage& message) {
  auto method = message.GetMethod();
  const auto log_prefix = "LSPContext::ExecuteLSPNotification(\"" + std::string(method) + "\"): ";
  const auto may_ignore = method.starts_with("$/");
  if (may_ignore) {
    method.remove_prefix(2);
  }

  const auto route_it = LSP_NOTIFICATION_MAP.find(method);
  const auto found_route = route_it != LSP_NOTIFICATION_MAP.end();
  if (!found_route) {
    if (may_ignore) {
      Log << Debug << log_prefix << "Ignoring notification";
    } else [[unlikely]] {
      Log << log_prefix << "No route found, ignoring notification";
    }

    return;
  }

  if (const auto is_exit_notification = method == "exit"; m_is_lsp_initialized || is_exit_notification) {
    (*this.*route_it->second)(message);
  } else [[unlikely]] {
    Log << log_prefix << "LSP not initialized, ignoring notification";
  }
}

void LSPContext::ExecuteRPC(const message::Message& message, bool& exit_requested) {
  switch (const auto method = message.GetMethod(); message.GetKind()) {
    case MessageKind::Notification: {
      Log << Debug << "LSPContext::ExecuteRPC(\"" << method << "\"): Executing LSP Notification";
      ExecuteLSPNotification(static_cast<const NotifyMessage&>(message));
      Log << Debug << "LSPContext::ExecuteRPC(\"" << method << "\"): Finished LSP Notification";

      exit_requested = m_exit_requested;

      break;
    }

    case MessageKind::Request: {
      Log << Debug << "LSPContext::ExecuteRPC(\"" << method << "\"): Executing LSP Request";
      auto response = ExecuteLSPRequest(static_cast<const RequestMessage&>(message));
      Log << Debug << "LSPContext::ExecuteRPC(\"" << method << "\"): Finished LSP Request";

      exit_requested = m_exit_requested;

      SendMessage(response);

      break;
    }

    case MessageKind::Response: {
      break;
    }
  }

  if (m_is_lsp_initialized) {
    FlushLogTraceQueue();
  }
}

void LSPContext::SendMessage(Message& message, bool log_transmission) {
  const auto json_response = nlohmann::to_string(*message.Finalize());

  {  // Critical section
    std::lock_guard lock(m_os_lock);
    m_os << "Content-Length: " << json_response.size() << "\r\n";
    m_os << "Content-Type: application/vscode-jsonrpc; charset=utf-8\r\n\r\n";
    m_os << json_response;
    m_os.flush();
  }

  if (log_transmission) {
    Log << Trace << "SendJsonRPCMessage(): Wrote response: " << json_response;
  }
}

LSPContext::LSPContext(std::ostream& os, std::mutex& os_lock)
    : m_os(os), m_os_lock(os_lock), m_fs(TextDocumentSyncKind::Incremental) {
  static std::once_flag init_flag;
  std::call_once(init_flag, []() {
    Log << Trace << "LSPContext::LSPContext(): Initializing LSP context";
    BoostFlyweightInit();
  });

  m_log_subscriber_id = Log->Subscribe([this](const ncc::LogMessage& log) {
    auto message = log.m_by.Format(log.m_message, log.m_sev);

    /// FIXME: Support log levels

    {
      std::lock_guard lock(m_log_trace_lock);
      m_log_trace_queue.push(message);
    }
  });
}

LSPContext::~LSPContext() { Log->Unsubscribe(m_log_subscriber_id); }
