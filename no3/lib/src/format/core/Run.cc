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

#include <libdeflate.h>

#include <core/GetOpt.hh>
#include <core/InterpreterImpl.hh>
#include <core/PackageConfig.hh>
#include <filesystem>
#include <fstream>
#include <memory>
#include <nitrate-core/Allocate.hh>
#include <nitrate-core/CatchAll.hh>
#include <nitrate-core/Environment.hh>
#include <nitrate-lexer/Lexer.hh>
#include <nitrate-parser/ASTBase.hh>
#include <nitrate-parser/CodeWriter.hh>
#include <nitrate-parser/Context.hh>
#include <nitrate-parser/Package.hh>
#include <random>
#include <sstream>
#include <unordered_map>

using namespace ncc;
using namespace no3::core;
using namespace no3::package;

enum class FormatMode { Standard, Minify, Deflate };

static bool ValidateConfiguration(const nlohmann::json& j);
static void AssignDefaultConfigurationSettings(nlohmann::json& j);
static bool FormatFile(const std::filesystem::path& src, const std::filesystem::path& dst, const nlohmann::json& config,
                       FormatMode mode, const ncc::parse::ImportConfig& import_config);

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

static auto SecondaryArgumentCheck(FormatOptions& options)
    -> std::optional<
        std::pair<std::unordered_map<std::filesystem::path, std::filesystem::path>, std::optional<parse::ImportName>>> {
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

  {  // Check for default configuration file in the source directory
    if (*is_directory && options.m_config_path.empty()) {
      if (SafeCheckFileExists(options.m_source_path / "format.json")) {
        options.m_config_path = options.m_source_path / "format.json";
        Log << Trace << "Using the format configuration file in the source directory: " << options.m_config_path;
      }
    }
  }

  {  // Ensure the configuration file is a file and load it
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

      Log << Trace << "Loaded the configuration file: " << options.m_config_path;
      if (!ValidateConfiguration(options.m_config)) {
        Log << "The JSON format configuration file is invalid: " << options.m_config_path;
        return std::nullopt;
      }

      Log << Trace << "The JSON format configuration file is valid: " << options.m_config_path;
      AssignDefaultConfigurationSettings(options.m_config);
    }
  }

  std::unordered_map<std::filesystem::path, std::filesystem::path> paths;
  std::optional<parse::ImportName> import_name;
  if (is_directory.value()) {
    Log << Trace << "Source path is a directory: " << options.m_source_path;

    auto contents = GetRecursiveDirectoryContents(options.m_source_path);
    if (!contents.has_value()) {
      Log << "Failed to get the contents of the source directory: " << options.m_source_path;
      return std::nullopt;
    }

    if (auto pkg_opt = PackageConfig::ParsePackage(options.m_source_path)) {
      import_name = pkg_opt.value().ImportName();
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

  return {{paths, import_name}};
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

  auto result = SecondaryArgumentCheck(options);
  if (!result.has_value()) {
    Log << Trace << "Failed secondary argument sanity check.";
    return false;
  }

  const auto& [mapping_opt, this_import_name_opt] = result.value();

  if (mapping_opt.empty()) {
    Log << Warning << "No source files found to format.";
    return true;
  }

  Log << Debug << "Formatting " << mapping_opt.size() << " source file(s).";

  ncc::parse::ImportConfig import_config = ncc::parse::ImportConfig::GetDefault();
  if (this_import_name_opt.has_value()) {
    import_config.SetThisImportName(this_import_name_opt.value());
  }

  size_t success_count = 0;
  size_t failure_count = 0;
  for (const auto& [src_file, dst_file] : mapping_opt) {
    import_config.ClearFilesToNotImport();
    import_config.AddFileToNotImport(src_file);

    Log << Info << "Applying format " << src_file << " => " << dst_file;
    if (!FormatFile(src_file, dst_file, options.m_config, options.m_mode, import_config)) {
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

#define schema_assert(__expr)                                               \
  if (!(__expr)) [[unlikely]] {                                             \
    Log << "Invalid configuration:" << " schema_assert(" << #__expr << ")"; \
    return false;                                                           \
  }

static bool ValidateConfiguration(const nlohmann::json& j) {
  schema_assert(j.is_object());

  schema_assert(j.contains("version"));
  schema_assert(j["version"].is_object());
  schema_assert(j["version"].contains("major"));
  schema_assert(j["version"]["major"].is_number_unsigned());
  schema_assert(j["version"].contains("minor"));
  schema_assert(j["version"]["minor"].is_number_unsigned());

  auto major = j["version"]["major"].get<size_t>();
  auto minor = j["version"]["minor"].get<size_t>();

  schema_assert(major == 1);
  schema_assert(minor == 0);

  for (const auto& [key, value] : j.items()) {
    schema_assert(key == "version" || key == "whitespace" || key == "comments");

    if (key == "version") {
      continue;
    }

    if (key == "whitespace") {
      schema_assert(j["whitespace"].is_object());

      for (const auto& [key, value] : j["whitespace"].items()) {
        schema_assert(key == "indentation");

        if (key == "indentation") {
          schema_assert(j["whitespace"]["indentation"].is_object());
          schema_assert(j["whitespace"]["indentation"].contains("size"));
          schema_assert(j["whitespace"]["indentation"]["size"].is_number_unsigned());
          schema_assert(j["whitespace"]["indentation"].contains("byte"));
          schema_assert(j["whitespace"]["indentation"]["byte"].is_string());
          continue;
        }
      }

      continue;
    }

    if (key == "comments") {
      schema_assert(j["comments"].is_object());
      for (const auto& [key, value] : j["comments"].items()) {
        schema_assert(key == "line" || key == "block");

        if (key == "line") {
          schema_assert(j["comments"]["line"].is_object());
          for (const auto& [key, value] : j["comments"]["line"].items()) {
            schema_assert(key == "start" || key == "end" || key == "convert-to-block");

            if (key == "start") {
              schema_assert(j["comments"]["line"]["start"].is_string());
            } else if (key == "end") {
              schema_assert(j["comments"]["line"]["end"].is_string());
            } else if (key == "convert-to-block") {
              schema_assert(j["comments"]["line"]["convert-to-block"].is_boolean());
            }
          }
          continue;
        }

        if (key == "block") {
          schema_assert(j["comments"]["block"].is_object());
          for (const auto& [key, value] : j["comments"]["block"].items()) {
            schema_assert(key == "start" || key == "end" || key == "convert-to-line");

            if (key == "start") {
              schema_assert(j["comments"]["block"]["start"].is_string());
            } else if (key == "end") {
              schema_assert(j["comments"]["block"]["end"].is_string());
            } else if (key == "convert-to-line") {
              schema_assert(j["comments"]["block"]["convert-to-line"].is_boolean());
            }
          }

          continue;
        }
      }
    }
  }

  return true;
}

#undef schema_assert

static void AssignDefaultConfigurationSettings(nlohmann::json& j) {
  j["whitespace"]["indentation"]["size"] = 2;
  j["whitespace"]["indentation"]["byte"] = " ";

  j["comments"]["line"]["start"] = "//";
  j["comments"]["line"]["end"] = "";
  j["comments"]["line"]["convert-to-block"] = true;

  j["comments"]["block"]["start"] = "/*";
  j["comments"]["block"]["end"] = "*/";
  j["comments"]["block"]["convert-to-line"] = false;
}

static bool DeflateStreams(std::istream& in, std::ostream& out) {
  constexpr int kCompressionLevel = 9;
  constexpr size_t kBufferSize = 32 * 1024 * 1024;  // 32 MiB

  libdeflate_compressor* compressor = libdeflate_alloc_compressor(kCompressionLevel);
  if (compressor == nullptr) {
    Log << "Failed to allocate the raw deflate compressor.";
    return false;
  }

  std::vector<uint8_t> input_buffer(kBufferSize);
  std::vector<uint8_t> output_buffer(kBufferSize);

  while (in.good()) {
    in.read(reinterpret_cast<char*>(input_buffer.data()), input_buffer.size());
    if (in.gcount() == 0) {
      break;
    }

    size_t compressed_size = libdeflate_deflate_compress(compressor, input_buffer.data(), in.gcount(),
                                                         output_buffer.data(), output_buffer.size());
    if (compressed_size == 0) {
      Log << "Failed to compress the input data.";
      return false;
    }

    out.write(reinterpret_cast<const char*>(output_buffer.data()), compressed_size);
  }

  libdeflate_free_compressor(compressor);

  return true;
}

static bool FormatFile(const std::filesystem::path& src, const std::filesystem::path& dst, const nlohmann::json& config,
                       FormatMode mode, const ncc::parse::ImportConfig& import_config) {
  std::ifstream src_file(src, std::ios::binary);
  if (!src_file.is_open()) {
    Log << "Failed to open the source file: " << src;
    return false;
  }

  bool quiet_parser = false;
  auto pool = ncc::DynamicArena();
  std::optional<FlowPtr<ncc::parse::Expr>> ptree_root;

  { /* Perform source code parsing */
    auto reenable_log = std::shared_ptr<void>(nullptr, [](auto) { Log.Enable(); });
    if (quiet_parser) {
      Log.Disable();
    }

    auto unit_env = std::make_shared<ncc::Environment>();
    auto tokenizer = ncc::lex::Tokenizer(src_file, unit_env);
    tokenizer.SetCurrentFilename(src.string());

    auto parser = ncc::parse::GeneralParser(tokenizer, unit_env, pool, import_config);
    auto ast_result = parser.Parse();

    Log << Trace << "The parser used " << pool.GetSpaceUsed() << " bytes of memory.";
    Log << Trace << "The parser reserved " << pool.GetSpaceManaged() << " bytes of memory.";

    if (ast_result.Check()) {
      ptree_root = ast_result.Get();
    } else {
      Log << "Failed to parse the source file: " << src;
      return false;
    }
  }

  qcore_assert(ptree_root.has_value());

  std::unique_ptr<std::ofstream> dst_file_ptr;
  std::filesystem::path temporary_path;

  if (src == dst) {
    std::string random_string;

    {
      static std::random_device rd;
      static std::mutex mutex;

      std::lock_guard lock(mutex);
      random_string = [&]() {
        std::string str;
        for (size_t i = 0; i < 16; i++) {
          str.push_back("0123456789abcdef"[rd() % 16]);
        }
        return str;
      }();
    }

    temporary_path = dst.string() + "." + random_string + ".fmt.no3.tmp";
    if (SafeCheckFileExists(temporary_path)) {
      Log << "The temporary file already exists: " << temporary_path;
      return false;
    }

    dst_file_ptr = std::make_unique<std::ofstream>(temporary_path, std::ios::binary | std::ios::trunc);
    if (!dst_file_ptr->is_open()) {
      Log << "Failed to open the temporary file: " << temporary_path;
      return false;
    }
  } else {
    dst_file_ptr = std::make_unique<std::ofstream>(dst, std::ios::binary | std::ios::trunc);
    if (!dst_file_ptr->is_open()) {
      Log << "Failed to open the destination file: " << dst;
      return false;
    }
  }

  (void)config;

  bool okay = false;

  /// TODO: Remove this ling
  Log << Info << ptree_root.value()->PrettyPrint();

  switch (mode) {
    case FormatMode::Standard: {
      /// TODO: Implement file formatting
      Log << "Standard formatting mode is not yet implemented.";
      break;
    }

    case FormatMode::Minify: {
      Log << Debug << "Format configuration is unused for code minification.";
      auto writer = parse::CodeWriterFactory::Create(*dst_file_ptr);
      ptree_root.value().Accept(*writer);
      okay = true;
      break;
    }

    case FormatMode::Deflate: {
      /**
       * 1. $M = code_minify(source_code)
       * 2. $C = raw_deflate($M)
       * 3. $D = "@(n.emit(n.raw_inflate(n.source_slice(44))))" + $C
       * 4. return $D
       */

      std::unique_ptr<std::stringstream> minified;
      std::unique_ptr<std::stringstream> deflated;

      { /* Perform code minification */
        minified = std::make_unique<std::stringstream>();
        auto writer = parse::CodeWriterFactory::Create(*minified);
        ptree_root.value().Accept(*writer);
      }

      { /* Perform raw deflate */
        deflated = std::make_unique<std::stringstream>();
        if (!DeflateStreams(*minified, *deflated)) {
          Log << "Failed to deflate the minified source code.";
          break;
        }

        minified.reset();
      }

      *dst_file_ptr << "@(n.emit(n.raw_inflate(n.source_slice(44))))" << deflated->rdbuf();

      okay = true;

      break;
    }
  }

  if (src == dst) {
    if (okay) {
      Log << Trace << "Moving temporary file to the source file: " << temporary_path;
      if (!OMNI_CATCH(std::filesystem::rename(temporary_path, dst))) {
        Log << "Failed to move the temporary file to the source file: " << temporary_path << " => " << dst;
        return false;
      }
      Log << Trace << "Successfully moved the temporary file to the source file: " << temporary_path << " => " << dst;
    } else {
      Log << Trace << "Removing temporary file: " << temporary_path;
      if (!OMNI_CATCH(std::filesystem::remove(temporary_path)).value_or(false)) {
        Log << "Failed to remove the temporary file: " << temporary_path;
        return false;
      }
      Log << Trace << "Successfully removed the temporary file: " << temporary_path;
    }
  }

  return okay;
}
