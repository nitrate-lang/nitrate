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

#include <lsp/core/protocol/Message.hh>
#include <lsp/core/protocol/StatusCode.hh>

namespace no3::lsp::message {
  using MessageSequenceID = std::variant<int64_t, std::string>;

  class ResponseMessage final : public Message {
    friend class RequestMessage;

    MessageSequenceID m_request_id;
    std::optional<StatusCode> m_status_code;

    ResponseMessage(MessageSequenceID request_id)
        : Message(MessageKind::Response), m_request_id(std::move(request_id)) {}

  protected:
    void FinalizeImpl() override {
      /// TODO:
    }

  public:
    ~ResponseMessage() override = default;

    [[nodiscard]] auto GetResponseID() const -> const MessageSequenceID& { return m_request_id; }
    [[nodiscard]] auto GetStatusCode() const -> std::optional<StatusCode> { return m_status_code; }
    [[nodiscard]] auto GetResult() const -> const nlohmann::json& { return *this; }
    [[nodiscard]] auto GetError() const -> const nlohmann::json& { return *this; }
    [[nodiscard]] auto IsValidResponse() const -> bool { return !m_status_code.has_value(); }
    [[nodiscard]] auto IsErrorResponse() const -> bool { return m_status_code.has_value(); }

    void SetStatusCode(std::optional<StatusCode> status_code) { m_status_code = status_code; }
  };
}  // namespace no3::lsp::message
