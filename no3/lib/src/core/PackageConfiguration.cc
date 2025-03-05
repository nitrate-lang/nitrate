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

#include <core/PackageConfigFormat.pb.h>

#include <core/PackageConfiguration.hh>
#include <fstream>
#include <nitrate-core/Assert.hh>
#include <nitrate-core/Logger.hh>

using namespace no3::package;
using namespace ncc;

using ConfigFormat = std::string_view;
using ConfigFormatPair = std::pair<ConfigFormat, std::filesystem::path>;

static constexpr ConfigFormat kConfigJSON = "JSON";
static constexpr ConfigFormat kConfigPROTOBUF = "PROTOBUF";
static constexpr ConfigFormat kConfigNITRATE = "NITRATE";

static std::optional<nlohmann::json> ParsePackageJsonConfig(const std::filesystem::path& config_file);
static std::optional<nlohmann::json> ParsePackageProtobufConfig(const std::filesystem::path& config_file);
static std::optional<nlohmann::json> ParsePackageNitrateConfig(const std::filesystem::path& config_file);
static bool ValidatePackageConfig(const nlohmann::json& json);
static void AssignDefaults(nlohmann::json& json);

static bool TestIsReadable(const std::filesystem::path& path) {
  std::ifstream file(path);
  return file.good();
}

static std::optional<ConfigFormatPair> LocatePackageConfigFile(const std::filesystem::path& package_dir) {
  const std::vector<ConfigFormatPair> config_format_precedence = {
      {kConfigJSON, package_dir / "no3.json"},
      {kConfigPROTOBUF, package_dir / "no3.pb"},
      {kConfigNITRATE, package_dir / "no3.n"},
  };

  for (const auto& [format, path] : config_format_precedence) {
    Log << Debug << "Searching for \"" << format << "\" package configuration file in " << package_dir;

    if (TestIsReadable(path)) {
      Log << Debug << "Found \"" << format << "\" package configuration file in " << package_dir;
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

  if (format == kConfigPROTOBUF) {
    return ParsePackageProtobufConfig(config_file);
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

  Log << Debug << "Using " << config_file << " as package configuration file";

  auto json = ParsePackageConfig(config_file, package_config->first);
  if (!json.has_value()) {
    Log << "Failed to parse package configuration file: " << config_file;
    return std::nullopt;
  }

  if (!ValidatePackageConfig(json.value())) {
    return std::nullopt;
  }

  Log << Debug << "Successfully parsed \"" << package_config->first << "\" package configuration file: " << config_file;

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

static std::optional<nlohmann::json> ParsePackageProtobufConfig(const std::filesystem::path& config_file) {
  using namespace nitrate::no3::package;

  Package package;

  {
    std::ifstream file(config_file, std::ios::in | std::ios::binary);
    if (!file.is_open()) {
      Log << "Failed to open PROTOBUF package configuration file: " << config_file;
      return std::nullopt;
    }

    if (!package.ParseFromIstream(&file)) {
      Log << "Failed to parse PROTOBUF package configuration file: " << config_file;
      return std::nullopt;
    }
  }

  // I think that the following line is not necessary, but ?
  package.CheckInitialized();

  /// TODO: Convert protobuf to json

  return std::nullopt;
}

static std::optional<nlohmann::json> ParsePackageNitrateConfig(const std::filesystem::path& config_file) {
  /// TODO: Parse Nitrate package configuration
  return std::nullopt;
}

#define rule(__expr, __explanation)                               \
  if (!(__expr)) [[unlikely]] {                                   \
    Log << "Package configuration is invalid: " << __explanation; \
    return false;                                                 \
  }

static bool ValidateMetadata(const nlohmann::json& json) {
  /// TODO: Validate metadata

  return true;
}

static bool ValidateBuild(const nlohmann::json& json) {
  /// TODO: Validate build

  return true;
}

static bool ValidateOwnership(const nlohmann::json& json) {
  /// TODO: Validate ownership

  return true;
}

static bool ValidatePackageConfig(const nlohmann::json& json) {
  rule(json.is_object(), "Root of package configuration must be an object");
  rule(json.contains("format"), "Package configuration must contain a \"format\" field");
  rule(json["format"].is_number_unsigned(), "Package configuration \"format\" field must be an unsigned integer");
  rule(json["format"].get<unsigned>() == 1,
       "Package configuration is in an unsupported format. The expected format version is 1");
  rule(json.contains("metadata"), "Package configuration must contain a \"metadata\" field");
  rule(json.contains("build"), "Package configuration must contain a \"build\" field");
  rule(json.contains("ownership"), "Package configuration must contain an \"ownership\" field");
  rule(ValidateMetadata(json["metadata"]), "Invalid metadata");
  rule(ValidateBuild(json["build"]), "Invalid build");
  rule(ValidateOwnership(json["ownership"]), "Invalid ownership");

  return true;
}

#undef rule

static void AssignDefaults(nlohmann::json& json) {
  /// TODO: Assign defaults to json
}
