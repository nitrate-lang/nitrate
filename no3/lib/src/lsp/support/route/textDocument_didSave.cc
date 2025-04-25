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

#ifndef NDEBUG
#include <fstream>
#endif

using namespace ncc;
using namespace no3::lsp;

static auto VerifyTextDocumentDidSave(const nlohmann::json& j) -> bool {
  if (!j.is_object()) {
    return false;
  }

  if (!j.contains("textDocument") || !j["textDocument"].is_object()) {
    return false;
  }

  if (!j["textDocument"].contains("uri") || !j["textDocument"]["uri"].is_string()) {
    return false;
  }

  if (!j.contains("text") || !j["text"].is_string()) {
    return false;
  }

  return true;
}

void core::Context::NotifyTextDocumentDidSave(const message::NotifyMessage& notice) {
  const auto& j = *notice;
  if (!VerifyTextDocumentDidSave(j)) {
    Log << "Invalid textDocument/didSave notification";
    return;
  }

  auto file_uri = FlyString(j["textDocument"]["uri"].get<std::string>());
  const auto& full_content = j["text"].get<std::string>();

  if (!m_fs.DidSave(file_uri, FlyByteString(full_content.begin(), full_content.end()))) {
    Log << "Failed to save text document: " << file_uri;
    return;
  }

  Log << Debug << "Saved text document: " << file_uri;

#ifndef NDEBUG
  {
    auto raw_content = *m_fs.GetFile(file_uri).value()->ReadAll();

    auto debug_output = std::fstream("/tmp/nitrate_lsp_debug.txt", std::ios::out | std::ios::trunc | std::ios::binary);
    if (!debug_output) {
      qcore_panic("Failed to open debug output file");
    }

    debug_output.write(reinterpret_cast<const char*>(raw_content.c_str()), raw_content.size());
  }
#endif
}
