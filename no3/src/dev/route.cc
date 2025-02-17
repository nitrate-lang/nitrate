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

#include <glog/logging.h>
#include <lsp/nitrated.h>
#include <nitrate-emit/Code.h>
#include <nitrate-emit/Lib.h>

#include <argparse.hpp>
#include <clean/Cleanup.hh>
#include <core/ANSI.hh>
#include <core/Config.hh>
#include <core/Logger.hh>
#include <fstream>
#include <iostream>
#include <memory>
#include <nitrate-core/Environment.hh>
#include <nitrate-core/Init.hh>
#include <nitrate-core/Logger.hh>
#include <nitrate-emit/Classes.hh>
#include <nitrate-ir/ABI/Name.hh>
#include <nitrate-ir/IR.hh>
#include <nitrate-ir/Init.hh>
#include <nitrate-ir/Module.hh>
#include <nitrate-lexer/Init.hh>
#include <nitrate-lexer/Lexer.hh>
#include <nitrate-parser/ASTWriter.hh>
#include <nitrate-parser/Context.hh>
#include <nitrate-parser/Init.hh>
#include <nitrate-seq/Init.hh>
#include <nitrate-seq/Sequencer.hh>
#include <string_view>
#include <unordered_map>

using namespace argparse;
using namespace no3;

using namespace ncc;
using namespace ncc::lex;
using namespace ncc::seq;
using namespace ncc::parse;
using namespace ncc::ir;

namespace no3::benchmark {
  extern std::string LexicalBenchmarkSource;

  template <typename T>
  struct Statistic {
    T m_mean;
    T m_variance;
    T m_stddev;
  };

  template <typename T>
  static auto CalculateStatistic(const std::vector<T> &data) -> Statistic<T> {
    T mean = 0.0;
    for (const auto &value : data) {
      mean += value;
    }
    mean /= data.size();

    T variance = 0.0;
    for (const auto &value : data) {
      variance += std::pow(value - mean, 2);
    }
    variance /= data.size();

    return {mean, variance, std::sqrt(variance)};
  }

  static auto LexerBenchmarkRound(std::shared_ptr<Environment> &env) -> size_t {
    std::stringstream source(LexicalBenchmarkSource);
    Tokenizer tokenizer(source, env);

    size_t tokens = 0;
    while (!tokenizer.IsEof()) {
      tokenizer.Next();
      tokens++;
    }

    return tokens;
  }

  static auto LexerBenchmark(std::shared_ptr<Environment> &env) -> int {
    size_t rounds = 128;
    size_t total_tokens = 0;
    std::vector<double> times;

    LOG(INFO) << "Starting lexer benchmark" << std::endl;
    LOG(INFO) << "  Rounds: " << rounds << std::endl;

    for (size_t i = 0; i < rounds; i++) {
      auto start = std::chrono::high_resolution_clock::now();
      total_tokens += LexerBenchmarkRound(env);
      auto end = std::chrono::high_resolution_clock::now();

      double nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
      times.push_back(nanoseconds);

      LOG(INFO) << "Round " << i << ": " << nanoseconds << "ns";
    }

    LOG(INFO) << "Lexer benchmark completed" << std::endl;

    double total_time = 0.0;
    for (auto time : times) {
      total_time += time;
    }

    LOG(INFO) << "Lexer benchmark results:" << std::endl;
    LOG(INFO) << "  Total tokens: " << total_tokens << std::endl;
    LOG(INFO) << "  Tokens per round: " << (total_tokens / rounds) << std::endl;
    LOG(INFO) << "  Rounds: " << rounds << std::endl;
    LOG(INFO) << "  Total time: " << total_time << "ns" << std::endl;

    if (total_tokens > 0) {
      auto stats = CalculateStatistic(times);
      LOG(INFO) << "  Round time mean: " << stats.m_mean << "ns" << std::endl;
      LOG(INFO) << "  Round time variance: " << stats.m_variance << "ns" << std::endl;
      LOG(INFO) << "  Round time standard deviation: " << stats.m_stddev << "ns" << std::endl;

      std::vector<double> time_per_token;
      time_per_token.reserve(times.size());
      for (auto time : times) {
        time_per_token.push_back(time / (total_tokens / rounds));
      }

      stats = CalculateStatistic(time_per_token);
      LOG(INFO) << "  Per-token time mean: " << stats.m_mean << "ns" << std::endl;
      LOG(INFO) << "  Per-token time variance: " << stats.m_variance << "ns" << std::endl;
      LOG(INFO) << "  Per-token time standard deviation: " << stats.m_stddev << "ns" << std::endl;

      const double input_size_mbit = (LexicalBenchmarkSource.size() / 1e6) * 8;

      std::vector<double> round_throughputs;
      for (auto time : times) {
        double throughput_mbps = input_size_mbit / (time / 1e9);
        round_throughputs.push_back(throughput_mbps);
      }

      stats = CalculateStatistic(round_throughputs);

      LOG(INFO) << "  Throughput mean: " << stats.m_mean << " mbps" << std::endl;
      LOG(INFO) << "  Throughput variance: " << stats.m_variance << " mbps" << std::endl;
      LOG(INFO) << "  Throughput standard deviation: " << stats.m_stddev << " mbps" << std::endl;
    }

    return 0;
  }

