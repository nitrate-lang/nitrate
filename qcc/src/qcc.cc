#include <filesystem>
#include <functional>
#include <iostream>
#include <map>
#include <optional>
#include <quixcc/Quix.hpp>
#include <string>
#include <thread>
#include <vector>

constexpr const char *VERSION_STR =
    "qcc version 0.1.0 [2024-03] (Debian 10.0.0-1)";
constexpr const char *FULL_LICENSE =
    R"(This file is part of QUIX Compiler Suite.
Copyright (C) 2024 Wesley C. Jones

The QUIX Compiler Suite is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2.1 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.)";

static std::string program_name;

enum class OperatingMode {
  ERROR,
  DISP_HELP,
  DISP_VERSION,
  DISP_LICENSE,

  IR,
  C11,
  IR_BITCODE,
  LEX,
  PARSE_TREE,

  ASSEMBLY,
  OBJECT,
  EXECUTABLE,

  DOCUMENTATION,
  QUIXCC_TARGET_DEFAULT = OperatingMode::EXECUTABLE
};

struct Options {
  std::vector<std::string> include_paths;
  std::vector<std::string> library_paths;
  std::vector<std::string> libraries;
  std::vector<std::string> undefines;
  std::vector<std::string> settings;
  std::vector<std::string> sources;
  std::string target_triple;
  std::string cpu;
  std::map<std::string, std::string> defines;
  std::string output;
  quixcc::Verbosity verbosity;
  quixcc::OptimizationLevel optimization;
  bool debug;
  OperatingMode mode;

  Options() {
    verbosity = quixcc::Verbosity::NORMAL;
    optimization = (quixcc::OptimizationLevel)-1;
    debug = false;
    mode = OperatingMode::QUIXCC_TARGET_DEFAULT;
    output = "a.out";
  }

  bool validate() {
    switch (mode) {
      case OperatingMode::ERROR:
        return false;
      case OperatingMode::DISP_HELP:
      case OperatingMode::DISP_VERSION:
      case OperatingMode::DISP_LICENSE:
        return true;
      case OperatingMode::IR:
      case OperatingMode::IR_BITCODE:
      case OperatingMode::C11:
      case OperatingMode::LEX:
        if (output.empty()) {
          std::cerr << "Error: missing output file" << std::endl;
          return false;
        }
        if (sources.empty()) {
          std::cerr << "Error: missing input files" << std::endl;
          return false;
        }
        if (optimization != quixcc::OptimizationLevel::NONE) {
          std::cerr << "Warning: ignoring optimization flags for IR generation"
                    << std::endl;
          optimization = quixcc::OptimizationLevel::NONE;
        }
        return true;
      case OperatingMode::PARSE_TREE:
        if (output.empty()) {
          std::cerr << "Error: missing output file" << std::endl;
          return false;
        }
        if (sources.empty()) {
          std::cerr << "Error: missing input files" << std::endl;
          return false;
        }
        if (optimization != quixcc::OptimizationLevel::NONE) {
          std::cerr << "Warning: ignoring optimization flags for parse tree "
                       "generation"
                    << std::endl;
          optimization = quixcc::OptimizationLevel::NONE;
        }
        return true;
      case OperatingMode::ASSEMBLY:
      case OperatingMode::OBJECT:
        if (output.empty()) {
          std::cerr << "Error: missing output file" << std::endl;
          return false;
        }
        if (sources.empty()) {
          std::cerr << "Error: missing input files" << std::endl;
          return false;
        }
        if (optimization == quixcc::OptimizationLevel::SPEED_4) {
          std::cerr << "Warning: -O4 optimization is being decreased to -O3 "
                       "for assembly and object generation"
                    << std::endl;
          optimization = quixcc::OptimizationLevel::SPEED_3;
        }
        return true;
      case OperatingMode::EXECUTABLE:
        if (output.empty()) {
          std::cerr << "Error: missing output file" << std::endl;
          return false;
        }
        if (sources.empty()) {
          std::cerr << "Error: missing input files" << std::endl;
          return false;
        }
        return true;
      default:
        return false;
    }
  }
};

static void println(const std::string &msg = "") {
  std::cout << msg << std::endl;
}

