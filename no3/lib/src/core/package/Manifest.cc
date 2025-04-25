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

#include <core/package/Manifest.hh>
#include <core/static/SPDX.hh>
#include <nitrate-core/Assert.hh>
#include <nitrate-core/Logger.hh>
#include <nlohmann/json.hpp>
#include <regex>
#include <set>
#include <sstream>

using namespace ncc;
using namespace no3::package;

namespace no3::package::check {
#define schema_assert(__expr)                                                    \
  if (!(__expr)) [[unlikely]] {                                                  \
    ncc::Log << "Invalid configuration:" << " schema_assert(" << #__expr << ")"; \
    return false;                                                                \
  }

  static auto ValidateUUID(const std::string& uuid) -> bool {
    schema_assert(uuid.size() == 36);
    schema_assert(uuid[8] == '-');
    schema_assert(uuid[13] == '-');
    schema_assert(uuid[18] == '-');
    schema_assert(uuid[23] == '-');

    schema_assert(std::all_of(uuid.begin(), uuid.end(), [](char c) { return std::isxdigit(c) || c == '-'; }));

    return true;
  }

  static auto ValidateEd25519PublicKey(const std::string& value) -> bool {
    schema_assert(value.size() == 64);
    schema_assert(std::all_of(value.begin(), value.end(), [](char c) { return std::isxdigit(c); }));
    return true;
  }

