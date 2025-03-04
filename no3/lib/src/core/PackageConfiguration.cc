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

#include <core/PackageConfiguration.hh>
#include <fstream>
#include <nitrate-core/Assert.hh>
#include <nitrate-core/Logger.hh>

using namespace no3::package;
using namespace ncc;

using ConfigFormat = std::string_view;
using ConfigFormatPair = std::pair<ConfigFormat, std::filesystem::path>;

static constexpr ConfigFormat kConfigJSON = "JSON";
static constexpr ConfigFormat kConfigTOML = "TOML";
static constexpr ConfigFormat kConfigYAML = "YAML";
static constexpr ConfigFormat kConfigNITRATE = "NITRATE";

static std::optional<nlohmann::json> ParsePackageJsonConfig(const std::filesystem::path& config_file);
static std::optional<nlohmann::json> ParsePackageTomlConfig(const std::filesystem::path& config_file);
static std::optional<nlohmann::json> ParsePackageYamlConfig(const std::filesystem::path& config_file);
static std::optional<nlohmann::json> ParsePackageNitrateConfig(const std::filesystem::path& config_file);
static bool ValidatePackageConfig(const nlohmann::json& json);
static void AssignDefaults(nlohmann::json& json);

static bool TestIsReadable(const std::filesystem::path& path) {
  std::ifstream file(path);
  return file.good();
}

static std::optional<ConfigFormatPair> LocatePackageConfigFile(const std::filesystem::path& package_dir) {
  const std::vector<ConfigFormatPair> config_format_precedence = {
      {kConfigJSON, package_dir / "package.json"},
      {kConfigTOML, package_dir / "package.toml"},
      {kConfigYAML, package_dir / "package.yaml"},
      {kConfigNITRATE, package_dir / "package.n"},
  };

  for (const auto& [format, path] : config_format_precedence) {
    Log << Debug << "Searching for \"" << format << "\" package configuration file in " << package_dir;

    if (TestIsReadable(path)) {
      Log << Info << "Found \"" << format << "\" package configuration file in " << package_dir;
      return {{format, path}};
    }
  }

  Log << Debug << "No known package configuration was found in " << package_dir;

  return std::nullopt;
}

static std::optional<nlohmann::json> ParsePackageConfig(const std::filesystem::path& config_file, ConfigFormat format) {
  if (format == kConfigJSON) {
    return ParsePackageJsonConfig(config_file);
  }

  if (format == kConfigTOML) {
    return ParsePackageTomlConfig(config_file);
  }

  if (format == kConfigYAML) {
    return ParsePackageYamlConfig(config_file);
  }

  if (format == kConfigNITRATE) {
    return ParsePackageNitrateConfig(config_file);
  }

  Log << "Unknown package configuration format: " << format;
  return std::nullopt;
}

std::optional<PackageConfiguration> PackageConfiguration::ParsePackage(const std::filesystem::path& package_dir) {
  const auto package_config = LocatePackageConfigFile(package_dir);
  if (!package_config.has_value()) {
    Log << "No known package configuration was found in " << package_dir;
    return std::nullopt;
  }

  const auto config_file = package_config->second;

  Log << Info << "Using " << config_file << " as package configuration file";

  auto json = ParsePackageConfig(config_file, package_config->first);
  if (!json.has_value()) {
    Log << "Failed to parse package configuration file: " << config_file;
    return std::nullopt;
  }

  if (!ValidatePackageConfig(json.value())) {
    Log << "Invalid package configuration file: " << config_file;
    return std::nullopt;
  }

  Log << Info << "Successfully parsed \"" << package_config->first << "\" package configuration file: " << config_file;

  return PackageConfiguration(std::move(*json));
}

[[nodiscard]] const nlohmann::json& PackageConfiguration::Json(bool defaults) const {
  if (defaults) {
    if (!m_full.has_value()) {
      m_full = m_raw;
      AssignDefaults(m_full.value());
    }

    return m_full.value();
  }

  return m_raw;
}

static std::optional<nlohmann::json> ParsePackageJsonConfig(const std::filesystem::path& config_file) {
  std::ifstream file(config_file);
  if (!file.good()) {
    Log << "Failed to open package configuration file: " << config_file;
    return std::nullopt;
  }

  nlohmann::json json = nlohmann::json::parse(file, nullptr, false);
  if (json.is_discarded()) {
    Log << "Package configuration file is not valid JSON: " << config_file;
    return std::nullopt;
  }

  Log << Debug << "Successfully parsed JSON package configuration file: " << config_file;

  return json;
}

static std::optional<nlohmann::json> ParsePackageTomlConfig(const std::filesystem::path& config_file) {
  /// TODO: Parse TOML package configuration
  return std::nullopt;
}

static std::optional<nlohmann::json> ParsePackageYamlConfig(const std::filesystem::path& config_file) {
  /// TODO: Parse YAML package configuration
  return std::nullopt;
}

static std::optional<nlohmann::json> ParsePackageNitrateConfig(const std::filesystem::path& config_file) {
  /// TODO: Parse Nitrate package configuration
  return std::nullopt;
}

static bool ValidatePackageConfig(const nlohmann::json& json) {
  /// TODO: Validate json
  return true;
}

static void AssignDefaults(nlohmann::json& json) {
  /// TODO: Assign defaults to json
}
