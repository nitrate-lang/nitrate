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

#include <nitrate-core/Logger.hh>
#include <nitrate-parser/ImportConfig.hh>
#include <unordered_set>

using namespace ncc::parse;

class ImportConfig::PImpl {
public:
  ImportName m_import_name;
  std::unordered_set<std::filesystem::path> m_package_search_path;
  std::unordered_set<std::filesystem::path> m_files_to_not_import;
  std::vector<std::string_view> m_import_name_chain;
  mutable std::optional<std::unordered_set<Package>> m_packages;

  PImpl(ImportName import_name, std::unordered_set<std::filesystem::path> package_search_path,
        std::unordered_set<std::filesystem::path> files_to_not_import)
      : m_import_name(std::move(import_name)),
        m_package_search_path(std::move(package_search_path)),
        m_files_to_not_import(std::move(files_to_not_import)) {
    m_import_name_chain = m_import_name.GetChain();
  }
};

ImportConfig::ImportConfig(const ImportName &import_name,
                           const std::unordered_set<std::filesystem::path> &package_search_path,
                           const std::unordered_set<std::filesystem::path> &files_to_not_import)
    : m_impl(std::make_unique<PImpl>(import_name, package_search_path, files_to_not_import)) {}

ImportConfig::ImportConfig(const ImportConfig &other) : m_impl(std::make_unique<PImpl>(*other.m_impl)) {}

ImportConfig::ImportConfig(ImportConfig &&other) noexcept : m_impl(std::move(other.m_impl)) {}

ImportConfig::~ImportConfig() = default;

auto ImportConfig::operator=(const ImportConfig &other) -> ImportConfig & {
  if (this != &other) {
    m_impl = std::make_unique<PImpl>(*other.m_impl);
  }
  return *this;
}

auto ImportConfig::operator=(ImportConfig &&other) noexcept -> ImportConfig & {
  if (this != &other) {
    m_impl = std::move(other.m_impl);
  }
  return *this;
}

auto ImportConfig::GetThisImportNameChain() const -> const std::vector<std::string_view> & {
  qcore_assert(m_impl != nullptr);
  return m_impl->m_import_name_chain;
}

auto ImportConfig::GetSearchPaths() const -> const std::unordered_set<std::filesystem::path> & {
  qcore_assert(m_impl != nullptr);
  return m_impl->m_package_search_path;
}

auto ImportConfig::GetThisImportName() const -> const ImportName & {
  qcore_assert(m_impl != nullptr);
  return m_impl->m_import_name;
}

auto ImportConfig::GetPackages() const -> const std::unordered_set<Package> & {
  qcore_assert(m_impl != nullptr);
  if (!m_impl->m_packages.has_value()) {
    m_impl->m_packages = FindPackages(m_impl->m_package_search_path);
  }

  return m_impl->m_packages.value();
}

auto ImportConfig::GetFilesToNotImport() const -> const std::unordered_set<std::filesystem::path> & {
  qcore_assert(m_impl != nullptr);
  return m_impl->m_files_to_not_import;
}

///=============================================================================

auto ImportConfig::SetThisImportName(const ImportName &this_import_name) -> void {
  qcore_assert(m_impl != nullptr);
  m_impl->m_import_name = this_import_name;
}

auto ImportConfig::SetSearchPaths(const std::unordered_set<std::filesystem::path> &package_search_paths) -> void {
  qcore_assert(m_impl != nullptr);
  m_impl->m_package_search_path = package_search_paths;
}

auto ImportConfig::AddSearchPath(const std::filesystem::path &package_search_path) -> void {
  qcore_assert(m_impl != nullptr);
  m_impl->m_package_search_path.insert(package_search_path);
}

auto ImportConfig::ClearSearchPaths() -> void {
  qcore_assert(m_impl != nullptr);
  m_impl->m_package_search_path.clear();
}

auto ImportConfig::AddFileToNotImport(const std::filesystem::path &file) -> void {
  qcore_assert(m_impl != nullptr);
  m_impl->m_files_to_not_import.insert(file);
}

auto ImportConfig::SetFilesToNotImport(const std::unordered_set<std::filesystem::path> &files_to_not_import) -> void {
  qcore_assert(m_impl != nullptr);
  m_impl->m_files_to_not_import = files_to_not_import;
}

auto ImportConfig::ClearFilesToNotImport() -> void {
  qcore_assert(m_impl != nullptr);
  m_impl->m_files_to_not_import.clear();
}

///=============================================================================

static std::optional<std::unordered_set<std::filesystem::path>> GetSearchPathFromEnv(
    const std::shared_ptr<ncc::IEnvironment> &env) {
  auto env_str = env->Get("NCC_PACKAGE_PATH").value_or("");
  size_t start = 0;
  size_t end = env_str->find_first_of(':');

  std::unordered_set<std::filesystem::path> paths;
  while (end != std::string_view::npos) {
    paths.emplace(env_str->substr(start, end - start));
    start = end + 1;
    end = env_str->find_first_of(':', start);
  }
  paths.emplace(env_str->substr(start));

  return paths;
}

auto ImportConfig::GetDefault(const std::shared_ptr<IEnvironment> &env) -> ImportConfig {
  ImportName name("");
  auto paths = GetSearchPathFromEnv(env);
  ImportConfig config(name, paths.value_or(std::unordered_set<std::filesystem::path>{}));
  return config;
}
