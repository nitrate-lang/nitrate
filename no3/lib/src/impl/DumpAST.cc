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
#include <cstring>
#include <filesystem>
#include <fstream>
#include <impl/Subcommands.hh>
#include <memory>
#include <nitrate-core/Environment.hh>
#include <nitrate-core/LogOStream.hh>
#include <nitrate-core/Logger.hh>
#include <nitrate-lexer/Lexer.hh>
#include <nitrate-parser/ASTBase.hh>
#include <nitrate-parser/ASTWriter.hh>
#include <nitrate-parser/CodeWriter.hh>
#include <nitrate-parser/Context.hh>

using namespace ncc;
using namespace no3::core;
using namespace no3::cmd_impl;

enum class OutputFormat { Json, Protobuf, Minify };

class ImplParseCommandArgumentParser {
  struct Options {
    std::filesystem::path& m_source_path;
    std::filesystem::path& m_output_path;
    OutputFormat m_output_format;
    bool m_dump;
    bool m_tracking;

    Options(std::filesystem::path& source_path, std::filesystem::path& output_path, OutputFormat output_format,
            bool dump, bool tracking)
        : m_source_path(source_path),
          m_output_path(output_path),
          m_output_format(output_format),
          m_dump(dump),
          m_tracking(tracking) {}
  };

  bool m_help = false;
  size_t m_dump = 0;
  size_t m_tracking = 0;
  size_t m_output_format_count = 0;
  OutputFormat m_output_format = OutputFormat::Protobuf;
  std::filesystem::path m_output_path;
  std::filesystem::path m_source_path;
  bool m_too_many_args = false;

  void DisplayHelp() {
    std::string_view help =
        R"(Usage: parse [--help] [--dump] [--tracking] [--to VAR] [--output VAR] path

Positional arguments:
  path    path to a source file to parse

Optional arguments:
  -h, --help          shows this help message and exits
  -d, --dump          pretty print the parse tree
  -s, --tracking      embed source location tracking information in the parse tree
  -t, --to            output format [nargs=0..1] [default: protobuf]
  -o, --output        file to write the serialized parse tree to [nargs=0..1] [default: "-"]
)";

    Log << Raw << help;
  }

  void DoParse(std::vector<std::string> args) {
    constexpr const char* kShortOptions = "hdst:o:";
    constexpr std::array kLongOptions = {
        option{"help", no_argument, nullptr, 'h'},         option{"dump", no_argument, nullptr, 'd'},
        option{"tracking", no_argument, nullptr, 's'},     option{"to", required_argument, nullptr, 't'},
        option{"output", required_argument, nullptr, 'o'}, option{nullptr, 0, nullptr, 0},
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

          case 'd': {
            Log << Trace << "Parsing command line argument: --dump, -d";

            m_output_format = OutputFormat::Json;

            if (m_dump++ > 0) {
              Log << "The -d, --dump argument was provided more than once.";
              m_too_many_args = true;
            }

            break;
          }

          case 's': {
            Log << Trace << "Parsing command line argument: -s, --tracking";
            if (m_tracking++ > 0) {
              Log << "The -s, --tracking argument was provided more than once.";
              m_too_many_args = true;
            }

            break;
          }

          case 't': {
            Log << Trace << "Parsing command line argument: --to, -t";
            if (m_output_format_count++ > 0) {
              Log << "The --to argument was provided more than once.";
              m_too_many_args = true;
              break;
            }

            if (std::strcmp(optarg, "protobuf") == 0) {
              m_output_format = OutputFormat::Protobuf;
            } else if (std::strcmp(optarg, "json") == 0) {
              m_output_format = OutputFormat::Json;
            } else if (std::strcmp(optarg, "minify") == 0) {
              m_output_format = OutputFormat::Minify;
            } else {
              Log << "Unknown output format: " << optarg;
              m_too_many_args = true;
            }

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
        Log << Trace << "No output path provided. Setting it to stdout.";
        m_output_path = "-";
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

    if (m_source_path.empty()) {
      Log << "path: 1 argument(s) expected. 0 provided.";
      okay = false;
    }

    if (m_dump == 1 && m_output_format != OutputFormat::Json) {
      Log << "Output format must be 'json' when using --dump.";
      okay = false;
    }

    return okay;
  }

public:
  ImplParseCommandArgumentParser(const std::vector<std::string>& args, bool& is_valid, bool& performed_action) {
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

  [[nodiscard]] auto GetOptions() -> Options {
    return {m_source_path, m_output_path, m_output_format, m_dump != 0, m_tracking != 0};
  }
};

auto no3::cmd_impl::subcommands::CommandImplParse(ConstArguments, const MutArguments& argv) -> bool {
  bool is_valid = false;
  bool performed_action = false;

  ImplParseCommandArgumentParser state(argv, is_valid, performed_action);

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
  Log << Trace << "options[\"dump\"] = " << (options.m_dump ? "true" : "false");
  Log << Trace << "options[\"tracking\"] = " << (options.m_tracking ? "true" : "false");
  switch (options.m_output_format) {
    case OutputFormat::Json:
      Log << Trace << "options[\"format\"] = json";
      break;
    case OutputFormat::Protobuf:
      Log << Trace << "options[\"format\"] = protobuf";
      break;
    case OutputFormat::Minify:
      Log << Trace << "options[\"format\"] = minify";
      break;
  }

  auto input_file = std::ifstream(options.m_source_path);
  if (!input_file.is_open()) {
    Log << "Failed to open the input file: " << options.m_source_path;
    return false;
  }

  {
    auto pool = ncc::DynamicArena();

    auto import_config = ncc::parse::ImportConfig::GetDefault();

    auto env = std::make_shared<ncc::Environment>();
    auto tokenizer = ncc::lex::Tokenizer(input_file, env);
    tokenizer.SetCurrentFilename(options.m_source_path.string());

    auto parser = ncc::parse::GeneralParser(tokenizer, env, pool, import_config);
    auto ast_result = parser.Parse().Get();

    std::unique_ptr<std::ostream> output_stream;

    if (options.m_output_path == "-") {
      output_stream = std::make_unique<std::ostream>(ncc::clog.rdbuf());
    } else {
      Log << Trace << "Opening the output file: " << options.m_output_path;

      output_stream = std::make_unique<std::ofstream>(options.m_output_path, std::ios::binary);
      if (!output_stream->good()) {
        Log << "Failed to open the output file: " << options.m_output_path;
        return false;
      }

      Log << Trace << "Opened the output file: " << options.m_output_path;
    }

    switch (options.m_output_format) {
      using namespace parse;

      case OutputFormat::Json: {
        auto source_provider = options.m_tracking ? OptionalSourceProvider(tokenizer) : std::nullopt;
        auto ast_writer = ASTWriter(*output_stream, ASTWriter::Format::JSON, source_provider);
        ast_result->Accept(ast_writer);
        break;
      }

      case OutputFormat::Protobuf: {
        auto source_provider = options.m_tracking ? OptionalSourceProvider(tokenizer) : std::nullopt;
        auto ast_writer = ASTWriter(*output_stream, ASTWriter::Format::PROTO, source_provider);
        ast_result->Accept(ast_writer);
        break;
      }

      case OutputFormat::Minify: {
        auto writer = CodeWriterFactory::Create(*output_stream);
        ast_result->Accept(*writer);
        break;
      }
    }

    output_stream->flush();
  }

  return true;
}
