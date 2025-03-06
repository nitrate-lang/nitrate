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
#include <core/argparse.hpp>
#include <init/InitPackage.hh>
#include <nitrate-core/LogOStream.hh>
#include <sstream>

using namespace ncc;
using namespace no3::package;

static void SetPackageInitCategoryOptions(argparse::ArgumentParser& program) {
  auto& exclusion = program.AddMutuallyExclusiveGroup(true);

  exclusion.AddArgument("--lib")
      .Help("Initialize a new Nitrate library package.")
      .ImplicitValue(true)
      .DefaultValue(false);

  exclusion.AddArgument("--standard-lib")
      .Help("Initialize a new Nitrate offical standard library component package.")
      .ImplicitValue(true)
      .DefaultValue(false);

  exclusion.AddArgument("--exe")
      .Help("Initialize a new Nitrate executable package.")
      .ImplicitValue(true)
      .DefaultValue(false);

  exclusion.AddArgument("--comptime")
      .Help("Initialize a new Nitrate comptime package.")
      .ImplicitValue(true)
      .DefaultValue(false);
}

static void SetPackageMetadataOptions(argparse::ArgumentParser& program) {
  constexpr std::string_view kDefaultPackageDescription = "No description was provided by the package creator.";

  program.AddArgument("--brief", "-b")
      .Help("A description of the package.")
      .DefaultValue(std::string(kDefaultPackageDescription));

  program.AddArgument("--license", "-l")
      .Help("The package SPDX license identifier.")
      .DefaultValue(std::string("LGPL-2.1+"));

  program.AddArgument("--version", "-v")
      .Help("Initial Semantic Version of the package.")
      .DefaultValue(std::string("0.1.0"));
}

static std::unique_ptr<argparse::ArgumentParser> CreateArgumentParser(bool& did_default) {
  auto program = std::make_unique<argparse::ArgumentParser>(ncc::clog, did_default, "impl", "1.0",
                                                            argparse::default_arguments::help);

  SetPackageInitCategoryOptions(*program);
  SetPackageMetadataOptions(*program);

  program->AddArgument("--output", "-o")
      .Help("The directory to create the package folder in.")
      .DefaultValue(std::string("."));

  program->AddArgument("package-name").Help("The name of the package to initialize.").Required();

  return program;
}

static PackageCategory GetPackageCategory(const argparse::ArgumentParser& program) {
  if (program.Get<bool>("--lib")) {
    return PackageCategory::Library;
  }

  if (program.Get<bool>("--standard-lib")) {
    return PackageCategory::StandardLibrary;
  }

  if (program.Get<bool>("--exe")) {
    return PackageCategory::Executable;
  }

  if (program.Get<bool>("--comptime")) {
    return PackageCategory::Comptime;
  }

  return PackageCategory::Library;
}

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

static std::filesystem::path GetNewPackagePath(const std::string& directory, const std::string& package_name) {
  /// TODO: Implement this function.
  return std::filesystem::path(directory) / package_name;
}

bool no3::Interpreter::PImpl::CommandInit(ConstArguments, const MutArguments& argv) {
  bool did_default = false;
  auto program = CreateArgumentParser(did_default);

  Log << Trace << "Parsing command line arguments for package initialization.";

  try {
    program->ParseArgs(argv);
  } catch (const std::runtime_error& e) {
    if (did_default) {
      Log << Trace << "Default action necessitates early return.";
      return true;
    }

    Log << e.what();
    return false;
  }

  if (did_default) {
    Log << Trace << "Default action necessitates early return.";
    return true;
  }

  Log << Trace << "Finished parsing command line arguments for package initialization.";

  const auto package_name = program->Get<std::string>("package-name");
  const auto package_description = program->Get<std::string>("--brief");
  const auto package_license = program->Get<std::string>("--license");
  const auto package_version = program->Get<std::string>("--version");
  const auto package_category = GetPackageCategory(*program);
  const auto package_output = program->Get<std::string>("--output");

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

  if (!std::filesystem::exists(package_output)) {
    Log << Trace << "Creating the output directory because it does not exist.";
    if (!std::filesystem::create_directories(package_output)) {
      Log << Error << "Failed to create the output directory: " << package_output;
      return false;
    }
  }

  const auto package_path = GetNewPackagePath(package_output, package_name);

  InitOptions options;
  options.m_package_name = package_name;
  options.m_package_description = package_description;
  options.m_package_license = package_license;
  options.m_package_version = package_version;
  options.m_package_category = package_category;

  if (!InitPackageUsingDefaults(package_path, options)) {
    Log << "Failed to initialize the package at: " << package_path;
    return false;
  }

  Log << "Successfully initialized the package at: " << package_path;

  return true;
}
