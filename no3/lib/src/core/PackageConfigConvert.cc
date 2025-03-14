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

#include <boost/bimap.hpp>
#include <core/PackageConfig.hh>
#include <nitrate-core/Logger.hh>
#include <nlohmann/json_fwd.hpp>

using namespace ncc;
using namespace no3::package;
using namespace nitrate::no3::package;
using Pool = google::protobuf::Arena;

template <typename L, typename R>
auto MakeBimap(std::initializer_list<typename boost::bimap<L, R>::value_type> list) -> boost::bimap<L, R> {
  return boost::bimap<L, R>(list.begin(), list.end());
}

static const auto PACKAGE_CATEGORY_MAP = MakeBimap<std::string_view, Package::Category>({
    {"app", Package::kApplication},
    {"lib", Package::kLibrary},
    {"std", Package::kStandardLibrary},
});

static const auto PACKAGE_CONTACT_ROLE_MAP = MakeBimap<std::string_view, Package::Contact::Role>({
    {"owner", Package::Contact::kOwner},
    {"contributor", Package::Contact::kContributor},
    {"maintainer", Package::Contact::kMaintainer},
    {"support", Package::Contact::kSupport},
});

static const auto BLOCKCHAIN_CATEGORY_MAP = MakeBimap<std::string_view, Package::BlockchainLink::LinkCategory>({
    {"eco-root", Package::BlockchainLink::kEcosystemRoot},
    {"eco-domain", Package::BlockchainLink::kEcosystemDomain},
    {"user-account", Package::BlockchainLink::kUserAccount},
    {"package", Package::BlockchainLink::kPackage},
    {"subpackage", Package::BlockchainLink::kSubPackage},
});

static const auto BLOCKCHAIN_PKEY_KIND_MAP = MakeBimap<std::string_view, Package::BlockchainLink::PublicKey::Type>({
    {"ed25519", Package::BlockchainLink::PublicKey::kED25519},
});

static const auto BLOCKCHAIN_SIG_KIND_MAP = MakeBimap<std::string_view, Package::BlockchainLink::Signature::Type>({
    {"ed25519", Package::BlockchainLink::Signature::kED25519},
});

static auto CreateSemVersion(Pool& mm, uint32_t major, uint32_t minor, uint32_t patch) -> Package::SemVersion* {
  auto* sem_version = Pool::CreateMessage<Package::SemVersion>(&mm);
  sem_version->SetMajor(major);
  sem_version->SetMinor(minor);
  sem_version->SetPatch(patch);
  return sem_version;
}

static auto CreatePlatforms(Pool& mm, const nlohmann::ordered_json& json) -> Package::Platforms* {
  auto* platforms = Pool::CreateMessage<Package::Platforms>(&mm);

  for (const auto& platform : json["allow"]) {
    platforms->AddAllow(platform);
  }

  for (const auto& platform : json["deny"]) {
    platforms->AddDeny(platform);
  }

  return platforms;
}

static auto CreateUUID(Pool& mm, std::string uuid) -> Package::UUID* {
  std::array<uint8_t, 16> uuid_bytes;
  uint64_t hi = 0;
  uint64_t lo = 0;

  std::erase(uuid, '-');

  for (size_t i = 0; i < 16; i++) {
    std::string byte_str = uuid.substr(i * 2, 2);
    uuid_bytes[i] = static_cast<uint8_t>(std::stoul(byte_str, nullptr, 16));
  }

  for (size_t i = 0; i < 8; i++) {
    hi = (hi << 8) | uuid_bytes[i];
  }

  for (size_t i = 8; i < 16; i++) {
    lo = (lo << 8) | uuid_bytes[i];
  }

  auto* uuid_message = Pool::CreateMessage<Package::UUID>(&mm);
  uuid_message->SetHi(hi);
  uuid_message->SetLo(lo);

  return uuid_message;
}

static auto CreateOptimizationSwitch(Pool& mm, const nlohmann::ordered_json& json) -> Package::OptimizationOptions::Switch* {
  auto* opt_switch = Pool::CreateMessage<Package::OptimizationOptions::Switch>(&mm);

  for (const auto& alpha_opt : json["alpha"]) {
    opt_switch->AddAlpha(alpha_opt);
  }

  for (const auto& beta_opt : json["beta"]) {
    opt_switch->AddBeta(beta_opt);
  }

  for (const auto& gamma_opt : json["gamma"]) {
    opt_switch->AddGamma(gamma_opt);
  }

  for (const auto& llvm_opt : json["llvm"]) {
    opt_switch->AddLlvm(llvm_opt);
  }

  for (const auto& lto : json["lto"]) {
    opt_switch->AddLto(lto);
  }

  for (const auto& runtime_opt : json["runtime"]) {
    opt_switch->AddRuntime(runtime_opt);
  }

  return opt_switch;
}

