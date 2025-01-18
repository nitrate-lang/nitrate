#pragma once

#include <glog/logging.h>

#include <filesystem>
#include <functional>
#include <iostream>
#include <lsp/core/LSP.hh>
#include <lsp/core/common.hh>
#include <lsp/core/thread-pool.hh>
#include <memory>
#include <optional>
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

  auto ParseConfig(const std::filesystem::path& path)
      -> std::optional<Configuration>;

  using Connection =
      std::pair<std::unique_ptr<std::istream>, std::unique_ptr<std::ostream>>;

  enum class ConnectionType { Pipe, Port, Stdio };

  auto OpenConnection(ConnectionType type,
                      const String& target) -> std::optional<Connection>;

  using RequestHandler = std::function<void(const protocol::RequestMessage&,
                                            protocol::ResponseMessage&)>;

  using NotificationHandler =
      std::function<void(const protocol::NotificationMessage&)>;

  class ServerContext {
    ThreadPool m_thread_pool;
    std::function<void(const protocol::Message* message)> m_callback;
    std::unordered_map<String, RequestHandler> m_request_handlers;
    std::unordered_map<String, NotificationHandler> m_notification_handlers;
    std::queue<std::function<void()>> m_request_queue;
    std::mutex m_request_queue_mutex;
    std::mutex m_io_mutex;

    ServerContext() = default;

    void RegisterHandlers();
    void RequestQueueLoop(const std::stop_token& st);

    void HandleRequest(const protocol::RequestMessage& request,
                       std::ostream& out);
    void HandleNotification(const protocol::NotificationMessage& notif);

    auto NextMessage(std::istream& in)
        -> std::optional<std::unique_ptr<protocol::Message>>;

    void Dispatch(const std::shared_ptr<protocol::Message>& message,
                  std::ostream& out);

  public:
    ServerContext(const ServerContext&) = delete;
    ServerContext(ServerContext&&) = delete;

    static auto The() -> ServerContext&;

    [[noreturn]] void StartServer(Connection& io);

    void SetCallback(
        std::function<void(const protocol::Message* message)> callback) {
      m_callback = std::move(callback);
    }

    void RegisterRequestHandler(std::string_view method,
                                RequestHandler handler) {
      m_request_handlers[String(method)] = std::move(handler);
    }

    void RegisterNotificationHandler(std::string_view method,
                                     NotificationHandler handler) {
      m_notification_handlers[String(method)] = std::move(handler);
    }
  };
}  // namespace no3::lsp::srv
