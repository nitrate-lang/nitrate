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
#include <git2/clone.h>
#include <git2/types.h>

#include <filesystem>
#include <install/Install.hh>
#include <iostream>
#include <regex>

static bool ValidatePackageName(const std::string &package_name) {
  static std::regex package_name_regex("^[a-zA-Z0-9_-]+$");
  return std::regex_match(package_name, package_name_regex);
}

bool DownloadGitRepo(const std::string &url, const std::string &dest) {
  std::cout << "Downloading package from: " << url << std::endl;

  /// TODO: Test if it works with git submodules

  git_repository *repo = nullptr;
  if (git_clone(&repo, url.c_str(), dest.c_str(), nullptr) != 0) {
    std::cerr << "Failed to download package" << std::endl;
    return false;
  }

  git_repository_free(repo);

  std::cerr << "Successfully downloaded package" << std::endl;

  return true;
}

bool no3::install::InstallFromUrl(std::string url, const std::string &dest,
                                  std::string &package_name, bool overwrite) {
  enum class FetchType {
    GIT,
    UNKNOWN,
  } fetch_type = FetchType::GIT;  // Assume git for now

  /*=========== PROCESS URL ===========*/
  if (url.ends_with("/")) {
    url = url.substr(0, url.size() - 1);
  }
  if (url.ends_with(".git")) {
    url = url.substr(0, url.size() - 4);
  }
  if (url.find("/") == std::string::npos) {
    std::cerr << "Excpected URL pattern like: 'https://example.com/package'"
              << std::endl;
    return false;
  }

  package_name = url.substr(url.find_last_of('/') + 1);
  if (!ValidatePackageName(package_name)) {
    std::cerr << "Invalid package name: " << package_name << std::endl;
    return false;
  }

  std::filesystem::path package_path = dest + "/" + package_name;

  try {
    bool exists = std::filesystem::exists(package_path);
    if (!overwrite && exists) {
      std::cerr << "Package already exists: " << package_name << std::endl;
      return false;
    } else if (exists) {
      std::filesystem::remove_all(package_path);
    }
  } catch (std::filesystem::filesystem_error &e) {
    std::cerr << e.what() << std::endl;
    std::cerr << "Failed to install package: " << package_name << std::endl;
    return false;
  }

  /*=========== FETCH PACKAGE ===========*/

  switch (fetch_type) {
    case FetchType::GIT:
      if (!DownloadGitRepo(url, package_path.string())) {
        std::cerr << "Failed to fetch package: " << package_name << std::endl;
        return false;
      }
      return true;

    default:
      std::cerr << "Unable to fetch package: " << package_name << std::endl;
      std::cerr << "Unknown repository type" << std::endl;
      return false;
  }
}