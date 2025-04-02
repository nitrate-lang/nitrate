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

#include <lsp/core/resource/FileBrowser.hh>
#include <nitrate-core/Assert.hh>
#include <nitrate-core/Logger.hh>

using namespace ncc;
using namespace no3::lsp::core;

class FileBrowser::PImpl {
public:
  struct FileHistory {
    std::unordered_map<FileRevision, std::shared_ptr<ConstFile>> m_track;
    FileRevision m_latest;
  };

  std::mutex m_mutex;
  std::unordered_map<FlyString, FileHistory> m_files;
};

FileBrowser::FileBrowser() : m_impl(std::make_unique<PImpl>()) {}

FileBrowser::~FileBrowser() = default;

auto FileBrowser::Create(protocol::TextDocumentSyncKind sync) -> std::optional<std::unique_ptr<FileBrowser>> {
  /// TODO: Implement the logic to create a FileBrowser instance based on the sync kind.
  return std::nullopt;
}

auto FileBrowser::DidOpen(FlyString file_uri, FileRevision revision, FlyString raw) -> bool {
  qcore_assert(m_impl != nullptr);
  std::lock_guard lock(m_impl->m_mutex);

  /// TODO: Implement
  return false;
}

auto FileBrowser::DidChange(FlyString file_uri, FileRevision new_revision, FlyString raw) -> bool {
  qcore_assert(m_impl != nullptr);
  std::lock_guard lock(m_impl->m_mutex);

  /// TODO: Implement
  return false;
}

auto FileBrowser::DidChange(FlyString file_uri, FileRevision new_revision, IncrementalChanges changes) -> bool {
  qcore_assert(m_impl != nullptr);
  std::lock_guard lock(m_impl->m_mutex);

  /// TODO: Implement
  return false;
}

auto FileBrowser::DidSave(FlyString file_uri) -> bool {
  qcore_assert(m_impl != nullptr);
  std::lock_guard lock(m_impl->m_mutex);

  /// TODO: Implement
  return false;
}

auto FileBrowser::DidClose(FlyString file_uri) -> bool {
  qcore_assert(m_impl != nullptr);
  std::lock_guard lock(m_impl->m_mutex);

  /// TODO: Implement
  return false;
}

auto FileBrowser::GetFile(const FlyString& file_uri,
                          std::optional<FileRevision> revision) const -> std::optional<ReadOnlyFile> {
  qcore_assert(m_impl != nullptr);
  std::lock_guard lock(m_impl->m_mutex);

  Log << Trace << "FileBrowser::GetFile(" << file_uri << ", " << revision.value_or(0) << ")";

  auto it = m_impl->m_files.find(file_uri);
  if (it == m_impl->m_files.end()) [[unlikely]] {
    Log << Error << "FileBrowser::GetFile: File not found: " << file_uri;
    return std::nullopt;
  }

  Log << Trace << "FileBrowser::GetFile: Found file: " << file_uri;

  auto& file_history = it->second;
  if (revision.has_value()) {
    auto rev_it = file_history.m_track.find(revision.value());
    if (rev_it == file_history.m_track.end()) [[unlikely]] {
      Log << Error << "FileBrowser::GetFile: Revision not found: " << revision.value();
      return std::nullopt;
    }

    Log << Trace << "FileBrowser::GetFile: Found revision: " << revision.value();

    return rev_it->second;
  }

  Log << Trace << "FileBrowser::GetFile: No revision specified, using latest: " << file_history.m_latest;

  return file_history.m_track.at(file_history.m_latest);
}