  static auto ValidateKeyPair(const nlohmann::ordered_json& json) -> bool {
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

  static auto ValidateEd25519Signature(const std::string& value) -> bool {
    schema_assert(value.size() == 128);
    schema_assert(std::all_of(value.begin(), value.end(), [](char c) { return std::isxdigit(c); }));
    return true;
  }

  static auto ValidateSignatureJson(const nlohmann::ordered_json& json) -> bool {
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

  static auto ValidateSemVersion(const nlohmann::ordered_json& json) -> bool {
    static std::regex semver_regex(R"(^\d+\.\d+(\.\d+)?$)");

    schema_assert(json.is_string());
    schema_assert(std::regex_match(json.get<std::string>(), semver_regex));

    return true;
  }

  static auto ValidateBuildOptimizationSwitch(const nlohmann::ordered_json& json) -> bool {
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

  static auto ValidateBuildOptimization(const nlohmann::ordered_json& json) -> bool {
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

      {  // key ["optimization"]["requirements"]["min-cores"]
        schema_assert(json["requirements"].contains("min-cores"));
        schema_assert(json["requirements"]["min-cores"].is_number_unsigned());
      }

      {  // key ["optimization"]["requirements"]["min-memory"]
        schema_assert(json["requirements"].contains("min-memory"));
        schema_assert(json["requirements"]["min-memory"].is_number_unsigned());
      }

      {  // key ["optimization"]["requirements"]["min-storage"]
        schema_assert(json["requirements"].contains("min-storage"));
        schema_assert(json["requirements"]["min-storage"].is_number_unsigned());
      }
    }

    return true;
  }

  static auto ValidateBlockchain(const nlohmann::ordered_json& json) -> bool {
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

  static bool VerifyUntrustedJSON(const nlohmann::json& j) {
    schema_assert(j.is_object());

    {  // key ["format"]
      schema_assert(j.contains("format"));
      schema_assert(ValidateSemVersion(j["format"]));
      schema_assert(j["format"].get<std::string>().starts_with("1."));
    }

    {  // key ["name"]
      schema_assert(j.contains("name"));
      schema_assert(j["name"].is_string());
      schema_assert(Manifest::IsValidName(j["name"].get<std::string>()));
    }

    {  // key ["description"]
      schema_assert(j.contains("description"));
      schema_assert(j["description"].is_string());
    }

    {  // key ["license"]
      schema_assert(j.contains("license"));
      schema_assert(j["license"].is_string());
      schema_assert(Manifest::IsValidLicense(j["license"].get<std::string>()));
    }

    {  // key ["category"]
      schema_assert(j.contains("category"));
      schema_assert(j["category"].is_string());
      schema_assert([&]() {
        auto v = j["category"].get<std::string>();
        schema_assert(v == "exe" || v == "lib" || v == "std");
        return true;
      }());
    }

    {  // key ["version"]
      schema_assert(j.contains("version"));
      schema_assert(ValidateSemVersion(j["version"]));
    }

    {  // key ["contacts"]
      schema_assert(j.contains("contacts"));
      schema_assert(j["contacts"].is_array());
      schema_assert(std::all_of(j["contacts"].begin(), j["contacts"].end(), [](const auto& contact) {
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
      schema_assert(j.contains("platforms"));
      schema_assert(j["platforms"].is_object());

      {  // key ["platforms"]["allow"]
        schema_assert(j["platforms"].contains("allow"));
        schema_assert(j["platforms"]["allow"].is_array());
        schema_assert(
            std::all_of(j["platforms"]["allow"].begin(), j["platforms"]["allow"].end(), [](const auto& platform) {
              schema_assert(platform.is_string());
              return true;
            }));
      }

      {  // key ["platforms"]["deny"]
        schema_assert(j["platforms"].contains("deny"));
        schema_assert(j["platforms"]["deny"].is_array());
        schema_assert(
            std::all_of(j["platforms"]["deny"].begin(), j["platforms"]["deny"].end(), [](const auto& platform) {
              schema_assert(platform.is_string());
              return true;
            }));
      }
    }

    {  // key ["optimization"]
      schema_assert(j.contains("optimization"));
      schema_assert(ValidateBuildOptimization(j["optimization"]));
    }

    {  // key ["dependencies"]
      schema_assert(j.contains("dependencies"));
      schema_assert(j["dependencies"].is_array());
      schema_assert(std::all_of(j["dependencies"].begin(), j["dependencies"].end(), [](const auto& dependency) {
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
      schema_assert(j.contains("blockchain"));
      schema_assert(ValidateBlockchain(j["blockchain"]));
    }

    return true;
  }

}  // namespace no3::package::check

namespace no3::package::convert {
  static auto EncodeSemanticVersion(uint32_t major, uint32_t minor, uint32_t patch) -> std::string {
    std::stringstream ss;
    ss << major << '.' << minor;
    if (patch != 0) {
      ss << '.' << patch;
    }
    return ss.str();
  }

  static auto ConvertCategory(const std::string& category) -> Manifest::Category {
    if (category == "std") {
      return Manifest::Category::StandardLibrary;
    }

    if (category == "lib") {
      return Manifest::Category::Library;
    }

    qcore_assert(category == "exe");
    return Manifest::Category::Executable;
  }

  static auto ConvertContactRole(const std::string& role) -> Manifest::Contact::Role {
    if (role == "owner") {
      return Manifest::Contact::Role::Owner;
    }

    if (role == "contributor") {
      return Manifest::Contact::Role::Contributor;
    }

    if (role == "maintainer") {
      return Manifest::Contact::Role::Maintainer;
    }

    qcore_assert(role == "support");
    return Manifest::Contact::Role::Support;
  }

  static auto ConvertSemanticVersion(const nlohmann::json& j) -> Manifest::Version {
    uint32_t major = 0;
    uint32_t minor = 0;
    uint32_t patch = 0;

    const auto version = j.get<std::string>();
    const auto pos1 = version.find('.');
    const auto pos2 = version.find('.', pos1 + 1);
    major = std::stoul(version.substr(0, pos1));
    minor = std::stoul(version.substr(pos1 + 1, pos2 - pos1 - 1));
    if (pos2 != std::string::npos) {
      patch = std::stoul(version.substr(pos2 + 1));
    }

    return Manifest::Version(major, minor, patch);
  }

  static auto ConvertContact(const nlohmann::json& j) -> Manifest::Contact {
    auto name = j["name"].get<std::string>();
    auto email = j["email"].get<std::string>();
    auto roles = [&]() {
      std::set<Manifest::Contact::Role> roles;
      for (const auto& role : j["roles"]) {
        roles.insert(ConvertContactRole(role.get<std::string>()));
      }
      return roles;
    };

    auto c = Manifest::Contact(std::move(name), std::move(email), roles());
    if (j.contains("phone")) {
      c.SetPhone(j["phone"].get<std::string>());
    }

    return c;
  }

  static auto ConvertPlatforms(const nlohmann::json& j) -> Manifest::Platforms {
    auto allow = [&]() {
      std::vector<std::string> allow;
      allow.reserve(j["allow"].size());
      for (const auto& platform : j["allow"]) {
        allow.push_back(platform.get<std::string>());
      }
      return allow;
    }();

    auto deny = [&]() {
      std::vector<std::string> deny;
      deny.reserve(j["deny"].size());
      for (const auto& platform : j["deny"]) {
        deny.push_back(platform.get<std::string>());
      }
      return deny;
    }();

    return {std::move(allow), std::move(deny)};
  }

  static auto ConvertOptimizationSwitchStage(const nlohmann::json& j) -> std::set<std::string> {
    std::set<std::string> v;
    for (const auto& opt_flag : j) {
      v.insert(opt_flag.get<std::string>());
    }
    return v;
  }

  static auto ConvertOptimizationSwitch(const nlohmann::json& j) -> Manifest::Optimization::Switch {
    return Manifest::Optimization::Switch(
        ConvertOptimizationSwitchStage(j["alpha"]), ConvertOptimizationSwitchStage(j["beta"]),
        ConvertOptimizationSwitchStage(j["gamma"]), ConvertOptimizationSwitchStage(j["llvm"]),
        ConvertOptimizationSwitchStage(j["lto"]), ConvertOptimizationSwitchStage(j["runtime"]));
  }

  static auto ConvertOptimizationRequirements(const nlohmann::json& j) -> Manifest::Optimization::Requirements {
    return Manifest::Optimization::Requirements(j["min-cores"].get<uint32_t>(), j["min-memory"].get<uint32_t>(),
                                                j["min-storage"].get<uint32_t>());
  }

  static auto ConvertOptimization(const nlohmann::json& j) -> Manifest::Optimization {
    Manifest::Optimization optimization;

    optimization.SetRequirements(ConvertOptimizationRequirements(j["requirements"]));

    for (const auto& [key, value] : j.items()) {
      if (key != "requirements") {
        optimization.SetProfile(key, ConvertOptimizationSwitch(value["switch"]));
      }
    }

    return optimization;
  }

  static auto ConvertDependency(const nlohmann::json& j) -> Manifest::Dependency {
    return {j["uuid"].get<std::string>(), ConvertSemanticVersion(j["version"])};
  }

}  // namespace no3::package::convert

static auto ObjectToInstance(const nlohmann::json& j, Manifest& m) -> Manifest& {
  m.SetName(j["name"].get<std::string>());
  m.SetDescription(j["description"].get<std::string>());
  m.SetLicense(j["license"].get<std::string>());
  m.SetCategory(convert::ConvertCategory(j["category"].get<std::string>()));
  m.SetVersion(convert::ConvertSemanticVersion(j["version"]));
  for (const auto& contact : j["contacts"]) {
    m.GetContacts().emplace_back(convert::ConvertContact(contact));
  }
  m.SetPlatforms(convert::ConvertPlatforms(j["platforms"]));
  m.SetOptimization(convert::ConvertOptimization(j["optimization"]));
  for (const auto& dependency : j["dependencies"]) {
    m.GetDependencies().emplace_back(convert::ConvertDependency(dependency));
  }

  /// TODO: Implement cryptographic dependency ownership blockchain

  return m;
}

auto Manifest::ToJson(std::ostream& os, bool& correct_schema, bool minify) const -> std::ostream& {
  correct_schema = false;

  nlohmann::ordered_json j;

  j["format"] = "1.0";

  j["name"] = GetName();
  j["description"] = GetDescription();
  j["license"] = GetLicense();
  j["category"] = [&] {
    switch (GetCategory()) {
      case Category::StandardLibrary:
        return "std";
      case Category::Library:
        return "lib";
      case Category::Executable:
        return "exe";
    }
  }();
  auto ver = GetVersion();
  j["version"] = convert::EncodeSemanticVersion(ver.GetMajor(), ver.GetMinor(), ver.GetPatch());
  j["contacts"] = [&] {
    nlohmann::ordered_json contacts = nlohmann::json::array();
    for (const auto& contact : GetContacts()) {
      nlohmann::ordered_json j_contact;
      j_contact["name"] = contact.GetName();
      j_contact["email"] = contact.GetEmail();
      if (contact.ContainsPhone()) {
        j_contact["phone"] = contact.GetPhone().value();
      }
      j_contact["roles"] = nlohmann::json::array();
      for (const auto& role : contact.GetRoles()) {
        switch (role) {
          case Contact::Role::Owner:
            j_contact["roles"].push_back("owner");
            break;
          case Contact::Role::Contributor:
            j_contact["roles"].push_back("contributor");
            break;
          case Contact::Role::Maintainer:
            j_contact["roles"].push_back("maintainer");
            break;
          case Contact::Role::Support:
            j_contact["roles"].push_back("support");
            break;
        }
      }
      contacts.push_back(std::move(j_contact));
    }
    return contacts;
  }();
  j["platforms"]["allow"] = GetPlatforms().GetAllow();
  j["platforms"]["deny"] = GetPlatforms().GetDeny();
  j["optimization"]["rapid"]["switch"] = [&] {
    const auto& rapid = GetOptimization().GetRapid();
    nlohmann::ordered_json j_rapid;
    j_rapid["alpha"] = rapid.GetAlpha();
    j_rapid["beta"] = rapid.GetBeta();
    j_rapid["gamma"] = rapid.GetGamma();
    j_rapid["llvm"] = rapid.GetLLVM();
    j_rapid["lto"] = rapid.GetLTO();
    j_rapid["runtime"] = rapid.GetRuntime();
    return j_rapid;
  }();
  j["optimization"]["debug"]["switch"] = [&] {
    const auto& debug = GetOptimization().GetDebug();
    nlohmann::ordered_json j_debug;
    j_debug["alpha"] = debug.GetAlpha();
    j_debug["beta"] = debug.GetBeta();
    j_debug["gamma"] = debug.GetGamma();
    j_debug["llvm"] = debug.GetLLVM();
    j_debug["lto"] = debug.GetLTO();
    j_debug["runtime"] = debug.GetRuntime();
    return j_debug;
  }();
  j["optimization"]["release"]["switch"] = [&] {
    const auto& release = GetOptimization().GetRelease();
    nlohmann::ordered_json j_release;
    j_release["alpha"] = release.GetAlpha();
    j_release["beta"] = release.GetBeta();
    j_release["gamma"] = release.GetGamma();
    j_release["llvm"] = release.GetLLVM();
    j_release["lto"] = release.GetLTO();
    j_release["runtime"] = release.GetRuntime();
    return j_release;
  }();
  const auto& requirements = GetOptimization().GetRequirements();
  j["optimization"]["requirements"]["min-cores"] = requirements.GetMinCores();
  j["optimization"]["requirements"]["min-memory"] = requirements.GetMinMemory();
  j["optimization"]["requirements"]["min-storage"] = requirements.GetMinStorage();
  j["dependencies"] = [&] {
    nlohmann::ordered_json j_dependencies = nlohmann::json::array();
    for (const auto& dependency : GetDependencies()) {
      nlohmann::ordered_json j_dependency;
      j_dependency["uuid"] = dependency.GetUUID();
      j_dependency["version"] = convert::EncodeSemanticVersion(
          dependency.GetVersion().GetMajor(), dependency.GetVersion().GetMinor(), dependency.GetVersion().GetPatch());
      j_dependencies.push_back(std::move(j_dependency));
    }
    return j_dependencies;
  }();
  j["blockchain"] = [&] {
    nlohmann::ordered_json j_blockchain = nlohmann::json::array();
    /// TODO: Implement cryptographic dependency ownership blockchain
    return j_blockchain;
  }();

  // We need to ensure fields like "name" and "license" have acceptable values
  // because they are not validated upon calls to SetName(), SetLicense(), etc.
  correct_schema = check::VerifyUntrustedJSON(j);
  os << j.dump(minify ? -1 : 2);

  return os;
}

auto Manifest::ToJson(bool& correct_schema, bool minify) const -> std::string {
  std::ostringstream oss;
  ToJson(oss, correct_schema, minify);
  return oss.str();
}

auto Manifest::FromJson(std::istream& is) -> std::optional<Manifest> {
  const auto j = nlohmann::json::parse(is, nullptr, false, true);
  if (j.is_discarded()) [[unlikely]] {
    return std::nullopt;
  }

  if (!check::VerifyUntrustedJSON(j)) [[unlikely]] {
    return std::nullopt;
  }

  Manifest manifest;
  return ObjectToInstance(j, manifest);
}

auto Manifest::FromJson(std::string_view json) -> std::optional<Manifest> {
  const auto j = nlohmann::json::parse(json, nullptr, false, true);
  if (j.is_discarded()) [[unlikely]] {
    return std::nullopt;
  }

  if (!check::VerifyUntrustedJSON(j)) [[unlikely]] {
    return std::nullopt;
  }

  Manifest manifest;
  return ObjectToInstance(j, manifest);
}

static constexpr std::string_view kManifestNameRegex =
    R"(^@([a-z]+-)?([a-zA-Z0-9]+|[a-zA-Z0-9][a-zA-Z0-9-]+[a-zA-Z0-9])\/([a-zA-Z0-9][a-zA-Z0-9-]{1,30}[a-zA-Z0-9])(:\d+)?$)";

static const std::regex PACKAGE_NAME_PATTERN(kManifestNameRegex.data(), kManifestNameRegex.size());

auto Manifest::IsValidLicense(std::string_view license) -> bool {
  if (!no3::constants::IsExactSPDXLicenseMatch(license)) {
    Log << Trace << "Failed to find match in SPDX license table: \"" << license << "\"";
    return false;
  }

  return true;
}

auto Manifest::IsValidName(std::string_view name) -> bool {
  if (!std::regex_match(std::string(name), PACKAGE_NAME_PATTERN)) {
    Log << Trace << "Package name failed format validation [regex mismatch]: \"" << name << "\"";
    return false;
  }

  // I couldn't find a way to do this with a regex, so I'm doing it manually.
  if (name.find("--") != std::string::npos) {
    Log << Trace << "Package name failed format validation [double hyphen]: \"" << name << "\"";
    return false;
  }

  // Only standard library packages are allowed to omit their Git provider prefix.
  const auto package_username = name.substr(1, name.find('/') - 1);
  const auto maybe_standard_lib = name.starts_with("@std/");
  if (!maybe_standard_lib && package_username.find('-') == std::string::npos) {
    Log << Trace << "Package name failed format validation [missing Git provider prefix]: \"" << name << "\"";
    return false;
  }

  return true;
}

auto Manifest::GetNameRegex() -> std::string_view { return kManifestNameRegex; }
