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

#include <boost/predef.h>
#include <nitrate-emit/Lib.h>

#include <boost/uuid/name_generator_sha1.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <core/InterpreterImpl.hh>
#include <core/argparse.hpp>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <nitrate-core/Init.hh>
#include <nitrate-ir/Init.hh>
#include <nitrate-lexer/Init.hh>
#include <nitrate-parser/Init.hh>
#include <nitrate-seq/Init.hh>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <sstream>
#include <utility>

using namespace ncc;

enum class SoftwareComponent {
  NitrateCore,
  NitrateLexer,
  NitrateSequencer,
  NitrateParser,
  NitrateIRAlpha,
  NitrateAlphaOptimizer,
  NitrateIRBeta,
  NitrateBetaOptimizer,
  NitrateIRGamma,
  NitrateGammaOptimizer,
  NitrateLLVM,
};

static const std::string NITRATE_CORE = "NitrateCore";
static const std::string NITRATE_LEXER = "NitrateLexer";
static const std::string NITRATE_SEQUENCER = "NitrateSequencer";
static const std::string NITRATE_PARSER = "NitrateParser";
static const std::string NITRATE_IR_ALPHA = "NitrateIRAlpha";
static const std::string NITRATE_ALPHA_OPTIMIZER = "NitrateAlphaOptimizer";
static const std::string NITRATE_IR_BETA = "NitrateIRBeta";
static const std::string NITRATE_BETA_OPTIMIZER = "NitrateBetaOptimizer";
static const std::string NITRATE_IR_GAMMA = "NitrateIRGamma";
static const std::string NITRATE_GAMMA_OPTIMIZER = "NitrateGammaOptimizer";
static const std::string NITRATE_LLVM = "NitrateLLVM";

static std::unique_ptr<argparse::ArgumentParser> CreateArgumentParser() {
  auto program = std::make_unique<argparse::ArgumentParser>("version");

  program->AddArgument("--of", "-O")
      .Help("The software component to include version info for")
      .Choices(NITRATE_CORE, NITRATE_LEXER, NITRATE_SEQUENCER, NITRATE_PARSER, NITRATE_IR_ALPHA,
               NITRATE_ALPHA_OPTIMIZER, NITRATE_IR_BETA, NITRATE_BETA_OPTIMIZER, NITRATE_IR_GAMMA,
               NITRATE_GAMMA_OPTIMIZER, NITRATE_LLVM)
      .Append();

  program->AddArgument("--system-info", "-S")
      .Help("Include information about the local system")
      .ImplicitValue(true)
      .DefaultValue(false);

  program->AddArgument("--minify", "-C").Help("Minify the output").ImplicitValue(true).DefaultValue(false);

  auto& exclude_group = program->AddMutuallyExclusiveGroup(false);
  exclude_group.AddArgument("--brief", "-B")
      .Help("Short human-readable output")
      .ImplicitValue(true)
      .DefaultValue(false);
  exclude_group.AddArgument("--json", "-J").Help("Output in JSON format").ImplicitValue(true).DefaultValue(false);

  return program;
}

static std::optional<std::vector<SoftwareComponent>> GetSoftwareComponents(const argparse::ArgumentParser& program) {
  static const std::unordered_map<std::string_view, SoftwareComponent> component_map = {
      {NITRATE_CORE, SoftwareComponent::NitrateCore},
      {NITRATE_LEXER, SoftwareComponent::NitrateLexer},
      {NITRATE_SEQUENCER, SoftwareComponent::NitrateSequencer},
      {NITRATE_PARSER, SoftwareComponent::NitrateParser},
      {NITRATE_IR_ALPHA, SoftwareComponent::NitrateIRAlpha},
      {NITRATE_ALPHA_OPTIMIZER, SoftwareComponent::NitrateAlphaOptimizer},
      {NITRATE_IR_BETA, SoftwareComponent::NitrateIRBeta},
      {NITRATE_BETA_OPTIMIZER, SoftwareComponent::NitrateBetaOptimizer},
      {NITRATE_IR_GAMMA, SoftwareComponent::NitrateIRGamma},
      {NITRATE_GAMMA_OPTIMIZER, SoftwareComponent::NitrateGammaOptimizer},
      {NITRATE_LLVM, SoftwareComponent::NitrateLLVM},
  };

  std::vector<SoftwareComponent> components;

  std::vector<std::string> params;
  try {
    if (program.IsUsed("--of")) {
      params = program.Get<std::vector<std::string>>("--of");
    }
  } catch (std::bad_any_cast&) {
    // I can't figure out the dynamic typing bug in ArgParser/my code.
    // If a single --of is provided with no value, it causes a type error.
    // This catch block is a workaround.
  }

  if (!params.empty()) {
    for (const auto& of : params) {
      auto it = component_map.find(of);
      if (it == component_map.end()) {
        Log << "Unknown software component: " << of;
        return std::nullopt;
      }

      components.push_back(it->second);
    }
  } else {
    components.push_back(SoftwareComponent::NitrateCore);
    components.push_back(SoftwareComponent::NitrateLexer);
    components.push_back(SoftwareComponent::NitrateSequencer);
    components.push_back(SoftwareComponent::NitrateParser);
    components.push_back(SoftwareComponent::NitrateIRAlpha);
    components.push_back(SoftwareComponent::NitrateAlphaOptimizer);
    components.push_back(SoftwareComponent::NitrateIRBeta);
    components.push_back(SoftwareComponent::NitrateBetaOptimizer);
    components.push_back(SoftwareComponent::NitrateIRGamma);
    components.push_back(SoftwareComponent::NitrateGammaOptimizer);
    components.push_back(SoftwareComponent::NitrateLLVM);
  }

  return components;
}

