
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

#include <core/package/Manifest.hh>
#include <filesystem>
#include <string>

namespace no3::package {
  struct InitOptions {
    std::string m_package_name;
    std::string m_package_description;
    std::string m_package_license;
    Manifest::Version m_package_version;
    Manifest::Category m_package_category;
  };

  auto CreatePackage(const std::filesystem::path& package_path, const InitOptions& options) -> bool;

  auto GenerateReadme(const InitOptions& options) -> std::string;
  auto GenerateLicense(const std::string& spdx_license) -> std::string;
  auto GenerateSecurityPolicy(const std::string& package_name) -> std::string;
  auto GenerateContributingPolicy(const InitOptions& options) -> std::string;
  auto GenerateCodeOfConduct() -> std::string;
  auto GenerateGitKeep() -> std::string;
  auto GenerateGitIgnore() -> std::string;
  auto GenerateDockerIgnore() -> std::string;
  auto GenerateDefaultLibrarySource() -> std::string;
  auto GenerateDefaultMainSource() -> std::string;
  auto GenerateCMakeListsTxt(const std::string& package_name) -> std::string;
}  // namespace no3::package
