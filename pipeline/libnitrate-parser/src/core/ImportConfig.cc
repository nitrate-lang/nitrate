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

using namespace ncc::parse;

class ImportConfig::PImpl {
public:
  ImportName m_package_name;
  std::vector<std::filesystem::path> m_package_search_path;
  std::vector<std::string_view> m_package_name_chain;
  mutable std::optional<std::unordered_set<Package>> m_packages;

  PImpl(ImportName package_name, std::vector<std::filesystem::path> package_search_path)
      : m_package_name(std::move(package_name)), m_package_search_path(std::move(package_search_path)) {
    if (m_package_name.IsValid()) {
      m_package_name_chain = m_package_name.GetChain();
    }
  }
};

ImportConfig::ImportConfig(const ImportName &package_name,
                           const std::vector<std::filesystem::path> &package_search_path)
    : m_impl(std::make_unique<PImpl>(package_name, package_search_path)) {}

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

auto ImportConfig::GetThisPackageNameChain() const -> const std::vector<std::string_view> & {
  qcore_assert(m_impl != nullptr);
  return m_impl->m_package_name_chain;
}

auto ImportConfig::GetSearchPaths() const -> const std::vector<std::filesystem::path> & {
  qcore_assert(m_impl != nullptr);
  return m_impl->m_package_search_path;
}

auto ImportConfig::GetThisPackageName() const -> const ImportName & {
  qcore_assert(m_impl != nullptr);
  return m_impl->m_package_name;
}

auto ImportConfig::GetPackages() const -> const std::unordered_set<Package> & {
  qcore_assert(m_impl != nullptr);
  if (!m_impl->m_packages.has_value()) {
    m_impl->m_packages = FindPackages(m_impl->m_package_search_path);
  }

  return m_impl->m_packages.value();
}

static std::optional<std::vector<std::filesystem::path>> GetSearchPathFromEnv() {
  const char *env = std::getenv("NCC_PACKAGE_PATH");
  if (env == nullptr) {
    return std::nullopt;
  }

  std::vector<std::filesystem::path> paths;
  std::string_view env_str(env);
  std::string_view::size_type start = 0;
  std::string_view::size_type end = env_str.find_first_of(':');
  while (end != std::string_view::npos) {
    paths.emplace_back(env_str.substr(start, end - start));
    start = end + 1;
    end = env_str.find_first_of(':', start);
  }
  paths.emplace_back(env_str.substr(start));

  return paths;
}

auto ImportConfig::GetDefault() -> ImportConfig {
  ImportName name{};  // empty name
  auto paths = GetSearchPathFromEnv();
  ImportConfig config(name, paths.value_or(std::vector<std::filesystem::path>{}));
  return config;
}
