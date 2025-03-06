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

#include <core/InterpreterImpl.hh>
#include <core/PackageConfig.hh>
#include <core/SPDX.hh>
#include <core/argh.hh>
#include <filesystem>
#include <init/InitPackage.hh>
#include <nitrate-core/CatchAll.hh>
#include <nitrate-core/LogOStream.hh>
#include <sstream>

using namespace ncc;
using namespace no3::package;

static void DisplayPoliteNameRejection(const std::string& package_name) {
  Log << "Sorry, the specified package name is not acceptable.";

  std::stringstream help;

  help << "Package names must satisfy the following regular expression:\n";
  help << "\t" << PackageConfig::PackageNameRegex << "\n";
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
  Log << "Did you mean to use '" << no3::constants::FindClosestSPDXLicense(package_license) << "'?";
  Log << Info << "For a complete list of valid SPDX license identifiers, visit https://spdx.org/licenses/";
}

static void DisplayPoliteVersionRejection(const std::string&) {
  Log << "Sorry, the specified version number is not a valid Semantic Version.";
  Log << Info << "An example of a valid Semantic Version is '1.0.0'.";
  Log << Info
      << "Semantic Versioning is a specification for version numbers that helps"
         " to communicate the nature of changes in a package.";
  Log << Info << "For more information on Semantic Versioning, visit https://semver.org/";
}

static std::optional<std::filesystem::path> GetNewPackagePath(const std::filesystem::path& directory,
                                                              const std::string& name) {
  std::string just_name = name.substr(name.find('/') + 1);
  size_t attempts = 0;

  while (true) {
    if (attempts > 0xffff) {
      Log << Trace << "Refuses to generate a unique package directory name after " << attempts << " attempts.";
      return std::nullopt;
    }

    const auto folder_name = (attempts == 0 ? just_name : just_name + "-" + std::to_string(attempts));
    auto canidate = directory / folder_name;

    Log << Trace << "Checking if the package directory already exists: " << canidate;

    auto status = OMNI_CATCH(std::filesystem::exists(canidate));
    if (!status.has_value()) {
      Log << Error << "Failed to check if the package directory exists: " << canidate;
      return std::nullopt;
    }

    if (std::any_cast<bool>(*status)) {
      Log << Warning << "The package directory already exists: " << canidate << ". Trying again with a suffix.";
      attempts++;
      continue;
    }

    Log << Trace << "The package directory does not exist: " << canidate;

    return canidate;
  }
}

static void DisplayHelp() {
  std::string_view help =
      R"(Usage: impl [--help] [[--lib]|[--standard-lib]|[--exe]|[--comptime]] [--brief VAR] [--license VAR] [--version VAR] [--output VAR] package-name

Positional arguments:
  package-name    The name of the package to initialize. [required]

Optional arguments:
  -h, --help      shows help message and exits 
  --lib           Initialize a new Nitrate library package. 
  --standard-lib  Initialize a new Nitrate offical standard library component package. 
  --exe           Initialize a new Nitrate executable package. 
  --comptime      Initialize a new Nitrate comptime package. 
  -b, --brief     A description of the package. [nargs=0..1] [default: "No description was provided by the package creator."]
  -l, --license   The package SPDX license identifier. [nargs=0..1] [default: "LGPL-2.1+"]
  -v, --version   Initial Semantic Version of the package. [nargs=0..1] [default: "0.1.0"]
  -o, --output    The directory to create the package folder in. [nargs=0..1] [default: "."]
)";

  Log << Raw << help;
}

