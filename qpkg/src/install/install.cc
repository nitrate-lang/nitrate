////////////////////////////////////////////////////////////////////////////////
///                                                                          ///
///           ░▒▓██████▓▒░░▒▓███████▓▒░░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░            ///
///          ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░           ///
///          ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░                  ///
///          ░▒▓█▓▒░░▒▓█▓▒░▒▓███████▓▒░░▒▓███████▓▒░░▒▓█▓▒▒▓███▓▒░           ///
///          ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░           ///
///          ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░           ///
///           ░▒▓██████▓▒░░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░            ///
///             ░▒▓█▓▒░                                                      ///
///              ░▒▓██▓▒░                                                    ///
///                                                                          ///
///   * QUIX PACKAGE MANAGER - The official tool for the Quix language.      ///
///   * Copyright (C) 2024 Wesley C. Jones                                   ///
///                                                                          ///
///   The QUIX Compiler Suite is free software; you can redistribute it or   ///
///   modify it under the terms of the GNU Lesser General Public             ///
///   License as published by the Free Software Foundation; either           ///
///   version 2.1 of the License, or (at your option) any later version.     ///
///                                                                          ///
///   The QUIX Compiler Suite is distributed in the hope that it will be     ///
///   useful, but WITHOUT ANY WARRANTY; without even the implied warranty of ///
///   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      ///
///   Lesser General Public License for more details.                        ///
///                                                                          ///
///   You should have received a copy of the GNU Lesser General Public       ///
///   License along with the QUIX Compiler Suite; if not, see                ///
///   <https://www.gnu.org/licenses/>.                                       ///
///                                                                          ///
////////////////////////////////////////////////////////////////////////////////

#include <filesystem>
#include <install/Install.hh>
#include <iostream>
#include <regex>

static bool validate_package_name(const std::string &package_name) {
  static std::regex package_name_regex("^[a-zA-Z0-9_-]+$");
  return std::regex_match(package_name, package_name_regex);
}

bool download_git_repo(const std::string &url, const std::string &dest) {
  std::cout << "Downloading package from: " << url << std::endl;

  setenv("QPKG_GIT_INJECT_URL", url.c_str(), 1);
  setenv("QPKG_GIT_INJECT_DEST", dest.c_str(), 1);

  bool e = system(
               "git clone --recurse-submodules --quiet $QPKG_GIT_INJECT_URL "
               "$QPKG_GIT_INJECT_DEST") == 0;
  if (e) {
    std::cerr << "Successfully downloaded package" << std::endl;
  } else {
    std::cerr << "Failed to download package" << std::endl;
  }
  return e;
}

bool qpkg::install::install_from_url(std::string url, const std::string &dest,
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
    std::cerr << "Excpected URL pattern like: 'https://example.com/package'" << std::endl;
    return false;
  }

  package_name = url.substr(url.find_last_of('/') + 1);
  if (!validate_package_name(package_name)) {
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
      if (!download_git_repo(url, package_path.string())) {
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