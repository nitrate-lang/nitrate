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
#include <core/SPDX.hh>
#include <nitrate-core/Assert.hh>
#include <nitrate-core/Logger.hh>
#include <regex>

using namespace ncc;
using namespace no3::package;

std::string PackageConfig::PackageNameRegex =
    R"(^@([a-z]+-)?([a-zA-Z0-9]+|[a-zA-Z0-9][a-zA-Z0-9-]+[a-zA-Z0-9])\/([a-zA-Z0-9][a-zA-Z0-9-]{1,30}[a-zA-Z0-9])(:\d+)?$)";

static const std::regex PACKAGE_NAME_PATTERN(PackageConfig::PackageNameRegex);

static const std::regex SEMVER_PATTERN(R"MAGIC(^\d+\.\d+\.\d+$)MAGIC");

bool PackageConfig::ValidatePackageName(const std::string& package_name, bool maybe_standard_lib) {
  Log << Trace << "Validating package name format: \"" << package_name << "\"";

  if (!std::regex_match(package_name, PACKAGE_NAME_PATTERN)) {
    Log << Trace << "Package name failed format validation [regex mismatch]: \"" << package_name << "\"";
    return false;
  }

  // I couldn't find a way to do this with a regex, so I'm doing it manually.
  if (package_name.find("--") != std::string::npos) {
    Log << Trace << "Package name failed format validation [double hyphen]: \"" << package_name << "\"";
    return false;
  }

  // Only standard library packages are allowed to omit their Git provider prefix.
  const auto package_username = package_name.substr(1, package_name.find('/') - 1);
  if (!maybe_standard_lib && package_username.find('-') == std::string::npos) {
    Log << Trace << "Package name failed format validation [missing Git provider prefix]: \"" << package_name << "\"";
    return false;
  }

  Log << Trace << "Package name passed format validation: \"" << package_name << "\"";

  return true;
}

bool PackageConfig::ValidatePackageLicense(const std::string& license) {
  Log << Trace << "Validating package license: \"" << license << "\"";

  if (!constants::IsExactSPDXLicenseMatch(license)) {
    Log << Trace << "Failed to find match in SPDX license table: \"" << license << "\"";
    return false;
  }

  Log << Trace << "Package license passed SPDX validation: \"" << license << "\"";

  return true;
}

bool PackageConfig::ValidatePackageVersion(const std::string& version) {
  Log << Trace << "Validating package version: \"" << version << "\"";
  if (!std::regex_match(version, SEMVER_PATTERN)) {
    Log << Trace << "Package version failed format validation [regex mismatch]: \"" << version << "\"";
    return false;
  }

  Log << Trace << "Package version passed format validation: \"" << version << "\"";

  return true;
}

#define schema_assert(__expr)                                               \
  if (!(__expr)) [[unlikely]] {                                             \
    Log << "Invalid configuration:" << " schema_assert(" << #__expr << ")"; \
    return false;                                                           \
  }

static bool ValidateUUID(const std::string& uuid) {
  schema_assert(uuid.size() == 36);
  schema_assert(uuid[8] == '-');
  schema_assert(uuid[13] == '-');
  schema_assert(uuid[18] == '-');
  schema_assert(uuid[23] == '-');

  schema_assert(std::all_of(uuid.begin(), uuid.end(), [](char c) { return std::isxdigit(c) || c == '-'; }));

  return true;
}

static bool ValidateEd25519PublicKey(const std::string& value) {
  schema_assert(value.size() == 64);
  schema_assert(std::all_of(value.begin(), value.end(), [](char c) { return std::isxdigit(c); }));
  return true;
}

static bool ValidateKeyPair(const nlohmann::ordered_json& json) {
  schema_assert(json.is_object());

  schema_assert(json.contains("type"));
  schema_assert(json["type"].is_string());
  schema_assert([&]() {
    auto v = json["type"].get<std::string>();
    schema_assert(v == "ed25519");
    return true;
  }());

  schema_assert(json.contains("value"));
  schema_assert(json["value"].is_string());
  schema_assert(ValidateEd25519PublicKey(json["value"].template get<std::string>()));

  schema_assert(json.size() == 2);

  return true;
}

static bool ValidateEd25519Signature(const std::string& value) {
  schema_assert(value.size() == 128);
  schema_assert(std::all_of(value.begin(), value.end(), [](char c) { return std::isxdigit(c); }));
  return true;
}

static bool ValidateSignatureJson(const nlohmann::ordered_json& json) {
  schema_assert(json.is_object());

  schema_assert(json.contains("type"));
  schema_assert(json["type"].is_string());
  schema_assert([&]() {
    auto v = json["type"].get<std::string>();
    schema_assert(v == "ed25519");
    return true;
  }());

  schema_assert(json.contains("value"));
  schema_assert(json["value"].is_string());
  schema_assert(ValidateEd25519Signature(json["value"].template get<std::string>()));

  return true;
}

