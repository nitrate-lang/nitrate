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

#include <nitrate-emit/Lib.h>

#include <core/InterpreterImpl.hh>
#include <core/argparse.hpp>
#include <iostream>
#include <memory>
#include <nitrate-core/Assert.hh>
#include <nitrate-core/Init.hh>
#include <nitrate-ir/Init.hh>
#include <nitrate-lexer/Init.hh>
#include <nitrate-parser/Init.hh>
#include <nitrate-seq/Init.hh>
#include <nlohmann/json.hpp>

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
  auto program = std::make_unique<argparse::ArgumentParser>("find");

  program->AddArgument("--of", "-O")
      .Help("The software component to include version info for")
      .Append()
      .Choices(NITRATE_CORE, NITRATE_LEXER, NITRATE_SEQUENCER, NITRATE_PARSER, NITRATE_IR_ALPHA,
               NITRATE_ALPHA_OPTIMIZER, NITRATE_IR_BETA, NITRATE_BETA_OPTIMIZER, NITRATE_IR_GAMMA,
               NITRATE_GAMMA_OPTIMIZER, NITRATE_LLVM);

  program->AddArgument("--system-info", "-S")
      .Help("Include information about the local system")
      .ImplicitValue(true)
      .DefaultValue(false);

  program->AddArgument("--minify", "-C").Help("Minify the output").ImplicitValue(true).DefaultValue(false);

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

  if (program.IsUsed("--of")) {
    for (const auto& of : program.Get<std::vector<std::string>>("--of")) {
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
  /// TODO: Implement system info

  return info;
}

static nlohmann::ordered_json GetComponentManifest(SoftwareComponent component);

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
  const auto components = GetSoftwareComponents(*program);
  if (!components) {
    Log << Raw << *program;
    return false;
  }

  nlohmann::ordered_json j;

  uint64_t microseconds_since_epoch =
      std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch())
          .count();

  j["metadata"]["timestamp"] = microseconds_since_epoch;
  j["metadata"]["application"] = "no3";

  if (system_info) {
    j["system"] = GetSystemInfo();
  } else {
    j["system"] = nullptr;
  }

  j["software"] = nlohmann::ordered_json::array();
  for (const auto& component : *components) {
    j["software"].push_back(GetComponentManifest(component));
  }

  if (minify) {
    std::cout << j.dump();
  } else {
    std::cout << j.dump(2);
  }

  return true;
}

struct ComponentManifest {
  std::string m_component_name;
  std::string m_license;
  std::string m_description;
  std::vector<std::string> m_dependencies;
  std::string m_tag;
  std::array<uint32_t, 3> m_version = {0, 0, 0};
  std::optional<std::string> m_commit;
  std::optional<std::string> m_branch;
};

static ComponentManifest GetComponentManifestForNitrateCore() {
  ComponentManifest r;
  r.m_component_name = NITRATE_CORE;
  r.m_license = "LGPL-2.1+";
  r.m_description = "The Nitrate Core Library";
  r.m_dependencies = {};
  r.m_tag = CoreLibrary.GetVersion();
  r.m_version = {CoreLibrary.GetMajorVersion(), CoreLibrary.GetMinorVersion(), CoreLibrary.GetPatchVersion()};

  return r;
}

static ComponentManifest GetComponentManifestForNitrateLexer() {
  ComponentManifest r;
  r.m_component_name = NITRATE_LEXER;
  r.m_license = "LGPL-2.1+";
  r.m_description = "The Nitrate Lexer Library";
  r.m_dependencies = {NITRATE_CORE};
  r.m_tag = lex::LexerLibrary.GetVersion();

  return r;
}

static ComponentManifest GetComponentManifestForNitrateSequencer() {
  ComponentManifest r;
  r.m_component_name = NITRATE_SEQUENCER;
  r.m_license = "LGPL-2.1+";
  r.m_description = "The Nitrate Sequencer (Preprocessor) Library";
  r.m_dependencies = {NITRATE_CORE, NITRATE_LEXER};
  r.m_tag = seq::SeqLibrary.GetVersion();

  return r;
}

static ComponentManifest GetComponentManifestForNitrateParser() {
  ComponentManifest r;
  r.m_component_name = NITRATE_PARSER;
  r.m_license = "LGPL-2.1+";
  r.m_description = "The Nitrate Parser Library";
  r.m_dependencies = {NITRATE_CORE, NITRATE_LEXER};
  r.m_tag = parse::ParseLibrary.GetVersion();

  return r;
}

static ComponentManifest GetComponentManifestForNitrateIRAlpha() {
  ComponentManifest r;
  r.m_component_name = NITRATE_IR_ALPHA;
  r.m_license = "LGPL-2.1+";
  r.m_description = "The Nitrate Alpha Intermediate Representation Library";
  r.m_dependencies = {NITRATE_CORE, NITRATE_PARSER};

  return r;
}