static nlohmann::ordered_json GetSystemInfo() {
  nlohmann::ordered_json info;

#if defined(BOOST_OS_LINUX) || defined(BOOST_OS_MACOS) || defined(BOOST_OS_UNIX)
  std::fstream proc_version("/proc/version", std::ios::in);
  if (proc_version.is_open()) {
    std::string line;
    std::getline(proc_version, line);
    proc_version.close();
    info["linux"]["proc"]["version"] = line;
  }

  std::fstream proc_cpuinfo("/proc/cpuinfo", std::ios::in);
  if (proc_cpuinfo.is_open()) {
    info["linux"]["proc"]["cpuinfo"] = std::string(std::istreambuf_iterator<char>(proc_cpuinfo), {});
    proc_cpuinfo.close();
  }

  std::fstream proc_meminfo("/proc/meminfo", std::ios::in);
  if (proc_meminfo.is_open()) {
    info["linux"]["proc"]["meminfo"] = std::string(std::istreambuf_iterator<char>(proc_meminfo), {});
    proc_meminfo.close();
  }

  std::fstream proc_uptime("/proc/uptime", std::ios::in);
  if (proc_uptime.is_open()) {
    std::string line;
    std::getline(proc_uptime, line);
    proc_uptime.close();
    info["linux"]["proc"]["uptime"] = line;
  }

  std::fstream proc_loadavg("/proc/loadavg", std::ios::in);
  if (proc_loadavg.is_open()) {
    std::string line;
    std::getline(proc_loadavg, line);
    proc_loadavg.close();
    info["linux"]["proc"]["loadavg"] = line;
  }

  std::fstream proc_stat("/proc/stat", std::ios::in);
  if (proc_stat.is_open()) {
    info["linux"]["proc"]["stat"] = std::string(std::istreambuf_iterator<char>(proc_stat), {});
    proc_stat.close();
  }

  std::fstream proc_diskstats("/proc/diskstats", std::ios::in);
  if (proc_diskstats.is_open()) {
    info["linux"]["proc"]["diskstats"] = std::string(std::istreambuf_iterator<char>(proc_diskstats), {});
    proc_diskstats.close();
  }

#endif

  return info;
}

static std::string GetSoftwareHash(const nlohmann::ordered_json& manifest) {
  constexpr boost::uuids::uuid kDnsNamespaceUuid = {0x85, 0xa2, 0xbc, 0x03, 0xde, 0x86, 0x49, 0x48,
                                                    0xb1, 0x6e, 0x5c, 0x63, 0x72, 0x8f, 0x38, 0x61};

  // Generate the version 5 UUID
  boost::uuids::name_generator_sha1 name_gen(kDnsNamespaceUuid);
  boost::uuids::uuid uuid = name_gen(manifest.dump());

  return boost::uuids::to_string(uuid);
}

struct ComponentManifest {
  std::string_view m_component_name;
  std::string_view m_license;
  std::string_view m_description;
  std::vector<std::string_view> m_dependencies;
  std::array<uint32_t, 3> m_version = {0, 0, 0};
  std::string_view m_commit;
  std::string_view m_build_date;
  std::string_view m_branch;

  ComponentManifest(std::string_view component_name, std::string_view license, std::string_view description,
                    std::vector<std::string_view> dependencies, std::array<uint32_t, 3> version,
                    std::string_view commit, std::string_view build_date, std::string_view branch)
      : m_component_name(component_name),
        m_license(license),
        m_description(description),
        m_dependencies(std::move(dependencies)),
        m_version(version),
        m_commit(commit),
        m_build_date(build_date),
        m_branch(branch) {}
};