static bool ValidateSemVersion(const nlohmann::ordered_json& json) {
  schema_assert(json.is_object());

  schema_assert(json.contains("major"));
  schema_assert(json["major"].is_number_unsigned());

  schema_assert(json.contains("minor"));
  schema_assert(json["minor"].is_number_unsigned());

  schema_assert(json.contains("patch"));
  schema_assert(json["patch"].is_number_unsigned());

  schema_assert(json.size() == 3);

  return true;
}

static bool ValidateBuildOptimizationSwitch(const nlohmann::ordered_json& json) {
  schema_assert(json.is_object());

  schema_assert(json.contains("alpha"));
  schema_assert(json["alpha"].is_array());
  schema_assert(std::all_of(json["alpha"].begin(), json["alpha"].end(), [](const auto& alpha_opt_flag) {
    schema_assert(alpha_opt_flag.is_string());
    return true;
  }));

  schema_assert(json.contains("beta"));
  schema_assert(json["beta"].is_array());
  schema_assert(std::all_of(json["beta"].begin(), json["beta"].end(), [](const auto& beta_opt_flag) {
    schema_assert(beta_opt_flag.is_string());
    return true;
  }));

  schema_assert(json.contains("gamma"));
  schema_assert(json["gamma"].is_array());
  schema_assert(std::all_of(json["gamma"].begin(), json["gamma"].end(), [](const auto& gamma_opt_flag) {
    schema_assert(gamma_opt_flag.is_string());
    return true;
  }));

  schema_assert(json.contains("llvm"));
  schema_assert(json["llvm"].is_array());
  schema_assert(std::all_of(json["llvm"].begin(), json["llvm"].end(), [](const auto& llvm_opt_flag) {
    schema_assert(llvm_opt_flag.is_string());
    return true;
  }));

  schema_assert(json.contains("lto"));
  schema_assert(json["lto"].is_array());
  schema_assert(std::all_of(json["lto"].begin(), json["lto"].end(), [](const auto& lto_opt_flag) {
    schema_assert(lto_opt_flag.is_string());
    return true;
  }));

  schema_assert(json.contains("runtime"));
  schema_assert(json["runtime"].is_array());
  schema_assert(std::all_of(json["runtime"].begin(), json["runtime"].end(), [](const auto& runtime_opt_flag) {
    schema_assert(runtime_opt_flag.is_string());
    return true;
  }));

  return true;
}

static bool ValidateBuildOptimization(const nlohmann::ordered_json& json) {
  schema_assert(json.is_object());

  {  // key ["optimization"]["rapid"]
    schema_assert(json.contains("rapid"));
    schema_assert(json["rapid"].is_object());
    schema_assert(json["rapid"].contains("switch"));
    schema_assert(ValidateBuildOptimizationSwitch(json["rapid"]["switch"]));
  }

  {  // key["optimization"]["debug"]
    schema_assert(json.contains("debug"));
    schema_assert(json["debug"].is_object());
    schema_assert(json["debug"].contains("switch"));
    schema_assert(ValidateBuildOptimizationSwitch(json["debug"]["switch"]));
  }

  {  // key ["optimization"]["release"]
    schema_assert(json.contains("release"));
    schema_assert(json["release"].is_object());
    schema_assert(json["release"].contains("switch"));
    schema_assert(ValidateBuildOptimizationSwitch(json["release"]["switch"]));
  }

  {  // key ["optimization"]["requirements"]
    schema_assert(json.contains("requirements"));
    schema_assert(json["requirements"].is_object());

    {  // key ["optimization"]["requirements"]["min-free-cores"]
      schema_assert(json["requirements"].contains("min-free-cores"));
      schema_assert(json["requirements"]["min-free-cores"].is_number_unsigned());
    }

    {  // key ["optimization"]["requirements"]["min-free-memory"]
      schema_assert(json["requirements"].contains("min-free-memory"));
      schema_assert(json["requirements"]["min-free-memory"].is_number_unsigned());
    }

    {  // key ["optimization"]["requirements"]["min-free-storage"]
      schema_assert(json["requirements"].contains("min-free-storage"));
      schema_assert(json["requirements"]["min-free-storage"].is_number_unsigned());
    }
  }

  return true;
}

static bool ValidateBlockchain(const nlohmann::ordered_json& json) {
  schema_assert(json.is_array());

  schema_assert(std::all_of(json.begin(), json.end(), [](const auto& blockchain_item) {
    schema_assert(blockchain_item.is_object());

    {  // key ["blockchain"][i]["uuid"]
      schema_assert(blockchain_item.contains("uuid"));
      schema_assert(blockchain_item["uuid"].is_string());
      schema_assert(ValidateUUID(blockchain_item["uuid"].template get<std::string>()));
    }

    {  // key ["blockchain"][i]["category"]
      schema_assert(blockchain_item.contains("category"));
      schema_assert(blockchain_item["category"].is_string());
      schema_assert([&]() {
        auto v = blockchain_item["category"].template get<std::string>();
        schema_assert(v == "eco-root" || v == "eco-domain" || v == "user-account" || v == "package" ||
                      v == "subpackage");
        return true;
      }());
    }

    {  // key ["blockchain"][i]["pubkey"]
      schema_assert(blockchain_item.contains("pubkey"));
      schema_assert(ValidateKeyPair(blockchain_item["pubkey"]));
    }

    {  // key ["blockchain"][i]["signature"]
      schema_assert(blockchain_item.contains("signature"));
      schema_assert(ValidateSignatureJson(blockchain_item["signature"]));
    }

    return true;
  }));

  return true;
}