  static auto SequencerBenchmarkRound(std::shared_ptr<Environment> &env) -> size_t {
    std::stringstream source(LexicalBenchmarkSource);
    Sequencer tokenizer(source, env);

    size_t tokens = 0;
    while (!tokenizer.IsEof()) {
      tokenizer.Next();
      tokens++;
    }

    return tokens;
  }

  static auto SequencerBenchmark(std::shared_ptr<Environment> &env) -> int {
    size_t rounds = 128;
    size_t total_tokens = 0;
    std::vector<double> times;

    LOG(INFO) << "Starting sequencer benchmark" << std::endl;
    LOG(INFO) << "  Rounds: " << rounds << std::endl;

    for (size_t i = 0; i < rounds; i++) {
      auto start = std::chrono::high_resolution_clock::now();
      total_tokens += SequencerBenchmarkRound(env);
      auto end = std::chrono::high_resolution_clock::now();

      double nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
      times.push_back(nanoseconds);

      LOG(INFO) << "Round " << i << ": " << nanoseconds << "ns";
    }

    LOG(INFO) << "Sequencer benchmark completed" << std::endl;

    double total_time = 0.0;
    for (auto time : times) {
      total_time += time;
    }

    LOG(INFO) << "Sequencer benchmark results:" << std::endl;
    LOG(INFO) << "  Total tokens: " << total_tokens << std::endl;
    LOG(INFO) << "  Tokens per round: " << (total_tokens / rounds) << std::endl;
    LOG(INFO) << "  Rounds: " << rounds << std::endl;
    LOG(INFO) << "  Total time: " << total_time << "ns" << std::endl;

    if (total_tokens > 0) {
      auto stats = CalculateStatistic(times);
      LOG(INFO) << "  Round time mean: " << stats.m_mean << "ns" << std::endl;
      LOG(INFO) << "  Round time variance: " << stats.m_variance << "ns" << std::endl;
      LOG(INFO) << "  Round time standard deviation: " << stats.m_stddev << "ns" << std::endl;

      std::vector<double> time_per_token;
      time_per_token.reserve(times.size());
      for (auto time : times) {
        time_per_token.push_back(time / (total_tokens / rounds));
      }

      stats = CalculateStatistic(time_per_token);
      LOG(INFO) << "  Per-token time mean: " << stats.m_mean << "ns" << std::endl;
      LOG(INFO) << "  Per-token time variance: " << stats.m_variance << "ns" << std::endl;
      LOG(INFO) << "  Per-token time standard deviation: " << stats.m_stddev << "ns" << std::endl;

      const double input_size_mbit = (LexicalBenchmarkSource.size() / 1e6) * 8;

      std::vector<double> round_throughputs;
      for (auto time : times) {
        double throughput_mbps = input_size_mbit / (time / 1e9);
        round_throughputs.push_back(throughput_mbps);
      }

      stats = CalculateStatistic(round_throughputs);

      LOG(INFO) << "  Throughput mean: " << stats.m_mean << " mbps" << std::endl;
      LOG(INFO) << "  Throughput variance: " << stats.m_variance << " mbps" << std::endl;
      LOG(INFO) << "  Throughput standard deviation: " << stats.m_stddev << " mbps" << std::endl;
    }

    return 0;
  }

