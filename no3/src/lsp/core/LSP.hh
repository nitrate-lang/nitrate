#pragma once

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include <lsp/core/common.hh>
#include <optional>
#include <utility>

namespace no3::lsp::protocol {
  enum class MessageType { Request, Notification };

  class Message {
    MessageType m_type;

  public:
    Message(MessageType type) : m_type(type) {}
    virtual ~Message() = default;

    [[nodiscard]] auto GetKind() const -> MessageType { return m_type; }
  };

  enum class MessageIdKind : int8_t { String, Int };

  class MessageId final {
    String m_data;
    int64_t m_int;
    MessageIdKind m_kind;

  public:
    MessageId(String id)
        : m_data(std::move(id)), m_int(0), m_kind(MessageIdKind::String) {}
    MessageId(int64_t id)
        : m_data(std::to_string(id)), m_int(id), m_kind(MessageIdKind::Int) {}

    [[nodiscard]] auto GetKind() const { return m_kind; }
    [[nodiscard]] auto GetString() const { return m_data; }
    [[nodiscard]] auto GetInt() const { return m_int; }
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
    std::optional<rapidjson::Document> m_data;
    String m_message;
    ErrorCodes m_code;

    ResponseError(auto code, auto message, auto data)
        : m_data(std::move(data)),
          m_message(std::move(message)),
          m_code(code) {}
  };

  class RequestMessage final : public Message {
    rapidjson::Document m_params;
    String m_method;
    MessageId m_id;

  public:
    RequestMessage(auto id, auto method, auto params)
        : Message(MessageType::Request),
          m_params(std::move(params)),
          m_method(std::move(method)),
          m_id(std::move(id)) {}
    ~RequestMessage() override = default;

    [[nodiscard]] auto GetMID() const -> const MessageId& { return m_id; }
    [[nodiscard]] auto GetMethod() const { return m_method; }

    [[nodiscard]] auto GetJSON() const -> const rapidjson::Document& {
      return m_params;
    }
  };

  class NotificationMessage final : public Message {
    String m_method;
    rapidjson::Document m_params;

  public:
    NotificationMessage(String method, rapidjson::Document params)
        : Message(MessageType::Notification),
          m_method(std::move(method)),
          m_params(std::move(params)) {}
    ~NotificationMessage() override = default;

    [[nodiscard]] auto Method() const { return m_method; }
    [[nodiscard]] auto GetJSON() const -> const rapidjson::Document& {
      return m_params;
    }
  };

  class ResponseMessage : public Message {
    MessageId m_id;
    std::optional<rapidjson::Document> m_result;
    std::optional<ResponseError> m_error;

    ResponseMessage(MessageId id)
        : Message(MessageType::Request), m_id(std::move(id)) {}

  public:
    ResponseMessage(ResponseMessage&& o) noexcept
        : Message(MessageType::Request), m_id(std::move(o.m_id)) {
      if (o.m_result.has_value()) {
        m_result = std::move(o.m_result.value());
      }
      if (o.m_error.has_value()) {
        m_error = std::move(o.m_error.value());
      }
    }

    static auto FromRequest(const RequestMessage& request) -> ResponseMessage {
      ResponseMessage response(request.GetMID());
      return response;
    }

    ~ResponseMessage() override = default;

    [[nodiscard]] auto Id() const -> const MessageId& { return m_id; }
    auto Result() -> std::optional<rapidjson::Document>& { return m_result; }
    auto Error() -> std::optional<ResponseError>& { return m_error; }

    auto operator->() -> rapidjson::Document* {
      if (!m_result.has_value()) {
        m_result = rapidjson::Document(rapidjson::kObjectType);
      }
      return &m_result.value();
    }

    auto operator*() -> rapidjson::Document& {
      if (!m_result.has_value()) {
        m_result = rapidjson::Document(rapidjson::kObjectType);
      }
      return m_result.value();
    }

    void Error(auto code, auto message,
               std::optional<rapidjson::Document> data = std::nullopt) {
      m_error = ResponseError(code, std::move(message), std::move(data));
    }
  };

}  // namespace no3::lsp::protocol