static auto CreateSystemRequirements(Pool& mm,
                                                                                  const nlohmann::ordered_json& json) -> Package::OptimizationOptions::SystemRequirements* {
  auto* sys_req = Pool::CreateMessage<Package::OptimizationOptions::SystemRequirements>(&mm);

  sys_req->SetMinFreeCores(json["min-free-cores"]);
  sys_req->SetMinFreeMemory(json["min-free-memory"]);
  sys_req->SetMinFreeStorage(json["min-free-storage"]);

  return sys_req;
}

static auto CreatePublicKey(Pool& mm, const nlohmann::ordered_json& json) -> Package::BlockchainLink::PublicKey* {
  auto* public_key = Pool::CreateMessage<Package::BlockchainLink::PublicKey>(&mm);

  public_key->SetType(BLOCKCHAIN_PKEY_KIND_MAP.left.at(json["type"].get<std::string>()));

  std::vector<uint8_t> key_bytes;
  const auto& key_str = json["value"].get<std::string>();
  for (size_t i = 0; i < key_str.size(); i += 2) {
    key_bytes.push_back(std::stoul(key_str.substr(i, 2), nullptr, 16));
  }

  public_key->set_value(key_bytes.data(), key_bytes.size());

  return public_key;
}

static auto CreateSignature(Pool& mm, const nlohmann::ordered_json& json) -> Package::BlockchainLink::Signature* {
  auto* signature = Pool::CreateMessage<Package::BlockchainLink::Signature>(&mm);

  signature->SetType(BLOCKCHAIN_SIG_KIND_MAP.left.at(json["type"].get<std::string>()));

  std::vector<uint8_t> sig_bytes;
  const auto& sig_str = json["value"].get<std::string>();
  for (size_t i = 0; i < sig_str.size(); i += 2) {
    sig_bytes.push_back(std::stoul(sig_str.substr(i, 2), nullptr, 16));
  }

  signature->set_value(sig_bytes.data(), sig_bytes.size());

  return signature;
}

static auto CreateBlockchainLink(Pool& mm, const nlohmann::ordered_json& json) -> Package::BlockchainLink* {
  auto* bc = Pool::CreateMessage<Package::BlockchainLink>(&mm);

  bc->SetCategory(BLOCKCHAIN_CATEGORY_MAP.left.at(json["category"].get<std::string>()));
  bc->SetAllocatedUuid(CreateUUID(mm, json["uuid"]));
  bc->SetAllocatedPubkey(CreatePublicKey(mm, json["pubkey"]));
  bc->SetAllocatedSignature(CreateSignature(mm, json["signature"]));

  return bc;
}

static auto UUIDFromQWords(uint64_t hi, uint64_t lo) -> std::string {
  std::stringstream ss;

  ss << std::setw(8) << std::setfill('0') << std::hex << ((hi >> 32) & 0xFFFFFFFF);
  ss << '-';
  ss << std::setw(4) << std::setfill('0') << std::hex << ((hi >> 16) & 0xFFFF);
  ss << '-';
  ss << std::setw(4) << std::setfill('0') << std::hex << (hi & 0xFFFF);
  ss << '-';
  ss << std::setw(4) << std::setfill('0') << std::hex << ((lo >> 48) & 0xFFFF);
  ss << '-';
  ss << std::setw(12) << std::setfill('0') << std::hex << (lo & 0xFFFFFFFFFFFF);

  return ss.str();
}

static auto EncodeLowercaseHex(const std::string& data) -> std::string {
  std::stringstream ss;
  ss << std::hex << std::setfill('0');
  for (char i : data) {
    ss << std::setw(2) << static_cast<int>(static_cast<uint8_t>(i));
  }

  return ss.str();
}

