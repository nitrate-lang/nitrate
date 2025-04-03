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
#include <nlohmann/json.hpp>

namespace no3::lsp::message {
  enum class MessageKind : uint8_t {
    Request,
    Response,
    Notification,
  };

  class Message {
    MessageKind m_kind;
    nlohmann::json m_json;

  public:
    Message(MessageKind kind) : m_kind(kind){};
    Message(MessageKind kind, nlohmann::json json) : m_kind(kind), m_json(std::move(json)) {}
    Message(const Message&) = delete;
    Message(Message&&) = default;
    virtual ~Message() = default;

    [[nodiscard]] auto GetKind() const -> MessageKind { return m_kind; }
    [[nodiscard]] auto IsRequest() const -> bool { return m_kind == MessageKind::Request; }
    [[nodiscard]] auto IsResponse() const -> bool { return m_kind == MessageKind::Response; }
    [[nodiscard]] auto IsNotification() const -> bool { return m_kind == MessageKind::Notification; }

    [[nodiscard]] virtual auto GetMethod() const -> std::string_view { return ""; }

    [[nodiscard]] auto operator->() const -> const nlohmann::json* { return &m_json; }
    [[nodiscard]] auto operator->() -> nlohmann::json* { return &m_json; }
    [[nodiscard]] auto operator*() const -> const nlohmann::json& { return m_json; }
    [[nodiscard]] auto operator*() -> nlohmann::json& { return m_json; }

    virtual auto Finalize() -> Message& = 0;
  };
}  // namespace no3::lsp::message