static void print_help() {
  println("Usage: " + program_name + " [options] file...\n");
  println("Options:");
  println("  -h, --help                Display this message");
  println("  -v, --version             Display version information");
  println("  --license                 Display license information");
  println("  -o, --output <file>       Output to <file>");
  println("  -I<dir>                   Add <dir> to the include path");
  println("  -L<dir>                   Add <dir> to the library path");
  println(
      "  -f<opt>                   Compiler settings. See docs for more info");
  println("  -l<name>                  Link with library <name>");
  println(
      "  -D<name>[=<value>]        Define macro <name> with optional <value>");
  println("  -U<name>                  Undefine macro <name>");
  println("  -T, --target <triple>     Specify the LLVM target triple");
  println("  -C, --cpu <cpu>           Specify the LLVM target CPU");
  println(
      "  -emit-ir                  Emit the intermediate representation (LLVM "
      "IR)");
  println(
      "  -emit-c11                 Emit the intermediate representation (LLVM "
      "IR)");
  println(
      "  -emit-bc                  Emit the intermediate representation (LLVM "
      "bitcode)");
  println("  -emit-tokens              Emit the tokenized source");
  println("  -emit-parse               Emit the parse tree");
  println("  -S                        Compile only; do not assemble or link");
  println("  -c                        Compile and assemble, but do not link");
  println("  -g                        Generate debug information");
  println("  -O0                       Disable all optimizations");
  println("  -Os                       Optimize for size");
  println("  -O1                       Optimize for speed (level 1); default");
  println("  -O2                       Optimize for speed (level 2)");
  println("  -O3                       Optimize for speed (level 3)");
  println(
      "  -O4                       Optimize -O3 and enable link-time "
      "optimization");
  println("  -vv                       Enable verbose output");
  println(
      "  -vvv                      Enable very verbose output; includes "
      "compiler internals");
  println();
  println("Examples:");
  println(" Just build a single source and output `a.out`:");
  println("   " + program_name + " source.q");
  println();
  println(" Build a single source and output to a specific file:");
  println("   " + program_name + " source.q -o myapp");
  println();
  println(
      " Build a single source and output to a specific file with debug "
      "information:");
  println("   " + program_name + " source.q -o myapp -g");
  println();
  println(
      " Build a multiple sources with maximum optimizations, link a library, "
      "and output to a specific file:");
  println("   " + program_name + " source1.q source2.q -o myapp -O4 -lcrypto");
  println();
  println("(See license for legal information)");
}

static void print_version() { println(VERSION_STR); }

static void print_license() {
  println(VERSION_STR);
  println();
  println(FULL_LICENSE);
}

static bool init(const std::vector<std::string> &args) {
  if (args.empty()) {
    std::cerr << "Internal error: 0 arguments" << std::endl;
    return false;
  }

  if (!quixcc_init()) {
    std::cerr << "Failed to initialize QUIXCC" << std::endl;
    return false;
  }

  return true;
}

static std::optional<Options> parse_options(
    const std::vector<std::string> &args) {
  Options options;

  if (args.size() == 1) {
    std::cerr << "Error: no input files" << std::endl;
    options.mode = OperatingMode::DISP_HELP;
    return options;
  }

  auto it = args.begin() + 1;

  for (; it != args.end(); ++it) {
    if (*it == "-h" || *it == "--help") {
      options.mode = OperatingMode::DISP_HELP;
      return options;
    } else if (*it == "-v" || *it == "--version") {
      options.mode = OperatingMode::DISP_VERSION;
      return options;
    } else if (*it == "--license") {
      options.mode = OperatingMode::DISP_LICENSE;
      return options;
    } else if (*it == "-o" || *it == "--output") {
      if (++it == args.end()) {
        std::cerr << "Error: missing argument for -o" << std::endl;
        return std::nullopt;
      }

      options.output = *it;
    } else if (it->starts_with("-I")) {
      auto sub = it->substr(2);
      if (sub.empty()) {
        std::cerr << "Error: missing argument for -I" << std::endl;
        return std::nullopt;
      }
      options.include_paths.push_back(sub);
    } else if (it->starts_with("-L")) {
      auto sub = it->substr(2);
      if (sub.empty()) {
        std::cerr << "Error: missing argument for -L" << std::endl;
        return std::nullopt;
      }
      options.library_paths.push_back(sub);
    } else if (it->starts_with("-f")) {
      if (it->size() == 2) {
        std::cerr << "Error: missing argument for -f" << std::endl;
        return std::nullopt;
      }
      options.settings.push_back(*it);
    } else if (it->starts_with("-l")) {
      auto sub = it->substr(2);
      if (sub.empty()) {
        std::cerr << "Error: missing argument for -l" << std::endl;
        return std::nullopt;
      }
      options.libraries.push_back(sub);
    } else if (it->starts_with("-D")) {
      std::string define = it->substr(2);
      if (define.empty()) {
        std::cerr << "Error: missing argument for -D" << std::endl;
        return std::nullopt;
      }

      auto pos = define.find('=');

      if (pos == std::string::npos)
        options.defines[define] = "1";
      else
        options.defines[define.substr(0, pos)] = define.substr(pos + 1);
    } else if (it->starts_with("-U")) {
      auto sub = it->substr(2);
      if (sub.empty()) {
        std::cerr << "Error: missing argument for -U" << std::endl;
        return std::nullopt;
      }
      options.undefines.push_back(sub);
    } else if (*it == "-T" || *it == "--target") {
      if (++it == args.end()) {
        std::cerr << "Error: missing argument for -T" << std::endl;
        return std::nullopt;
      }

      options.target_triple = *it;
    } else if (*it == "-C" || *it == "--cpu") {
      if (++it == args.end()) {
        std::cerr << "Error: missing argument for -C" << std::endl;
        return std::nullopt;
      }

      options.cpu = *it;
    } else if (*it == "-emit-ir") {
      options.mode = OperatingMode::IR;
    } else if (*it == "-emit-c11") {
      options.mode = OperatingMode::C11;
    } else if (*it == "-emit-bc") {
      options.mode = OperatingMode::IR_BITCODE;
    } else if (*it == "-emit-tokens") {
      options.mode = OperatingMode::LEX;
    } else if (*it == "-emit-parse") {
      options.mode = OperatingMode::PARSE_TREE;
    } else if (*it == "-S") {
      options.mode = OperatingMode::ASSEMBLY;
    } else if (*it == "-c") {
      options.mode = OperatingMode::OBJECT;
    } else if (*it == "-g") {
      options.debug = true;
    } else if (*it == "-O0") {
      options.optimization = quixcc::OptimizationLevel::NONE;
    } else if (*it == "-Os") {
      options.optimization = quixcc::OptimizationLevel::SIZE;
    } else if (*it == "-O1") {
      options.optimization = quixcc::OptimizationLevel::SPEED_1;
    } else if (*it == "-O2") {
      options.optimization = quixcc::OptimizationLevel::SPEED_2;
    } else if (*it == "-O3") {
      options.optimization = quixcc::OptimizationLevel::SPEED_3;
    } else if (*it == "-O4") {
      options.optimization = quixcc::OptimizationLevel::SPEED_4;
    } else if (*it == "-vv") {
      options.verbosity = quixcc::Verbosity::VERBOSE;
    } else if (*it == "-vvv") {
      options.verbosity = quixcc::Verbosity::VERY_VERBOSE;
    } else {
      if (!std::filesystem::exists(*it)) {
        std::cerr << "Error: unknown option " << *it << std::endl;
        return std::nullopt;
      }

      options.sources.push_back(*it);
    }
  }

  // set default optimization level to -O1 if not specified
  switch (options.mode) {
    case OperatingMode::ASSEMBLY:
    case OperatingMode::OBJECT:
    case OperatingMode::EXECUTABLE:
      if ((int)options.optimization == -1)
        options.optimization = quixcc::OptimizationLevel::SPEED_1;
      break;
    default:
      if ((int)options.optimization == -1)
        options.optimization = quixcc::OptimizationLevel::NONE;
  }

  return options;
}