static bool GetCheckedArguments(const argh::parser& cmdl, std::string& package_name, std::string& package_description,
                                std::string& package_license, std::string& package_version, std::string& package_output,
                                PackageCategory& package_category) {
  cmdl({"-b", "--brief"}) >> package_description;
  cmdl({"-l", "--license"}) >> package_license;
  cmdl({"-v", "--version"}) >> package_version;
  cmdl({"-o", "--output"}) >> package_output;
  cmdl(1) >> package_name;
  const bool said_lib = cmdl[{"--lib"}];
  const bool said_stdlib = cmdl[{"--standard-lib"}];
  const bool said_exe = cmdl[{"--exe"}];
  const bool said_comptime = cmdl[{"--comptime"}];
  package_category = [&]() {
    if (said_lib) {
      return PackageCategory::Library;
    }

    if (said_stdlib) {
      return PackageCategory::StandardLibrary;
    }

    if (said_exe) {
      return PackageCategory::Executable;
    }

    return PackageCategory::Comptime;
  }();

  if (package_name.empty()) {
    Log << "package-name: 1 argument(s) expected. 0 provided.";
    return false;
  }

  int type_sum = (int)said_lib + (int)said_stdlib + (int)said_exe + (int)said_comptime;
  if (type_sum == 0) {
    Log << "One of '--exe', '--lib', '--comptime', '--standard-lib' is required.";
    return false;
  }

  if (type_sum != 1) {
    Log << "Arguments '--exe', '--lib', '--comptime', '--standard-lib' are mutually exclusive.";
    return false;
  }

  if (package_description.empty()) {
    package_description = "No description was provided by the package creator.";
  }

  if (package_license.empty()) {
    package_license = "LGPL-2.1+";
  }

  if (package_version.empty()) {
    package_version = "0.1.0";
  }

  if (package_output.empty()) {
    package_output = ".";
  }

  return true;
}

bool no3::Interpreter::PImpl::CommandInit(ConstArguments, const MutArguments& argv) {
  argh::parser cmdl;
  cmdl.add_params({"help", "brief", "b", "license", "l", "version", "v", "output", "o"});

  Log << Trace << "Parsing command line arguments for package initialization.";
  cmdl.parse(argv, argh::parser::SINGLE_DASH_IS_MULTIFLAG);

  if (cmdl[{"-h", "--help"}]) {
    DisplayHelp();
    return true;
  }

  std::string package_name;
  std::string package_description;
  std::string package_license;
  std::string package_version;
  std::string package_output;
  PackageCategory package_category{};

  if (!GetCheckedArguments(cmdl, package_name, package_description, package_license, package_version, package_output,
                           package_category)) {
    return false;
  }

  Log << Trace << R"(args["package-name"] = ")" << package_name << "\"";
  Log << Trace << R"(args["brief"] = ")" << package_description << "\"";
  Log << Trace << R"(args["license"] = ")" << package_license << "\"";
  Log << Trace << R"(args["version"] = ")" << package_version << "\"";
  Log << Trace << R"(args["output"] = ")" << package_output << "\"";

  Log << Trace << "Finished parsing command line arguments for package initialization.";

  if (!PackageConfig::ValidatePackageVersion(package_version)) {
    DisplayPoliteVersionRejection(package_version);
    Log << Trace << "Aborting package initialization due to invalid version number.";
    return false;
  }

  if (!PackageConfig::ValidatePackageLicense(package_license)) {
    DisplayPoliteLicenseRejection(package_license);
    Log << Trace << "Aborting package initialization due to invalid license identifier.";
    return false;
  }

  if (!PackageConfig::ValidatePackageName(package_name, package_category == PackageCategory::StandardLibrary)) {
    DisplayPoliteNameRejection(package_name);
    Log << Trace << "Aborting package initialization due to invalid package name.";
    return false;
  }

  auto package_output_exists = OMNI_CATCH(std::filesystem::exists(package_output));
  if (!package_output_exists.has_value()) {
    Log << Error << "Failed to check if the output directory exists: " << package_output;
    return false;
  }

  if (!std::any_cast<bool>(*package_output_exists)) {
    Log << Trace << "Creating the output directory because it does not exist.";

    auto package_output_created = OMNI_CATCH(std::filesystem::create_directories(package_output));
    if (!package_output_created.has_value() || !std::any_cast<bool>(*package_output_created)) {
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
  options.m_package_description = package_description;
  options.m_package_license = package_license;
  options.m_package_version = package_version;
  options.m_package_category = package_category;

  Log << Info << "Initializing the package at: " << package_path.value();
  if (!InitPackageUsingDefaults(package_path.value(), options)) {
    Log << "Failed to initialize the package at: " << package_path.value();
    return false;
  }

  Log << Info << "Successfully initialized the package at: " << package_path.value();

  return true;
}
