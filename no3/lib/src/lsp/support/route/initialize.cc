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

#include <lsp/core/LSPContext.hh>
#include <nitrate-core/Logger.hh>

using namespace ncc;
using namespace no3::lsp;

static auto VerifyInitializeRequest(const nlohmann::json& j) -> bool {
  /// TODO: Verify the request
  return true;
}

void core::LSPContext::RequestInitialize(const message::RequestMessage& request, message::ResponseMessage& response) {
  const auto& req = *request;
  if (!VerifyInitializeRequest(req)) [[unlikely]] {
    Log << "Invalid initialize request";
    response.SetStatusCode(message::StatusCode::InvalidRequest);
    return;
  }

  ////==========================================================================
  auto& j = *response;

  j["serverInfo"]["name"] = "nitrateLanguageServer";
  j["serverInfo"]["version"] = "0.0.1";

  j["capabilities"]["positionEncoding"] = "utf-16";
  j["capabilities"]["textDocumentSync"] = protocol::TextDocumentSyncKind::Incremental;
  j["capabilities"]["documentFormattingProvider"] = true;

  ////==========================================================================
  m_is_lsp_initialized = true;
  Log << Debug << "LSPContext::RequestInitialize(): LSP initialize requested";
}