namespace no3::package {
  auto ConfigJsonToProtobuf(google::protobuf::Arena& mm,
                                                       const nlohmann::ordered_json& json) -> nitrate::no3::package::Package* {
    auto* package = Pool::CreateMessage<Package>(&mm);

    {  // Set ['format']
      uint32_t major = json["format"]["major"];
      uint32_t minor = json["format"]["minor"];
      uint32_t patch = json["format"]["patch"];

      auto* fmt_ver = CreateSemVersion(mm, major, minor, patch);
      package->SetAllocatedFormat(fmt_ver);
    }

    {  // Set ['name']
      package->set_name(json["name"]);
    }

    {  // Set ['description']
      package->set_description(json["description"]);
    }

    {  // Set ['license']
      package->set_license(json["license"]);
    }

    {  // Set ['category']
      package->SetCategory(PACKAGE_CATEGORY_MAP.left.at(json["category"].get<std::string>()));
    }

    {  // Set ['version']
      uint32_t major = json["version"]["major"];
      uint32_t minor = json["version"]["minor"];
      uint32_t patch = json["version"]["patch"];

      auto* ver = CreateSemVersion(mm, major, minor, patch);
      package->SetAllocatedVersion(ver);
    }

    {  // Set ['contacts']
      for (const auto& contact : json["contacts"]) {
        auto* contact_message = Pool::CreateMessage<Package::Contact>(&mm);

        contact_message->set_name(contact["name"]);
        contact_message->set_email(contact["email"]);

        if (contact.contains("phone")) {
          contact_message->set_phone(contact["phone"]);
        }

        for (const auto& role : contact["roles"]) {
          contact_message->AddRoles(PACKAGE_CONTACT_ROLE_MAP.left.at(role.get<std::string>()));
        }

        package->AddContacts()->CopyFrom(*contact_message);
      }
    }

    {  // Set ['platforms']
      auto* platforms = CreatePlatforms(mm, json["platforms"]);
      package->SetAllocatedPlatforms(platforms);
    }

    {  // Set ['optimization']
      auto* rapid_profile = CreateOptimizationSwitch(mm, json["optimization"]["rapid"]["switch"]);
      auto* debug_profile = CreateOptimizationSwitch(mm, json["optimization"]["debug"]["switch"]);
      auto* release_profile = CreateOptimizationSwitch(mm, json["optimization"]["release"]["switch"]);
      auto* system_req = CreateSystemRequirements(mm, json["optimization"]["requirements"]);

      auto* rapid_conf = Pool::CreateMessage<Package::OptimizationOptions::Rapid>(&mm);
      rapid_conf->SetAllocatedSwitch(rapid_profile);

      auto* debug_conf = Pool::CreateMessage<Package::OptimizationOptions::Debug>(&mm);
      debug_conf->SetAllocatedSwitch(debug_profile);

      auto* release_conf = Pool::CreateMessage<Package::OptimizationOptions::Release>(&mm);
      release_conf->SetAllocatedSwitch(release_profile);

      auto* opt_options = Pool::CreateMessage<Package::OptimizationOptions>(&mm);
      opt_options->SetAllocatedRapid(rapid_conf);
      opt_options->SetAllocatedDebug(debug_conf);
      opt_options->SetAllocatedRelease(release_conf);
      opt_options->SetAllocatedRequirements(system_req);

      package->SetAllocatedOptimization(opt_options);
    }

    {  // Set ['dependencies']
      for (const auto& dependency : json["dependencies"]) {
        uint32_t major = dependency["version"]["major"];
        uint32_t minor = dependency["version"]["minor"];
        uint32_t patch = dependency["version"]["patch"];
        auto* ver = CreateSemVersion(mm, major, minor, patch);

        auto* dep = Pool::CreateMessage<Package::Dependency>(&mm);
        auto* uuid = CreateUUID(mm, dependency["uuid"]);
        dep->SetAllocatedUuid(uuid);
        dep->SetAllocatedVersion(ver);

        package->AddDependencies()->CopyFrom(*dep);
      }
    }

    {  // Set ['blockchain']
      for (const auto& blockchain : json["blockchain"]) {
        auto* bc = CreateBlockchainLink(mm, blockchain);
        package->AddBlockchain()->CopyFrom(*bc);
      }
    }

    package->CheckInitialized();

    return package;
  }