  static void ParserBenchmarkRound(std::shared_ptr<Environment> &env) {
    std::stringstream source(LexicalBenchmarkSource);
    Tokenizer tokenizer(source, env);

    auto ast = Parser::Create(tokenizer, env)->Parse();
    if (!ast.Check()) {
      LOG(ERROR) << "Failed to parse benchmark source";
    }
  }

  static auto ParserBenchmark(std::shared_ptr<Environment> &env) -> int {
    size_t rounds = 128;
    std::vector<double> times;

    LOG(INFO) << "Starting parser benchmark" << std::endl;
    LOG(INFO) << "  Rounds: " << rounds << std::endl;

    for (size_t i = 0; i < rounds; i++) {
      auto start = std::chrono::high_resolution_clock::now();
      ParserBenchmarkRound(env);
      auto end = std::chrono::high_resolution_clock::now();

      double nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
      times.push_back(nanoseconds);

      LOG(INFO) << "Round " << i << ": " << nanoseconds << "ns";
    }

    LOG(INFO) << "Parser benchmark completed" << std::endl;

    double total_time = 0.0;
    for (auto time : times) {
      total_time += time;
    }

    LOG(INFO) << "Parser benchmark results:" << std::endl;
    LOG(INFO) << "  Rounds: " << rounds << std::endl;
    LOG(INFO) << "  Total time: " << total_time << "ns" << std::endl;

    {
      const double input_size_mbit = (LexicalBenchmarkSource.size() / 1e6) * 8;

      std::vector<double> round_throughputs;
      for (auto time : times) {
        double throughput_mbps = input_size_mbit / (time / 1e9);
        round_throughputs.push_back(throughput_mbps);
      }
      auto stats = CalculateStatistic(round_throughputs);

      LOG(INFO) << "  Throughput mean: " << stats.m_mean << " mbps" << std::endl;
      LOG(INFO) << "  Throughput variance: " << stats.m_variance << " mbps" << std::endl;
      LOG(INFO) << "  Throughput standard deviation: " << stats.m_stddev << " mbps" << std::endl;
    }

    return 0;
  }

  enum class Benchmark { LEXER, SEQUENCER, PARSER, NITRATE_IR, LLVM_IR, LLVM_CODEGEN, C11_CODEGEN, PIPELINE };

  static auto DoBenchmark(std::shared_ptr<Environment> &env, Benchmark bench_type) -> int {
    int r = -1;

    switch (bench_type) {
      case Benchmark::LEXER: {
        r = LexerBenchmark(env);
        break;
      }

      case Benchmark::SEQUENCER: {
        r = SequencerBenchmark(env);
        break;
      }

      case Benchmark::PARSER: {
        r = ParserBenchmark(env);
        break;
      }

      case Benchmark::NITRATE_IR: {
        /// TODO: Implement benchmark
        break;
      }

      case Benchmark::LLVM_IR: {
        /// TODO: Implement benchmark
        break;
      }

      case Benchmark::LLVM_CODEGEN: {
        /// TODO: Implement benchmark
        break;
      }

      case Benchmark::C11_CODEGEN: {
        /// TODO: Implement benchmark
        break;
      }

      case Benchmark::PIPELINE: {
        /// TODO: Implement benchmark
        break;
      }
    }

    return r;
  }
}  // namespace no3::benchmark

