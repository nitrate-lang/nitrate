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

#include <lsp/core/RPC.hh>
#include <lsp/route/RoutesList.hh>
#include <memory>
#include <mutex>
#include <nitrate-core/Assert.hh>
#include <nitrate-core/Logger.hh>
#include <nlohmann/json.hpp>

using namespace ncc;
using namespace no3::lsp::core;
using namespace no3::lsp::message;

namespace no3::lsp::core {
  using LSPRequestFunc = std::function<void(const message::RequestMessage&, message::ResponseMessage&)>;
  using LSPNotifyFunc = std::function<void(const message::NotifyMessage&)>;

  static const std::unordered_map<std::string_view, LSPRequestFunc> LSP_REQUEST_MAP = {
      {"initialize", rpc::RequestInitialize},
      {"shutdown", rpc::RequestShutdown},
  };

  static const std::unordered_map<std::string_view, LSPNotifyFunc> LSP_NOTIFICATION_MAP = {
      {"initialized", rpc::NotifyInitialized},
      {"exit", rpc::NotifyExit},
      {"textDocument/didOpen", rpc::NotifyTextDocumentDidOpen},
      {"textDocument/didChange", rpc::NotifyTextDocumentDidChange},
      {"textDocument/didClose", rpc::NotifyTextDocumentDidClose},
      {"textDocument/didSave", rpc::NotifyTextDocumentDidSave},
  };
}  // namespace no3::lsp::core

void LSPScheduler::ExecuteLSPRequest(const message::RequestMessage& message) {
  auto method = message.GetMethod();
  auto response = message.GetResponseObject();
  const auto log_prefix = "LSPServer: LSPScheduler::ExecuteLSPRequest(\"" + std::string(method) + "\"): ";
  const auto may_ignore = method.starts_with("$/");
  if (may_ignore) {
    method.remove_prefix(2);
  }

  if (const auto is_initialize_request = method == "initialize"; m_is_lsp_initialized || is_initialize_request) {
    const auto route_it = LSP_REQUEST_MAP.find(method);
    if (route_it != LSP_REQUEST_MAP.end()) {
      Log << Trace << log_prefix << "Found route, executing";
      route_it->second(message, response);
      Log << Trace << log_prefix << "Finished executing route";

      if (is_initialize_request) [[unlikely]] {
        m_is_lsp_initialized = true;
        Log << Debug << log_prefix << "LSP initialized";
      }
    } else [[unlikely]] {
      if (may_ignore) {
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

  const auto json_response = nlohmann::to_string(*response.Finalize());

  {  // Critical section
    std::lock_guard lock(m_io_lock);
    m_io << "Content-Length: " << json_response.size() << "\r\n";
    m_io << "Content-Type: application/vscode-jsonrpc; charset=utf-8\r\n\r\n";
    m_io << json_response;
    m_io.flush();
  }

  Log << Trace << log_prefix << "Wrote response: " << json_response;
}

void LSPScheduler::ExecuteLSPNotification(const message::NotifyMessage& message) {
  auto method = message.GetMethod();
  const auto log_prefix = "LSPServer: LSPScheduler::ExecuteLSPNotification(\"" + std::string(method) + "\"): ";
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
    Log << Trace << log_prefix << "Found route, executing";
    route_it->second(message);
    Log << Trace << log_prefix << "Finished executing route";

    if (is_exit_notification) [[unlikely]] {
      m_exit_requested = true;
      Log << Info << log_prefix << "Exit requested";
    }
  } else [[unlikely]] {
    Log << log_prefix << "LSP not initialized, ignoring notification";
  }
}

void LSPScheduler::ExecuteRPC(const message::Message& message) {
  const auto method = message.GetMethod();

  switch (message.GetKind()) {
    case MessageKind::Notification: {
      Log << Trace << "LSPServer: LSPScheduler::ExecuteRPC(\"" << method << "\"): Executing notification";
      ExecuteLSPNotification(static_cast<const NotifyMessage&>(message));
      Log << Trace << "LSPServer: LSPScheduler::ExecuteRPC(\"" << method << "\"): Finished notification";
      break;
    }

    case MessageKind::Request: {
      Log << Trace << "LSPServer: LSPScheduler::ExecuteRPC(\"" << method << "\"): Executing request";
      ExecuteLSPRequest(static_cast<const RequestMessage&>(message));
      Log << Trace << "LSPServer: LSPScheduler::ExecuteRPC(\"" << method << "\"): Finished request";
      break;
    }

    case MessageKind::Response: {
      break;
    }
  }
}

auto LSPScheduler::IsConcurrentRequest(const message::Message& message) -> bool {
  /// TODO: Decide if the request is concurrent or not

  (void)message;

  return false;
}

void LSPScheduler::Schedule(std::unique_ptr<Message> request) {
  if (m_exit_requested) [[unlikely]] {
    Log << Trace << "LSPServer: LSPScheduler::Schedule(): Exit requested, ignoring request";
    return;
  }

  if (!m_thread_pool.has_value()) [[unlikely]] {
    Log << Trace << "LSPServer: LSPScheduler::Schedule(): Starting thread pool";

    m_thread_pool.emplace();
    m_thread_pool->Start();
  }

  const auto method = request->GetMethod();

  if (IsConcurrentRequest(*request)) {
    Log << Trace << "LSPServer: LSPScheduler::Schedule(\"" << method << "\"): Scheduling concurrent request";

    auto sh = std::make_shared<std::unique_ptr<Message>>(std::move(request));
    m_thread_pool->Schedule([this, sh](const std::stop_token&) { ExecuteRPC(**sh); });
  } else {
    Log << Trace << "LSPServer: LSPScheduler::Schedule(\"" << method << "\"): Concurrency disallowed";

    // Enforce strict ordering
    while (!m_thread_pool->Empty()) {
      std::this_thread::yield();
    }

    // We execute this on the main thread
    ExecuteRPC(*request);
  }
}