static ComponentManifest GetComponentManifestForNitrateAlphaOptimizer() {
  ComponentManifest r;
  r.m_component_name = NITRATE_ALPHA_OPTIMIZER;
  r.m_license = "LGPL-2.1+";
  r.m_description = "The Nitrate Alpha Intermediate Representation Optimizer Library";
  r.m_dependencies = {NITRATE_CORE, NITRATE_IR_ALPHA};

  return r;
}

static ComponentManifest GetComponentManifestForNitrateIRBeta() {
  ComponentManifest r;
  r.m_component_name = NITRATE_IR_BETA;
  r.m_license = "LGPL-2.1+";
  r.m_description = "The Nitrate Beta Intermediate Representation Library";
  r.m_dependencies = {NITRATE_CORE, NITRATE_IR_ALPHA};

  return r;
}

static ComponentManifest GetComponentManifestForNitrateBetaOptimizer() {
  ComponentManifest r;
  r.m_component_name = NITRATE_BETA_OPTIMIZER;
  r.m_license = "LGPL-2.1+";
  r.m_description = "The Nitrate Beta Intermediate Representation Optimizer Library";
  r.m_dependencies = {NITRATE_CORE, NITRATE_IR_BETA};

  return r;
}

static ComponentManifest GetComponentManifestForNitrateIRGamma() {
  ComponentManifest r;
  r.m_component_name = NITRATE_IR_GAMMA;
  r.m_license = "LGPL-2.1+";
  r.m_description = "The Nitrate Gamma Intermediate Representation Library";
  r.m_dependencies = {NITRATE_CORE, NITRATE_IR_BETA};

  return r;
}

static ComponentManifest GetComponentManifestForNitrateGammaOptimizer() {
  ComponentManifest r;
  r.m_component_name = NITRATE_GAMMA_OPTIMIZER;
  r.m_license = "LGPL-2.1+";
  r.m_description = "The Nitrate Gamma Intermediate Representation Optimizer Library";
  r.m_dependencies = {NITRATE_CORE, NITRATE_IR_GAMMA};

  return r;
}

static ComponentManifest GetComponentManifestForNitrateLLVM() {
  ComponentManifest r;
  r.m_component_name = NITRATE_LLVM;
  r.m_license = "LGPL-2.1+";
  r.m_description = "The Nitrate LLVM Backend Library";
  r.m_dependencies = {NITRATE_CORE, NITRATE_IR_GAMMA};

  return r;
}

static nlohmann::ordered_json GetComponentManifest(SoftwareComponent component) {
  ComponentManifest manifest;

  switch (component) {
    case SoftwareComponent::NitrateCore: {
      manifest = GetComponentManifestForNitrateCore();
      break;
    }

    case SoftwareComponent::NitrateLexer: {
      manifest = GetComponentManifestForNitrateLexer();
      break;
    }

    case SoftwareComponent::NitrateSequencer: {
      manifest = GetComponentManifestForNitrateSequencer();
      break;
    }

    case SoftwareComponent::NitrateParser: {
      manifest = GetComponentManifestForNitrateParser();
      break;
    }

    case SoftwareComponent::NitrateIRAlpha: {
      manifest = GetComponentManifestForNitrateIRAlpha();
      break;
    }

    case SoftwareComponent::NitrateAlphaOptimizer: {
      manifest = GetComponentManifestForNitrateAlphaOptimizer();
      break;
    }

    case SoftwareComponent::NitrateIRBeta: {
      manifest = GetComponentManifestForNitrateIRBeta();
      break;
    }

    case SoftwareComponent::NitrateBetaOptimizer: {
      manifest = GetComponentManifestForNitrateBetaOptimizer();
      break;
    }

    case SoftwareComponent::NitrateIRGamma: {
      manifest = GetComponentManifestForNitrateIRGamma();
      break;
    }

    case SoftwareComponent::NitrateGammaOptimizer: {
      manifest = GetComponentManifestForNitrateGammaOptimizer();
      break;
    }

    case SoftwareComponent::NitrateLLVM: {
      manifest = GetComponentManifestForNitrateLLVM();
      break;
    }
  }

  nlohmann::ordered_json j;
  j["component_name"] = manifest.m_component_name;
  j["description"] = manifest.m_description;
  j["license"] = manifest.m_license;
  j["version"] = {
      {"major", manifest.m_version[0]},
      {"minor", manifest.m_version[1]},
      {"patch", manifest.m_version[2]},
      {"tag", manifest.m_tag},
  };
  if (manifest.m_commit) {
    j["build"]["commit"] = *manifest.m_commit;
  } else {
    j["build"]["commit"] = nullptr;
  }
  if (manifest.m_branch) {
    j["build"]["branch"] = *manifest.m_branch;
  } else {
    j["build"]["branch"] = nullptr;
  }
  j["dependencies"] = nlohmann::ordered_json::array();
  for (const auto& dependency : manifest.m_dependencies) {
    j["dependencies"].push_back(dependency);
  }

  return j;
}
