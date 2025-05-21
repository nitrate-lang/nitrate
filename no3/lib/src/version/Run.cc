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

#include <boost/program_options.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/uuid/name_generator_sha1.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <core/cli/Interpreter.hh>
#include <iomanip>
#include <nitrate-core/CatchAll.hh>
#include <nitrate-core/Init.hh>
#include <nlohmann/json.hpp>
#include <sstream>
#include <utility>

using namespace ncc;

enum class SoftwareComponent {
  Core,
  Lexer,
  Sequencer,
  Parser,
  IRAlpha,
  AlphaOptimizer,
  IRBeta,
  BetaOptimizer,
  IRGamma,
  GammaOptimizer,
  Codegen,
};

static const std::string NITRATE_CORE = "Core";
static const std::string NITRATE_LEXER = "Lexer";
static const std::string NITRATE_SEQUENCER = "Sequencer";
static const std::string NITRATE_PARSER = "Parser";
static const std::string NITRATE_IR_ALPHA = "IRAlpha";
static const std::string NITRATE_IR_ALPHA_OPT = "IRAlphaOpt";
static const std::string NITRATE_IR_BETA = "IRBeta";
static const std::string NITRATE_IR_BETA_OPT = "IRBetaOpt";
static const std::string NITRATE_IR_GAMMA = "IRGamma";
static const std::string NITRATE_IR_GAMMA_OPT = "IRGammaOpt";
static const std::string NITRATE_CODEGEN = "Codegen";

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

static auto GetComponentManifestForCore() -> ComponentManifest {
  const auto& lib = CoreLibrary;

  return {NITRATE_CORE,        "LGPL-2.1+",         "The Nitrate Core Library", {},
          lib.GetSemVersion(), lib.GetCommitHash(), lib.GetCompileDate(),       lib.GetBranch()};
}

static auto GetComponentManifestForLexer() -> ComponentManifest {
  const auto& lib = lex::LexerLibrary;

  return {NITRATE_LEXER,       "LGPL-2.1+",         "The Nitrate Lexer Library", {NITRATE_CORE},
          lib.GetSemVersion(), lib.GetCommitHash(), lib.GetCompileDate(),        lib.GetBranch()};
}

