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
#include <core/cli/GetOpt.hh>
#include <core/cli/Interpreter.hh>
#include <core/package/Manifest.hh>
#include <core/static/SPDX.hh>
#include <filesystem>
#include <init/InitPackage.hh>
#include <nitrate-core/CatchAll.hh>
#include <nlohmann/json.hpp>
#include <source_location>
#include <sstream>

using namespace ncc;
using namespace no3::core;
using namespace no3::package;

void DisplayHelp() {
  std::string_view help =
      R"(Usage: init [--help] [[--lib]|[--standard-lib]|[--exe]] [--license VAR] [--output VAR] package-name

Positional arguments:
  package-name                The name of the package to initialize. [required]

Optional arguments:
  -h [ --help ]               produce help message
  -c [ --lib ]                create a library package
  -s [ --standard-lib ]       create a standard library package
  -e [ --exe ]                create an executable package
  -l [ --license ] arg (=MIT) set the package's SPDX license
  -o [ --output ] arg (=.)    output directory for the package
)";

  Log << Raw << help;
}

static void DisplayPoliteNameRejection(const std::string& package_name) {
  Log << "Sorry, the specified package name is not acceptable.";

  std::stringstream help;

  help << "Package names must satisfy the following regular expression:\n";
  help << "\t" << Manifest::GetNameRegex() << "\n";
  help << "\tAlso, there must be no duplicate hyphens.\n\n";

  help << "The package name you provided was: \"" << package_name << "\"\n\n";

  help << "Here is a breakdown of the package name format:\n";
  help << "\t- \x1b[32mPackage names must start with '\x1b[0m\x1b[33m@\x1b[0m\x1b[32m'.\x1b[0m\n\n";

  help << "\t- \x1b[32mImmediately following the '\x1b[0m\x1b[33m@\x1b[0m\x1b[32m' symbol is the Git hosting "
          "provider's prefix.\x1b[0m\n";
  help << "\t  For example, if you are publishing a package with GitHub use \"\x1b[33mgh-\x1b[0m\",\n";
  help << "\t  or if you are publishing a package with GitLab use \"\x1b[33mgl-\x1b[0m\".\n";
  help << "\t  This prefix always ends with a hyphen \"\x1b[33m-\x1b[0m\".\n\n";

  help << "\t- \x1b[32mImmediately following the hyphen is the username of the package owner.\x1b[0m\n";
  help << "\t  The username must be an existing username on the Git hosting provider\n";
  help << "\t  specified by the prefix.\n\n";

  help << "\t- \x1b[32mFollowing the username is a forward slash \"\x1b[0m\x1b[33m/\x1b[0m\x1b[32m\" "
          "character.\x1b[0m\n\n";

  help << "\t- \x1b[32mFinally, following the forward slash is the package's actual name.\x1b[0m\n";
  help << "\t  The package name must be between 3 and 32 characters long.\n";
  help << "\t  It may only contain alphanumeric characters and hyphens.\n";
  help << "\t  It must start and end with an alphanumeric character, may not\n";
  help << "\t  contain two consecutive hyphens.\n\n";

  help << "\t- \x1b[32mOptionally, a colon \"\x1b[0m\x1b[33m:\x1b[0m\x1b[32m\" character may be used to specify "
          "the\x1b[0m\n";
  help << "\t  \x1b[32mpackage generation (major version).\x1b[0m\n";
  help << "\t  The generation must be a positive integer.\n";
  help << "\t  If no generation is specified, the default generation is 1.\n\n";

  help << "Here are some examples of valid package names:\n";
  help << "\t- \x1b[36m@gh-openssl/openssl:2\x1b[0m\n";
  help << "\t- \x1b[36m@gh-gpg/gpg\x1b[0m\n";
  help << "\t- \x1b[36m@gh-john-doe/my-package\x1b[0m\n";
  help << "\t- \x1b[36m@gl-we-use-gitlab/super-useful-package:1\x1b[0m\n";
  help << "\t- \x1b[36m@std/core\x1b[0m\t// Some approved packages don't have a prefix.\n";

  Log << Raw << help.str() << "\n";
}

static void DisplayPoliteLicenseRejection(const std::string& package_license) {
  Log << "Sorry, the specified license is not a valid SPDX license identifier.";
  Log << Info << "Did you mean to use '" << no3::constants::FindClosestSPDXLicense(package_license) << "'?";
  Log << Info << "For a complete list of valid SPDX license identifiers, visit https://spdx.org/licenses/";
}

static auto GetNewPackagePath(const std::filesystem::path& directory,
                              const std::string& name) -> std::optional<std::filesystem::path> {
  std::string just_name = name.substr(name.find('/') + 1);
  size_t attempts = 0;

  while (true) {
    if (attempts > 0xffff) {
      Log << Trace << "Refuses to generate a unique package directory name after " << attempts << " attempts.";
      return std::nullopt;
    }

    const auto folder_name = (attempts == 0 ? just_name : just_name + "-" + std::to_string(attempts));
    const std::filesystem::path canidate = directory / folder_name;

    Log << Trace << "Checking if the package directory already exists: " << canidate;

    auto status = OMNI_CATCH(std::filesystem::exists(canidate));
    if (!status.has_value()) {
      Log << "Failed to check if the package directory exists: " << canidate;
      return std::nullopt;
    }

    if (*status) {
      Log << Warning << "The package directory already exists: " << canidate << ". Trying again with a suffix.";
      attempts++;
      continue;
    }

    Log << Trace << "The package directory does not exist: " << canidate;

    return OMNI_CATCH(std::filesystem::absolute(canidate).lexically_normal()).value_or(canidate);
  }
}

