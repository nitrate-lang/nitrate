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

#include <argparse.h>
#include <glog/logging.h>
#include <lsp/nitrated.h>
#include <nitrate-emit/Code.h>
#include <nitrate-emit/Lib.h>
#include <nitrate-ir/IR.h>
#include <nitrate-ir/Lib.h>
#include <nitrate-seq/Lib.h>

#include <clean/Cleanup.hh>
#include <core/ANSI.hh>
#include <core/Config.hh>
#include <core/Logger.hh>
#include <fstream>
#include <iostream>
#include <memory>
#include <nitrate-core/Init.hh>
#include <nitrate-core/Logger.hh>
#include <nitrate-emit/Classes.hh>
#include <nitrate-ir/Classes.hh>
#include <nitrate-lexer/Init.hh>
#include <nitrate-lexer/Lexer.hh>
#include <nitrate-parser/ASTWriter.hh>
#include <nitrate-parser/Context.hh>
#include <nitrate-parser/Init.hh>
#include <nitrate-seq/Classes.hh>
#include <string_view>
#include <unordered_map>

using namespace argparse;
using namespace no3;
using namespace ncc;
using namespace ncc::lex;
using namespace ncc::parse;

namespace no3::benchmark {
  extern std::string lexical_benchmark_source;

  template <typename T>
  struct Statistic {
    T mean;
    T variance;
    T stddev;
  };

