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

#include <lsp/protocol/Message.hh>
#include <lsp/protocol/StatusCode.hh>

namespace no3::lsp::message {
  using MessageSequenceID = std::variant<int64_t, std::string>;

  class ResponseMessage : public Message {
    friend class RequestMessage;

    MessageSequenceID m_request_id;
    std::optional<StatusCode> m_status_code;

    ResponseMessage(MessageSequenceID request_id)
        : Message(MessageKind::Response), m_request_id(std::move(request_id)) {}

  public:
    ResponseMessage(const ResponseMessage&) = delete;
    ResponseMessage(ResponseMessage&&) = default;
    ~ResponseMessage() override = default;

    [[nodiscard]] auto GetResponseID() const -> const MessageSequenceID& { return m_request_id; }
    [[nodiscard]] auto GetStatusCode() const -> std::optional<StatusCode> { return m_status_code; }
    [[nodiscard]] auto GetResult() const -> const nlohmann::json& { return **this; }
    [[nodiscard]] auto GetError() const -> const nlohmann::json& { return **this; }
    [[nodiscard]] auto IsValidResponse() const -> bool { return !m_status_code.has_value(); }
    [[nodiscard]] auto IsErrorResponse() const -> bool { return m_status_code.has_value(); }

    void SetStatusCode(std::optional<StatusCode> status_code) { m_status_code = status_code; }

    auto Finalize() -> ResponseMessage& override {
      auto& this_json = **this;

      if (IsValidResponse()) {
        nlohmann::json tmp = this_json;
        this_json.clear();
        this_json["result"] = std::move(tmp);
      } else {
        nlohmann::json tmp = this_json;
        this_json.clear();
        this_json["error"] = std::move(tmp);
        this_json["error"]["code"] = static_cast<int>(m_status_code.value());
      }

      this_json["jsonrpc"] = "2.0";

      if (std::holds_alternative<int64_t>(m_request_id)) {
        this_json["id"] = std::get<int64_t>(m_request_id);
      } else {
        this_json["id"] = std::get<std::string>(m_request_id);
      }

      return *this;
    }
  };
}  // namespace no3::lsp::message
