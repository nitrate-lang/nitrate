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
#include <nlohmann/json.hpp>
#include <optional>

namespace no3::package {
  enum class PackageCategory { Executable, Library, StandardLibrary };

  class PackageConfig {
    nlohmann::ordered_json m_raw;
    mutable std::optional<nlohmann::ordered_json> m_full;
    mutable std::optional<std::string> m_protobuf_raw, m_protobuf_full;

    PackageConfig(nlohmann::ordered_json raw) : m_raw(std::move(raw)) {}

  public:
    ~PackageConfig() = default;

    static std::optional<PackageConfig> ParsePackage(const std::filesystem::path& package_dir);

    [[nodiscard]] const nlohmann::ordered_json& Json(bool defaults = true) const;
    [[nodiscard]] const std::string& Protobuf(bool defaults = true) const;

    static std::string PackageNameRegex;

    [[nodiscard]] static bool ValidatePackageName(const std::string& package_name, bool maybe_standard_lib = false);
    [[nodiscard]] static bool ValidatePackageLicense(const std::string& license);
    [[nodiscard]] static bool ValidatePackageVersion(const std::string& version);

    [[nodiscard]] static nlohmann::ordered_json CreateInitialConfiguration(const std::string& name,
                                                                           const std::string& description,
                                                                           const std::string& license,
                                                                           const std::string& version,
                                                                           PackageCategory category);
  };
}  // namespace no3::package