static auto GetComponentManifestForSequencer() -> ComponentManifest {
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

static auto GetComponentManifestForParser() -> ComponentManifest {
  const auto& lib = parse::ParseLibrary;

  return {NITRATE_PARSER,      "LGPL-2.1+",         "The Nitrate Parser Library", {NITRATE_CORE, NITRATE_LEXER},
          lib.GetSemVersion(), lib.GetCommitHash(), lib.GetCompileDate(),         lib.GetBranch()};
}

static auto GetComponentManifestForIRAlpha() -> ComponentManifest {
  return {NITRATE_IR_ALPHA,
          "LGPL-2.1+",
          "The Nitrate Alpha Intermediate Representation Library",
          {NITRATE_CORE, NITRATE_PARSER},
          {0, 0, 0},
          "",
          "",
          ""};
}

static auto GetComponentManifestForAlphaOptimizer() -> ComponentManifest {
  return {NITRATE_IR_ALPHA_OPT,
          "LGPL-2.1+",
          "The Nitrate Alpha Intermediate Representation Optimizer Library",
          {NITRATE_CORE, NITRATE_IR_ALPHA},
          {0, 0, 0},
          "",
          "",
          ""};
}

static auto GetComponentManifestForIRBeta() -> ComponentManifest {
  return {NITRATE_IR_BETA,
          "LGPL-2.1+",
          "The Nitrate Beta Intermediate Representation Library",
          {NITRATE_CORE, NITRATE_IR_ALPHA},
          {0, 0, 0},
          "",
          "",
          ""};
}

static auto GetComponentManifestForBetaOptimizer() -> ComponentManifest {
  return {NITRATE_IR_BETA_OPT,
          "LGPL-2.1+",
          "The Nitrate Beta Intermediate Representation Optimizer Library",
          {NITRATE_CORE, NITRATE_IR_BETA},
          {0, 0, 0},
          "",
          "",
          ""};
}

static auto GetComponentManifestForIRGamma() -> ComponentManifest {
  return {NITRATE_IR_GAMMA,
          "LGPL-2.1+",
          "The Nitrate Gamma Intermediate Representation Library",
          {NITRATE_CORE, NITRATE_IR_BETA},
          {0, 0, 0},
          "",
          "",
          ""};
}

static auto GetComponentManifestForGammaOptimizer() -> ComponentManifest {
  return {NITRATE_IR_GAMMA_OPT,
          "LGPL-2.1+",
          "The Nitrate Gamma Intermediate Representation Optimizer Library",
          {NITRATE_CORE, NITRATE_IR_GAMMA},
          {0, 0, 0},
          "",
          "",
          ""};
}

static auto GetComponentManifestForCodegen() -> ComponentManifest {
  return {NITRATE_CODEGEN,
          "LGPL-2.1+",
          "The Nitrate Codegen and Linking Library",
          {NITRATE_CORE, NITRATE_IR_GAMMA},
          {0, 0, 0},
          "",
          "",
          ""};
}

static auto GetComponentManifest(SoftwareComponent component) -> nlohmann::ordered_json {
  ComponentManifest manifest = [component]() {
    switch (component) {
      case SoftwareComponent::Core:
        return GetComponentManifestForCore();

      case SoftwareComponent::Lexer:
        return GetComponentManifestForLexer();

      case SoftwareComponent::Sequencer:
        return GetComponentManifestForSequencer();

      case SoftwareComponent::Parser:
        return GetComponentManifestForParser();

      case SoftwareComponent::IRAlpha:
        return GetComponentManifestForIRAlpha();

      case SoftwareComponent::AlphaOptimizer:
        return GetComponentManifestForAlphaOptimizer();

      case SoftwareComponent::IRBeta:
        return GetComponentManifestForIRBeta();

      case SoftwareComponent::BetaOptimizer:
        return GetComponentManifestForBetaOptimizer();

      case SoftwareComponent::IRGamma:
        return GetComponentManifestForIRGamma();

      case SoftwareComponent::GammaOptimizer:
        return GetComponentManifestForGammaOptimizer();

      case SoftwareComponent::Codegen:
        return GetComponentManifestForCodegen();
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

static auto GetSoftwareVersionArray(const std::vector<SoftwareComponent>& components) -> nlohmann::ordered_json {
  nlohmann::ordered_json j = nlohmann::ordered_json::array();

  for (const auto& component : components) {
    j.push_back(GetComponentManifest(component));
  }

  return j;
}

static auto GetSoftwareHash() -> std::string {
  constexpr boost::uuids::uuid kDnsNamespaceUuid = {0x85, 0xa2, 0xbc, 0x03, 0xde, 0x86, 0x49, 0x48,
                                                    0xb1, 0x6e, 0x5c, 0x63, 0x72, 0x8f, 0x38, 0x61};

  const auto versions = GetSoftwareVersionArray({
      SoftwareComponent::Core,
      SoftwareComponent::Lexer,
      SoftwareComponent::Sequencer,
      SoftwareComponent::Parser,
      SoftwareComponent::IRAlpha,
      SoftwareComponent::AlphaOptimizer,
      SoftwareComponent::IRBeta,
      SoftwareComponent::BetaOptimizer,
      SoftwareComponent::IRGamma,
      SoftwareComponent::GammaOptimizer,
      SoftwareComponent::Codegen,
  });

  // Generate the version 5 UUID
  boost::uuids::name_generator_sha1 name_gen(kDnsNamespaceUuid);
  boost::uuids::uuid uuid = name_gen(versions.dump());

  return boost::uuids::to_string(uuid);
}

static auto GetSystemInfo() -> nlohmann::ordered_json {
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

static auto GetVersionUsingJson(bool minify, bool system_info,
                                const nlohmann::ordered_json& version_array) -> std::string {
  nlohmann::ordered_json j;

  const auto microseconds_since_epoch =
      std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch())
          .count();

  j["application"] = "no3";
  j["timestamp"] = microseconds_since_epoch;
  j["uuid"] = GetSoftwareHash();

  if (system_info) {
    j["system"] = GetSystemInfo();
  } else {
    j["system"] = nullptr;
  }

  j["software"] = version_array;

  return j.dump(minify ? -1 : 2);
}

static auto GetVersionUsingBrief(const nlohmann::ordered_json& version_array) -> std::string {
  std::stringstream brief_log;

  brief_log << "╭──────────────────────────────────────────────────────────────────────────────╮\n";
  brief_log << "│ Software UUID: " << GetSoftwareHash() << "                          │\n";
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

static auto GetSoftwareComponents(const std::vector<std::string>& of) -> std::optional<std::vector<SoftwareComponent>> {
  static const std::unordered_map<std::string_view, SoftwareComponent> component_map = {
      {NITRATE_CORE, SoftwareComponent::Core},           {NITRATE_LEXER, SoftwareComponent::Lexer},
      {NITRATE_SEQUENCER, SoftwareComponent::Sequencer}, {NITRATE_PARSER, SoftwareComponent::Parser},
      {NITRATE_IR_ALPHA, SoftwareComponent::IRAlpha},    {NITRATE_IR_ALPHA_OPT, SoftwareComponent::AlphaOptimizer},
      {NITRATE_IR_BETA, SoftwareComponent::IRBeta},      {NITRATE_IR_BETA_OPT, SoftwareComponent::BetaOptimizer},
      {NITRATE_IR_GAMMA, SoftwareComponent::IRGamma},    {NITRATE_IR_GAMMA_OPT, SoftwareComponent::GammaOptimizer},
      {NITRATE_CODEGEN, SoftwareComponent::Codegen},
  };

  std::vector<SoftwareComponent> components;
  for (const auto& component_name : of) {
    auto map_it = component_map.find(component_name);
    if (map_it == component_map.end()) {
      Log << "Unknown software component: " << component_name;
      return std::nullopt;
    }

    components.push_back(map_it->second);
  }

  return components;
}

static void DisplayHelp() {
  std::string_view message = R"(Usage: version [--help] [--of VAR]... [--system-info] [--minify] [[--brief]|[--json]]

Optional arguments:
  -h, --help         shows help message and exits
  -O, --of           The software component to include version info for [may be repeated]
  -S, --system-info  Include information about the local system
  -C, --minify       Minify the output
  -B, --brief        Short human-readable output
  -J, --json         Output in JSON format
)";

  Log << Raw << message;
}

auto no3::Interpreter::PImpl::CommandVersion(ConstArguments, const MutArguments& argv) -> bool {
  namespace po = boost::program_options;

  po::options_description desc("Allowed options");

  auto add_option = desc.add_options();
  add_option("help,h", "produce help message");
  add_option("of,O", po::value<std::vector<std::string>>(), "software component to include version info for");
  add_option("system-info,S", "include information about the local system");
  add_option("minify,C", "minify the output");
  add_option("brief,B", "short human-readable output");
  add_option("json,J", "output in JSON format");

  std::vector<const char*> args;
  args.reserve(argv.size());
  for (const auto& arg : argv) {
    args.push_back(arg.c_str());
  }

  po::variables_map vm;
  if (auto cli_parser = OMNI_CATCH(po::parse_command_line(args.size(), args.data(), desc));
      !cli_parser || !OMNI_CATCH(po::store(*cli_parser, vm)) || !OMNI_CATCH(po::notify(vm))) {
    Log << Error << "Failed to parse command line arguments.";
    desc.print(*(Log << Raw));
    return false;
  };

  if (vm.contains("help")) {
    DisplayHelp();
    return true;
  }

  std::vector<std::string> of;
  if (vm.contains("of")) {
    of = vm.at("of").as<std::vector<std::string>>();
  } else {
    of = {NITRATE_CORE,     NITRATE_LEXER,        NITRATE_SEQUENCER, NITRATE_PARSER,
          NITRATE_IR_ALPHA, NITRATE_IR_ALPHA_OPT, NITRATE_IR_BETA,   NITRATE_IR_BETA_OPT,
          NITRATE_IR_GAMMA, NITRATE_IR_GAMMA_OPT, NITRATE_CODEGEN};
  }

  const auto system_info = vm.contains("system-info");
  const auto minify = vm.contains("minify");
  const auto brief = vm.contains("brief");
  const auto json = vm.contains("json");

  const auto components = GetSoftwareComponents(of);
  if (!components) {
    DisplayHelp();
    return false;
  }

  if (!json && (system_info || minify)) {
    Log << "The --system-info and --minify options are only valid when using --json";
    DisplayHelp();
    return false;
  }

  if (brief && json) {
    Log << "The --brief and --json options are mutually exclusive";
    DisplayHelp();
    return false;
  }

  const auto version_array = GetSoftwareVersionArray(*components);

  if (json) {
    Log << Raw << GetVersionUsingJson(minify, system_info, version_array) << "\n";
  } else {
    Log << Raw << GetVersionUsingBrief(version_array) << "\n";
  }

  return true;
}