auto no3::Interpreter::PImpl::CommandInit(ConstArguments, const MutArguments& argv) -> bool {
  Log << Trace << "Executing the " << std::source_location::current().function_name();

  namespace po = boost::program_options;

  po::options_description desc;
  po::positional_options_description p;
  p.add("package-name", 1);

  auto add_option = desc.add_options();
  add_option("help,h", "produce help message");
  add_option("lib,c", "create a library package");
  add_option("standard-lib,s", "create a standard library package");
  add_option("exe,e", "create an executable package");
  add_option("license,l", po::value<std::string>()->default_value("MIT"), "set the package's SPDX license");
  add_option("output,o", po::value<std::string>()->default_value("."), "output directory for the package");
  add_option("package-name", po::value<std::string>(), "name of the package to initialize");

  std::vector<const char*> args;
  args.reserve(argv.size());
  for (const auto& arg : argv) {
    args.push_back(arg.c_str());
  }

  po::variables_map vm;
  if (auto cli_parser = OMNI_CATCH(po::command_line_parser(args.size(), args.data()).options(desc).positional(p).run());
      !cli_parser || !OMNI_CATCH(po::store(*cli_parser, vm)) || !OMNI_CATCH(po::notify(vm))) {
    Log << Error << "Failed to parse command line arguments.";
    DisplayHelp();
    return false;
  };

  Log << Trace << "Parsed command line arguments.";

  if (vm.contains("help")) {
    DisplayHelp();
    return true;
  }

  if (!vm.contains("package-name")) {
    Log << "package-name: 1 argument(s) expected. 0 provided.";
    DisplayHelp();
    return false;
  }

  const auto is_lib = vm.count("lib");
  const auto is_standard_lib = vm.count("standard-lib");
  const auto is_exe = vm.count("exe");
  const auto sum = is_lib + is_standard_lib + is_exe;
  if (sum > 1) {
    Log << "Only one of --lib, --standard-lib, or --exe may be specified.";
    DisplayHelp();
    return false;
  }

  const auto& package_name = vm.at("package-name").as<std::string>();
  const auto& package_license = vm.at("license").as<std::string>();
  const auto& package_output = vm.at("output").as<std::string>();
  const auto& package_category = [&] {
    if (vm.contains("lib")) {
      return Manifest::Category::Library;
    }

    if (vm.contains("standard-lib")) {
      return Manifest::Category::StandardLibrary;
    }

    if (vm.contains("exe")) {
      return Manifest::Category::Executable;
    }

    return Manifest::Category::Executable;
  }();

  Log << Trace << R"(args["package-name"] = ")" << package_name << "\"";
  Log << Trace << R"(args["license"] = ")" << package_license << "\"";
  Log << Trace << R"(args["output"] = ")" << package_output << "\"";

  Log << Trace << "Finished parsing command line arguments.";

  if (!Manifest::IsValidLicense(package_license)) {
    DisplayPoliteLicenseRejection(package_license);
    Log << Trace << "Aborting package initialization due to an invalid SPDX license identifier.";
    return false;
  }

  if (!Manifest::IsValidName(package_name)) {
    DisplayPoliteNameRejection(package_name);
    Log << Trace << "Aborting package initialization due to an invalid package name.";
    return false;
  }

  auto package_output_exists = OMNI_CATCH(std::filesystem::exists(package_output));
  if (!package_output_exists.has_value()) {
    Log << "Failed to check if the output directory exists: " << package_output;
    return false;
  }

  if (!*package_output_exists) {
    Log << Trace << "Creating the output directory because it does not exist.";

    auto package_output_created = OMNI_CATCH(std::filesystem::create_directories(package_output)).value_or(false);
    if (!package_output_created) {
      Log << "Failed to create the output directory: " << package_output;
      return false;
    }

    Log << Trace << "Successfully created the output directory: " << package_output;
  }

  const auto package_path = GetNewPackagePath(package_output, package_name);
  if (!package_path) {
    Log << "Failed to generate a unique package directory name.";
    return false;
  }

  InitOptions options;
  options.m_package_name = package_name;
  options.m_package_description = "No description was provided by the package creator.";
  options.m_package_license = constants::FindClosestSPDXLicense(package_license);  // convert to proper letter case
  options.m_package_version = {0, 1, 0};
  options.m_package_category = package_category;

  Log << Info << "Initializing the package at: " << package_path.value();
  if (!CreatePackage(package_path.value(), options)) {
    Log << "Failed to initialize the package at: " << package_path.value();
    auto removed = OMNI_CATCH(std::filesystem::remove_all(package_path.value())).value_or(0);
    if (removed == 0) {
      Log << "Failed to remove the package directory: " << package_path.value();
    }

    return false;
  }

  Log << Info << "Successfully initialized the package at: " << package_path.value();

  return true;
}
