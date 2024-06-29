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

#define ROUNDS_PER_BENCH 10

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
    std::vector<double> lexer;
    std::vector<double> parser;
    std::vector<double> quix_ir;
    std::vector<double> delta_ir;
    std::vector<double> llvm_ir;
    std::vector<double> c11;
    std::vector<double> llvm_codegen;
  } suite_perf;

#define RUN_BENCH(name)                                                       \
  do {                                                                        \
    std::chrono::system_clock::time_point start, end;                         \
    std::vector<double> perf;                                                 \
    std::cerr << "\rRunning benchmark for " #name "...          ";            \
    for (size_t i = 0; i < ROUNDS_PER_BENCH; i++) {                           \
      if (!do_bench_##name(start, end)) {                                     \
        std::cerr << "\nFailed to run benchmark for " #name "." << std::endl; \
        return -1;                                                            \
      }                                                                       \
      double time_ns =                                                        \
          std::chrono::duration_cast<std::chrono::nanoseconds>(end - start)   \
              .count();                                                       \
      size_t total_kbit =                                                     \
          qpkg::dev::bench::test_source_code.size() / 1024 * 8;               \
      double kbit_per_ns = total_kbit / time_ns;                              \
      perf.push_back(kbit_per_ns * 1e9);                                      \
    }                                                                         \
    std::cerr << "\rBenchmark for " #name " completed.        \r";            \
    suite_perf.name = perf;                                                   \
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

#define JSON_ARRAY(name)               \
  writer.Key(#name);                   \
  writer.StartArray();                 \
  for (auto &perf : suite_perf.name) { \
    writer.Double(perf);               \
  }                                    \
  writer.EndArray()

  /* Machine readable output for tooling */
  rapidjson::StringBuffer s;
  rapidjson::Writer<rapidjson::StringBuffer> writer(s);
  writer.StartObject();
  JSON_ARRAY(lexer);
  JSON_ARRAY(parser);
  JSON_ARRAY(quix_ir);
  JSON_ARRAY(delta_ir);
  JSON_ARRAY(llvm_ir);
  JSON_ARRAY(c11);
  JSON_ARRAY(llvm_codegen);
  writer.EndObject();

  std::cout << s.GetString() << std::endl;

  return 0;
}