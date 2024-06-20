////////////////////////////////////////////////////////////////////////////////
///                                                                          ///
///           ░▒▓██████▓▒░░▒▓███████▓▒░░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░            ///
///          ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░           ///
///          ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░                  ///
///          ░▒▓█▓▒░░▒▓█▓▒░▒▓███████▓▒░░▒▓███████▓▒░░▒▓█▓▒▒▓███▓▒░           ///
///          ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░           ///
///          ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░           ///
///           ░▒▓██████▓▒░░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░            ///
///             ░▒▓█▓▒░                                                      ///
///              ░▒▓██▓▒░                                                    ///
///                                                                          ///
///   * QUIX PACKAGE MANAGER - The official tool for the Quix language.      ///
///   * Copyright (C) 2024 Wesley C. Jones                                   ///
///                                                                          ///
///   The QUIX Compiler Suite is free software; you can redistribute it or   ///
///   modify it under the terms of the GNU Lesser General Public             ///
///   License as published by the Free Software Foundation; either           ///
///   version 2.1 of the License, or (at your option) any later version.     ///
///                                                                          ///
///   The QUIX Compiler Suite is distributed in the hope that it will be     ///
///   useful, but WITHOUT ANY WARRANTY; without even the implied warranty of ///
///   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      ///
///   Lesser General Public License for more details.                        ///
///                                                                          ///
///   You should have received a copy of the GNU Lesser General Public       ///
///   License along with the QUIX Compiler Suite; if not, see                ///
///   <https://www.gnu.org/licenses/>.                                       ///
///                                                                          ///
////////////////////////////////////////////////////////////////////////////////

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include <dev/bench/bench.hh>
#include <iostream>

bool do_bench_lexer(std::chrono::system_clock::time_point &start,
                    std::chrono::system_clock::time_point &end);
bool do_bench_parser(std::chrono::system_clock::time_point &start,
                     std::chrono::system_clock::time_point &end);
bool do_bench_quix_ir(std::chrono::system_clock::time_point &start,
                      std::chrono::system_clock::time_point &end);
bool do_bench_delta_ir(std::chrono::system_clock::time_point &start,
                       std::chrono::system_clock::time_point &end);
bool do_bench_llvm_ir(std::chrono::system_clock::time_point &start,
                      std::chrono::system_clock::time_point &end);
bool do_bench_c11(std::chrono::system_clock::time_point &start,
                  std::chrono::system_clock::time_point &end);
bool do_bench_llvm_codegen(std::chrono::system_clock::time_point &start,
                           std::chrono::system_clock::time_point &end);

int qpkg::dev::bench::run_benchmark_pipeline() {
  struct SuitePerf {
    double lexer;
    double parser;
    double quix_ir;
    double delta_ir;
    double llvm_ir;
    double c11;
    double llvm_codegen;
  } suite_perf{};

#define RUN_BENCH(name)                                                     \
  do {                                                                      \
    std::chrono::system_clock::time_point start, end;                       \
    std::cerr << "\rRunning benchmark for " #name "...          ";          \
    if (!do_bench_##name(start, end)) {                                     \
      std::cerr << "\nFailed to run benchmark for " #name "." << std::endl; \
      return -1;                                                            \
    }                                                                       \
    std::cerr << "\rBenchmark for " #name " completed.        \r";          \
    double time_ns =                                                        \
        std::chrono::duration_cast<std::chrono::nanoseconds>(end - start)   \
            .count();                                                       \
    size_t total_kbit = qpkg::dev::bench::test_source_code.size() / 1024 * 8;    \
    double kbit_per_ns = total_kbit / time_ns;                              \
    double kbit_per_s = kbit_per_ns * 1e9;                                  \
    suite_perf.name = kbit_per_s;                                           \
                                                                            \
  } while (0)

  RUN_BENCH(lexer);
  RUN_BENCH(parser);
  RUN_BENCH(quix_ir);
  RUN_BENCH(delta_ir);
  RUN_BENCH(llvm_ir);
  RUN_BENCH(c11);
  RUN_BENCH(llvm_codegen);

  std::cerr << "\r                                              \r";

  /* Machine readable output for tooling */
  rapidjson::StringBuffer s;
  rapidjson::Writer<rapidjson::StringBuffer> writer(s);
  writer.StartObject();
  writer.Key("lexer");
  writer.Double(suite_perf.lexer);
  writer.Key("parser");
  writer.Double(suite_perf.parser);
  writer.Key("quix_ir");
  writer.Double(suite_perf.quix_ir);
  writer.Key("delta_ir");
  writer.Double(suite_perf.delta_ir);
  writer.Key("llvm_ir");
  writer.Double(suite_perf.llvm_ir);
  writer.Key("c11");
  writer.Double(suite_perf.c11);
  writer.Key("llvm_codegen");
  writer.Double(suite_perf.llvm_codegen);
  writer.EndObject();

  std::cout << s.GetString() << std::endl;

  return 0;
}