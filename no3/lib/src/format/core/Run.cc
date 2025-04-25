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

#include <boost/program_options.hpp>
#include <boost/program_options/parsers.hpp>
#include <core/cli/Interpreter.hh>
#include <core/package/Manifest.hh>
#include <filesystem>
#include <format/tree/Visitor.hh>
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
#include <nlohmann/json.hpp>
#include <random>
#include <sstream>
#include <unordered_map>

using namespace ncc;
using namespace no3::package;

enum class FormatMode { Standard, Minify, Deflate };

using FileMapping =
    std::pair<std::unordered_map<std::filesystem::path, std::filesystem::path>, std::optional<parse::ImportName>>;

struct FormatOptions {
  FormatMode m_mode = FormatMode::Standard;
  std::filesystem::path m_source_path;
  std::filesystem::path m_output_path;
  std::optional<std::filesystem::path> m_config_path;
  nlohmann::json m_config;
};

static auto SafeCheckFileExists(const std::string& path) -> bool {
  if (auto exists = OMNI_CATCH(std::filesystem::exists(path))) {
    return *exists;
  }

  Log << "Failed to check if the file exists: " << path;
  return false;
}

static auto DeflateStreams(std::istream& in, std::ostream& out) -> bool {
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

static auto FormatFile(const std::filesystem::path& src, const std::filesystem::path& dst, const nlohmann::json& config,
                       FormatMode mode, const ncc::parse::ImportConfig& import_config,
                       const std::shared_ptr<IEnvironment>& env) -> bool {
  Log << Trace << "Formatting file: " << src << " => " << dst;

  std::ifstream src_file(src, std::ios::binary);
  if (!src_file.is_open()) {
    Log << "Failed to open the source file: " << src;
    return false;
  }

  bool quiet_parser = false;
  auto pool = ncc::DynamicArena();
  std::optional<FlowPtr<ncc::parse::Expr>> ptree_root;

  { /* Perform source code parsing */
    auto reenable_log = std::shared_ptr<void>(nullptr, [](auto) { Log->Enable(); });
    if (quiet_parser) {
      Log->Disable();
    }

    auto tokenizer = ncc::lex::Tokenizer(src_file, env);
    tokenizer.SetCurrentFilename(src.string());

    auto parser = ncc::parse::GeneralParser(tokenizer, env, pool, import_config);
    auto ast_result = parser.Parse();

    Log << Trace << "The parser used " << pool.GetSpaceUsed() << " bytes of memory.";
    Log << Trace << "The pipeline allocated " << pool.GetSpaceManaged() << " bytes of memory.";

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

  switch (mode) {
    case FormatMode::Standard: {
      bool has_errors = false;
      auto writer = no3::format::CanonicalFormatterFactory::Create(*dst_file_ptr, has_errors);
      ptree_root.value().Accept(*writer);
      okay = !has_errors;
      if (has_errors) {
        Log << "Failed to format the source file: " << src;
      }

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

      std::unique_ptr<std::stringstream> minified_ss;
      std::unique_ptr<std::stringstream> deflated_ss;

      { /* Perform code minification */
        minified_ss = std::make_unique<std::stringstream>();
        auto writer = parse::CodeWriterFactory::Create(*minified_ss);
        ptree_root.value().Accept(*writer);
      }

      { /* Perform raw deflate */
        deflated_ss = std::make_unique<std::stringstream>();
        if (!DeflateStreams(*minified_ss, *deflated_ss)) {
          Log << "Failed to deflate the minified source code.";
          break;
        }
      }

      const std::string deflated = deflated_ss->str();
      const std::string minified = minified_ss->str();

      if (deflated.size() + 44 < minified.size()) {
        *dst_file_ptr << "@(n.emit(n.raw_inflate(n.source_slice(44))))" << deflated;
      } else {
        *dst_file_ptr << minified;
      }

      okay = true;

      break;
    }
  }

  if (src == dst) {
    if (okay) {
      Log << Trace << "Moving temporary file " << temporary_path << " to the source file.";
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

  Log << Debug << "Successfully formatted the source file: " << src;

  return okay;
}

static bool FormatFiles(const std::optional<parse::ImportName>& current_package_opt,
                        const std::unordered_map<std::filesystem::path, std::filesystem::path>& mapping,
                        FormatMode mode, const nlohmann::json& config) {
  Log << Debug << "Formatting " << mapping.size() << " source file(s).";

  size_t success_count = 0;
  size_t failure_count = 0;
  auto pipeline_env = std::make_shared<ncc::Environment>();

  ncc::parse::ImportConfig import_config = ncc::parse::ImportConfig::GetDefault(pipeline_env);
  if (current_package_opt.has_value()) {
    import_config.SetThisImportName(current_package_opt.value());
    Log << Trace << "Current package name: " << *current_package_opt;
  }

  for (const auto& [src_file, dst_file] : mapping) {
    pipeline_env->Reset();

    import_config.ClearFilesToNotImport();
    import_config.AddFileToNotImport(src_file);

    if (!FormatFile(src_file, dst_file, config, mode, import_config, pipeline_env)) {
      Log << "Unable to format file: " << src_file;
      failure_count++;
      continue;
    }

    Log << Info << "Formatted " << src_file << " => " << dst_file;

    success_count++;
  }

  if (failure_count > 0) {
    Log << Warning << "Unable to format " << failure_count << " source file(s).";
  }

  if (success_count > 0) {
    Log << Info << "Successfully formatted " << success_count << " source file(s).";
  }

  Log << Trace << "Formatted files result: " << success_count << " success, " << failure_count << " failure.";

  return failure_count == 0;
}

#define schema_assert(__expr)                                               \
  if (!(__expr)) [[unlikely]] {                                             \
    Log << "Invalid configuration:" << " schema_assert(" << #__expr << ")"; \
    return false;                                                           \
  }

static auto ValidateConfiguration(const nlohmann::json& j) -> bool {
  Log << Trace << "Validating the JSON format configuration file.";

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

  Log << Trace << "The JSON format configuration file is valid.";

  return true;
}

static void AssignDefaultConfigurationSettings(nlohmann::json& j) {
  Log << Trace << "Assigning default configuration settings.";

  j["whitespace"]["indentation"]["size"] = 2;
  j["whitespace"]["indentation"]["byte"] = " ";

  j["comments"]["line"]["start"] = "//";
  j["comments"]["line"]["end"] = "";
  j["comments"]["line"]["convert-to-block"] = true;

  j["comments"]["block"]["start"] = "/*";
  j["comments"]["block"]["end"] = "*/";
  j["comments"]["block"]["convert-to-line"] = false;

  Log << Trace << "Assigned default configuration settings.";
}

static auto LoadConfigurationFile(const std::filesystem::path& path, nlohmann::json& config) -> bool {
  Log << Trace << "Loading the JSON format configuration file: " << path;

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

  if (!ValidateConfiguration(config)) {
    Log << "The JSON format configuration file is invalid: " << path;
    return false;
  }

  AssignDefaultConfigurationSettings(config);

  Log << Trace << "Loaded the configuration file: " << path;

  return true;
}

static auto GetRecursiveDirectoryContents(const std::filesystem::path& path)
    -> std::optional<std::vector<std::filesystem::path>> {
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

static auto FormulateFileMapping(bool is_directory, const FormatOptions& options) -> std::optional<FileMapping> {
  std::unordered_map<std::filesystem::path, std::filesystem::path> paths;
  std::optional<parse::ImportName> import_name;

  Log << Trace << "Formulating file mapping";

  if (is_directory) {
    Log << Trace << "Source path is a directory: " << options.m_source_path;

    auto contents = GetRecursiveDirectoryContents(options.m_source_path);
    if (!contents.has_value()) {
      Log << "Failed to get the contents of the source directory: " << options.m_source_path;
      return std::nullopt;
    }

    auto manifest_ifstream = std::ifstream(options.m_source_path / "no3.json");
    if (manifest_ifstream.is_open()) {
      if (auto manifest = Manifest::FromJson(manifest_ifstream)) {
        import_name = manifest->GetName();
      }
    }

    Log << Trace << "Found " << contents.value().size() << " files in the source directory.";

    for (const auto& path : contents.value()) {
      if (path.extension() != ".nit") {
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

  Log << Trace << "Formulated file mapping";

  return {{paths, import_name}};
}

static auto DecodeArguments(FormatOptions& options) -> std::optional<FileMapping> {
  {  // Check if the source file exists and absolutize it
    if (!SafeCheckFileExists(options.m_source_path)) {
      Log << "The source path does not exist: " << options.m_source_path;
      return std::nullopt;
    }

    options.m_source_path = std::filesystem::absolute(options.m_source_path);
    Log << Trace << "Source path (absolute) exists: " << options.m_source_path;
  }

  auto is_directory = OMNI_CATCH(std::filesystem::is_directory(options.m_source_path));
  if (!is_directory.has_value()) {
    Log << "Failed to check if the source path is a directory: " << options.m_source_path;
    return std::nullopt;
  }

  {  // Create the output path if it does not exist
    if (*is_directory && !SafeCheckFileExists(options.m_output_path)) {
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
    Log << Trace << "Output path (absolute) exists: " << options.m_output_path;
  }

  {  // Check for the default configuration file in the source directory
    if (*is_directory && !options.m_config_path) {
      if (SafeCheckFileExists(options.m_source_path / "format.json")) {
        options.m_config_path = options.m_source_path / "format.json";
        Log << Debug << "Using the format configuration file in the source directory: " << *options.m_config_path;
      }
    }
  }

  {  // Ensure the configuration file is a file and load it
    if (options.m_config_path) {
      if (!SafeCheckFileExists(*options.m_config_path)) {
        Log << "The configuration file does not exist: " << *options.m_config_path;
        return std::nullopt;
      }
      Log << Trace << "Configuration file exists: " << *options.m_config_path;

      if (!OMNI_CATCH(std::filesystem::is_regular_file(*options.m_config_path)).value_or(false)) {
        Log << "The configuration file is not a regular file: " << *options.m_config_path;
        return std::nullopt;
      }
      Log << Trace << "Configuration file is a regular file: " << *options.m_config_path;

      if (!LoadConfigurationFile(*options.m_config_path, options.m_config)) {
        Log << "Failed to load the configuration file: " << *options.m_config_path;
        return std::nullopt;
      }
    }
  }

  return FormulateFileMapping(*is_directory, options);
}

static auto ParseArguments(const std::vector<const char*>& args, bool& failed) -> std::optional<FormatOptions> {
  failed = true;

  namespace po = boost::program_options;

  Log << Trace << "Parsing command line arguments";

  po::options_description desc("Allowed options");
  po::positional_options_description p;
  p.add("path", 1);

  auto add_option = desc.add_options();
  add_option("help,h", "produce help message");
  add_option("std,s", "canonical source format");
  add_option("minify,m", "source minification (human readable)");
  add_option("deflate,d", "source minification (non-human readable)");
  add_option("config,c", po::value<std::filesystem::path>(), "format configuration file");
  add_option("output,o", po::value<std::filesystem::path>(), "output file or directory");
  add_option("path,f", po::value<std::filesystem::path>(), "source file or directory to format");

  po::variables_map vm;
  if (auto cli_parser = OMNI_CATCH(po::command_line_parser(args.size(), args.data()).options(desc).positional(p).run());
      !cli_parser || !OMNI_CATCH(po::store(*cli_parser, vm)) || !OMNI_CATCH(po::notify(vm))) {
    Log << Error << "Failed to parse command line arguments.";
    desc.print(*(Log << Raw));
    return std::nullopt;
  };

  if (vm.contains("help")) {
    desc.print(*(Log << Raw));
    failed = false;
    return std::nullopt;
  }

  if (!vm.contains("path")) {
    Log << Error << "path: 1 argument(s) expected. 0 provided.";
    desc.print(*(Log << Raw));
    return std::nullopt;
  }

  FormatOptions options;

  options.m_source_path = vm.at("path").as<std::filesystem::path>();
  options.m_output_path = vm.contains("output") ? vm.at("output").as<std::filesystem::path>() : options.m_source_path;
  if (vm.contains("config")) {
    options.m_config_path = vm["config"].as<std::filesystem::path>();
  }

  const auto is_std = vm.count("std");
  const auto is_minify = vm.count("minify");
  const auto is_deflate = vm.count("deflate");
  const auto sum = is_std + is_minify + is_deflate;

  if (sum > 1) {
    Log << Error << "Only one of --std, --minify, or --deflate can be specified.";
    return std::nullopt;
  }

  if (vm.contains("std")) {
    options.m_mode = FormatMode::Standard;
  } else if (vm.contains("minify")) {
    options.m_mode = FormatMode::Minify;
  } else if (vm.contains("deflate")) {
    options.m_mode = FormatMode::Deflate;
  } else {
    options.m_mode = FormatMode::Standard;
  }

  if (options.m_output_path == options.m_source_path) {
    Log << Warning << "The output path is the same as the source path. The source file will be overwritten.";
  }

  Log << Trace << "Command line arguments parsing completed.";

  failed = false;

  return options;
}

auto no3::Interpreter::PImpl::CommandFormat(ConstArguments, const MutArguments& argv) -> bool {
  Log << Trace << "Executing the " << std::source_location::current().function_name();

  FormatOptions options;

  {  // Parse command line arguments
    std::vector<const char*> args;
    args.reserve(argv.size());
    for (const auto& arg : argv) {
      args.push_back(arg.c_str());
    }

    bool failed = true;

    auto options_opt = ParseArguments(args, failed);
    if (!failed && !options_opt.has_value()) {
      return true;
    }

    if (!options_opt.has_value() || failed) {
      Log << Error << "Failed to parse command line arguments.";
      return false;
    }

    options = std::move(*options_opt);
  }

  const auto& [mode, source_path, output_path, config_path, config] = options;
  Log << Trace << "options[\"source\"] = " << source_path;
  Log << Trace << "options[\"output\"] = " << output_path;
  Log << Trace << "options[\"config\"] = " << config_path.value_or("");
  Log << Trace << "options[\"mode\"] = " << static_cast<int>(mode);

  const auto result = DecodeArguments(options);
  if (!result.has_value()) {
    Log << Trace << "Failed to use the command line arguments.";
    return false;
  }

  const auto& [mapping_opt, current_package_opt] = result.value();
  if (mapping_opt.empty()) {
    Log << Warning << "No source files found to format.";
    return true;
  }

  return FormatFiles(current_package_opt, mapping_opt, mode, config);
}
