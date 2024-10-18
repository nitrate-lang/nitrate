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
    rapidjson::Document m_params;

  public:
    RequestMessage(const MessageId& id, const std::string& method, rapidjson::Document params)
        : Message(MessageType::Request), m_id(id), m_method(method), m_params(std::move(params)) {}
    virtual ~RequestMessage() = default;

    const MessageId& id() const { return m_id; }
    const std::string& method() const { return m_method; }
    const rapidjson::Document& params() const { return m_params; }

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
    rapidjson::Document m_params;

  public:
    NotificationMessage(const std::string& method, rapidjson::Document params)
        : Message(MessageType::Notification), m_method(method), m_params(std::move(params)) {}
    virtual ~NotificationMessage() = default;

    const std::string& method() const { return m_method; }
    const rapidjson::Document& params() const { return m_params; }

    void print(std::ostream& os) const { os << "{\"method\": \"" << m_method << "\"}"; }
  };

  enum class ErrorCodes {
    // Defined by JSON-RPC
    ParseError = -32700,
    InvalidRequest = -32600,
    MethodNotFound = -32601,
    InvalidParams = -32602,
    InternalError = -32603,

    /**
     * This is the start range of JSON-RPC reserved error codes.
     * It doesn't denote a real error code. No LSP error codes should
     * be defined between the start and end range. For backwards
     * compatibility the `ServerNotInitialized` and the `UnknownErrorCode`
     * are left in the range.
     *
     * @since 3.16.0
     */
    jsonrpcReservedErrorRangeStart = -32099,
    /** @deprecated use jsonrpcReservedErrorRangeStart */
    serverErrorStart = jsonrpcReservedErrorRangeStart,

    /**
     * Error code indicating that a server received a notification or
     * request before the server has received the `initialize` request.
     */
    ServerNotInitialized = -32002,
    UnknownErrorCode = -32001,

    /**
     * This is the end range of JSON-RPC reserved error codes.
     * It doesn't denote a real error code.
     *
     * @since 3.16.0
     */
    jsonrpcReservedErrorRangeEnd = -32000,
    /** @deprecated use jsonrpcReservedErrorRangeEnd */
    serverErrorEnd = jsonrpcReservedErrorRangeEnd,

    /**
     * This is the start range of LSP reserved error codes.
     * It doesn't denote a real error code.
     *
     * @since 3.16.0
     */
    lspReservedErrorRangeStart = -32899,

    /**
     * A request failed but it was syntactically correct, e.g the
     * method name was known and the parameters were valid. The error
     * message should contain human readable information about why
     * the request failed.
     *
     * @since 3.17.0
     */
    RequestFailed = -32803,

    /**
     * The server cancelled the request. This error code should
     * only be used for requests that explicitly support being
     * server cancellable.
     *
     * @since 3.17.0
     */
    ServerCancelled = -32802,

    /**
     * The server detected that the content of a document got
     * modified outside normal conditions. A server should
     * NOT send this error code if it detects a content change
     * in it unprocessed messages. The result even computed
     * on an older state might still be useful for the client.
     *
     * If a client decides that a result is not of any use anymore
     * the client should cancel the request.
     */
    ContentModified = -32801,

    /**
     * The client has canceled a request and a server has detected
     * the cancel.
     */
    RequestCancelled = -32800,

    /**
     * This is the end range of LSP reserved error codes.
     * It doesn't denote a real error code.
     *
     * @since 3.16.0
     */
    lspReservedErrorRangeEnd = -32800,
  };

  struct ResponseError {
    ErrorCodes m_code;
    std::string m_message;
    std::optional<rapidjson::Document> m_data;

    ResponseError(ErrorCodes code, const std::string& message,
                  std::optional<rapidjson::Document> data = std::nullopt)
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

    void error(ErrorCodes code, const std::string& message,
               std::optional<rapidjson::Document> data = std::nullopt) {
      m_error = ResponseError(code, message, std::move(data));
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

  void handle_request(const lsp::RequestMessage& request, std::ostream& out);
  void handle_notification(const lsp::NotificationMessage& notif);

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
