
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

#include <filesystem>
#include <string>

namespace no3::package {
  enum class PackageCategory { Library, StandardLibrary, Executable };

  struct InitOptions {
    std::string m_package_name;
    std::string m_package_description;
    std::string m_package_license;
    std::string m_package_version;
    PackageCategory m_package_category;
  };

  bool InitPackageUsingDefaults(const std::filesystem::path& package_path, const InitOptions& options);

  std::string GenerateReadme(const InitOptions& options);
  std::string GenerateLicense(const std::string& spdx_license);
  std::string GenerateSecurityPolicy(const std::string& package_name);
  std::string GenerateContributingPolicy(const InitOptions& options);
  std::string GenerateCodeOfConduct();
  std::string GenerateGitKeep();
  std::string GenerateGitIgnore();
  std::string GenerateDockerIgnore();
  std::string GenerateDefaultLibrarySource();
  std::string GenerateDefaultMainSource();
  std::string GenerateCMakeListsTxt();
}  // namespace no3::package
