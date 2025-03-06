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
#include <init/InitPackage.hh>
#include <nitrate-core/Logger.hh>

using namespace ncc;

static bool InitPackageDirectoryStructure(const std::filesystem::path& package_path) {
  /// TODO: Implement this function.
  return false;
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

  if (!InitPackageDirectoryStructure(package_path)) {
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
