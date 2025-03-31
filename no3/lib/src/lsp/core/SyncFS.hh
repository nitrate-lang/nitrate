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

#include <cstddef>
#include <memory>
#include <nitrate-core/Logger.hh>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>

using namespace ncc;

class SyncFSFile {
  std::shared_ptr<std::string> m_content;
  std::mutex m_mutex;

public:
  SyncFSFile() = default;
  using Digest = std::array<uint8_t, 20>;

  auto Replace(size_t offset, int64_t length, std::string_view text) -> bool {
    std::lock_guard<std::mutex> lock(m_mutex);

    if (length < 0) {  // negative length starts from the end
      length = m_content->size() - offset + length + 1;
    }

    if (offset + length > m_content->size()) {
      Log << "Invalid replace operation: offset=" << offset << ", length=" << length << ", size=" << m_content->size();
      return false;
    }

    m_content->replace(offset, length, text);

    return true;
  }

  auto Content() -> std::shared_ptr<const std::string> {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_content;
  }

  auto Size() -> size_t {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_content->size();
  };

  void SetContent(std::shared_ptr<std::string> content) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_content = std::move(content);
  }
};

class SyncFS {
  struct Impl;
  std::unordered_map<std::string, std::shared_ptr<SyncFSFile>> m_files;
  std::mutex m_mutex;

  SyncFS();
  ~SyncFS();

public:
  SyncFS(const SyncFS&) = delete;
  auto operator=(const SyncFS&) -> SyncFS& = delete;
  SyncFS(SyncFS&&) = delete;

  static auto The() -> SyncFS&;

  auto Open(std::string path) -> std::optional<std::shared_ptr<SyncFSFile>>;

  void Close(const std::string& name);
};
