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
#include <lsp/core/protocol/Response.hh>
#include <lsp/core/protocol/StatusCode.hh>

namespace no3::lsp::message {
  class RequestMessage final : public Message {
    std::string m_method;
    MessageSequenceID m_request_id;

  protected:
    void FinalizeImpl() override {
      /// TODO:
    }

  public:
    RequestMessage(std::string method, MessageSequenceID request_id, nlohmann::json params)
        : Message(MessageKind::Request, std::move(params)),
          m_method(std::move(method)),
          m_request_id(std::move(request_id)) {}
    ~RequestMessage() override = default;

    [[nodiscard]] auto GetRequestID() const -> const MessageSequenceID& { return m_request_id; }
    [[nodiscard]] auto GetParams() const -> const nlohmann::json& { return *this; }
    [[nodiscard]] auto GetMethod() const -> std::string_view override { return m_method; }

    [[nodiscard]] auto GetResponseObject() const -> ResponseMessage { return {m_request_id}; }
  };
}  // namespace no3::lsp::message
