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
#include <nitrate-core/Assert.hh>
#include <nitrate-core/Logger.hh>

#include "lsp/protocol/Request.hh"
#include "lsp/protocol/Response.hh"

using namespace ncc;
using namespace no3::lsp;

static auto VerifyTextDocumentCompletion(const nlohmann::json& j) -> bool {
  if (!j.is_object()) {
    return false;
  }

  if (!j.contains("textDocument") || !j["textDocument"].is_object()) {
    return false;
  }

  if (!j["textDocument"].contains("uri") || !j["textDocument"]["uri"].is_string()) {
    return false;
  }

  if (!j.contains("position") || !j["position"].is_object()) {
    return false;
  }
  if (!j["position"].contains("line") || !j["position"]["line"].is_number_unsigned()) {
    return false;
  }
  if (!j["position"].contains("character") || !j["position"]["character"].is_number_unsigned()) {
    return false;
  }

  return true;
}

void core::Context::RequestCompletion(const message::RequestMessage& request, message::ResponseMessage&) {
  const auto& j = *request;
  if (!VerifyTextDocumentCompletion(j)) {
    Log << "Invalid textDocument/completion request";
    return;
  }

  const auto file_uri = FlyString(j["textDocument"]["uri"].get<std::string>());
  const auto line = j["position"]["line"].get<uint64_t>();
  const auto character = j["position"]["character"].get<uint64_t>();

  Log << Trace << "RequestCompletion: file: " << file_uri << ", line: " << line << ", character: " << character;

  auto file = m_fs.GetFile(file_uri).value_or(nullptr);
  if (!file) {
    Log << "File not opened: " << file_uri;
    return;
  }

  auto offset = file->GetOffset(line, character);
  if (!offset) {
    Log << "Invalid position: " << line << ":" << character;
    return;
  }

  // auto rd = file->ReadAll();
  // rd->seekg(*offset);

  // auto snippet = std::basic_string((std::istreambuf_iterator(*rd)), std::istreambuf_iterator<uint8_t>());

  // (Log << Trace << "RequestCompletion: line_str: ")
  //     ->write(reinterpret_cast<const char*>(snippet.data()), snippet.size());

  /// TODO: Implement completion logic here
}
