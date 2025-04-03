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

#include <core/cli/GetOpt.hh>
#include <core/cli/InterpreterImpl.hh>
#include <core/package/Config.hh>
#include <core/static/SPDX.hh>
#include <filesystem>
#include <init/InitPackage.hh>
#include <nitrate-core/CatchAll.hh>
#include <nitrate-core/LogOStream.hh>
#include <source_location>
#include <sstream>

using namespace ncc;
using namespace no3::core;
using namespace no3::package;

class RunCommandArgumentParser {
  bool m_help = false;
  size_t m_lib = 0;
  size_t m_standard_lib = 0;
  size_t m_exe = 0;
  PackageCategory m_category = PackageCategory::Library;
  std::string m_license;
  std::string m_output;
  std::string m_package_name;
  bool m_too_many_args = false;

  void DisplayHelp() {
    std::string_view help =
        R"(Usage: impl [--help] [[--lib]|[--standard-lib]|[--exe]] [--license VAR] [--output VAR] package-name

Positional arguments:
  package-name    The name of the package to initialize. [required]

Optional arguments:
  -h, --help          shows this help message and exits
  -c, --lib           create a library package
  -s, --standard-lib  create a standard library package
  -e, --exe           create an executable package
  -l, --license       set the package's SPDX license [nargs=0..1] [default: "MIT"]
  -o, --output        the folder that will contain the package [nargs=0..1] [default: "."]
)";

    Log << Raw << help;
  }

  void DoParse(std::vector<std::string> args) {
    constexpr const char* kShortOptions = "hl:v:o:";
    constexpr std::array kLongOptions = {
        option{"help", no_argument, nullptr, 'h'},
        option{"lib", no_argument, nullptr, 'c'},
        option{"standard-lib", no_argument, nullptr, 's'},
        option{"exe", no_argument, nullptr, 'e'},
        option{"license", required_argument, nullptr, 'l'},
        option{"output", required_argument, nullptr, 'o'},
        option{nullptr, 0, nullptr, 0},
    };

    std::vector<char*> argv(args.size());
    std::transform(args.begin(), args.end(), argv.begin(), [](auto& str) { return str.data(); });

    {
      std::lock_guard lock(GET_OPT);

      Log << Trace << "Starting to parse command line arguments";

      int c;
      int option_index = 0;

      opterr = 0;
      while ((c = GET_OPT.getopt_long(args.size(), argv.data(), kShortOptions, kLongOptions.data(), &option_index)) !=
             -1) {
        switch (c) {
          case 'h': {
            Log << Trace << "Parsing command line argument: --help";
            m_help = true;
            break;
          }

          case 'l': {
            Log << Trace << "Parsing command line argument: --license";
            if (!m_license.empty()) {
              Log << "The --license argument was provided more than once.";
              m_too_many_args = true;
              break;
            }

            m_license = optarg;
            break;
          }

          case 'o': {
            Log << Trace << "Parsing command line argument: --output";
            if (!m_output.empty()) {
              Log << "The --output argument was provided more than once.";
              m_too_many_args = true;
              break;
            }

            m_output = optarg;
            break;
          }

          case 'c': {
            Log << Trace << "Parsing command line argument: --lib";
            m_category = PackageCategory::Library;

            if (m_lib++ > 0) {
              Log << "The --lib argument was provided more than once.";
              m_too_many_args = true;
            }

            break;
          }

          case 's': {
            Log << Trace << "Parsing command line argument: --standard-lib";
            m_category = PackageCategory::StandardLibrary;

            if (m_standard_lib++ > 0) {
              Log << "The --standard-lib argument was provided more than once.";
              m_too_many_args = true;
            }

            break;
          }

          case 'e': {
            Log << Trace << "Parsing command line argument: --exe";
            m_category = PackageCategory::Executable;

            if (m_exe++ > 0) {
              Log << "The --exe argument was provided more than once.";
              m_too_many_args = true;
            }

            break;
          }

          case '?': {
            Log << "Unknown command line argument: -" << (char)optopt;
            m_too_many_args = true;
            break;
          }

          default: {
            Log << "Unknown command line argument: -" << (char)c;
            m_too_many_args = true;
            break;
          }
        }
      }

      if (m_license.empty()) {
        Log << Trace << "Setting the default value for the --license argument.";
        m_license = "MIT";
      }

      if (m_output.empty()) {
        Log << Trace << "Setting the default value for the --output argument.";
        m_output = ".";
      }

      if ((size_t)optind == args.size() - 1) {
        Log << Trace << "Parsing command line argument: package-name";
        m_package_name = args[optind];
      } else if ((size_t)optind < args.size()) {
        m_too_many_args = true;
      }

      Log << Trace << "Finished parsing command line arguments";
    }
  }

  [[nodiscard]] auto Check() const -> bool {
    bool okay = true;

    if (m_too_many_args) {
      Log << "Too many arguments provided.";
      okay = false;
    }

    if (m_package_name.empty()) {
      Log << "package-name: 1 argument(s) expected. 0 provided.";
      okay = false;
    }

    if (m_lib + m_standard_lib + m_exe == 0) {
      Log << "One of '--exe', '--lib', '--standard-lib' is required.";
      okay = false;
    } else if (m_lib + m_standard_lib + m_exe != 1) {
      Log << "Arguments '--exe', '--lib', '--standard-lib' are mutually exclusive.";
      okay = false;
    }

    return okay;
  }

public:
  RunCommandArgumentParser(const std::vector<std::string>& args, bool& is_valid, bool& performed_action) {
    is_valid = false;
    performed_action = false;

    DoParse(args);

    if (m_help) {
      DisplayHelp();
      is_valid = true;
      performed_action = true;
      return;
    }

    is_valid = Check();
  }

  [[nodiscard]] auto GetPackageName() const -> const auto& { return m_package_name; }
  [[nodiscard]] auto GetLicense() const -> const auto& { return m_license; }
  [[nodiscard]] auto GetOutput() const -> const auto& { return m_output; }
  [[nodiscard]] auto GetCategory() const -> const auto& { return m_category; }
};

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

  bool is_valid = false;
  bool performed_action = false;
  RunCommandArgumentParser state(argv, is_valid, performed_action);

  if (!is_valid) {
    Log << Trace << "Failed to parse command line arguments.";
    return false;
  }

  if (performed_action) {
    Log << Trace << "Performed built-in action.";
    return true;
  }

  const auto& package_name = state.GetPackageName();
  const auto& package_license = state.GetLicense();
  const auto& package_output = state.GetOutput();
  const auto& package_category = state.GetCategory();

  Log << Trace << R"(args["package-name"] = ")" << package_name << "\"";
  Log << Trace << R"(args["license"] = ")" << package_license << "\"";
  Log << Trace << R"(args["output"] = ")" << package_output << "\"";

  Log << Trace << "Finished parsing command line arguments.";

  if (!PackageConfig::ValidatePackageLicense(package_license)) {
    DisplayPoliteLicenseRejection(package_license);
    Log << Trace << "Aborting package initialization due to an invalid SPDX license identifier.";
    return false;
  }

  if (!PackageConfig::ValidatePackageName(package_name, package_category == PackageCategory::StandardLibrary)) {
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
  options.m_package_version = "0.1.0";
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