  auto ConfigProtobufToJson(const nitrate::no3::package::Package& protobuf) -> std::optional<nlohmann::ordered_json> {
    nlohmann::ordered_json j;

    j["format"]["major"] = protobuf.Format().Major();
    j["format"]["minor"] = protobuf.Format().Minor();
    j["format"]["patch"] = protobuf.Format().Patch();
    j["name"] = protobuf.Name();
    j["description"] = protobuf.Description();
    j["license"] = protobuf.License();

    if (PACKAGE_CATEGORY_MAP.right.count(protobuf.Category()) == 0) {
      Log << "Unknown package category: " << protobuf.Category();
      return std::nullopt;
    }
    j["category"] = PACKAGE_CATEGORY_MAP.right.at(protobuf.Category());

    j["version"]["major"] = protobuf.Version().Major();
    j["version"]["minor"] = protobuf.Version().Minor();
    j["version"]["patch"] = protobuf.Version().Patch();

    for (const auto& contact : protobuf.Contacts()) {
      nlohmann::ordered_json contact_json;
      contact_json["name"] = contact.Name();
      contact_json["email"] = contact.Email();

      if (contact.HasPhone()) {
        contact_json["phone"] = contact.Phone();
      }

      for (auto role : contact.Roles()) {
        auto r = static_cast<Package::Contact::Role>(role);
        if (PACKAGE_CONTACT_ROLE_MAP.right.count(r) == 0) {
          Log << "Unknown contact role: " << role;
          return std::nullopt;
        }
        contact_json["roles"].push_back(PACKAGE_CONTACT_ROLE_MAP.right.at(r));
      }

      j["contacts"].push_back(std::move(contact_json));
    }

    nlohmann::ordered_json platforms_json;
    for (const auto& platform : protobuf.Platforms().Allow()) {
      platforms_json["allow"].push_back(platform);
    }
    for (const auto& platform : protobuf.Platforms().Deny()) {
      platforms_json["deny"].push_back(platform);
    }
    j["platforms"] = std::move(platforms_json);

    for (const auto& dependency : protobuf.Dependencies()) {
      nlohmann::ordered_json dep_json;
      dep_json["uuid"] = UUIDFromQWords(dependency.Uuid().Hi(), dependency.Uuid().Lo());
      dep_json["version"]["major"] = dependency.Version().Major();
      dep_json["version"]["minor"] = dependency.Version().Minor();
      dep_json["version"]["patch"] = dependency.Version().Patch();
      j["dependencies"].push_back(std::move(dep_json));
    }

    nlohmann::ordered_json optimization_json;
    optimization_json["rapid"]["switch"]["alpha"] = protobuf.Optimization().Rapid().Switch().Alpha();
    optimization_json["rapid"]["switch"]["beta"] = protobuf.Optimization().Rapid().Switch().Beta();
    optimization_json["rapid"]["switch"]["gamma"] = protobuf.Optimization().Rapid().Switch().Gamma();
    optimization_json["rapid"]["switch"]["llvm"] = protobuf.Optimization().Rapid().Switch().Llvm();
    optimization_json["rapid"]["switch"]["lto"] = protobuf.Optimization().Rapid().Switch().Lto();
    optimization_json["rapid"]["switch"]["runtime"] = protobuf.Optimization().Rapid().Switch().Runtime();

    optimization_json["debug"]["switch"]["alpha"] = protobuf.Optimization().Debug().Switch().Alpha();
    optimization_json["debug"]["switch"]["beta"] = protobuf.Optimization().Debug().Switch().Beta();
    optimization_json["debug"]["switch"]["gamma"] = protobuf.Optimization().Debug().Switch().Gamma();
    optimization_json["debug"]["switch"]["llvm"] = protobuf.Optimization().Debug().Switch().Llvm();
    optimization_json["debug"]["switch"]["lto"] = protobuf.Optimization().Debug().Switch().Lto();
    optimization_json["debug"]["switch"]["runtime"] = protobuf.Optimization().Debug().Switch().Runtime();

    optimization_json["release"]["switch"]["alpha"] = protobuf.Optimization().Release().Switch().Alpha();
    optimization_json["release"]["switch"]["beta"] = protobuf.Optimization().Release().Switch().Beta();
    optimization_json["release"]["switch"]["gamma"] = protobuf.Optimization().Release().Switch().Gamma();
    optimization_json["release"]["switch"]["llvm"] = protobuf.Optimization().Release().Switch().Llvm();
    optimization_json["release"]["switch"]["lto"] = protobuf.Optimization().Release().Switch().Lto();
    optimization_json["release"]["switch"]["runtime"] = protobuf.Optimization().Release().Switch().Runtime();

    optimization_json["requirements"]["min-free-cores"] = protobuf.Optimization().Requirements().MinFreeCores();
    optimization_json["requirements"]["min-free-memory"] = protobuf.Optimization().Requirements().MinFreeMemory();
    optimization_json["requirements"]["min-free-storage"] = protobuf.Optimization().Requirements().MinFreeStorage();

    j["optimization"] = std::move(optimization_json);

    for (const auto& blockchain : protobuf.Blockchain()) {
      if (BLOCKCHAIN_CATEGORY_MAP.right.count(blockchain.Category()) == 0) {
        Log << "Unknown blockchain category: " << blockchain.Category();
        return std::nullopt;
      }

      if (BLOCKCHAIN_PKEY_KIND_MAP.right.count(blockchain.Pubkey().Type()) == 0) {
        Log << "Unknown blockchain public key type: " << blockchain.Pubkey().Type();
        return std::nullopt;
      }

      if (BLOCKCHAIN_SIG_KIND_MAP.right.count(blockchain.Signature().Type()) == 0) {
        Log << "Unknown blockchain signature type: " << blockchain.Signature().Type();
        return std::nullopt;
      }

      nlohmann::ordered_json bc_json;
      bc_json["category"] = BLOCKCHAIN_CATEGORY_MAP.right.at(blockchain.Category());
      bc_json["uuid"] = UUIDFromQWords(blockchain.Uuid().Hi(), blockchain.Uuid().Lo());

      nlohmann::ordered_json pubkey_json;
      pubkey_json["type"] = BLOCKCHAIN_PKEY_KIND_MAP.right.at(blockchain.Pubkey().Type());
      pubkey_json["value"] = EncodeLowercaseHex(blockchain.Pubkey().Value());
      bc_json["pubkey"] = std::move(pubkey_json);

      nlohmann::ordered_json sig_json;
      sig_json["type"] = BLOCKCHAIN_SIG_KIND_MAP.right.at(blockchain.Signature().Type());
      sig_json["value"] = EncodeLowercaseHex(blockchain.Signature().Value());
      bc_json["signature"] = std::move(sig_json);

      j["blockchain"].push_back(std::move(bc_json));
    }

    return j;
  }