  template <typename T>
  static Statistic<T> calculate_statistic(const std::vector<T> &data) {
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

  static size_t lexer_benchmark_round(std::shared_ptr<Environment> &env) {
    std::stringstream source(lexical_benchmark_source);
    Tokenizer tokenizer(source, env);

    size_t tokens = 0;
    while (!tokenizer.IsEof()) {
      tokenizer.Next();
      tokens++;
    }

    return tokens;
  }

  static int lexer_benchmark() {
    size_t rounds = 128, total_tokens = 0;
    std::vector<double> times;

    auto env = std::make_shared<Environment>();

    LOG(INFO) << "Starting lexer benchmark" << std::endl;
    LOG(INFO) << "  Rounds: " << rounds << std::endl;

    for (size_t i = 0; i < rounds; i++) {
      auto start = std::chrono::high_resolution_clock::now();
      total_tokens += lexer_benchmark_round(env);
      auto end = std::chrono::high_resolution_clock::now();

      double nanoseconds =
          std::chrono::duration_cast<std::chrono::nanoseconds>(end - start)
              .count();
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
      auto stats = calculate_statistic(times);
      LOG(INFO) << "  Round time mean: " << stats.mean << "ns" << std::endl;
      LOG(INFO) << "  Round time variance: " << stats.variance << "ns"
                << std::endl;
      LOG(INFO) << "  Round time standard deviation: " << stats.stddev << "ns"
                << std::endl;

      std::vector<double> time_per_token;
      for (auto time : times) {
        time_per_token.push_back(time / (total_tokens / rounds));
      }

      stats = calculate_statistic(time_per_token);
      LOG(INFO) << "  Per-token time mean: " << stats.mean << "ns" << std::endl;
      LOG(INFO) << "  Per-token time variance: " << stats.variance << "ns"
                << std::endl;
      LOG(INFO) << "  Per-token time standard deviation: " << stats.stddev
                << "ns" << std::endl;

      const double input_size_mbit =
          (lexical_benchmark_source.size() / 1000'000.0) * 8;

      std::vector<double> round_throughputs;
      for (auto time : times) {
        double throughput_mbps = input_size_mbit / (time / 1000000000.0);
        round_throughputs.push_back(throughput_mbps);
      }

      stats = calculate_statistic(round_throughputs);

      LOG(INFO) << "  Throughput mean: " << stats.mean << " mbps" << std::endl;
      LOG(INFO) << "  Throughput variance: " << stats.variance << " mbps"
                << std::endl;
      LOG(INFO) << "  Throughput standard deviation: " << stats.stddev
                << " mbps" << std::endl;
    }

    return 0;
  }

  enum class Benchmark {
    LEXER,
    PARSER,
    NITRATE_IR,
    DELTA_IR,
    LLVM_IR,
    LLVM_CODEGEN,
    C11_CODEGEN,
    PIPELINE
  };

  static int do_benchmark(Benchmark bench_type) {
    int R = -1;

    switch (bench_type) {
      case Benchmark::LEXER: {
        R = lexer_benchmark();
        break;
      }

      case Benchmark::PARSER: {
        /// TODO: Implement benchmark
        break;
      }

      case Benchmark::NITRATE_IR: {
        /// TODO: Implement benchmark
        break;
      }

      case Benchmark::DELTA_IR: {
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

    return R;
  }
}  // namespace no3::benchmark

static int do_parse(std::string source, std::string output, bool verbose) {
  auto env = std::make_shared<Environment>();

  std::fstream file(source, std::ios::in);
  if (!file.is_open()) {
    LOG(ERROR) << "Failed to open source file: " << source;
    return 1;
  }

  qprep lexer(file, "in", env);
  auto parser = Parser::Create(*lexer.get(), env);

  auto ast = parser->parse();

  { /* Write output */
    std::ostream *out = nullptr;
    std::shared_ptr<std::ostream> out_ptr;

    if (output.empty()) {
      out = &std::cout;
    } else {
      out_ptr = std::make_shared<std::ofstream>(output);
      out = out_ptr.get();
    }

    WriterSourceProvider rd =
        verbose ? WriterSourceProvider(*lexer.get()) : std::nullopt;

    AST_JsonWriter writer(*out, rd);
    ast.get().accept(writer);
    *out << std::endl;
  }

  return 0;
}

static int do_nr(std::string source, std::string output, std::string opts,
                 bool verbose) {
  if (!opts.empty()) {
    LOG(ERROR) << "Options are not implemented yet";
  }

  auto env = std::make_shared<Environment>();

  std::fstream file(source, std::ios::in);
  if (!file.is_open()) {
    LOG(ERROR) << "Failed to open source file: " << source;
    return 1;
  }

  qprep lexer(file, "in", env);
  auto parser = Parser::Create(*lexer.get(), env);

  auto ast = parser->parse();

  qmodule mod;
  bool ok = nr_lower(&mod.get(), ast.get().get(), "module", true);

  nr_diag_read(
      mod.get(), ansi::IsUsingColors() ? NR_DIAG_COLOR : NR_DIAG_NOCOLOR,
      [](const uint8_t *msg, size_t len, nr_level_t lvl, uintptr_t verbose) {
        if (verbose || lvl != NR_LEVEL_DEBUG) {
          std::cerr << std::string_view((const char *)msg, len) << std::endl;
        }
      },
      verbose);

  if (!ok) {
    LOG(ERROR) << "Failed to lower source file: " << source;
    return 1;
  }

  { /* Write output */
    FILE *out = output.empty() ? stdout : fopen(output.c_str(), "wb");

    if (!out) {
      LOG(ERROR) << "Failed to open output file: " << output;
      return 1;
    }

    ok = nr_write(mod.get(), nullptr, NR_SERIAL_CODE, out, nullptr, 0);
    if (out != stdout) {
      fclose(out);
    }

    if (!ok) {
      LOG(ERROR) << "Failed to write output file: " << output;
      return 1;
    }
  }

  return 0;
}

static int do_codegen(std::string source, std::string output, std::string opts,
                      std::string target, bool verbose) {
  if (!opts.empty()) {
    LOG(ERROR) << "Options are not implemented yet";
  }

  auto env = std::make_shared<Environment>();

  std::fstream file(source, std::ios::in);
  if (!file.is_open()) {
    LOG(ERROR) << "Failed to open source file: " << source;
    return 1;
  }

  qprep lexer(file, "in", env);
  auto parser = Parser::Create(*lexer.get(), env);

  auto ast = parser->parse();

  qmodule mod;
  bool ok = nr_lower(&mod.get(), ast.get().get(), "module", true);

  nr_diag_read(
      mod.get(), ansi::IsUsingColors() ? NR_DIAG_COLOR : NR_DIAG_NOCOLOR,
      [](const uint8_t *msg, size_t len, nr_level_t lvl, uintptr_t verbose) {
        if (!verbose && lvl == NR_LEVEL_DEBUG) {
          return;
        }

        std::cerr << std::string_view((const char *)msg, len) << std::endl;
      },
      verbose);

  if (!ok) {
    LOG(ERROR) << "Failed to lower source file: " << source;
    return 1;
  }

  bool use_tmpfile = output.empty();

  FILE *out = use_tmpfile ? tmpfile() : fopen(output.c_str(), "wb");

  if (!out) {
    LOG(ERROR) << "Failed to open output file: " << output;
    return 1;
  }

  qcode_conf codegen_conf;
  if (target == "ir") {
    ok = qcode_ir(mod.get(), codegen_conf.get(), stderr, out);
  } else if (target == "asm") {
    ok = qcode_asm(mod.get(), codegen_conf.get(), stderr, out);
  } else if (target == "obj") {
    ok = qcode_obj(mod.get(), codegen_conf.get(), stderr, out);
  } else {
    LOG(ERROR) << "Unknown target specified: " << target;
    return 1;
  }

  if (use_tmpfile) {
    rewind(out);
    char buf[4096];

    while (!feof(out)) {
      size_t len = fread(buf, 1, sizeof(buf), out);
      fwrite(buf, 1, len, stdout);
    }
  }

  fclose(out);

  if (!ok) {
    LOG(ERROR) << "Failed to generate code for source file: " << source;
    return 1;
  }

  return 0;
}

static int do_dev_test() {
  /// TODO: Implement testing
  LOG(ERROR) << "The integrated test suite is not implemented yet";
  return 1;
}

namespace no3::router {
  int run_dev_mode(
      const ArgumentParser &parser,
      const std::unordered_map<std::string_view,
                               std::unique_ptr<ArgumentParser>> &subparsers) {
    qcore_bind_logger([](qcore_log_t, const char *msg, size_t,
                         void *) { std::cerr << msg << std::endl; },
                      nullptr);

    if (parser.is_subcommand_used("bench")) {
      using namespace no3::benchmark;

      auto &bench_parser = *subparsers.at("bench");
      core::SetDebugMode(bench_parser["--verbose"] == true);

      if (bench_parser["--list"] == true) {
        std::cout << "Available benchmarks:" << std::endl;
        std::cout << "  lexer" << std::endl;
        std::cout << "  parser" << std::endl;
        std::cout << "  nitrate-ir" << std::endl;
        std::cout << "  delta-ir" << std::endl;
        std::cout << "  llvm-ir" << std::endl;
        std::cout << "  llvm-codegen" << std::endl;
        std::cout << "  c11-codegen" << std::endl;
        std::cout << "  pipeline" << std::endl;
        return 0;
      }

      if (!bench_parser.is_used("--name")) {
        LOG(ERROR) << "No benchmark specified" << std::endl;
        LOG(ERROR) << bench_parser;
        return 1;
      }

      std::string bench_name = bench_parser.get<std::string>("--name");

      static const std::unordered_map<std::string, Benchmark> name_map = {
          {"lexer", Benchmark::LEXER},
          {"parser", Benchmark::PARSER},
          {"nitrate-ir", Benchmark::NITRATE_IR},
          {"delta-ir", Benchmark::DELTA_IR},
          {"llvm-ir", Benchmark::LLVM_IR},
          {"llvm-codegen", Benchmark::LLVM_CODEGEN},
          {"c11-codegen", Benchmark::C11_CODEGEN},
          {"pipeline", Benchmark::PIPELINE}};

      if (!name_map.contains(bench_name)) {
        LOG(ERROR) << "Unknown benchmark specified" << std::endl;
        LOG(ERROR) << bench_parser;
        return 1;
      }

      return do_benchmark(name_map.at(bench_name));
    } else if (parser.is_subcommand_used("test")) {
      auto &test_parser = *subparsers.at("test");
      core::SetDebugMode(test_parser["--verbose"] == true);

      return do_dev_test();
    } else if (parser.is_subcommand_used("parse")) {
      auto &parse_parser = *subparsers.at("parse");
      bool verbose = parse_parser["--verbose"] == true;
      core::SetDebugMode(verbose);

      std::string source = parse_parser.get<std::string>("source");
      std::string output = parse_parser.get<std::string>("--output");

      return do_parse(source, output, verbose);
    } else if (parser.is_subcommand_used("nr")) {
      auto &nr_parser = *subparsers.at("nr");

      core::SetDebugMode(nr_parser["--verbose"] == true);

      std::string source = nr_parser.get<std::string>("source");
      std::string output = nr_parser.get<std::string>("--output");
      std::string opts = nr_parser.get<std::string>("--opts");

      return do_nr(source, output, opts, nr_parser["--verbose"] == true);
    } else if (parser.is_subcommand_used("codegen")) {
      auto &nr_parser = *subparsers.at("codegen");

      core::SetDebugMode(nr_parser["--verbose"] == true);

      std::string source = nr_parser.get<std::string>("source");
      std::string output = nr_parser.get<std::string>("--output");
      std::string opts = nr_parser.get<std::string>("--opts");
      std::string target = nr_parser.get<std::string>("--target");

      return do_codegen(source, output, opts, target,
                        nr_parser["--verbose"] == true);
    } else if (parser.is_used("--demangle")) {
      std::string mangled_name = parser.get<std::string>("--demangle");
      if (mangled_name.starts_with("@")) {
        mangled_name.erase(0);
      }

      nr::SymbolEncoding codec;
      auto demangled_name = codec.demangle_name(mangled_name);
      if (!demangled_name) {
        LOG(ERROR) << "Failed to demangle symbol" << std::endl;
        return 1;
      }

      std::cout << demangled_name.value() << std::endl;
      return 0;
    } else {
      std::cerr << "Unknown subcommand for dev" << std::endl;
      std::cerr << parser;

      return 1;
    }
  }
}  // namespace no3::router
