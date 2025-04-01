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

#include <boost/flyweight.hpp>
#include <lsp/core/protocol/StatusCode.hh>
#include <nlohmann/json.hpp>
#include <optional>
#include <utility>
#include <variant>

namespace no3::lsp::message {
  enum class MessageIdKind : uint8_t { String, Int };

  class MessageId final {
    std::string m_data;
    MessageIdKind m_kind;

  public:
    MessageId(std::string id) : m_data(std::move(id)), m_kind(MessageIdKind::String) {}
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
    std::string m_method;
    MessageId m_id;

  public:
    RequestMessage(auto id, auto method, auto params)
        : m_params(std::move(params)), m_method(std::move(method)), m_id(std::move(id)) {}

    [[nodiscard]] constexpr auto operator==(const RequestMessage& o) const -> bool {
      return m_id == o.m_id && m_method == o.m_method && m_params == o.m_params;
    }

    [[nodiscard]] constexpr auto GetHash() const -> size_t {
      /// FIXME: Fix the hash function
      return std::hash<std::string>{}(m_method);
    }

    [[nodiscard]] auto GetMID() const { return m_id; }
    [[nodiscard]] auto GetMethod() const { return m_method; }
    [[nodiscard]] constexpr auto GetJSON() const -> const nlohmann::json& { return m_params; }
  };

  class NotificationMessage final {
    std::string m_method;
    nlohmann::json m_params;

  public:
    NotificationMessage(std::string method, nlohmann::json params)
        : m_method(std::move(method)), m_params(std::move(params)) {}

    [[nodiscard]] constexpr auto operator==(const NotificationMessage& o) const -> bool {
      return m_method == o.m_method && m_params == o.m_params;
    }

    [[nodiscard]] constexpr auto GetHash() const -> size_t {
      /// FIXME: Fix the hash function
      return std::hash<std::string>{}(m_method);
    }

    [[nodiscard]] auto Method() const { return m_method; }
    [[nodiscard]] auto GetJSON() const -> const nlohmann::json& { return m_params; }
  };

  struct ResponseError {
    std::optional<nlohmann::json> m_data;
    std::string m_message;
    StatusCode m_code;

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
      return std::hash<std::string>{}(m_id.GetString());
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
}  // namespace no3::lsp::message
