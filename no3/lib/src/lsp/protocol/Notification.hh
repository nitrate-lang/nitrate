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

namespace no3::lsp::message {
  class NotifyMessage : public Message {
    std::string m_method;
    nlohmann::json m_params;

  public:
    NotifyMessage(std::string method, nlohmann::json params)
        : Message(MessageKind::Notification, std::move(params)),
          m_method(std::move(method)),
          m_params(std::move(params)) {}
    NotifyMessage(const NotifyMessage&) = delete;
    NotifyMessage(NotifyMessage&&) = default;
    ~NotifyMessage() override = default;

    [[nodiscard]] auto GetParams() const -> const nlohmann::json& { return m_params; }
    [[nodiscard]] auto GetMethod() const -> std::string_view override { return m_method; }

    auto Finalize() -> NotifyMessage& override {
      auto& this_json = **this;

      {
        nlohmann::json tmp = this_json;
        this_json.clear();
        this_json["params"] = std::move(tmp);
      }

      this_json["method"] = m_method;
      this_json["jsonrpc"] = "2.0";

      return *this;
    }
  };

}  // namespace no3::lsp::message
