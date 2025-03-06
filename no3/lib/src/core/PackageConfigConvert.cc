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
    {"azide-lib", Package::AZIDE_LIBRARY},
    {"basic-lib", Package::BASIC_LIBRARY},
    {"dynamic-executable", Package::DYNAMIC_EXECUTABLE},
    {"static-executable", Package::STATIC_EXECUTABLE},
});

static const auto PACKAGE_CONTACT_ROLE_MAP = MakeBimap<std::string_view, Package::Contact::Role>({
    {"owner", Package::Contact::OWNER},
    {"contributor", Package::Contact::CONTRIBUTOR},
    {"maintainer", Package::Contact::MAINTAINER},
    {"support", Package::Contact::SUPPORT},
});

static const auto BLOCKCHAIN_CATEGORY_MAP = MakeBimap<std::string_view, Package::BlockchainLink::LinkCategory>({
    {"eco-root", Package::BlockchainLink::ECOSYSTEM_ROOT},
    {"eco-domain", Package::BlockchainLink::ECOSYSTEM_DOMAIN},
    {"user-account", Package::BlockchainLink::USER_ACCOUNT},
    {"package", Package::BlockchainLink::PACKAGE},
    {"subpackage", Package::BlockchainLink::SUB_PACKAGE},
});

static const auto BLOCKCHAIN_PKEY_KIND_MAP = MakeBimap<std::string_view, Package::BlockchainLink::PublicKey::Type>({
    {"ed25519", Package::BlockchainLink::PublicKey::ED25519},
});

static const auto BLOCKCHAIN_SIG_KIND_MAP = MakeBimap<std::string_view, Package::BlockchainLink::Signature::Type>({
    {"ed25519", Package::BlockchainLink::Signature::ED25519},
});

static Package::SemVersion* CreateSemVersion(Pool& mm, uint32_t major, uint32_t minor, uint32_t patch) {
  auto* sem_version = Pool::CreateMessage<Package::SemVersion>(&mm);
  sem_version->set_major(major);
  sem_version->set_minor(minor);
  sem_version->set_patch(patch);
  return sem_version;
}

static Package::Platforms* CreatePlatforms(Pool& mm, const nlohmann::ordered_json& json) {
  auto* platforms = Pool::CreateMessage<Package::Platforms>(&mm);

  for (const auto& platform : json["allow"]) {
    platforms->add_allow(platform);
  }

  for (const auto& platform : json["deny"]) {
    platforms->add_deny(platform);
  }

  return platforms;
}

static Package::UUID* CreateUUID(Pool& mm, std::string uuid) {
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
  uuid_message->set_hi(hi);
  uuid_message->set_lo(lo);

  return uuid_message;
}

static Package::OptimizationOptions::Switch* CreateOptimizationSwitch(Pool& mm, const nlohmann::ordered_json& json) {
  auto* opt_switch = Pool::CreateMessage<Package::OptimizationOptions::Switch>(&mm);

  for (const auto& alpha_opt : json["alpha"]) {
    opt_switch->add_alpha(alpha_opt);
  }

  for (const auto& beta_opt : json["beta"]) {
    opt_switch->add_beta(beta_opt);
  }

  for (const auto& gamma_opt : json["gamma"]) {
    opt_switch->add_gamma(gamma_opt);
  }

  for (const auto& llvm_opt : json["llvm"]) {
    opt_switch->add_llvm(llvm_opt);
  }

  for (const auto& lto : json["lto"]) {
    opt_switch->add_lto(lto);
  }

  for (const auto& runtime_opt : json["runtime"]) {
    opt_switch->add_runtime(runtime_opt);
  }

  return opt_switch;
}

static Package::OptimizationOptions::SystemRequirements* CreateSystemRequirements(Pool& mm,
                                                                                  const nlohmann::ordered_json& json) {
  auto* sys_req = Pool::CreateMessage<Package::OptimizationOptions::SystemRequirements>(&mm);

  sys_req->set_min_free_cores(json["min-free-cores"]);
  sys_req->set_min_free_memory(json["min-free-memory"]);
  sys_req->set_min_free_storage(json["min-free-storage"]);

  return sys_req;
}

