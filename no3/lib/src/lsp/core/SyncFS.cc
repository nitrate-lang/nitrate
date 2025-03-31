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

#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <lsp/core/Server.hh>
#include <lsp/core/SyncFS.hh>
#include <nitrate-core/Logger.hh>

using namespace ncc;

SyncFS::SyncFS() { Log << Info << "Creating mirrored file system abstraction"; }

SyncFS::~SyncFS() { Log << Info << "Destroying mirrored file system abstraction"; }

auto SyncFS::The() -> SyncFS& {
  static SyncFS instance;
  return instance;
}

///===========================================================================

static auto UrlDecode(std::string_view str) -> std::string {
  std::string result;
  result.reserve(str.size());

  for (size_t i = 0; i < str.size(); i++) {
    if (str[i] == '%' && i + 2 < str.size()) {
      char c = 0;
      for (size_t j = 1; j <= 2; j++) {
        c <<= 4;
        if (str[i + j] >= '0' && str[i + j] <= '9') {
          c |= str[i + j] - '0';
        } else if (str[i + j] >= 'A' && str[i + j] <= 'F') {
          c |= str[i + j] - 'A' + 10;
        } else if (str[i + j] >= 'a' && str[i + j] <= 'f') {
          c |= str[i + j] - 'a' + 10;
        } else {
          return {};
        }
      }
      result.push_back(c);
      i += 2;
    } else {
      result.push_back(str[i]);
    }
  }

  return result;
}

auto SyncFS::Open(std::string path) -> std::optional<std::shared_ptr<SyncFSFile>> {
  path = UrlDecode(path);
  if (path.starts_with("file://")) {
    path = path.substr(7);
  }

  std::lock_guard<std::mutex> lock(m_mutex);

  auto it = m_files.find(path);
  if (it != m_files.end()) [[likely]] {
    return it->second;
  }

  if (!std::filesystem::exists(path)) {
    Log << "File not found: " << path;
    return std::nullopt;
  }

  Log << Info << "Reading file...: " << path;

  std::ifstream file(path);
  if (!file.is_open()) {
    Log << "Failed to open file: " << path;
    return std::nullopt;
  }

  std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

  auto ptr = std::make_shared<SyncFSFile>();
  ptr->SetContent(std::make_shared<std::string>(std::move(content)));

  m_files[path] = ptr;

  return ptr;
}

void SyncFS::Close(const std::string& name) {
  std::lock_guard<std::mutex> lock(m_mutex);
  m_files.erase(name);
}
