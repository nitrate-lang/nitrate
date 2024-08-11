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

#include <clean/Cleanup.hh>
#include <conf/Parser.hh>
#include <conf/Validate.hh>
#include <core/Logger.hh>

static std::optional<qpkg::conf::Config> get_config(const std::filesystem::path &base) {
  if (std::filesystem::exists(base / "qpkg.yaml")) {
    auto c = qpkg::conf::YamlConfigParser().parsef(base / "qpkg.yaml");

    if (!c) {
      LOG(qpkg::core::ERROR) << "Failed to parse configuration file: " << base / "qpkg.yaml"
                             << std::endl;
      return std::nullopt;
    }

    if (!qpkg::conf::ValidateConfig(*c, base)) {
      LOG(qpkg::core::ERROR) << "Failed to validate configuration" << std::endl;
      return std::nullopt;
    }

    qpkg::conf::PopulateConfig(*c);

    return c;
  } else {
    LOG(qpkg::core::ERROR) << "No configuration file found in package source directory"
                           << std::endl;
    return std::nullopt;
  }
}

static bool recursve_subpackages(const std::filesystem::path &base, bool verbose) {
  auto c = get_config(base);

  if (!c) return false;

  auto packages = (*c)["packages"].as<std::vector<std::string>>();

  for (const auto &p : packages) qpkg::clean::CleanPackageSource(base / p, verbose);

  return true;
}

bool qpkg::clean::CleanPackageSource(const std::string &package_src, bool verbose) {
  std::filesystem::path package_src_path(package_src);

  if (!std::filesystem::exists(package_src_path)) {
    LOG(core::ERROR) << "Package source path does not exist: " << package_src << std::endl;
    return false;
  }

  if (!std::filesystem::is_directory(package_src_path)) {
    LOG(core::ERROR) << "Package source path is not a directory: " << package_src << std::endl;
    return false;
  }

  if (verbose) {
    LOG(core::INFO) << "Cleaning package source recursively" << std::endl;
  }

  // std::filesystem::path cache_dir = package_src_path / ".qpkg" / "cache";
  // std::filesystem::path build_dir = package_src_path / ".qpkg" / "build";

  // if (std::filesystem::exists(cache_dir)) {
  //   if (verbose)
  //     LOG(core::INFO) << "Removing cache directory: " << cache_dir <<
  //     std::endl;

  //   std::filesystem::remove_all(cache_dir);
  // }

  // if (std::filesystem::exists(build_dir)) {
  //   if (verbose)
  //     LOG(core::INFO) << "Removing build directory: " << build_dir <<
  //     std::endl;
  //   std::filesystem::remove_all(build_dir);
  // }

  std::filesystem::path qpkg_dir = package_src_path / ".qpkg";

  if (std::filesystem::exists(qpkg_dir)) {
    if (verbose) LOG(core::INFO) << "Removing .qpkg directory: " << qpkg_dir << std::endl;

    std::filesystem::remove_all(qpkg_dir);
  }

  auto conf = get_config(package_src_path);
  if (!conf) {
    LOG(core::ERROR) << "Failed to get configuration" << std::endl;
    return false;
  }

  std::string name = conf.value()["name"].as<std::string>(), tmp;

#define RMFILE(_file)                                                                         \
  tmp = _file;                                                                                \
  if (std::filesystem::is_regular_file(package_src_path / tmp)) {                             \
    if (verbose) LOG(core::INFO) << "Removing file: " << package_src_path / tmp << std::endl; \
    std::filesystem::remove(package_src_path / tmp);                                          \
  }

  RMFILE(name);
  RMFILE(name + ".exe");
  RMFILE(name + ".dll");
  RMFILE("lib" + name + ".dll");
  RMFILE("lib" + name + ".so");
  RMFILE("lib" + name + ".dylib");
  RMFILE("lib" + name + ".a");
  RMFILE("lib" + name + ".lib");
  RMFILE("lib" + name + ".la");

  recursve_subpackages(package_src_path, verbose);

  if (verbose) LOG(core::INFO) << "Package " << package_src << " cleaned" << std::endl;

  return true;
}