static Package::BlockchainLink::PublicKey* CreatePublicKey(Pool& mm, const nlohmann::ordered_json& json) {
  auto* public_key = Pool::CreateMessage<Package::BlockchainLink::PublicKey>(&mm);

  public_key->set_type(BLOCKCHAIN_PKEY_KIND_MAP.left.at(json["type"].get<std::string>()));

  std::vector<uint8_t> key_bytes;
  const auto& key_str = json["value"].get<std::string>();
  for (size_t i = 0; i < key_str.size(); i += 2) {
    key_bytes.push_back(std::stoul(key_str.substr(i, 2), nullptr, 16));
  }

  public_key->set_value(key_bytes.data(), key_bytes.size());

  return public_key;
}

static Package::BlockchainLink::Signature* CreateSignature(Pool& mm, const nlohmann::ordered_json& json) {
  auto* signature = Pool::CreateMessage<Package::BlockchainLink::Signature>(&mm);

  signature->set_type(BLOCKCHAIN_SIG_KIND_MAP.left.at(json["type"].get<std::string>()));

  std::vector<uint8_t> sig_bytes;
  const auto& sig_str = json["value"].get<std::string>();
  for (size_t i = 0; i < sig_str.size(); i += 2) {
    sig_bytes.push_back(std::stoul(sig_str.substr(i, 2), nullptr, 16));
  }

  signature->set_value(sig_bytes.data(), sig_bytes.size());

  return signature;
}

static Package::BlockchainLink* CreateBlockchainLink(Pool& mm, const nlohmann::ordered_json& json) {
  auto* bc = Pool::CreateMessage<Package::BlockchainLink>(&mm);

  bc->set_category(BLOCKCHAIN_CATEGORY_MAP.left.at(json["category"].get<std::string>()));
  bc->set_allocated_uuid(CreateUUID(mm, json["uuid"]));
  bc->set_allocated_pubkey(CreatePublicKey(mm, json["pubkey"]));
  bc->set_allocated_signature(CreateSignature(mm, json["signature"]));

  return bc;
}

