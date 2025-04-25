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

static auto VerifyTextDocumentDidOpen(const nlohmann::json& j) -> bool {
  if (!j.is_object()) {
    return false;
  }

  if (!j.contains("textDocument") || !j["textDocument"].is_object()) {
    return false;
  }

  if (!j["textDocument"].contains("uri") || !j["textDocument"]["uri"].is_string()) {
    return false;
  }

  if (!j["textDocument"].contains("version") || !j["textDocument"]["version"].is_number_integer()) {
    return false;
  }

  if (!j["textDocument"].contains("text") || !j["textDocument"]["text"].is_string()) {
    return false;
  }

  return true;
}

void core::Context::NotifyTextDocumentDidOpen(const message::NotifyMessage& notice) {
  const auto& j = *notice;
  if (!VerifyTextDocumentDidOpen(j)) {
    Log << "Invalid textDocument/didOpen notification";
    return;
  }

  const auto& uri = j["textDocument"]["uri"].get<std::string>();
  const auto& version = j["textDocument"]["version"].get<int64_t>();
  const auto& text = j["textDocument"]["text"].get<std::string>();

  if (!m_fs.DidOpen(FlyString(uri), version, FlyByteString(text.begin(), text.end()))) {
    Log << "Failed to open text document: " << uri;
    return;
  }

  Log << Debug << "Opened text document: " << uri;
}
