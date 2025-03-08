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

#include <core/GetOpt.hh>
#include <core/InterpreterImpl.hh>
#include <core/PackageConfig.hh>
#include <filesystem>
#include <fstream>
#include <nitrate-core/CatchAll.hh>
#include <unordered_map>

using namespace ncc;
using namespace no3::core;
using namespace no3::package;

enum class FormatMode { Standard, Minify, Deflate };

struct FormatOptions {
  FormatMode m_mode;
  std::filesystem::path& m_source_path;
  std::filesystem::path& m_output_path;
  std::filesystem::path& m_config_path;
  nlohmann::json m_config;

  FormatOptions(FormatMode mode, std::filesystem::path& source_path, std::filesystem::path& output_path,
                std::filesystem::path& config_path)
      : m_mode(mode), m_source_path(source_path), m_output_path(output_path), m_config_path(config_path) {}
};

class FormatCommandArgumentParser {
  bool m_help = false;
  size_t m_std = 0;
  size_t m_minify = 0;
  size_t m_deflate = 0;
  FormatMode m_mode = FormatMode::Standard;
  std::filesystem::path m_config_path;
  std::filesystem::path m_output_path;
  std::filesystem::path m_source_path;
  bool m_too_many_args = false;

  void DisplayHelp() {
    std::string_view help =
        R"(Usage: format [--help] [[--std]|[--minify]|[--deflate]] [--config VAR] [--output VAR] path

Positional arguments:
  path    path to a package folder or source file [required]

Optional arguments:
  -h, --help          shows this help message and exits
  -s, --std           standard format the sources
  -m, --minify        minify the sources
  -d, --deflate       minify the sources without the whole-file UTF-8 only constraint
  -c, --config        path to a format configuration file [nargs=0..1] [default: ""]
  -o, --output        optional output path, otherwise the input file will be overwritten [nargs=0..1] [default: path]
)";