static auto DoParse(std::shared_ptr<Environment> &env, const std::string &source, std::ostream &output,
                    bool verbose) -> int {
  std::fstream file(source, std::ios::in);
  if (!file.is_open()) {
    LOG(ERROR) << "Failed to open source file: " << source;
    return 1;
  }

  Sequencer scanner(file, env);
  scanner.SetFetchFunc(FileSystemFetchModule);

  auto parser = Parser::Create(scanner, env);

  auto ast = parser->Parse();

  WriterSourceProvider rd = verbose ? WriterSourceProvider(scanner) : std::nullopt;

  output << ast.Get()->DebugString(rd);

  return 0;
}

static auto DoNr(std::shared_ptr<Environment> &env, const std::string &source, std::ostream &output,
                 const std::string &opts) -> int {
  if (!opts.empty()) {
    LOG(ERROR) << "Options are not implemented yet";
  }

  std::fstream file(source, std::ios::in);
  if (!file.is_open()) {
    LOG(ERROR) << "Failed to open source file: " << source;
    return 1;
  }

  Sequencer scanner(file, env);
  scanner.SetFetchFunc(FileSystemFetchModule);

  auto parser = Parser::Create(scanner, env);

  auto ast = parser->Parse();

  if (auto module = NrLower(ast.Get().get(), "module", true)) {
    NrWrite(module.get(), nullptr, output);
  } else {
    LOG(ERROR) << "Failed to lower source file: " << source;
    return 1;
  }

  return 0;
}

static auto DoCodegen(std::shared_ptr<Environment> &env, const std::string &source, const std::string &output,
                      const std::string &opts, const std::string &target) -> int {
  if (!opts.empty()) {
    LOG(ERROR) << "Options are not implemented yet";
  }

  std::fstream file(source, std::ios::in);
  if (!file.is_open()) {
    LOG(ERROR) << "Failed to open source file: " << source;
    return 1;
  }

  Sequencer scanner(file, env);
  scanner.SetFetchFunc(FileSystemFetchModule);

  auto parser = Parser::Create(scanner, env);

  auto ast = parser->Parse();

  if (auto module = NrLower(ast.Get().get(), "module", true)) {
    bool use_tmpfile = output.empty();

    FILE *out = use_tmpfile ? tmpfile() : fopen(output.c_str(), "wb");

    if (out == nullptr) {
      LOG(ERROR) << "Failed to open output file: " << output;
      return 1;
    }

    bool ok = false;

    QcodeConf codegen_conf;
    if (target == "ir") {
      ok = QcodeIR(module.get(), codegen_conf.Get(), stderr, out);
    } else if (target == "asm") {
      ok = QcodeAsm(module.get(), codegen_conf.Get(), stderr, out);
    } else if (target == "obj") {
      ok = QcodeObj(module.get(), codegen_conf.Get(), stderr, out);
    } else {
      LOG(ERROR) << "Unknown target specified: " << target;
      return 1;
    }

    if (use_tmpfile) {
      rewind(out);
      char buf[4096];

      while (feof(out) == 0) {
        size_t len = fread(buf, 1, sizeof(buf), out);
        fwrite(buf, 1, len, stdout);
      }
    }

    fclose(out);

    if (!ok) {
      LOG(ERROR) << "Failed to generate code for source file: " << source;
      return 1;
    }
  } else {
    LOG(ERROR) << "Failed to lower source file: " << source;
    return 1;
  }

  return 0;
}

static auto DoDevTest() -> int {
  /// TODO: Implement testing
  LOG(ERROR) << "The integrated test suite is not implemented yet";
  return 1;
}

