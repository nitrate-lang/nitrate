#pragma once

#include <glog/logging.h>

#include <filesystem>
#include <functional>
#include <iostream>
#include <lsp/core/Common.hh>
#include <lsp/core/LSP.hh>
#include <lsp/core/ThreadPool.hh>
#include <memory>
#include <optional>
#include <shared_mutex>
#include <utility>

namespace no3::lsp::srv {
  using namespace no3::lsp::protocol;

  class Configuration {
    Configuration() = default;

  public:
    static auto Defaults() -> Configuration {
      Configuration config;
      return config;
    }
  };

  auto ParseConfig(const std::filesystem::path& path) -> std::optional<Configuration>;

  using Connection = std::pair<std::unique_ptr<std::istream>, std::unique_ptr<std::ostream>>;

  enum class ConnectionType { Pipe, Port, Stdio };

  auto OpenConnection(ConnectionType type, const String& target) -> std::optional<Connection>;

  using RequestHandler = std::function<void(const RequestMessage&, ResponseMessage&)>;

  using NotificationHandler = std::function<void(const NotificationMessage&)>;

  class ServerContext {
    ThreadPool m_thread_pool;
    std::unordered_map<String, RequestHandler> m_request_handlers;
    std::unordered_map<String, NotificationHandler> m_notification_handlers;
    std::queue<std::function<void()>> m_request_queue;
    std::mutex m_request_queue_mutex;
    std::shared_mutex m_task_mutex;
    std::mutex m_io_mutex;

    ServerContext() = default;

    auto RegisterHandlers() -> void;
    auto RequestQueueLoop(const std::stop_token& st) -> void;

    auto NextMessage(std::istream& in) -> std::optional<MessageObject>;
    auto Dispatch(MessageObject message, std::ostream& out) -> void;

    auto DoRequest(const RequestMessage& request, std::ostream& out) -> void;
    auto DoNotification(const NotificationMessage& notif) -> void;

  public:
    ServerContext(const ServerContext&) = delete;
    ServerContext(ServerContext&&) = delete;

    static auto The() -> ServerContext&;

    [[noreturn]] void StartServer(Connection& io);

    void RegisterRequestHandler(std::string_view method, RequestHandler handler) {
      m_request_handlers[String(method)] = std::move(handler);
    }

    void RegisterNotificationHandler(std::string_view method, NotificationHandler handler) {
      m_notification_handlers[String(method)] = std::move(handler);
    }
  };
}  // namespace no3::lsp::srv