static ComponentManifest GetComponentManifestForNitrateCore() {
  const auto& lib = CoreLibrary;

  return {NITRATE_CORE,        "LGPL-2.1+",         "The Nitrate Core Library", {},
          lib.GetSemVersion(), lib.GetCommitHash(), lib.GetCompileDate(),       lib.GetBranch()};
}

static ComponentManifest GetComponentManifestForNitrateLexer() {
  const auto& lib = lex::LexerLibrary;

  return {NITRATE_LEXER,       "LGPL-2.1+",         "The Nitrate Lexer Library", {NITRATE_CORE},
          lib.GetSemVersion(), lib.GetCommitHash(), lib.GetCompileDate(),        lib.GetBranch()};
}

static ComponentManifest GetComponentManifestForNitrateSequencer() {
  const auto& lib = seq::SeqLibrary;

  return {NITRATE_SEQUENCER,
          "LGPL-2.1+",
          "The Nitrate Sequencer (Preprocessor) Library",
          {NITRATE_CORE, NITRATE_LEXER},
          lib.GetSemVersion(),
          lib.GetCommitHash(),
          lib.GetCompileDate(),
          lib.GetBranch()};
}

static ComponentManifest GetComponentManifestForNitrateParser() {
  const auto& lib = parse::ParseLibrary;

  return {NITRATE_PARSER,      "LGPL-2.1+",         "The Nitrate Parser Library", {NITRATE_CORE, NITRATE_LEXER},
          lib.GetSemVersion(), lib.GetCommitHash(), lib.GetCompileDate(),         lib.GetBranch()};
}

static ComponentManifest GetComponentManifestForNitrateIRAlpha() {
  return {NITRATE_IR_ALPHA,
          "LGPL-2.1+",
          "The Nitrate Alpha Intermediate Representation Library",
          {NITRATE_CORE, NITRATE_PARSER},
          {0, 0, 0},
          "",
          "",
          ""};
}

static ComponentManifest GetComponentManifestForNitrateAlphaOptimizer() {
  return {NITRATE_ALPHA_OPTIMIZER,
          "LGPL-2.1+",
          "The Nitrate Alpha Intermediate Representation Optimizer Library",
          {NITRATE_CORE, NITRATE_IR_ALPHA},
          {0, 0, 0},
          "",
          "",
          ""};
}

static ComponentManifest GetComponentManifestForNitrateIRBeta() {
  return {NITRATE_IR_BETA,
          "LGPL-2.1+",
          "The Nitrate Beta Intermediate Representation Library",
          {NITRATE_CORE, NITRATE_IR_ALPHA},
          {0, 0, 0},
          "",
          "",
          ""};
}

static ComponentManifest GetComponentManifestForNitrateBetaOptimizer() {
  return {NITRATE_BETA_OPTIMIZER,
          "LGPL-2.1+",
          "The Nitrate Beta Intermediate Representation Optimizer Library",
          {NITRATE_CORE, NITRATE_IR_BETA},
          {0, 0, 0},
          "",
          "",
          ""};
}

static ComponentManifest GetComponentManifestForNitrateIRGamma() {
  return {NITRATE_IR_GAMMA,
          "LGPL-2.1+",
          "The Nitrate Gamma Intermediate Representation Library",
          {NITRATE_CORE, NITRATE_IR_BETA},
          {0, 0, 0},
          "",
          "",
          ""};
}

static ComponentManifest GetComponentManifestForNitrateGammaOptimizer() {
  return {NITRATE_GAMMA_OPTIMIZER,
          "LGPL-2.1+",
          "The Nitrate Gamma Intermediate Representation Optimizer Library",
          {NITRATE_CORE, NITRATE_IR_GAMMA},
          {0, 0, 0},
          "",
          "",
          ""};
}

static ComponentManifest GetComponentManifestForNitrateLLVM() {
  return {NITRATE_LLVM,
          "LGPL-2.1+",
          "The Nitrate LLVM Codegen and Linking Library",
          {NITRATE_CORE, NITRATE_IR_GAMMA},
          {0, 0, 0},
          "",
          "",
          ""};
}