namespace no3::router {
  auto RunDevMode(const ArgumentParser &parser,
                  const std::unordered_map<std::string_view, std::unique_ptr<ArgumentParser>> &subparsers) -> int {
    ncc::EnableSync = false;

    std::shared_ptr<Environment> env = std::make_shared<Environment>();

    if (parser.IsSubcommandUsed("bench")) {
      using namespace no3::benchmark;

      auto &bench_parser = *subparsers.at("bench");
      core::SetDebugMode(bench_parser["--verbose"] == true);

      if (bench_parser["--list"] == true) {
        std::cout << "Available benchmarks:" << std::endl;
        std::cout << "  lexer" << std::endl;
        std::cout << "  sequencer" << std::endl;
        std::cout << "  parser" << std::endl;
        std::cout << "  nitrate-ir" << std::endl;
        std::cout << "  llvm-ir" << std::endl;
        std::cout << "  llvm-codegen" << std::endl;
        std::cout << "  c11-codegen" << std::endl;
        std::cout << "  pipeline" << std::endl;
        return 0;
      }

      if (!bench_parser.IsUsed("--name")) {
        LOG(ERROR) << "No benchmark specified" << std::endl;
        LOG(ERROR) << bench_parser;
        return 1;
      }

      auto bench_name = bench_parser.Get<std::string>("--name");

      static const std::unordered_map<std::string, Benchmark> name_map = {{"lexer", Benchmark::LEXER},
                                                                          {"sequencer", Benchmark::SEQUENCER},
                                                                          {"parser", Benchmark::PARSER},
                                                                          {"nitrate-ir", Benchmark::NITRATE_IR},
                                                                          {"llvm-ir", Benchmark::LLVM_IR},
                                                                          {"llvm-codegen", Benchmark::LLVM_CODEGEN},
                                                                          {"c11-codegen", Benchmark::C11_CODEGEN},
                                                                          {"pipeline", Benchmark::PIPELINE}};

      if (!name_map.contains(bench_name)) {
        LOG(ERROR) << "Unknown benchmark specified" << std::endl;
        LOG(ERROR) << bench_parser;
        return 1;
      }

      return DoBenchmark(env, name_map.at(bench_name));
    }

    if (parser.IsSubcommandUsed("test")) {
      auto &test_parser = *subparsers.at("test");
      core::SetDebugMode(test_parser["--verbose"] == true);

      return DoDevTest();
    }

    if (parser.IsSubcommandUsed("parse")) {
      auto &parse_parser = *subparsers.at("parse");
      bool verbose = parse_parser["--verbose"] == true;
      core::SetDebugMode(verbose);

      auto source = parse_parser.Get<std::string>("source");
      auto output = parse_parser.Get<std::string>("--output");

      auto out =
          output.empty() ? std::make_unique<std::ostream>(std::cout.rdbuf()) : std::make_unique<std::ofstream>(output);

      return DoParse(env, source, *out, verbose);
    }

    if (parser.IsSubcommandUsed("nr")) {
      auto &nr_parser = *subparsers.at("nr");

      core::SetDebugMode(nr_parser["--verbose"] == true);

      auto source = nr_parser.Get<std::string>("source");
      auto output = nr_parser.Get<std::string>("--output");
      auto opts = nr_parser.Get<std::string>("--opts");

      auto out =
          output.empty() ? std::make_unique<std::ostream>(std::cout.rdbuf()) : std::make_unique<std::ofstream>(output);

      return DoNr(env, source, *out, opts);
    }

    if (parser.IsSubcommandUsed("codegen")) {
      auto &nr_parser = *subparsers.at("codegen");

      core::SetDebugMode(nr_parser["--verbose"] == true);

      auto source = nr_parser.Get<std::string>("source");
      auto output = nr_parser.Get<std::string>("--output");
      auto opts = nr_parser.Get<std::string>("--opts");
      auto target = nr_parser.Get<std::string>("--target");

      return DoCodegen(env, source, output, opts, target);
    }

    if (parser.IsUsed("--demangle")) {
      auto mangled_name = parser.Get<std::string>("--demangle");
      if (mangled_name.starts_with("@")) {
        mangled_name.erase(0);
      }

      auto demangled_name = ExpandSymbolName(mangled_name);
      if (!demangled_name) {
        LOG(ERROR) << "Failed to demangle symbol" << std::endl;
        return 1;
      }

      std::cout << demangled_name.value() << std::endl;
      return 0;
    }

    std::cerr << "Unknown subcommand for dev" << std::endl;
    std::cerr << parser;

    return 1;
  }
}  // namespace no3::router
