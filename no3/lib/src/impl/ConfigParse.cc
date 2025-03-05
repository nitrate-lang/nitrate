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
#include <impl/Subcommands.hh>
#include <nitrate-core/LogOStream.hh>
#include <nitrate-core/Logger.hh>

using namespace ncc;
using namespace no3::cmd_impl;

static std::unique_ptr<argparse::ArgumentParser> CreateArgumentParser() {
  auto program = std::make_unique<argparse::ArgumentParser>(ncc::clog, "config-parse", "1.0",
                                                            argparse::default_arguments::help, false);

  program->AddArgument("package").Help("The package to parse the configuration for").Required();
  program->AddArgument("--minify", "-C").Help("Minify the output JSON").ImplicitValue(true).DefaultValue(false);

  return program;
}

bool no3::cmd_impl::subcommands::CommandImplConfigParse(ConstArguments, const MutArguments& argv) {
  auto program = CreateArgumentParser();

  try {
    program->ParseArgs(argv);
  } catch (const std::exception& e) {
    if (program->Get<bool>("--help")) {
      return true;
    }

    Log << e.what();
    Log << Raw << *program;
    return false;
  }

  if (program->Get<bool>("--help")) {
    return true;
  }

  auto package_dir = program->Get<std::string>("package");

  if (!std::filesystem::exists(package_dir)) {
    Log << "The package does not exist: " << package_dir;
    return false;
  }

  if (auto config = no3::package::PackageConfig::ParsePackage(package_dir)) {
    bool minify = program->Get<bool>("--minify");
    clog << config->Json().dump(minify ? -1 : 2);
    return true;
  }

  Log << "Failed to parse package configuration: " << package_dir;

  return false;
}
