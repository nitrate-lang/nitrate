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

#include <lsp/core/Server.hh>
#include <lsp/core/protocol/Notification.hh>
#include <lsp/core/protocol/Request.hh>
#include <lsp/core/protocol/Response.hh>

namespace no3::lsp::rpc {
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

  using namespace message;

  using LSPRequestFunc = std::function<void(const message::RequestMessage&, message::ResponseMessage&)>;
  using LSPNotifyFunc = std::function<void(const message::NotifyMessage&)>;

  static inline const std::unordered_map<std::string_view, LSPRequestFunc> LSP_REQUEST_MAP = {
      {"initialize", rpc::RequestInitialize},
      {"shutdown", rpc::RequestShutdown},
  };

  static inline const std::unordered_map<std::string_view, LSPNotifyFunc> LSP_NOTIFICATION_MAP = {
      {"initialized", rpc::NotifyInitialized},
      {"exit", rpc::NotifyExit},
      {"textDocument/didOpen", rpc::NotifyTextDocumentDidOpen},
      {"textDocument/didChange", rpc::NotifyTextDocumentDidChange},
      {"textDocument/didClose", rpc::NotifyTextDocumentDidClose},
      {"textDocument/didSave", rpc::NotifyTextDocumentDidSave},
  };
}  // namespace no3::lsp::rpc
