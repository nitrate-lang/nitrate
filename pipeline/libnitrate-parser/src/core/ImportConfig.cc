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
#include <nitrate-parser/Context.hh>

#include "nitrate-parser/Package.hh"

using namespace ncc::parse;

class ImportConfig::PImpl {
public:
  ImportName m_package_name;
  std::vector<std::filesystem::path> m_package_search_path;
  std::vector<std::string_view> m_package_name_chain;

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

auto ImportConfig::GetPackageNameChain() const -> const std::vector<std::string_view> & {
  qcore_assert(m_impl != nullptr);
  return m_impl->m_package_name_chain;
}

auto ImportConfig::GetPackagePath() const -> const std::vector<std::filesystem::path> & {
  qcore_assert(m_impl != nullptr);
  return m_impl->m_package_search_path;
}

auto ImportConfig::GetPackageName() const -> const ImportName & {
  qcore_assert(m_impl != nullptr);
  return m_impl->m_package_name;
}

auto ImportConfig::GetDefault() -> ImportConfig {
  ImportName name;
  ImportConfig config(name, {});
  return config;
}
