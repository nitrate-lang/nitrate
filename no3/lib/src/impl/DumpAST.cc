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
#include <cstring>
#include <fstream>
#include <impl/Subcommands.hh>
#include <memory>
#include <nitrate-core/Allocate.hh>
#include <nitrate-core/CatchAll.hh>
#include <nitrate-core/Environment.hh>
#include <nitrate-core/Logger.hh>
#include <nitrate-lexer/Lexer.hh>
#include <nitrate-parser/ASTBase.hh>
#include <nitrate-parser/ASTWriter.hh>
#include <nitrate-parser/CodeWriter.hh>
#include <nitrate-parser/Context.hh>

using namespace ncc;
using namespace no3::cmd_impl;

enum class OutputFormat { Json, Protobuf, Minify };

static bool ParseFile(const auto& source_path, const auto& output_path, const auto& dump, const auto& tracking,
                      const auto& output_format, auto& env) {
  env->Reset();

  Log << Trace << "options[\"source\"] = " << source_path;
  Log << Trace << "options[\"output\"] = " << output_path;
  Log << Trace << "options[\"dump\"] = " << (dump ? "true" : "false");
  Log << Trace << "options[\"tracking\"] = " << (tracking ? "true" : "false");
  switch (output_format) {
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

  auto input_file = std::ifstream(source_path);
  if (!input_file.is_open()) {
    Log << "Failed to open the input file: " << source_path;
    return false;
  }

  {
    auto pool = ncc::DynamicArena();
    auto import_config = ncc::parse::ImportConfig::GetDefault(env);
    auto tokenizer = ncc::lex::Tokenizer(input_file, env);
    tokenizer.SetCurrentFilename(source_path);

    auto parser = ncc::parse::GeneralParser(tokenizer, env, pool, import_config);
    auto ast_result = parser.Parse().Get();

    std::unique_ptr<std::ostream> output_stream;

    if (output_path == "-") {
      output_stream = (Log << Raw);
    } else {
      Log << Trace << "Opening the output file: " << output_path;

      output_stream = std::make_unique<std::ofstream>(output_path, std::ios::binary);
      if (!output_stream->good()) {
        Log << "Failed to open the output file: " << output_path;
        return false;
      }

      Log << Trace << "Opened the output file: " << output_path;
    }

    switch (output_format) {
      using namespace parse;

      case OutputFormat::Json: {
        auto source_provider = tracking ? OptionalSourceProvider(tokenizer) : std::nullopt;
        auto ast_writer = ASTWriter(*output_stream, ASTWriter::Format::JSON, source_provider);
        ast_result->Accept(ast_writer);
        break;
      }

      case OutputFormat::Protobuf: {
        auto source_provider = tracking ? OptionalSourceProvider(tokenizer) : std::nullopt;
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

auto no3::cmd_impl::subcommands::CommandImplParse(ConstArguments, const MutArguments& argv) -> bool {
  namespace po = boost::program_options;

  po::options_description desc("Allowed options");
  po::positional_options_description p;
  p.add("source", -1);

  auto add_option = desc.add_options();
  add_option("help,h", "produce help message");
  add_option("dump,d", "pretty print the parse tree");
  add_option("tracking,t", "retain source location information");
  add_option("format,f", po::value<std::string>()->default_value("json"), "output format");
  add_option("output,o", po::value<std::string>()->default_value("-"), "destination of serialized parse tree");
  add_option("source,s", po::value<std::vector<std::string>>(), "source file to parse");

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

  if (!vm.contains("source")) {
    Log << "source: 1 argument(s) expected. 0 provided.";
    desc.print(*(Log << Raw));
    return false;
  }

  const auto dump = vm.contains("dump");
  const auto tracking = vm.contains("tracking");
  const auto source_paths = vm.at("source").as<std::vector<std::string>>();
  const auto output_path = vm.at("output").as<std::string>();
  const auto output_format = [&] {
    std::string format = vm.at("format").as<std::string>();
    if (format == "json") {
      return OutputFormat::Json;
    }

    if (format == "protobuf") {
      return OutputFormat::Protobuf;
    }

    if (format == "minify") {
      return OutputFormat::Minify;
    }

    Log << Error << "Invalid output format: " << format;
    return OutputFormat::Json;  // Default to JSON
  }();

  auto env = std::make_shared<ncc::Environment>();

  for (const auto& source_path : source_paths) {
    if (!ParseFile(source_path, output_path, dump, tracking, output_format, env)) {
      Log << Error << "Failed to parse file: " << source_path;
      return false;
    }
  }

  return true;
}
