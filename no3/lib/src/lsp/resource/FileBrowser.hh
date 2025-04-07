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

#include <lsp/protocol/TextDocument.hh>
#include <lsp/resource/File.hh>
#include <memory>
#include <span>

namespace no3::lsp::core {
  class FileBrowser final {
    class PImpl;
    std::unique_ptr<PImpl> m_impl;

  public:
    FileBrowser(protocol::TextDocumentSyncKind sync);
    FileBrowser(const FileBrowser&) = delete;
    FileBrowser(FileBrowser&&) = default;
    FileBrowser& operator=(const FileBrowser&) = delete;
    FileBrowser& operator=(FileBrowser&&) = default;
    ~FileBrowser();

    using IncrementalChanges = std::span<const protocol::TextDocumentContentChangeEvent>;
    using ReadOnlyFile = std::shared_ptr<ConstFile>;

    [[nodiscard]] auto DidOpen(const FlyString& file_uri, FileVersion version, FlyByteString raw) -> bool;
    [[nodiscard]] auto DidChange(const FlyString& file_uri, FileVersion version, FlyByteString raw) -> bool;
    [[nodiscard]] auto DidChanges(const FlyString& file_uri, FileVersion version, IncrementalChanges changes) -> bool;
    [[nodiscard]] auto DidSave(const FlyString& file_uri,
                               std::optional<FlyByteString> full_content = std::nullopt) -> bool;
    [[nodiscard]] auto DidClose(const FlyString& file_uri) -> bool;
    [[nodiscard]] auto GetFile(const FlyString& file_uri) const -> std::optional<ReadOnlyFile>;
  };
}  // namespace no3::lsp::core
