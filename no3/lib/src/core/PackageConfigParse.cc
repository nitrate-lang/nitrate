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
#include <google/protobuf/util/json_util.h>

#include <core/PackageConfig.hh>
#include <fstream>
#include <nitrate-core/Assert.hh>
#include <nitrate-core/Logger.hh>

using namespace ncc;
using namespace no3::package;

using ConfigFormat = std::string_view;
using ConfigFormatPair = std::pair<ConfigFormat, std::filesystem::path>;

static constexpr ConfigFormat kConfigJSON = "JSON";
static constexpr ConfigFormat kConfigPROTOBUF = "PROTOBUF";
static constexpr ConfigFormat kConfigNITRATE = "NITRATE";

namespace no3::package {
  bool ValidatePackageConfig(const nlohmann::ordered_json& json);
  nitrate::no3::package::Package* ConfigJsonToProtobuf(google::protobuf::Arena& mm, const nlohmann::ordered_json& json);
  std::optional<nlohmann::ordered_json> ConfigProtobufToJson(const nitrate::no3::package::Package& protobuf);
}  // namespace no3::package

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

static std::optional<nlohmann::ordered_json> ParsePackageJsonConfig(const std::filesystem::path& config_file) {
  std::ifstream file(config_file);
  if (!file.good()) {
    Log << "Failed to open package configuration file: " << config_file;
    return std::nullopt;
  }

  nlohmann::ordered_json json = nlohmann::ordered_json::parse(file, nullptr, false);
  if (json.is_discarded()) {
    Log << "Package configuration file is not valid JSON: " << config_file;
    return std::nullopt;
  }

  Log << Debug << "Successfully parsed JSON package configuration file: " << config_file;

  return json;
}

static std::optional<nlohmann::ordered_json> ParsePackageProtobufConfig(const std::filesystem::path& config_file) {
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

  return ConfigProtobufToJson(package);
}

static std::optional<nlohmann::ordered_json> ParsePackageNitrateConfig(const std::filesystem::path& config_file) {
  /// TODO: Parse Nitrate package configuration
  (void)config_file;

  return std::nullopt;
}

static std::optional<nlohmann::ordered_json> ParsePackageConfig(const std::filesystem::path& config_file,
                                                                ConfigFormat format) {
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

static void AssignDefaults(nlohmann::ordered_json& json) {
  // Nothing to do here
  (void)json;
}

[[nodiscard]] const nlohmann::ordered_json& PackageConfig::Json(bool defaults) const {
  if (defaults) {
    if (!m_full.has_value()) {
      m_full = m_raw;
      AssignDefaults(m_full.value());
    }

    return m_full.value();
  }

  return m_raw;
}

[[nodiscard]] const std::string& PackageConfig::Protobuf(bool defaults) const {
  google::protobuf::Arena mm;

  if (defaults) {
    if (!m_protobuf_full.has_value()) {
      auto* protobuf = ConfigJsonToProtobuf(mm, Json(true));
      if (protobuf == nullptr) {
        qcore_panic("Failed to convert JSON package configuration to PROTOBUF");
      }

      m_protobuf_full = protobuf->SerializeAsString();
    }

    return m_protobuf_full.value();
  }

  if (!m_protobuf_raw.has_value()) {
    auto* protobuf = ConfigJsonToProtobuf(mm, Json(false));
    if (protobuf == nullptr) {
      qcore_panic("Failed to convert JSON package configuration to PROTOBUF");
    }

    m_protobuf_raw = protobuf->SerializeAsString();
  }

  return m_protobuf_raw.value();
}

std::optional<PackageConfig> PackageConfig::ParsePackage(const std::filesystem::path& package_dir) {
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

  return PackageConfig(std::move(*json));
}
