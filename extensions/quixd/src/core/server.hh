#pragma once

#include <glog/logging.h>
#include <rapidjson/document.h>

#include <core/thread-pool.hh>
#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <optional>
#include <variant>

struct MyLogSink : google::LogSink {
private:
  std::ofstream m_log_file;

  static std::string escape_message(std::string_view str);

public:
  MyLogSink(const std::string& log_file);

  void send(google::LogSeverity severity, const char*, const char* base_filename, int line,
            const struct tm* tm, const char* message, std::size_t message_len) override;
};

struct Configuration {
private:
  Configuration() = default;

public:
  static Configuration defaults() {
    Configuration config;
    return config;
  }
};

std::optional<Configuration> parse_config(const std::string& path);

enum class ConnectionType { Pipe, Port, Stdio };

class BufIStream : public std::istream {
  std::shared_ptr<std::streambuf> m_buf;

public:
  BufIStream(std::shared_ptr<std::streambuf> buf) : std::istream(buf.get()), m_buf(buf) {}
};

class BufOStream : public std::ostream {
  std::shared_ptr<std::streambuf> m_buf;

public:
  BufOStream(std::shared_ptr<std::streambuf> buf) : std::ostream(buf.get()), m_buf(buf) {}
  ~BufOStream();
};

using Connection = std::pair<std::unique_ptr<BufIStream>, std::unique_ptr<BufOStream>>;

std::optional<Connection> open_connection(ConnectionType type, const std::string& param);

namespace lsp {
  enum class MessageType { Request, Notification };

  class Message {
    MessageType m_type;

  public:
    Message(MessageType type) : m_type(type) {}
    virtual ~Message() = default;

    MessageType type() const { return m_type; }
  };

  typedef std::variant<std::string, int64_t> MessageId;

  class RequestMessage : public Message {
    MessageId m_id;
    std::string m_method;
    rapidjson::Value m_params;

  public:
    RequestMessage(const MessageId& id, const std::string& method, rapidjson::Value params)
        : Message(MessageType::Request), m_id(id), m_method(method), m_params(std::move(params)) {}
    virtual ~RequestMessage() = default;

    const MessageId& id() const { return m_id; }
    const std::string& method() const { return m_method; }
    const rapidjson::Value& params() const { return m_params; }

    void print(std::ostream& os) const {
      os << "{\"id\": ";
      if (std::holds_alternative<std::string>(m_id)) {
        os << "\"" << std::get<std::string>(m_id) << "\"";
      } else {
        os << std::get<int64_t>(m_id);
      }
      os << ", \"method\": \"" << m_method << "\"}";
    }
  };

  class NotificationMessage : public Message {
    std::string m_method;
    rapidjson::Value m_params;

  public:
    NotificationMessage(const std::string& method, rapidjson::Value params)
        : Message(MessageType::Notification), m_method(method), m_params(std::move(params)) {}
    virtual ~NotificationMessage() = default;

    const std::string& method() const { return m_method; }
    const rapidjson::Value& params() const { return m_params; }

    void print(std::ostream& os) const { os << "{\"method\": \"" << m_method << "\"}"; }
  };

  struct ResponseError {
    int m_code;
    std::string m_message;
    std::optional<rapidjson::Value> m_data;

    ResponseError(int code, const std::string& message,
                  std::optional<rapidjson::Value> data = std::nullopt)
        : m_code(code), m_message(message), m_data(std::move(data)) {}
  };

  class ResponseMessage : public Message {
    MessageId m_id;
    std::optional<rapidjson::Document> m_result;
    std::optional<ResponseError> m_error;

    ResponseMessage() : Message(MessageType::Request) {}

  public:
    ResponseMessage(ResponseMessage&& o) : Message(MessageType::Request) {
      m_id = std::move(o.m_id);
      if (o.m_result.has_value()) {
        m_result = std::move(o.m_result.value());
      }
      if (o.m_error.has_value()) {
        m_error = std::move(o.m_error.value());
      }
    }

    static ResponseMessage from_request(const RequestMessage& request) {
      ResponseMessage response;
      response.m_id = request.id();
      return response;
    }

    virtual ~ResponseMessage() = default;

    const MessageId& id() const { return m_id; }
    std::optional<rapidjson::Document>& result() { return m_result; }
    std::optional<ResponseError>& error() { return m_error; }

    rapidjson::Document* operator->() {
      if (!m_result.has_value()) {
        m_result = rapidjson::Document(rapidjson::kObjectType);
      }
      return &m_result.value();
    }

    rapidjson::Document& operator*() {
      if (!m_result.has_value()) {
        m_result = rapidjson::Document(rapidjson::kObjectType);
      }
      return m_result.value();
    }
  };
}  // namespace lsp

typedef std::function<void(const lsp::RequestMessage&, lsp::ResponseMessage&)> RequestHandler;
typedef std::function<void(const lsp::NotificationMessage&)> NotificationHandler;

class ServerContext {
  ThreadPool m_thread_pool;
  std::function<void(const lsp::Message* message)> m_callback;
  std::unordered_map<std::string, RequestHandler> m_request_handlers;
  std::unordered_map<std::string, NotificationHandler> m_notification_handlers;
  std::queue<std::function<void()>> m_request_queue;
  std::mutex m_request_queue_mutex;
  std::mutex m_io_mutex;

  ServerContext() = default;

  ServerContext(const ServerContext&) = delete;
  ServerContext(ServerContext&&) = delete;

  void request_queue_loop(std::stop_token st);

  void handle_request(const lsp::RequestMessage& request, std::ostream& out) noexcept;
  void handle_notification(const lsp::NotificationMessage& notif) noexcept;

  std::optional<std::unique_ptr<lsp::Message>> next_message(std::istream& in);

  void dispatch(const std::shared_ptr<lsp::Message> message, std::ostream& out);

public:
  static ServerContext& the();

  [[noreturn]] void start_server(Connection& io);

  void set_callback(std::function<void(const lsp::Message* message)> callback) {
    m_callback = std::move(callback);
  }

  void register_request_handler(std::string method, RequestHandler handler) {
    m_request_handlers[method] = std::move(handler);
  }

  void register_notification_handler(std::string method, NotificationHandler handler) {
    m_notification_handlers[method] = std::move(handler);
  }
};

#define ADD_REQUEST_HANDLER(method, handler)                                                   \
  namespace {                                                                                  \
    struct Register_##handler {                                                                \
      Register_##handler() { ServerContext::the().register_request_handler(method, handler); } \
    } register_##handler;                                                                      \
  }

#define ADD_NOTIFICATION_HANDLER(method, handler)                            \
  namespace {                                                                \
    struct Register_##handler {                                              \
      Register_##handler() {                                                 \
        ServerContext::the().register_notification_handler(method, handler); \
      }                                                                      \
    } register_##handler;                                                    \
  }
