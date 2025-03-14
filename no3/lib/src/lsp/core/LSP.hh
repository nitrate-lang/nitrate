#pragma once

#include <lsp/core/Common.hh>
#include <nlohmann/json.hpp>
#include <optional>
#include <utility>
#include <variant>

namespace no3::lsp::protocol {
  enum class LSPStatus {
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

  enum class MessageIdKind : uint8_t { String, Int };

  class MessageId final {
    String m_data;
    MessageIdKind m_kind;

  public:
    MessageId(String id) : m_data(std::move(id)), m_kind(MessageIdKind::String) {}
    MessageId(int64_t id) : m_data(std::to_string(id)), m_kind(MessageIdKind::Int) {}

    [[nodiscard]] constexpr auto GetKind() const { return m_kind; }
    [[nodiscard]] constexpr auto GetInt() const { return std::stoll(m_data); }
    [[nodiscard]] auto GetString() const { return m_data; }

    [[nodiscard]] constexpr auto operator==(const MessageId& o) const -> bool {
      return m_kind == o.m_kind && m_data == o.m_data;
    }

    [[nodiscard]] constexpr auto operator<(const MessageId& o) const -> bool {
      if (m_kind != o.m_kind) {
        return m_kind < o.m_kind;
      }

      return m_data < o.m_data;
    }
  };

  class RequestMessage final {
    nlohmann::json m_params;
    String m_method;
    MessageId m_id;

  public:
    RequestMessage(auto id, auto method, auto params)
        : m_params(std::move(params)), m_method(std::move(method)), m_id(std::move(id)) {}

    [[nodiscard]] constexpr auto operator==(const RequestMessage& o) const -> bool {
      return m_id == o.m_id && m_method == o.m_method && m_params == o.m_params;
    }

    [[nodiscard]] constexpr auto GetHash() const -> size_t {
      /// FIXME: Fix the hash function
      return std::hash<String>{}(m_method);
    }

    [[nodiscard]] auto GetMID() const { return m_id; }
    [[nodiscard]] auto GetMethod() const { return m_method; }
    [[nodiscard]] constexpr auto GetJSON() const -> const nlohmann::json& { return m_params; }
  };

  class NotificationMessage final {
    String m_method;
    nlohmann::json m_params;

  public:
    NotificationMessage(String method, nlohmann::json params)
        : m_method(std::move(method)), m_params(std::move(params)) {}

    [[nodiscard]] constexpr auto operator==(const NotificationMessage& o) const -> bool {
      return m_method == o.m_method && m_params == o.m_params;
    }

    [[nodiscard]] constexpr auto GetHash() const -> size_t {
      /// FIXME: Fix the hash function
      return std::hash<String>{}(m_method);
    }

    [[nodiscard]] auto Method() const { return m_method; }
    [[nodiscard]] auto GetJSON() const -> const nlohmann::json& { return m_params; }
  };

  struct ResponseError {
    std::optional<nlohmann::json> m_data;
    String m_message;
    LSPStatus m_code;

    ResponseError(auto code, auto message, auto data)
        : m_data(std::move(data)), m_message(std::move(message)), m_code(code) {}

    [[nodiscard]] constexpr auto operator==(const ResponseError& o) const -> bool {
      return m_code == o.m_code && m_message == o.m_message && m_data == o.m_data;
    }
  };

  class ResponseMessage final {
    MessageId m_id;
    nlohmann::json m_result;
    std::optional<ResponseError> m_error;

    ResponseMessage(MessageId id) : m_id(std::move(id)) { m_result = nlohmann::json::object(); }

  public:
    static auto FromRequest(const RequestMessage& request) -> ResponseMessage {
      ResponseMessage response(request.GetMID());
      return response;
    }

    ResponseMessage(ResponseMessage&& o) noexcept : m_id(std::move(o.m_id)) {
      m_result = std::move(o.m_result);
      if (o.m_error.has_value()) {
        m_error = std::move(o.m_error.value());
      }
    }

    [[nodiscard]] constexpr auto operator==(const ResponseMessage& o) const -> bool {
      return m_id == o.m_id && m_result == o.m_result && m_error == o.m_error;
    }

    [[nodiscard]] auto GetHash() const -> size_t {
      /// FIXME: Fix the hash function
      return std::hash<String>{}(m_id.GetString());
    }

    [[nodiscard]] auto GetMID() const -> const MessageId& { return m_id; }
    auto GetJSON() -> nlohmann::json& { return m_result; }

    auto Error() -> std::optional<ResponseError>& { return m_error; }
    auto Error(auto code, auto message) { m_error = ResponseError(code, std::move(message), std::nullopt); }

    [[nodiscard]] auto operator->() -> auto& { return m_result; }
    [[nodiscard]] auto operator*() -> auto& { return m_result; }

    [[nodiscard]] auto GetJSON() const -> const nlohmann::json& { return m_result; }
  };

  using MessageVariant = std::variant<RequestMessage, NotificationMessage, ResponseMessage>;
  [[nodiscard]]

  static inline size_t hash_value(const MessageVariant& m) {  // NOLINT
    return std::visit([](const auto& m) { return m.GetHash(); }, m);
  }

  using MessageObject = boost::flyweight<MessageVariant>;
}  // namespace no3::lsp::protocol