    Log << Raw << help;
  }

  void DoParse(std::vector<std::string> args) {
    constexpr const char* kShortOptions = "hsmdc:o:";
    constexpr std::array kLongOptions = {
        option{"help", no_argument, nullptr, 'h'},
        option{"std", no_argument, nullptr, 's'},
        option{"minify", no_argument, nullptr, 'm'},
        option{"deflate", no_argument, nullptr, 'd'},
        option{"config", required_argument, nullptr, 'c'},
        option{"output", required_argument, nullptr, 'o'},
        option{nullptr, 0, nullptr, 0},
    };

    std::vector<char*> argv(args.size());
    std::transform(args.begin(), args.end(), argv.begin(), [](auto& str) { return str.data(); });

    {  // Lock the mutex to prevent multiple threads from calling getopt_long at the same time.
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

          case 's': {
            Log << Trace << "Parsing command line argument: --std";
            m_mode = FormatMode::Standard;

            if (m_std++ > 0) {
              Log << "The --std argument was provided more than once.";
              m_too_many_args = true;
            }

            break;
          }

          case 'm': {
            Log << Trace << "Parsing command line argument: --minify";
            m_mode = FormatMode::Minify;

            if (m_minify++ > 0) {
              Log << "The --minify argument was provided more than once.";
              m_too_many_args = true;
            }

            break;
          }

          case 'd': {
            Log << Trace << "Parsing command line argument: --deflate";
            m_mode = FormatMode::Deflate;

            if (m_deflate++ > 0) {
              Log << "The --deflate argument was provided more than once.";
              m_too_many_args = true;
            }

            break;
          }

          case 'c': {
            Log << Trace << "Parsing command line argument: --config";
            if (!m_config_path.empty()) {
              Log << "The --config argument was provided more than once.";
              m_too_many_args = true;
              break;
            }

            m_config_path = optarg;
            break;
          }

          case 'o': {
            Log << Trace << "Parsing command line argument: --output";
            if (!m_output_path.empty()) {
              Log << "The --output argument was provided more than once.";
              m_too_many_args = true;
              break;
            }

            m_output_path = optarg;
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

      if ((size_t)optind == args.size() - 1) {
        Log << Trace << "Parsing command line argument: path";
        m_source_path = args[optind];
      } else if ((size_t)optind < args.size()) {
        m_too_many_args = true;
      }

      if (m_output_path.empty()) {
        Log << Trace << "No output path provided. Setting it to the source path.";
        m_output_path = m_source_path;
      }

      Log << Trace << "Finished parsing command line arguments";
    }
  }

  [[nodiscard]] bool Check() const {
    bool okay = true;

    if (m_too_many_args) {
      Log << "Too many arguments provided.";
      okay = false;
    }

    if (m_source_path.empty()) {
      Log << "path: 1 argument(s) expected. 0 provided.";
      okay = false;
    }

    if (m_std + m_minify + m_deflate == 0) {
      Log << "One of '--std', '--minify', '--deflate' is required.";
      okay = false;
    } else if (m_std + m_minify + m_deflate != 1) {
      Log << "Arguments '--std', '--minify', '--deflate' are mutually exclusive.";
      okay = false;
    }

    return okay;
  }

public:
  FormatCommandArgumentParser(const std::vector<std::string>& args, bool& is_valid, bool& performed_action) {
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

  [[nodiscard]] FormatOptions GetOptions() { return {m_mode, m_source_path, m_output_path, m_config_path}; }
};

static bool SafeCheckFileExists(const std::string& path) {
  if (auto exists = OMNI_CATCH(std::filesystem::exists(path))) {
    return *exists;
  }

  Log << "Failed to check if the file exists: " << path;
  return false;
}

static std::optional<std::vector<std::filesystem::path>> GetRecursiveDirectoryContents(
    const std::filesystem::path& path) {
  return OMNI_CATCH([&]() -> std::vector<std::filesystem::path> {
    std::vector<std::filesystem::path> paths;  // Might mem leak if exception is thrown
    for (const auto& entry : std::filesystem::recursive_directory_iterator(path)) {
      if (OMNI_CATCH(entry.is_regular_file()).value_or(false)) {
        paths.push_back(std::filesystem::absolute(entry.path()));
      }
    }

    return paths;
  }());
}

static bool LoadConfigurationFile(const std::filesystem::path& path, nlohmann::json& config) {
  std::ifstream config_file(path);
  if (!config_file.is_open()) {
    Log << "Failed to open the JSON format configuration file: " << path;
    return false;
  }

  std::string config_contents((std::istreambuf_iterator<char>(config_file)), std::istreambuf_iterator<char>());

  Log << Trace << "Parsing the JSON format configuration file: " << path;
  config = nlohmann::json::parse(config_contents, nullptr, false);
  if (config.is_discarded()) {
    Log << "Failed to parse the JSON format configuration file: " << path;
    return false;
  }

  Log << Trace << "Successfully parsed the JSON format configuration file: " << path;

  return true;
}

static std::optional<std::unordered_map<std::filesystem::path, std::filesystem::path>> SecondaryArgumentCheck(
    FormatOptions& options) {
  {  // Check if the source file exists and absolutize it
    if (!SafeCheckFileExists(options.m_source_path)) {
      Log << "The source file does not exist: " << options.m_source_path;
      return std::nullopt;
    }

    options.m_source_path = std::filesystem::absolute(options.m_source_path);
    Log << Trace << "Source file exists: " << options.m_source_path;
  }

  auto is_directory = OMNI_CATCH(std::filesystem::is_directory(options.m_source_path));
  if (!is_directory.has_value()) {
    Log << "Failed to check if the source path is a directory: " << options.m_source_path;
    return std::nullopt;
  }

  {  // Create the output path if it does not exist
    if (!SafeCheckFileExists(options.m_output_path)) {
      Log << Trace << "The output path does not exist: " << options.m_output_path;
      Log << Trace << "Creating the output directory because it does not exist.";
      if (auto created = OMNI_CATCH(std::filesystem::create_directories(options.m_output_path)).value_or(false);
          !created) {
        Log << "Failed to create the output directory: " << options.m_output_path;
        return std::nullopt;
      }

      Log << Trace << "Successfully created the output directory: " << options.m_output_path;
    }

    options.m_output_path = std::filesystem::absolute(options.m_output_path);
    Log << Trace << "Output file exists: " << options.m_output_path;
  }

  if (options.m_config_path.empty() && *is_directory) {
    if (SafeCheckFileExists(options.m_source_path / "format.json")) {
      options.m_config_path = options.m_source_path / "format.json";
      Log << Trace << "Using the format configuration file in the source directory: " << options.m_config_path;
    }
  }

  if (!options.m_config_path.empty()) {
    if (!SafeCheckFileExists(options.m_config_path)) {
      Log << "The configuration file does not exist: " << options.m_config_path;
      return std::nullopt;
    }

    Log << Trace << "Configuration file exists: " << options.m_config_path;
    if (!OMNI_CATCH(std::filesystem::is_regular_file(options.m_config_path)).value_or(false)) {
      Log << "The configuration file is not a regular file: " << options.m_config_path;
      return std::nullopt;
    }

    Log << Trace << "Configuration file is a regular file: " << options.m_config_path;
    if (!LoadConfigurationFile(options.m_config_path, options.m_config)) {
      Log << "Failed to load the configuration file: " << options.m_config_path;
      return std::nullopt;
    }

    Log << Trace << "Successfully loaded the configuration file: " << options.m_config_path;
  }

  std::unordered_map<std::filesystem::path, std::filesystem::path> paths;
  if (is_directory.value()) {
    Log << Trace << "Source path is a directory: " << options.m_source_path;

    auto contents = GetRecursiveDirectoryContents(options.m_source_path);
    if (!contents.has_value()) {
      Log << "Failed to get the contents of the source directory: " << options.m_source_path;
      return std::nullopt;
    }

    Log << Trace << "Found " << contents.value().size() << " files in the source directory.";

    for (const auto& path : contents.value()) {
      if (path.extension() != ".n" && path.extension() != ".nit") {
        Log << Trace << "Skipping non-source file: " << path;
        continue;
      }

      Log << Trace << "Found source file: " << path;
      paths[path] = options.m_output_path / path.lexically_relative(options.m_source_path);
    }
  } else {  // The source must be a file
    Log << Trace << "Source path is a file: " << options.m_source_path;
    paths[options.m_source_path] = options.m_output_path;
  }

  for (const auto& [src, dst] : paths) {
    Log << Trace << "Mapping [" << dst << "] = " << src;
  }

  return paths;
}

static bool FormatFile(const std::filesystem::path& src, const std::filesystem::path& dst, const nlohmann::json& config,
                       FormatMode mode) {
  (void)src;
  (void)dst;
  (void)config;
  (void)mode;

  /// TODO: Implement file formatting

  return false;
}

bool no3::Interpreter::PImpl::CommandFormat(ConstArguments, const MutArguments& argv) {
  Log << Trace << "Executing the " << std::source_location::current().function_name();

  bool is_valid = false;
  bool performed_action = false;
  FormatCommandArgumentParser state(argv, is_valid, performed_action);

  if (!is_valid) {
    Log << Trace << "Failed to parse command line arguments.";
    return false;
  }

  if (performed_action) {
    Log << Trace << "Performed built-in action.";
    return true;
  }

  auto options = state.GetOptions();

  Log << Trace << "options[\"source\"] = " << options.m_source_path;
  Log << Trace << "options[\"output\"] = " << options.m_output_path;
  Log << Trace << "options[\"config\"] = " << options.m_config_path;
  Log << Trace << "options[\"mode\"] = " << static_cast<int>(options.m_mode);

  auto mapping_opt = SecondaryArgumentCheck(options);
  if (!mapping_opt) {
    Log << Trace << "Failed secondary argument sanity check.";
    return false;
  }

  if (mapping_opt.value().empty()) {
    Log << Warning << "No source files found to format.";
    return true;
  }

  Log << Debug << "Formatting " << mapping_opt.value().size() << " source file(s).";

  size_t success_count = 0;
  size_t failure_count = 0;
  for (const auto& [src_file, dst_file] : mapping_opt.value()) {
    Log << Info << "Applying format " << src_file << " => " << dst_file;
    if (!FormatFile(src_file, dst_file, options.m_config, options.m_mode)) {
      Log << "Unable to format file: " << src_file;
      failure_count++;
      continue;
    }
    Log << Trace << "Successfully formatted source file: " << src_file;

    success_count++;
  }

  if (failure_count > 0) {
    Log << Warning << "Unable to format " << failure_count << " source file(s).";
  }

  if (success_count > 0) {
    Log << Info << "Successfully formatted " << success_count << " source file(s).";
  }

  return failure_count == 0;
}
