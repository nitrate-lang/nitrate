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

#include <core/PackageConfig.hh>
#include <fstream>
#include <init/InitPackage.hh>
#include <nitrate-core/CatchAll.hh>
#include <nitrate-core/Logger.hh>

using namespace ncc;
using namespace no3::package;

static bool CreateLocalFile(const std::filesystem::path& path, std::string_view init) {
  Log << Trace << "Creating a local file at: " << path;

  if (OMNI_CATCH(false, std::filesystem::exists(path))) {
    Log << Warning << "The file already exists: " << path;
    return false;
  }

  if (OMNI_CATCH(false, std::filesystem::create_directories(path.parent_path()))) {
    Log << "Failed to create the parent directory: " << path.parent_path();
    return false;
  }

  std::fstream file(path, std::ios::out | std::ios::trunc);
  if (!file.is_open()) {
    Log << "Failed to create the file: " << path;
    return false;
  }

  file << init;
  file.close();

  Log << Trace << "Successfully created a local file at: " << path;
  Log << Trace << "Wrote " << init.size() << " bytes to the file: " << path;

  return true;
}

static bool InitPackageDirectoryStructure(const std::filesystem::path& package_path, const InitOptions& options) {
  Log << Trace << "Initializing a default package directory structure at: " << package_path;

  if (!OMNI_CATCH(false, std::filesystem::create_directories(package_path / "src"))) {
    Log << "Failed to create the source directory: " << package_path / "src";
    return false;
  }

  if (!OMNI_CATCH(false, std::filesystem::create_directories(package_path / "docs"))) {
    Log << "Failed to create the documentation directory: " << package_path / "docs";
    return false;
  }

  if (!CreateLocalFile(package_path / "docs" / ".gitkeep", GenerateGitKeep())) {
    Log << "Failed to create the .gitkeep file: " << package_path / "docs" / ".gitkeep";
    return false;
  }

  switch (options.m_package_category) {
    case no3::package::PackageCategory::Library:
    case no3::package::PackageCategory::StandardLibrary: {
      if (!CreateLocalFile(package_path / "src" / "lib.n", GenerateDefaultLibrarySource())) {
        Log << "Failed to create the lib.n file: " << package_path / "src" / "lib.n";
        return false;
      }
      break;
    }

    case no3::package::PackageCategory::Comptime:
    case no3::package::PackageCategory::Executable: {
      if (!CreateLocalFile(package_path / "src" / "main.n", GenerateDefaultMainSource())) {
        Log << "Failed to create the main.n file: " << package_path / "src" / "main.n";
        return false;
      }
      break;
    }
  }

  if (!CreateLocalFile(package_path / "README.md", GenerateReadme(options))) {
    Log << "Failed to create the README.md file: " << package_path / "README.md";
    return false;
  }

  if (!CreateLocalFile(package_path / "LICENSE", GenerateLicense(options.m_package_license))) {
    Log << "Failed to create the LICENSE file: " << package_path / "LICENSE";
    return false;
  }

  if (!CreateLocalFile(package_path / "CODE_OF_CONDUCT.md", GenerateCodeOfConduct())) {
    Log << "Failed to create the CODE_OF_CONDUCT.md file: " << package_path / "CODE_OF_CONDUCT.md";
    return false;
  }

  if (!CreateLocalFile(package_path / "CONTRIBUTING.md", GenerateContributingPolicy(package_path))) {
    Log << "Failed to create the CONTRIBUTING.md file: " << package_path / "CONTRIBUTING.md";
    return false;
  }

  if (!CreateLocalFile(package_path / "SECURITY.md", GenerateSecurityPolicy(options.m_package_name))) {
    Log << "Failed to create the SECURITY.md file: " << package_path / "SECURITY.md";
    return false;
  }

  if (!CreateLocalFile(package_path / ".gitignore", GenerateGitIgnore())) {
    Log << "Failed to create the .gitignore file: " << package_path / ".gitignore";
    return false;
  }

  if (!CreateLocalFile(package_path / ".dockerignore", GenerateDockerIgnore())) {
    Log << "Failed to create the .dockerignore file: " << package_path / ".dockerignore";
    return false;
  }

  if (!CreateLocalFile(package_path / "CMakeLists.txt", GenerateCMakeListsTxt())) {
    Log << "Failed to create the CMakeLists.txt file: " << package_path / "CMakeLists.txt";
    return false;
  }

  Log << Trace << "Successfully initialized a default package directory structure at: " << package_path;

  return true;
}

static bool InitPackageDefaultConfigure(const std::filesystem::path& package_path) {
  /// TODO: Implement this function.
  return false;
}

static bool InitPackageRepository(const std::filesystem::path& package_path) {
  /// TODO: Implement this function.

  return false;
}

bool no3::package::InitPackageUsingDefaults(const std::filesystem::path& package_path, const InitOptions& options) {
  Log << Trace << "Initializing a default constructed package at: " << package_path;

  if (OMNI_CATCH(false, std::filesystem::exists(package_path))) {
    Log << Warning << "The package directory already exists: " << package_path;
    return false;
  }

  if (!InitPackageDirectoryStructure(package_path, options)) {
    Log << Trace << "Failed to initialize a default package directory structure: " << package_path;
    return false;
  }

  if (!InitPackageRepository(package_path)) {
    Log << Trace << "Failed to initialize a default package repository: " << package_path;
    return false;
  }

  if (!InitPackageDefaultConfigure(package_path)) {
    Log << Trace << "Failed to initialize a default package configuration: " << package_path;
    return false;
  }

  Log << Trace << "Successfully initialized a default package at: " << package_path;

  return true;
}