static nlohmann::ordered_json GetComponentManifest(SoftwareComponent component) {
  ComponentManifest manifest = [component]() {
    switch (component) {
      case SoftwareComponent::NitrateCore:
        return GetComponentManifestForNitrateCore();

      case SoftwareComponent::NitrateLexer:
        return GetComponentManifestForNitrateLexer();

      case SoftwareComponent::NitrateSequencer:
        return GetComponentManifestForNitrateSequencer();

      case SoftwareComponent::NitrateParser:
        return GetComponentManifestForNitrateParser();

      case SoftwareComponent::NitrateIRAlpha:
        return GetComponentManifestForNitrateIRAlpha();

      case SoftwareComponent::NitrateAlphaOptimizer:
        return GetComponentManifestForNitrateAlphaOptimizer();

      case SoftwareComponent::NitrateIRBeta:
        return GetComponentManifestForNitrateIRBeta();

      case SoftwareComponent::NitrateBetaOptimizer:
        return GetComponentManifestForNitrateBetaOptimizer();

      case SoftwareComponent::NitrateIRGamma:
        return GetComponentManifestForNitrateIRGamma();

      case SoftwareComponent::NitrateGammaOptimizer:
        return GetComponentManifestForNitrateGammaOptimizer();

      case SoftwareComponent::NitrateLLVM:
        return GetComponentManifestForNitrateLLVM();
    }
  }();

  nlohmann::ordered_json j;
  j["component_name"] = manifest.m_component_name;
  j["description"] = manifest.m_description;
  j["license"] = manifest.m_license;
  j["version"] = {
      {"major", manifest.m_version[0]},
      {"minor", manifest.m_version[1]},
      {"patch", manifest.m_version[2]},
  };
  j["build"] = {
      {"commit", manifest.m_commit},
      {"date", manifest.m_build_date},
      {"branch", manifest.m_branch},
  };

  j["dependencies"] = nlohmann::ordered_json::array();
  for (const auto& dependency : manifest.m_dependencies) {
    j["dependencies"].push_back(dependency);
  }

  return j;
}

static nlohmann::ordered_json GetSoftwareVersionArray(std::span<const SoftwareComponent> components) {
  nlohmann::ordered_json j = nlohmann::ordered_json::array();

  for (const auto& component : components) {
    j.push_back(GetComponentManifest(component));
  }

  return j;
}

static std::string GetVersionUsingJson(bool minify, bool system_info, const nlohmann::ordered_json& version_array) {
  nlohmann::ordered_json j;

  const auto microseconds_since_epoch =
      std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch())
          .count();

  j["application"] = "no3";
  j["timestamp"] = microseconds_since_epoch;
  j["uuid"] = GetSoftwareHash(version_array);

  if (system_info) {
    j["system"] = GetSystemInfo();
  } else {
    j["system"] = nullptr;
  }

  j["software"] = version_array;

  return j.dump(minify ? -1 : 2);
}

static std::string GetVersionUsingBrief(const nlohmann::ordered_json& version_array) {
  std::stringstream brief_log;

  brief_log << "╭──────────────────────────────────────────────────────────────────────────────╮\n";
  brief_log << "│ Software UUID: " << GetSoftwareHash(version_array) << "                          │\n";
  brief_log << "├──────────────────────────────────────────────────────────────────────────────┤\n";

  for (const auto& component : version_array) {
    brief_log << "│ " << std::setw(24) << std::setfill(' ') << component["component_name"].get<std::string>() << " v"
              << component["version"]["major"] << "." << component["version"]["minor"] << "."
              << component["version"]["patch"];
    std::string commit = component["build"]["commit"].get<std::string>().substr(0, 8);
    std::string date = component["build"]["date"].get<std::string>();

    if (commit.empty() && date.empty()) {
      brief_log << " (unknown)                                   ";
    } else if (!commit.empty() && date.empty()) {
      brief_log << " (commit-" << commit << ", unknown)                  ";
    } else if (!date.empty() && commit.empty()) {
      brief_log << " (unknown,         " << date << ")";
    } else {
      brief_log << " (commit-" << commit << ", " << date << ")";
    }

    brief_log << " │\n";
  }

  brief_log << "╰──────────────────────────────────────────────────────────────────────────────╯";

  return brief_log.str();
}

bool no3::Interpreter::PImpl::CommandVersion(ConstArguments, const MutArguments& argv) {
  auto program = CreateArgumentParser();

  try {
    program->ParseArgs(argv);
  } catch (const std::exception& e) {
    Log << e.what();
    Log << Raw << *program;
    return false;
  }

  const auto minify = program->Get<bool>("--minify");
  const auto system_info = program->Get<bool>("--system-info");
  const auto json_mode = program->Get<bool>("--json");
  const auto components = GetSoftwareComponents(*program);
  if (!components) {
    Log << Raw << *program;
    return false;
  }

  if (!json_mode && (system_info || minify)) {
    Log << "The --system-info and --minify options are only valid when using --json";
    Log << Raw << *program;
    return false;
  }

  const auto version_array = GetSoftwareVersionArray(*components);

  if (json_mode) {
    Log << Raw << GetVersionUsingJson(minify, system_info, version_array);
  } else {
    Log << Raw << GetVersionUsingBrief(version_array);
  }

  return true;
}