namespace no3::package {
  bool ValidatePackageConfig(const nlohmann::ordered_json& json) {
    schema_assert(json.is_object());

    {  // key ["format"]
      schema_assert(json.contains("format"));
      schema_assert(ValidateSemVersion(json["format"]));
      schema_assert(json["format"]["major"].get<unsigned int>() == 1);
    }

    {  // key ["name"]
      schema_assert(json.contains("name"));
      schema_assert(json["name"].is_string());
    }

    {  // key ["description"]
      schema_assert(json.contains("description"));
      schema_assert(json["description"].is_string());
    }

    {  // key ["license"]
      schema_assert(json.contains("license"));
      schema_assert(json["license"].is_string());
    }

    {  // key ["category"]
      schema_assert(json.contains("category"));
      schema_assert(json["category"].is_string());
      schema_assert([&]() {
        auto v = json["category"].get<std::string>();
        schema_assert(v == "azide-lib" || v == "basic-lib" || v == "dynamic-executable" || v == "static-executable");
        return true;
      }());
    }

    {  // key ["version"]
      schema_assert(json.contains("version"));
      schema_assert(ValidateSemVersion(json["version"]));
    }

    {  // key ["contacts"]
      schema_assert(json.contains("contacts"));
      schema_assert(json["contacts"].is_array());
      schema_assert(std::all_of(json["contacts"].begin(), json["contacts"].end(), [](const auto& contact) {
        schema_assert(contact.is_object());

        {  // key ["contacts"][i]["name"]
          schema_assert(contact.contains("name"));
          schema_assert(contact["name"].is_string());
        }

        {  // key ["contacts"][i]["email"]
          schema_assert(contact.contains("email"));
          schema_assert(contact["email"].is_string());
        }

        {  // key ["contacts"][i]["phone"]
          if (contact.contains("phone")) {
            schema_assert(contact["phone"].is_string());
          }
        }

        {  // key ["contacts"][i]["roles"]
          schema_assert(contact.contains("roles"));
          schema_assert(contact["roles"].is_array());
          schema_assert(std::all_of(contact["roles"].begin(), contact["roles"].end(), [](const auto& role) {
            schema_assert(role.is_string());
            schema_assert(role == "owner" || role == "contributor" || role == "maintainer" || role == "support");
            return true;
          }));
        }

        return true;
      }));
    }

    {  // key ["platforms"]
      schema_assert(json.contains("platforms"));
      schema_assert(json["platforms"].is_object());

      {  // key ["platforms"]["allow"]
        schema_assert(json["platforms"].contains("allow"));
        schema_assert(json["platforms"]["allow"].is_array());
        schema_assert(
            std::all_of(json["platforms"]["allow"].begin(), json["platforms"]["allow"].end(), [](const auto& platform) {
              schema_assert(platform.is_string());
              return true;
            }));
      }

      {  // key ["platforms"]["deny"]
        schema_assert(json["platforms"].contains("deny"));
        schema_assert(json["platforms"]["deny"].is_array());
        schema_assert(
            std::all_of(json["platforms"]["deny"].begin(), json["platforms"]["deny"].end(), [](const auto& platform) {
              schema_assert(platform.is_string());
              return true;
            }));
      }
    }

    {  // key ["optimization"]
      schema_assert(json.contains("optimization"));
      schema_assert(ValidateBuildOptimization(json["optimization"]));
    }

    {  // key ["dependencies"]
      schema_assert(json.contains("dependencies"));
      schema_assert(json["dependencies"].is_array());
      schema_assert(std::all_of(json["dependencies"].begin(), json["dependencies"].end(), [](const auto& dependency) {
        schema_assert(dependency.is_object());

        {  // key ["dependencies"][i]["uuid"]
          schema_assert(dependency.contains("uuid"));
          schema_assert(dependency["uuid"].is_string());
          schema_assert(ValidateUUID(dependency["uuid"].template get<std::string>()));
        }

        {  // key ["dependencies"][i]["version"]
          schema_assert(dependency.contains("version"));
          schema_assert(ValidateSemVersion(dependency["version"]));
        }

        return true;
      }));
    }

    {  // key ["blockchain"]
      schema_assert(json.contains("blockchain"));
      schema_assert(ValidateBlockchain(json["blockchain"]));
    }

    return true;
  }
}  // namespace no3::package