static std::string UUIDFromQWords(uint64_t hi, uint64_t lo) {
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

static std::string EncodeLowercaseHex(const std::string& data) {
  std::stringstream ss;
  ss << std::hex << std::setfill('0');
  for (char i : data) {
    ss << std::setw(2) << static_cast<int>(static_cast<uint8_t>(i));
  }

  return ss.str();
}

namespace no3::package {
  nitrate::no3::package::Package* ConfigJsonToProtobuf(google::protobuf::Arena& mm,
                                                       const nlohmann::ordered_json& json) {
    auto* package = Pool::CreateMessage<Package>(&mm);

    {  // Set ['format']
      uint32_t major = json["format"]["major"];
      uint32_t minor = json["format"]["minor"];
      uint32_t patch = json["format"]["patch"];

      auto* fmt_ver = CreateSemVersion(mm, major, minor, patch);
      package->set_allocated_format(fmt_ver);
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
      package->set_category(PACKAGE_CATEGORY_MAP.left.at(json["category"].get<std::string>()));
    }

    {  // Set ['version']
      uint32_t major = json["version"]["major"];
      uint32_t minor = json["version"]["minor"];
      uint32_t patch = json["version"]["patch"];

      auto* ver = CreateSemVersion(mm, major, minor, patch);
      package->set_allocated_version(ver);
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
          contact_message->add_roles(PACKAGE_CONTACT_ROLE_MAP.left.at(role.get<std::string>()));
        }

        package->add_contacts()->CopyFrom(*contact_message);
      }
    }

    {  // Set ['platforms']
      auto* platforms = CreatePlatforms(mm, json["platforms"]);
      package->set_allocated_platforms(platforms);
    }

    {  // Set ['optimization']
      auto* rapid_profile = CreateOptimizationSwitch(mm, json["optimization"]["rapid"]["switch"]);
      auto* debug_profile = CreateOptimizationSwitch(mm, json["optimization"]["debug"]["switch"]);
      auto* release_profile = CreateOptimizationSwitch(mm, json["optimization"]["release"]["switch"]);
      auto* system_req = CreateSystemRequirements(mm, json["optimization"]["requirements"]);

      auto* rapid_conf = Pool::CreateMessage<Package::OptimizationOptions::Rapid>(&mm);
      rapid_conf->set_allocated_switch_(rapid_profile);

      auto* debug_conf = Pool::CreateMessage<Package::OptimizationOptions::Debug>(&mm);
      debug_conf->set_allocated_switch_(debug_profile);

      auto* release_conf = Pool::CreateMessage<Package::OptimizationOptions::Release>(&mm);
      release_conf->set_allocated_switch_(release_profile);

      auto* opt_options = Pool::CreateMessage<Package::OptimizationOptions>(&mm);
      opt_options->set_allocated_rapid(rapid_conf);
      opt_options->set_allocated_debug(debug_conf);
      opt_options->set_allocated_release(release_conf);
      opt_options->set_allocated_requirements(system_req);

      package->set_allocated_optimization(opt_options);
    }

    {  // Set ['dependencies']
      for (const auto& dependency : json["dependencies"]) {
        uint32_t major = dependency["version"]["major"];
        uint32_t minor = dependency["version"]["minor"];
        uint32_t patch = dependency["version"]["patch"];
        auto* ver = CreateSemVersion(mm, major, minor, patch);

        auto* dep = Pool::CreateMessage<Package::Dependency>(&mm);
        auto* uuid = CreateUUID(mm, dependency["uuid"]);
        dep->set_allocated_uuid(uuid);
        dep->set_allocated_version(ver);

        package->add_dependencies()->CopyFrom(*dep);
      }
    }

    {  // Set ['blockchain']
      for (const auto& blockchain : json["blockchain"]) {
        auto* bc = CreateBlockchainLink(mm, blockchain);
        package->add_blockchain()->CopyFrom(*bc);
      }
    }

    package->CheckInitialized();

    return package;
  }

  std::optional<nlohmann::ordered_json> ConfigProtobufToJson(const nitrate::no3::package::Package& protobuf) {
    nlohmann::ordered_json j;

    j["format"]["major"] = protobuf.format().major();
    j["format"]["minor"] = protobuf.format().minor();
    j["format"]["patch"] = protobuf.format().patch();
    j["name"] = protobuf.name();
    j["description"] = protobuf.description();
    j["license"] = protobuf.license();

    if (PACKAGE_CATEGORY_MAP.right.count(protobuf.category()) == 0) {
      Log << "Unknown package category: " << protobuf.category();
      return std::nullopt;
    }
    j["category"] = PACKAGE_CATEGORY_MAP.right.at(protobuf.category());

    j["version"]["major"] = protobuf.version().major();
    j["version"]["minor"] = protobuf.version().minor();
    j["version"]["patch"] = protobuf.version().patch();

    for (const auto& contact : protobuf.contacts()) {
      nlohmann::ordered_json contact_json;
      contact_json["name"] = contact.name();
      contact_json["email"] = contact.email();

      if (contact.has_phone()) {
        contact_json["phone"] = contact.phone();
      }

      for (auto role : contact.roles()) {
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
    for (const auto& platform : protobuf.platforms().allow()) {
      platforms_json["allow"].push_back(platform);
    }
    for (const auto& platform : protobuf.platforms().deny()) {
      platforms_json["deny"].push_back(platform);
    }
    j["platforms"] = std::move(platforms_json);

    for (const auto& dependency : protobuf.dependencies()) {
      nlohmann::ordered_json dep_json;
      dep_json["uuid"] = UUIDFromQWords(dependency.uuid().hi(), dependency.uuid().lo());
      dep_json["version"]["major"] = dependency.version().major();
      dep_json["version"]["minor"] = dependency.version().minor();
      dep_json["version"]["patch"] = dependency.version().patch();
      j["dependencies"].push_back(std::move(dep_json));
    }

    nlohmann::ordered_json optimization_json;
    optimization_json["rapid"]["switch"]["alpha"] = protobuf.optimization().rapid().switch_().alpha();
    optimization_json["rapid"]["switch"]["beta"] = protobuf.optimization().rapid().switch_().beta();
    optimization_json["rapid"]["switch"]["gamma"] = protobuf.optimization().rapid().switch_().gamma();
    optimization_json["rapid"]["switch"]["llvm"] = protobuf.optimization().rapid().switch_().llvm();
    optimization_json["rapid"]["switch"]["lto"] = protobuf.optimization().rapid().switch_().lto();
    optimization_json["rapid"]["switch"]["runtime"] = protobuf.optimization().rapid().switch_().runtime();

    optimization_json["debug"]["switch"]["alpha"] = protobuf.optimization().debug().switch_().alpha();
    optimization_json["debug"]["switch"]["beta"] = protobuf.optimization().debug().switch_().beta();
    optimization_json["debug"]["switch"]["gamma"] = protobuf.optimization().debug().switch_().gamma();
    optimization_json["debug"]["switch"]["llvm"] = protobuf.optimization().debug().switch_().llvm();
    optimization_json["debug"]["switch"]["lto"] = protobuf.optimization().debug().switch_().lto();
    optimization_json["debug"]["switch"]["runtime"] = protobuf.optimization().debug().switch_().runtime();

    optimization_json["release"]["switch"]["alpha"] = protobuf.optimization().release().switch_().alpha();
    optimization_json["release"]["switch"]["beta"] = protobuf.optimization().release().switch_().beta();
    optimization_json["release"]["switch"]["gamma"] = protobuf.optimization().release().switch_().gamma();
    optimization_json["release"]["switch"]["llvm"] = protobuf.optimization().release().switch_().llvm();
    optimization_json["release"]["switch"]["lto"] = protobuf.optimization().release().switch_().lto();
    optimization_json["release"]["switch"]["runtime"] = protobuf.optimization().release().switch_().runtime();

    optimization_json["requirements"]["min-free-cores"] = protobuf.optimization().requirements().min_free_cores();
    optimization_json["requirements"]["min-free-memory"] = protobuf.optimization().requirements().min_free_memory();
    optimization_json["requirements"]["min-free-storage"] = protobuf.optimization().requirements().min_free_storage();

    j["optimization"] = std::move(optimization_json);

    for (const auto& blockchain : protobuf.blockchain()) {
      if (BLOCKCHAIN_CATEGORY_MAP.right.count(blockchain.category()) == 0) {
        Log << "Unknown blockchain category: " << blockchain.category();
        return std::nullopt;
      }

      if (BLOCKCHAIN_PKEY_KIND_MAP.right.count(blockchain.pubkey().type()) == 0) {
        Log << "Unknown blockchain public key type: " << blockchain.pubkey().type();
        return std::nullopt;
      }

      if (BLOCKCHAIN_SIG_KIND_MAP.right.count(blockchain.signature().type()) == 0) {
        Log << "Unknown blockchain signature type: " << blockchain.signature().type();
        return std::nullopt;
      }

      nlohmann::ordered_json bc_json;
      bc_json["category"] = BLOCKCHAIN_CATEGORY_MAP.right.at(blockchain.category());
      bc_json["uuid"] = UUIDFromQWords(blockchain.uuid().hi(), blockchain.uuid().lo());

      nlohmann::ordered_json pubkey_json;
      pubkey_json["type"] = BLOCKCHAIN_PKEY_KIND_MAP.right.at(blockchain.pubkey().type());
      pubkey_json["value"] = EncodeLowercaseHex(blockchain.pubkey().value());
      bc_json["pubkey"] = std::move(pubkey_json);

      nlohmann::ordered_json sig_json;
      sig_json["type"] = BLOCKCHAIN_SIG_KIND_MAP.right.at(blockchain.signature().type());
      sig_json["value"] = EncodeLowercaseHex(blockchain.signature().value());
      bc_json["signature"] = std::move(sig_json);

      j["blockchain"].push_back(std::move(bc_json));
    }

    return j;
  }

}  // namespace no3::package
