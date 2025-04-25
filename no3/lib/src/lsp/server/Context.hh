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

#include <atomic>
#include <lsp/protocol/Message.hh>
#include <lsp/protocol/Notification.hh>
#include <lsp/protocol/Request.hh>
#include <lsp/protocol/Response.hh>
#include <lsp/resource/FileBrowser.hh>
#include <nitrate-core/Logger.hh>

namespace no3::lsp::core {
  class Context final {
    enum class TraceValue {
      Off,
      Messages,
      Verbose,
    };

    std::ostream& m_os;
    std::mutex& m_os_lock;

    FileBrowser m_fs;
    std::atomic<bool> m_is_lsp_initialized, m_can_send_trace, m_exit_requested;
    std::atomic<TraceValue> m_trace = TraceValue::Messages;
    ncc::LogSubscriberID m_log_subscriber_id;

    [[nodiscard]] auto ExecuteLSPRequest(const message::RequestMessage& message) -> message::ResponseMessage;
    void ExecuteLSPNotification(const message::NotifyMessage& message);

    ///========================================================================================================

#define LSP_REQUEST(name) void Request##name(const message::RequestMessage&, message::ResponseMessage&)
#define LSP_NOTIFY(name) void Notify##name(const message::NotifyMessage&)

    LSP_REQUEST(Initialize);
    LSP_REQUEST(Shutdown);
    LSP_REQUEST(Completion);

    LSP_NOTIFY(Initialized);
    LSP_NOTIFY(SetTrace);
    LSP_NOTIFY(Exit);
    LSP_NOTIFY(TextDocumentDidChange);
    LSP_NOTIFY(TextDocumentDidClose);
    LSP_NOTIFY(TextDocumentDidOpen);
    LSP_NOTIFY(TextDocumentDidSave);

#undef REQUEST_HANDLER
#undef NOTIFICATION_HANDLER

    using LSPRequestFunc = void (Context::*)(const message::RequestMessage&, message::ResponseMessage&);
    using LSPNotifyFunc = void (Context::*)(const message::NotifyMessage&);

    static inline const std::unordered_map<std::string_view, LSPRequestFunc> LSP_REQUEST_MAP = {
        {"initialize", &Context::RequestInitialize},
        {"shutdown", &Context::RequestShutdown},
        {"textDocument/completion", &Context::RequestCompletion},
    };

    static inline const std::unordered_map<std::string_view, LSPNotifyFunc> LSP_NOTIFICATION_MAP = {
        {"initialized", &Context::NotifyInitialized},
        {"$/setTrace", &Context::NotifySetTrace},
        {"exit", &Context::NotifyExit},

        {"textDocument/didOpen", &Context::NotifyTextDocumentDidOpen},
        {"textDocument/didChange", &Context::NotifyTextDocumentDidChange},
        {"textDocument/didClose", &Context::NotifyTextDocumentDidClose},
        {"textDocument/didSave", &Context::NotifyTextDocumentDidSave},
    };

    ///========================================================================================================

  public:
    Context(std::ostream& os, std::mutex& os_lock);
    Context(const Context&) = delete;
    Context(Context&&) = delete;
    ~Context();

    void ExecuteRPC(const message::Message& message, bool& exit_requested);
    void SendMessage(message::Message& message, bool log_transmission = true);
  };
}  // namespace no3::lsp::core