int main(int argc, char *argv[]) {
  std::vector<std::string> args(argv, argv + argc);

  if (program_name = args[0], !init(args)) return 1;

  std::optional<Options> options;

  if (options = parse_options(args), !options.has_value()) return 2;

  if (!options->validate()) return 3;

  switch (options->mode) {
    case OperatingMode::DISP_HELP:
      print_help();
      return 0;
    case OperatingMode::DISP_VERSION:
      print_version();
      return 0;
    case OperatingMode::DISP_LICENSE:
      print_license();
      return 0;
    default:
      break;
  }

  auto builder = quixcc::CompilerBuilder();

  for (const auto &path : options->sources) builder.add_source(path);
  for (const auto &path : options->include_paths) builder.add_include(path);
  for (const auto &path : options->library_paths) builder.add_library(path);
  for (const auto &lib : options->libraries) builder.link_library(lib);
  for (const auto &[name, value] : options->defines)
    builder.define(name, value);
  for (const auto &name : options->undefines) builder.undefine(name);
  for (const auto &setting : options->settings) builder.set_flag(setting);
  builder.set_output(options->output);
  builder.set_verbosity(options->verbosity);
  builder.set_optimization(options->optimization);
  builder.set_debug(options->debug);

  try {
    if (!options->target_triple.empty()) builder.target(options->target_triple);

    if (!options->cpu.empty()) builder.cpu(options->cpu);
  } catch (const quixcc::TargetTripleException &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 5;
  } catch (const quixcc::CpuException &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 6;
  }

  switch (options->mode) {
    case OperatingMode::IR:
      builder.opt("-emit-ir");
      break;
    case OperatingMode::C11:
      builder.opt("-emit-c11");
      break;
    case OperatingMode::IR_BITCODE:
      builder.opt("-emit-bc");
      break;
    case OperatingMode::LEX:
      builder.opt("-emit-tokens");
      break;
    case OperatingMode::PARSE_TREE:
      builder.opt("-emit-parse");
      break;
    case OperatingMode::ASSEMBLY:
      builder.opt("-S");
      break;
    case OperatingMode::OBJECT:
      builder.opt("-c");
      break;
    case OperatingMode::EXECUTABLE:
      break;
    case OperatingMode::DOCUMENTATION:
      throw std::runtime_error("Not implemented");

    default:
      return 4;
  }

  auto compiler = builder.build();

  if (compiler.run().puts().ok()) return 0;

  return -1;
}