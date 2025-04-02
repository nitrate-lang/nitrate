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
  });
}

LSPContext::LSPContext() : m_fs(TextDocumentSyncKind::Incremental) {
  static std::once_flag init_flag;
  std::call_once(init_flag, []() {
    Log << Trace << "LSPContext::LSPContext(): Initializing LSP context";
    BoostFlyweightInit();
  });
}

LSPContext::~LSPContext() = default;

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

      if (is_initialize_request) [[unlikely]] {
        m_is_lsp_initialized = true;
        Log << Debug << log_prefix << "LSP initialized";
      }
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

    if (is_exit_notification) [[unlikely]] {
      m_exit_requested = true;
      Log << Info << log_prefix << "Exit requested";
    }
  } else [[unlikely]] {
    Log << log_prefix << "LSP not initialized, ignoring notification";
  }
}

auto LSPContext::ExecuteRPC(const message::Message& message,
                            bool& exit_requested) -> std::optional<message::ResponseMessage> {
  switch (const auto method = message.GetMethod(); message.GetKind()) {
    case MessageKind::Notification: {
      Log << Trace << "LSPContext::ExecuteRPC(\"" << method << "\"): Executing LSP Notification";
      ExecuteLSPNotification(static_cast<const NotifyMessage&>(message));
      Log << Trace << "LSPContext::ExecuteRPC(\"" << method << "\"): Finished LSP Notification";

      exit_requested = m_exit_requested;

      return std::nullopt;
    }

    case MessageKind::Request: {
      Log << Trace << "LSPContext::ExecuteRPC(\"" << method << "\"): Executing LSP Request";
      auto response = ExecuteLSPRequest(static_cast<const RequestMessage&>(message));
      Log << Trace << "LSPContext::ExecuteRPC(\"" << method << "\"): Finished LSP Request";

      exit_requested = m_exit_requested;

      return response;
    }

    case MessageKind::Response: {
      return std::nullopt;
    }
  }
}