  static auto SplitSemVer(const std::string& version) -> std::tuple<size_t, size_t, size_t> {
    size_t major = 0;
    size_t minor = 0;
    size_t patch = 0;

    std::stringstream ss(version);
    ss >> major;
    ss.ignore(1);
    ss >> minor;
    ss.ignore(1);
    ss >> patch;

    return {major, minor, patch};
  }

  auto PackageConfig::CreateInitialConfiguration(const std::string& name,
                                                                   const std::string& description,
                                                                   const std::string& license,
                                                                   const std::string& version,
                                                                   PackageCategory category) -> nlohmann::ordered_json {
    nlohmann::ordered_json j;

    j["format"]["major"] = 1;
    j["format"]["minor"] = 0;
    j["format"]["patch"] = 0;

    j["name"] = name;

    j["description"] = description;

    j["license"] = license;

    j["category"] = [&]() {
      switch (category) {
        case no3::package::PackageCategory::Executable:
          return "app";
        case no3::package::PackageCategory::Library:
          return "lib";
        case no3::package::PackageCategory::StandardLibrary:
          return "std";
      }
    }();

    {
      auto [major, minor, patch] = SplitSemVer(version);
      j["version"]["major"] = major;
      j["version"]["minor"] = minor;
      j["version"]["patch"] = patch;
    }

    j["contacts"] = nlohmann::json::array();

    j["platforms"]["allow"] = {"*"};
    j["platforms"]["deny"] = {"*"};

    j["optimization"]["rapid"]["switch"] = {
        {"alpha", {"-O0"}}, {"beta", {"-O0"}}, {"gamma", {"-O0"}},
        {"llvm", {"-O1"}},  {"lto", {"-O0"}},  {"runtime", {"-O0"}},
    };

    j["optimization"]["debug"]["switch"] = {
        {"alpha", {"-O2"}}, {"beta", {"-O2"}}, {"gamma", {"-O2"}},
        {"llvm", {"-O3"}},  {"lto", {"-O0"}},  {"runtime", {"-O1"}},
    };

    j["optimization"]["release"]["switch"] = {
        {"alpha", {"-O3"}}, {"beta", {"-O3"}}, {"gamma", {"-O3"}},
        {"llvm", {"-O3"}},  {"lto", {"-O3"}},  {"runtime", {"-O3"}},
    };

    j["optimization"]["requirements"] = {
        {"min-free-cores", 1},
        {"min-free-memory", 1048576},
        {"min-free-storage", 1048576},
    };

    j["dependencies"] = nlohmann::json::array();
    j["blockchain"] = nlohmann::json::array();

    return j;
  }

}  // namespace no3::package
