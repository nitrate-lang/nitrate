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

#include <core/cli/argh.hh>
#include <core/package/Config.hh>
#include <filesystem>
#include <fstream>
#include <impl/Subcommands.hh>
#include <memory>
#include <nitrate-core/CatchAll.hh>
#include <nitrate-core/Logger.hh>

using namespace ncc;
using namespace no3::cmd_impl;

static void DisplayHelp() {
  std::string_view message = R"(Usage: config-parse [--help] [--minify] [--to VAR] [--output VAR] package

Positional arguments:
  package       The package to parse the configuration for [required]

Optional arguments:
  -h, --help    shows help message and exits
  -C, --minify  Minify the output JSON
  -t, --to      Translate the configuration to another format [nargs=0..1] [default: "json"]
  -o, --output  The output file to write the configuration to [nargs=0..1] [default: "-"]
)";

  Log << Raw << message;
}

auto no3::cmd_impl::subcommands::CommandImplConfigParse(ConstArguments, const MutArguments& argv) -> bool {
  argh::parser cmdl;
  cmdl.add_params({"help", "minify", "C", "to", "t", "output", "o"});
  cmdl.parse(argv, argh::parser::SINGLE_DASH_IS_MULTIFLAG);

  if (cmdl[{"-h", "--help"}]) {
    DisplayHelp();
    return true;
  }

  std::string to_format;
  std::string output_file;
  std::string package_dir;
  cmdl({"-t", "--to"}) >> to_format;
  cmdl({"-o", "--output"}) >> output_file;
  cmdl(1) >> package_dir;
  const bool minify = cmdl[{"-C", "--minify"}];

  if (package_dir.empty()) {
    Log << "package: 1 argument(s) expected. 0 provided.";
    return false;
  }

  if (to_format.empty()) {
    to_format = "json";
  }

  if (output_file.empty()) {
    output_file = "-";
  }

  if (to_format != "json" && to_format != "protobuf") {
    Log << "to: invalid choice: " << to_format << " (choose from 'json', 'protobuf')";
    return false;
  }

  if (minify && to_format != "json") {
    Log << "minify: option only valid for JSON output";
    return false;
  }

  auto package_dir_exists = OMNI_CATCH(std::filesystem::exists(package_dir));
  if (!package_dir_exists.has_value()) {
    Log << "Failed to check if the package exists: " << package_dir;
    return false;
  }

  if (!package_dir_exists.value()) {
    Log << "The package does not exist: " << package_dir;
    return false;
  }

  if (auto config = no3::package::PackageConfig::ParsePackage(package_dir)) {
    std::unique_ptr<std::ostream> file;
    std::unique_ptr<std::ostream> stream;

    if (output_file != "-") {
      file = std::make_unique<std::ofstream>(output_file, std::ios::out | std::ios::trunc);
      if (!file->good()) {
        Log << "Failed to open output file: " << output_file;
        std::remove(output_file.c_str());
        return false;
      }
    } else {
      stream = (ncc::Log << Raw);
      file = std::make_unique<std::ostream>(stream->rdbuf());
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
