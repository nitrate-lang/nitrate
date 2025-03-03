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

#include <clean/Cleanup.hh>
#include <conf/Parser.hh>
#include <conf/Validate.hh>
#include <nitrate-core/Logger.hh>

using namespace ncc;

static auto GetConfig(const std::filesystem::path &base) -> std::optional<no3::conf::Config> {
  if (std::filesystem::exists(base / "no3.yaml")) {
    auto c = no3::conf::YamlConfigParser().Parsef(base / "no3.yaml");

    if (!c) {
      Log << "Failed to parse configuration file: " << base / "no3.yaml";
      return std::nullopt;
    }

    if (!no3::conf::ValidateConfig(*c, base)) {
      Log << "Failed to validate configuration";
      return std::nullopt;
    }

    no3::conf::PopulateConfig(*c);

    return c;
  }
  Log << "No configuration file found in package source directory";
  return std::nullopt;
}

static auto RecursveSubpackages(const std::filesystem::path &base, bool verbose) -> bool {
  auto c = GetConfig(base);

  if (!c) {
    return false;
  }

  auto packages = (*c)["packages"].As<std::vector<std::string>>();

  for (const auto &p : packages) {
    no3::clean::CleanPackageSource(base / p, verbose);
  }

  return true;
}

auto no3::clean::CleanPackageSource(const std::string &package_src, bool verbose) -> bool {
  std::filesystem::path package_src_path(package_src);

  if (!std::filesystem::exists(package_src_path)) {
    Log << "Package source path does not exist: " << package_src;
    return false;
  }

  if (!std::filesystem::is_directory(package_src_path)) {
    Log << "Package source path is not a directory: " << package_src;
    return false;
  }

  if (verbose) {
    Log << Info << "Cleaning package source recursively";
  }

  // std::filesystem::path cache_dir = package_src_path / ".no3" / "cache";
  // std::filesystem::path build_dir = package_src_path / ".no3" / "build";

  // if (std::filesystem::exists(cache_dir)) {
  //   if (verbose)
  //     Log << Info << "Removing cache directory: " << cache_dir <<
  //     std::endl;

  //   std::filesystem::remove_all(cache_dir);
  // }

  // if (std::filesystem::exists(build_dir)) {
  //   if (verbose)
  //     Log << Info << "Removing build directory: " << build_dir <<
  //     std::endl;
  //   std::filesystem::remove_all(build_dir);
  // }

  std::filesystem::path no3_dir = package_src_path / ".no3";

  if (std::filesystem::exists(no3_dir)) {
    if (verbose) {
      Log << Info << "Removing .no3 directory: " << no3_dir;
    }

    std::filesystem::remove_all(no3_dir);
  }

  auto conf = GetConfig(package_src_path);
  if (!conf) {
    Log << "Failed to get configuration";
    return false;
  }

  auto name = conf.value()["name"].As<std::string>();
  std::string tmp;

#define RMFILE(_file)                                                        \
  tmp = _file;                                                               \
  if (std::filesystem::is_regular_file(package_src_path / tmp)) {            \
    if (verbose) Log << Info << "Removing file: " << package_src_path / tmp; \
    std::filesystem::remove(package_src_path / tmp);                         \
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

  RecursveSubpackages(package_src_path, verbose);

  if (verbose) {
    Log << Info << "Package " << package_src << " cleaned";
  }

  return true;
}