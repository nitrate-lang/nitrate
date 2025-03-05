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
#include <core/argparse.hpp>
#include <filesystem>
#include <fstream>
#include <impl/Subcommands.hh>
#include <nitrate-core/LogOStream.hh>
#include <nitrate-core/Logger.hh>

using namespace ncc;
using namespace no3::cmd_impl;

static std::unique_ptr<argparse::ArgumentParser> CreateArgumentParser(bool& did_default) {
  auto program = std::make_unique<argparse::ArgumentParser>(ncc::clog, did_default, "config-parse", "1.0",
                                                            argparse::default_arguments::help);

  program->AddArgument("--minify", "-C").Help("Minify the output JSON").ImplicitValue(true).DefaultValue(false);

  program->AddArgument("--to", "-t")
      .Help("Translate the configuration to another format")
      .Choices("json", "protobuf")
      .DefaultValue("json");

  program->AddArgument("--output", "-o")
      .Help("The output file to write the configuration to")
      .DefaultValue(std::string("-"));

  program->AddArgument("package").Help("The package to parse the configuration for").Required();

  return program;
}

bool no3::cmd_impl::subcommands::CommandImplConfigParse(ConstArguments, const MutArguments& argv) {
  bool did_default;
  const auto program = CreateArgumentParser(did_default);

  try {
    program->ParseArgs(argv);
  } catch (const std::exception& e) {
    if (did_default) {
      return true;
    }

    Log << e.what();
    return false;
  }

  if (did_default) {
    return true;
  }

  const auto minify = program->Get<bool>("--minify");
  const auto to_format = program->Get<std::string>("--to");
  const auto output_file = program->Get<std::string>("--output");
  const auto package_dir = program->Get<std::string>("package");

  if (!std::filesystem::exists(package_dir)) {
    Log << "The package does not exist: " << package_dir;
    return false;
  }

  if (auto config = no3::package::PackageConfig::ParsePackage(package_dir)) {
    std::unique_ptr<std::ostream> file;

    if (output_file != "-") {
      file = std::make_unique<std::ofstream>(output_file, std::ios::out | std::ios::trunc);
      if (!file->good()) {
        Log << "Failed to open output file: " << output_file;
        std::remove(output_file.c_str());
        return false;
      }
    } else {
      file = std::make_unique<std::ostream>(ncc::clog.rdbuf());
    }

    if (to_format == "json") {
      *file << config->Json().dump(minify ? -1 : 2);
      return true;
    }

    if (to_format == "protobuf") {
      *file << config->Protobuf() << std::flush;
      return true;
    }

    Log << "Unknown format: " << to_format;
    return false;
  }

  return false;
}
