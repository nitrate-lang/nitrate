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

#include <lsp/server/Context.hh>
#include <nitrate-core/Logger.hh>

using namespace ncc;
using namespace no3::lsp;

static auto VerifyInitializeRequest(const nlohmann::json& j) -> bool {
  if (j.contains("trace")) {
    if (!j["trace"].is_string()) {
      return false;
    }
  }

  return true;
}

void core::Context::RequestInitialize(const message::RequestMessage& request, message::ResponseMessage& response) {
  const auto& req = *request;
  if (!VerifyInitializeRequest(req)) [[unlikely]] {
    Log << "Invalid initialize request";
    response.SetStatusCode(message::StatusCode::InvalidRequest);
    return;
  }

  if (req.contains("trace")) {
    if (req["trace"] == "messages") {
      m_trace = TraceValue::Messages;
    } else if (req["trace"] == "verbose") {
      m_trace = TraceValue::Verbose;
    } else {
      m_trace = TraceValue::Off;
    }
  }

  ////==========================================================================
  auto& j = *response;

  j["serverInfo"]["name"] = "nitrateLanguageServer";
  j["serverInfo"]["version"] = "0.0.1";

  j["capabilities"]["positionEncoding"] = "utf-16";
  j["capabilities"]["textDocumentSync"] = {
      {"openClose", true},
      {"change", protocol::TextDocumentSyncKind::Incremental},
      {"save", {{"includeText", true}}},
  };
  j["capabilities"]["completionProvider"] = {
      {"triggerCharacters", {".", "::"}},
  };

  ////==========================================================================
  Log << Debug << "Context::RequestInitialize(): LSP initialize requested";
  m_is_lsp_initialized = true;
}
