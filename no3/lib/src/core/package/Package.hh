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

#include <core/package/LazyResource.hh>
#include <core/package/Manifest.hh>
#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>

namespace no3::package {
  class Package {
    friend class PackageBuilder;

    Manifest m_manifest;
    std::unordered_map<std::string, LazyResource> m_resources;

    Package(Manifest manifest, std::unordered_map<std::string, LazyResource> resources)
        : m_manifest(std::move(manifest)), m_resources(std::move(resources)) {}

  public:
    Package(const Package&) = delete;
    Package(Package&&) = default;
    Package& operator=(const Package&) = delete;
    Package& operator=(Package&&) = default;
    ~Package() = default;

    [[nodiscard]] auto GetManifest() const -> const Manifest& { return m_manifest; }
    [[nodiscard]] auto GetManifest() -> Manifest& { return m_manifest; }
    [[nodiscard]] auto operator->() const -> const Manifest* { return &m_manifest; }
    [[nodiscard]] auto operator->() -> Manifest* { return &m_manifest; }

    [[nodiscard]] auto GetResource(const std::string& name) const -> std::optional<LazyResource>;
    [[nodiscard]] auto HasResource(const std::string& name) const -> bool { return m_resources.contains(name); }
    [[nodiscard]] auto AddResource(const std::string& name, LazyResource resource) -> bool;
    [[nodiscard]] auto EraseResource(const std::string& name) -> bool;
    [[nodiscard]] auto ClearResources() -> size_t;
    [[nodiscard]] auto GetResourceNames() const -> std::vector<std::string>;
    [[nodiscard]] auto GetResourceCount() const -> size_t { return m_resources.size(); }

    ///=============================================================================================
    ///==                                  VALIDATION FUNCTIONS                                   ==
    enum class PackageStatus {
      ManifestFormatError,  // Example: Invalid SPDX license identifier
      SemanticError,        // Example: Severe syntax or semantic errors in known dot file formats.
      SecurityError,        // Example: Invalid digital signature of the package, or a dependency thereof.
      Valid,
    };

    [[nodiscard]] auto Validate(std::vector<PackageStatus>& issues) const -> bool;
    [[nodiscard]] auto Validate() const -> bool;

    ///=============================================================================================
    ///==                                    STORE FUNCTIONS                                      ==
    enum class GenericStoreKind {
      Folder,
      TextFile,
      BinaryFile,
    };

    /**
     * @brief An interface to facilitate the arbitrary storage of a Package.
     * @param kind Type of filesystem thing being written.
     * @param relative_path The path to the file or folder relative to the project root.
     * @param writer On success, this should be set to a valid stream for writing, or nullptr if `kind` is Folder.
     * @return If `kind` is Folder, return true if the folder was created successfully.
     *         If `kind` is TextFile or BinaryFile, return true if the stream was created successfully.
     * @note The `writer` stream will not be flushed or closed by the Package class.
     *       It is the responsibility of the caller to manage the stream.
     * @note No references to any shared streams will be retained after the Store() call returns.
     */
    using GenericStoreOperation = std::function<bool(GenericStoreKind kind, const std::filesystem::path& relative_path,
                                                     std::shared_ptr<std::ostream>& writer)>;

    /**
     * @brief Store the package.
     * @param dest Implementation of the GenericStoreOperation interface.
     * @param write_content Should the content of emission files will be written to their stream.
     * @return true if no failures occurred during the store operation. Otherwise, false.
     *         These failures may include the GenericStoreOperation returning false, the stream write
     *         failing, or GenericStoreOperation(File|BinaryFile) returning true, but the output
     *         parameter being nullptr.
     * @note In the event of any failure, the Store() operation may cancel prematurely, leaving the
     *       partially written package in an unspecified state.
     */
    [[nodiscard]] auto Store(const GenericStoreOperation& dest, bool write_content = true) -> bool;
    [[nodiscard]] auto LocalStore(const std::filesystem::path& base_path) const -> bool;

    ///=============================================================================================
    ///==                                    DEBUG FUNCTIONS                                      ==
    [[nodiscard]] auto SummarizeTree(std::ostream& os) const -> bool;
  };
}  // namespace no3::package
