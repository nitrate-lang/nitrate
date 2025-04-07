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

#include <git2.h>
#include <git2/types.h>

#include <core/package/Manifest.hh>
#include <fstream>
#include <init/InitPackage.hh>
#include <nitrate-core/CatchAll.hh>
#include <nitrate-core/Logger.hh>

using namespace ncc;
using namespace no3::package;

static auto CreateDirectories(const std::filesystem::path& path) -> bool {
  Log << Trace << "Creating directories at: " << path;

  auto dirs_exists = OMNI_CATCH(std::filesystem::exists(path));
  if (!dirs_exists.has_value()) {
    Log << "Failed to check if the directories exist: " << path;
    return false;
  }

  if (*dirs_exists) {
    Log << Trace << "The directories already exist: " << path;
    return true;
  }

  auto dirs_created = OMNI_CATCH(std::filesystem::create_directories(path)).value_or(false);
  if (!dirs_created) {
    Log << "Failed to create directories at: " << path;
    return false;
  }

  Log << Trace << "Successfully created directories at: " << path;

  return true;
}

static auto CreateLocalFile(const std::filesystem::path& path, std::string_view init) -> bool {
  Log << Trace << "Creating a local file at: " << path;

  auto file_exists = OMNI_CATCH(std::filesystem::exists(path));
  if (!file_exists.has_value()) {
    Log << "Failed to check if the file exists: " << path;
    return false;
  }

  if (*file_exists) {
    Log << Warning << "The file already exists: " << path;
    return false;
  }

  if (!CreateDirectories(path.parent_path())) {
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

static auto InitPackageDirectoryStructure(const std::filesystem::path& package_path,
                                          const InitOptions& options) -> bool {
  Log << Trace << "Initializing a the default package files at: " << package_path;

  if (!CreateLocalFile(package_path / "docs" / ".gitkeep", GenerateGitKeep())) {
    Log << "Failed to create the .gitkeep file: " << package_path / "docs" / ".gitkeep";
    return false;
  }

  switch (options.m_package_category) {
    case no3::package::Manifest::Category::Library:
    case no3::package::Manifest::Category::StandardLibrary: {
      if (!CreateLocalFile(package_path / "src" / "lib.nit", GenerateDefaultLibrarySource())) {
        Log << "Failed to create the lib.n file: " << package_path / "src" / "lib.nit";
        return false;
      }
      break;
    }

    case no3::package::Manifest::Category::Executable: {
      if (!CreateLocalFile(package_path / "src" / "main.nit", GenerateDefaultMainSource())) {
        Log << "Failed to create the main.n file: " << package_path / "src" / "main.nit";
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

  if (!CreateLocalFile(package_path / "CONTRIBUTING.md", GenerateContributingPolicy(options))) {
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

  if (!CreateLocalFile(package_path / "CMakeLists.txt", GenerateCMakeListsTxt(options.m_package_name))) {
    Log << "Failed to create the CMakeLists.txt file: " << package_path / "CMakeLists.txt";
    return false;
  }

  {
    bool correct_schema = false;
    auto initial_config = Manifest(options.m_package_name, options.m_package_category)
                              .SetDescription(options.m_package_description)
                              .SetLicense(options.m_package_license)
                              .SetVersion(options.m_package_version)
                              .SetOptimization(Manifest::Optimization()
                                                   .SetProfile("rapid", Manifest::Optimization::Switch()
                                                                            .SetAlpha({"-O0"})
                                                                            .SetBeta({"-O0"})
                                                                            .SetGamma({"-O0"})
                                                                            .SetLLVM({"-O1"})
                                                                            .SetLTO({"-O0"})
                                                                            .SetRuntime({"-O0"}))
                                                   .SetProfile("debug", Manifest::Optimization::Switch()
                                                                            .SetAlpha({"-O2"})
                                                                            .SetBeta({"-O2"})
                                                                            .SetGamma({"-O2"})
                                                                            .SetLLVM({"-O3"})
                                                                            .SetLTO({"-O0"})
                                                                            .SetRuntime({"-O1"}))
                                                   .SetProfile("release", Manifest::Optimization::Switch()
                                                                              .SetAlpha({"-O3"})
                                                                              .SetBeta({"-O3"})
                                                                              .SetGamma({"-O3"})
                                                                              .SetLLVM({"-O3"})
                                                                              .SetLTO({"-O3"})
                                                                              .SetRuntime({"-O3"})))
                              .ToJson(correct_schema);

    if (!correct_schema) {
      Log << "Failed to create the initial package configuration: " << package_path / "no3.json";
      return false;
    }

    if (!CreateLocalFile(package_path / "no3.json", initial_config)) {
      Log << "Failed to create the no3.json file: " << package_path / "no3.json";
      return false;
    }
  }

  Log << Trace << "Successfully initialized the package directory structure at: " << package_path;

  return true;
}

static auto InitPackageRepository(const std::filesystem::path& package_path) -> bool {
  Log << Trace << "Initializing a git repository in: " << package_path;

  git_repository* repo = nullptr;
  if (git_repository_init(&repo, package_path.c_str(), 0) != 0) {
    Log << "git_repository_init(): Failed to initialize a git repository in: " << package_path;
    return false;
  }

  git_repository_free(repo);

  Log << Trace << "Successfully initialized a git repository in: " << package_path;
  Log << Trace << "Successfully created package repository in: " << package_path;

  return true;
}

auto no3::package::CreatePackage(const std::filesystem::path& package_path, const InitOptions& options) -> bool {
  Log << Trace << "Initializing a new package at: " << package_path;

  auto package_path_exists = OMNI_CATCH(std::filesystem::exists(package_path));
  if (!package_path_exists.has_value()) {
    Log << "Failed to check if the package directory exists: " << package_path;
    return false;
  }

  if (*package_path_exists) {
    Log << Warning << "The package directory already exists: " << package_path;
    return false;
  }

  if (!InitPackageDirectoryStructure(package_path, options)) {
    Log << Trace << "Failed to initialize the package directory structure: " << package_path;
    return false;
  }

  if (!InitPackageRepository(package_path)) {
    Log << Trace << "Failed to initialize the package repository: " << package_path;
    return false;
  }

  Log << Trace << "Successfully initialized package contents at: " << package_path;

  return true;
}
