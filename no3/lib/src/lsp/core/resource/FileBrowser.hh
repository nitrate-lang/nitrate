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

#include <lsp/core/protocol/TextDocument.hh>
#include <lsp/core/resource/File.hh>
#include <memory>
#include <span>

namespace no3::lsp::core {
  class FileBrowser final {
    friend class FileBrowserSyncronizer;

    class PImpl;
    std::unique_ptr<PImpl> m_impl;

    FileBrowser();
    ~FileBrowser();

  public:
    static std::optional<std::unique_ptr<FileBrowser>> Create(protocol::TextDocumentSyncKind sync);

    using IncrementalChanges = std::span<const protocol::TextDocumentContentChangeEvent>;

    [[nodiscard]] auto DidOpen(FlyPath file_uri, FileRevision revision, FlyString raw) -> bool;
    [[nodiscard]] auto DidChange(FlyPath file_uri, FileRevision new_revision, FlyString raw) -> bool;
    [[nodiscard]] auto DidChange(FlyPath file_uri, FileRevision new_revision, IncrementalChanges changes) -> bool;
    [[nodiscard]] auto DidSave(FlyPath file_uri) -> bool;
    [[nodiscard]] auto DidClose(FlyPath file_uri) -> bool;

    [[nodiscard]] auto GetFile(FlyPath file_uri, std::optional<FileRevision> revision = std::nullopt) const
        -> std::optional<std::shared_ptr<ConstFile>>;
  };
}  // namespace no3::lsp::core
