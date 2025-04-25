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
#include <core/package/Manifest.hh>
#include <fstream>
#include <impl/Subcommands.hh>
#include <memory>
#include <nitrate-core/CatchAll.hh>
#include <nitrate-core/Logger.hh>
#include <sstream>

using namespace ncc;
using namespace no3::cmd_impl;

auto no3::cmd_impl::subcommands::CommandImplConfigParse(ConstArguments, const MutArguments& argv) -> bool {
  namespace po = boost::program_options;
  po::options_description desc("Allowed options");
  po::positional_options_description p;
  p.add("manifest-file", -1);

  auto add_option = desc.add_options();
  add_option("help,h", "Display this help message");
  add_option("output,o", po::value<std::string>()->default_value("-"), "Output file (default: -)");
  add_option("minify,m", "Minify the output");
  add_option("manifest-file,f", po::value<std::string>(), "Path to the package manifest file");

  std::vector<const char*> args;
  args.reserve(argv.size());
  for (const auto& arg : argv) {
    args.push_back(arg.c_str());
  }

  po::variables_map vm;
  if (auto cli_parser = OMNI_CATCH(po::command_line_parser(args.size(), args.data()).options(desc).positional(p).run());
      !cli_parser || !OMNI_CATCH(po::store(*cli_parser, vm)) || !OMNI_CATCH(po::notify(vm))) {
    Log << Error << "Failed to parse command line arguments.";
    desc.print(*(Log << Raw));
    return false;
  };

  Log << Trace << "Parsed command line arguments.";

  if (vm.contains("help")) {
    desc.print(*(Log << Raw));
    return true;
  }

  if (!vm.contains("manifest-file")) {
    Log << "manifest-file: 1 argument(s) expected. 0 provided.";
    desc.print(*(Log << Raw));
    return false;
  }

  auto manifest_file = vm.at("manifest-file").as<std::string>();

  auto input_stream = std::ifstream(manifest_file);
  if (!input_stream.is_open()) {
    Log << "Failed to open manifest file: " << manifest_file;
    return false;
  }

  if (auto manifest = package::Manifest::FromJson(input_stream)) {
    std::stringstream ss;
    bool correct_schema = false;

    manifest->ToJson(ss, correct_schema, vm.contains("minify"));

    if (correct_schema) {
      auto output_file = vm.at("output").as<std::string>();
      if (output_file != "-") {
        auto file = std::make_unique<std::ofstream>(output_file, std::ios::out | std::ios::trunc);
        if (!file->good()) {
          Log << "Failed to open output file: " << output_file;
          std::remove(output_file.c_str());
          return false;
        }

        *file << ss.str();

        return true;
      }

      ncc::Log << Raw << ss.str();

      return true;
    }
  }

  Log << "Manifest file schema is incorrect.";

  return false;
}
