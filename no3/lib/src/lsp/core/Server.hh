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

#include <functional>
#include <iostream>
#include <lsp/core/LSP.hh>
#include <lsp/core/ThreadPool.hh>
#include <lsp/core/connect/Connection.hh>
#include <optional>
#include <queue>
#include <shared_mutex>
#include <utility>

namespace no3::lsp::core {
  enum class ConnectionType : uint8_t { Pipe, Port, Stdio };
  using RequestHandler = std::function<void(const message::RequestMessage&, message::ResponseMessage&)>;
  using NotificationHandler = std::function<void(const message::NotificationMessage&)>;

  auto OpenConnection(ConnectionType type, const std::string& target) -> std::optional<DuplexStream>;

  class ServerContext {
    ThreadPool m_thread_pool;
    std::unordered_map<std::string, RequestHandler> m_request_handlers;
    std::unordered_map<std::string, NotificationHandler> m_notification_handlers;
    std::queue<std::function<void()>> m_request_queue;
    std::mutex m_request_queue_mutex;
    std::shared_mutex m_task_mutex;
    std::mutex m_io_mutex;

    ServerContext() = default;

    auto RegisterHandlers() -> void;
    auto RequestQueueLoop(const std::stop_token& st) -> void;

    auto NextMessage(std::istream& in) -> std::optional<message::MessageObject>;
    auto Dispatch(message::MessageObject message, std::ostream& out) -> void;

    auto DoRequest(const message::RequestMessage& request, std::ostream& out) -> void;
    auto DoNotification(const message::NotificationMessage& notif) -> void;

  public:
    ServerContext(const ServerContext&) = delete;
    ServerContext(ServerContext&&) = delete;

    static auto The() -> ServerContext&;

    [[noreturn]] void StartServer(DuplexStream& io);

    void RegisterRequestHandler(std::string_view method, RequestHandler handler) {
      m_request_handlers[std::string(method)] = std::move(handler);
    }

    void RegisterNotificationHandler(std::string_view method, NotificationHandler handler) {
      m_notification_handlers[std::string(method)] = std::move(handler);
    }
  };
}  // namespace no3::lsp::core
