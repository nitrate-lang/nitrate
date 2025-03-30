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

#ifndef __NITRATE_AST_eIMPORT_CONFIG_H__
#define __NITRATE_AST_eIMPORT_CONFIG_H__

#include <nitrate-parser/Package.hh>
#include <unordered_set>

namespace ncc::parse {
  class NCC_EXPORT ImportConfig final {
    class PImpl;
    std::unique_ptr<PImpl> m_impl;

  public:
    static auto GetDefault() -> ImportConfig;

    ImportConfig(const ImportName &this_import_name,
                 const std::unordered_set<std::filesystem::path> &package_search_paths = {},
                 const std::unordered_set<std::filesystem::path> &files_to_not_import = {});
    ImportConfig(const ImportConfig &);
    ImportConfig(ImportConfig &&) noexcept;
    ~ImportConfig();
    auto operator=(const ImportConfig &) -> ImportConfig &;
    auto operator=(ImportConfig &&) noexcept -> ImportConfig &;

    [[nodiscard]] auto GetThisImportNameChain() const -> const std::vector<std::string_view> &;
    [[nodiscard]] auto GetThisImportName() const -> const ImportName &;
    [[nodiscard]] auto GetSearchPaths() const -> const std::unordered_set<std::filesystem::path> &;
    [[nodiscard]] auto GetPackages() const -> const std::unordered_set<Package> &;
    [[nodiscard]] auto GetFilesToNotImport() const -> const std::unordered_set<std::filesystem::path> &;

    auto SetThisImportName(const ImportName &this_import_name) -> void;
    auto SetSearchPaths(const std::unordered_set<std::filesystem::path> &package_search_paths) -> void;
    auto ClearSearchPaths() -> void;
    auto AddSearchPath(const std::filesystem::path &package_search_path) -> void;
    auto AddFileToNotImport(const std::filesystem::path &file) -> void;
    auto SetFilesToNotImport(const std::unordered_set<std::filesystem::path> &files_to_not_import) -> void;
    auto ClearFilesToNotImport() -> void;
  };
}  // namespace ncc::parse

#endif  // __NITRATE_AST_PARSER_H__
