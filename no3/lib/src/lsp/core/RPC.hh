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

#include <istream>
#include <lsp/core/ThreadPool.hh>
#include <lsp/core/protocol/Message.hh>
#include <lsp/core/protocol/Notification.hh>
#include <lsp/core/protocol/Request.hh>
#include <lsp/core/protocol/Response.hh>
#include <lsp/core/resource/FileBrowser.hh>

namespace no3::lsp::core {
  auto LSPReadRequest(std::istream& in, std::mutex& in_lock) -> std::optional<std::unique_ptr<message::Message>>;

  class LSPScheduler {
    FileBrowser m_fs;
    std::optional<ThreadPool> m_thread_pool;
    std::iostream& m_io;
    std::mutex& m_io_lock;
    std::atomic<bool> m_exit_requested = false;
    bool m_is_lsp_initialized = false;

    static auto IsConcurrentRequest(const message::Message& message) -> bool;

    void ExecuteRPC(const message::Message& message);
    void ExecuteLSPRequest(const message::RequestMessage& message);
    void ExecuteLSPNotification(const message::NotifyMessage& message);

    ///========================================================================================================

#define LSP_REQUEST(name) void Request##name(const message::RequestMessage&, message::ResponseMessage&)
#define LSP_NOTIFY(name) void Notify##name(const message::NotifyMessage&)

    LSP_REQUEST(Initialize);
    LSP_REQUEST(Shutdown);
    LSP_REQUEST(Formatting);

    LSP_NOTIFY(Initialized);
    LSP_NOTIFY(Exit);
    LSP_NOTIFY(SetTrace);
    LSP_NOTIFY(TextDocumentDidChange);
    LSP_NOTIFY(TextDocumentDidClose);
    LSP_NOTIFY(TextDocumentDidOpen);
    LSP_NOTIFY(TextDocumentDidSave);

#undef REQUEST_HANDLER
#undef NOTIFICATION_HANDLER

    using LSPRequestFunc = void (LSPScheduler::*)(const message::RequestMessage&, message::ResponseMessage&);
    using LSPNotifyFunc = void (LSPScheduler::*)(const message::NotifyMessage&);

    static inline const std::unordered_map<std::string_view, LSPRequestFunc> LSP_REQUEST_MAP = {
        {"initialize", &LSPScheduler::RequestInitialize},
        {"shutdown", &LSPScheduler::RequestShutdown},
    };

    static inline const std::unordered_map<std::string_view, LSPNotifyFunc> LSP_NOTIFICATION_MAP = {
        {"initialized", &LSPScheduler::NotifyInitialized},
        {"exit", &LSPScheduler::NotifyExit},
        {"textDocument/didOpen", &LSPScheduler::NotifyTextDocumentDidOpen},
        {"textDocument/didChange", &LSPScheduler::NotifyTextDocumentDidChange},
        {"textDocument/didClose", &LSPScheduler::NotifyTextDocumentDidClose},
        {"textDocument/didSave", &LSPScheduler::NotifyTextDocumentDidSave},
    };

    ///========================================================================================================

  public:
    LSPScheduler(std::iostream& io, std::mutex& io_lock);
    ~LSPScheduler() = default;

    [[nodiscard]] bool IsExitRequested() const { return m_exit_requested; }
    void Schedule(std::unique_ptr<message::Message> request);
  };
}  // namespace no3::lsp::core
