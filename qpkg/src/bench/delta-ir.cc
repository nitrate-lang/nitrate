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

#include <quixcc/Quix.h>

#include <bench/bench.hh>
#include <chrono>
#include <cmath>
#include <fstream>
#include <iostream>

#define ROUNDS 10

#define RESULT_FILE "delta_ir_benchmark.csv"

bool do_bench_delta_ir(std::chrono::system_clock::time_point &start,
                       std::chrono::system_clock::time_point &end) {
  size_t outlen = 0;
  char *outbuf = nullptr;
  FILE *outf = nullptr, *code = nullptr;

  outf = open_memstream(&outbuf, &outlen);
  if (!outf) {
    return false;
  }

  code = fmemopen((void *)qpkg::bench::test_source_code.data(),
                  qpkg::bench::test_source_code.size(), "r");
  if (!code) {
    std::cerr << "do_bench (internal error): Failed to open code stream."
              << std::endl;
    fclose(outf);
    return false;
  }

  start = std::chrono::system_clock::now();
  const char *options[] = {"-emit-delta-ir", NULL};
  char **result = quixcc_compile(code, outf, options);
  if (result != NULL) {
    fclose(outf);
    fclose(code);
    free(outbuf);

    std::cerr << "do_bench (internal error): Compilation failed." << std::endl;
    for (size_t i = 0; result[i] != NULL; i++) {
      std::cerr << result[i] << std::endl;
    }
    return false;
  }
  end = std::chrono::system_clock::now();

  fclose(outf);
  fclose(code);
  free(outbuf);

  return true;
}

static void write_delta_ir_result_csv(const std::vector<double> &throughput) {
  std::ofstream file(RESULT_FILE);
  if (!file.is_open()) {
    std::cerr << "Failed to open file " RESULT_FILE << std::endl;
    return;
  }

  file << "Round,Throughput (Kbit/s)" << std::endl;
  for (size_t i = 0; i < throughput.size(); i++) {
    file << i + 1 << "," << throughput[i] << std::endl;
  }

  file.close();
}

int qpkg::bench::run_benchmark_delta_ir() {
  Progress progress("DeltaIR Codegen");
  std::vector<double> times;

  /*=================== DO BENCHMARK ===================*/
  for (size_t i = 0; i < ROUNDS; i++) {
    std::chrono::system_clock::time_point start, end;
    if (!do_bench_delta_ir(start, end)) {
      std::cerr << "Failed to run benchmark for delta_ir." << std::endl;
      return -1;
    }

    double time_ns =
        std::chrono::duration_cast<std::chrono::nanoseconds>(end - start)
            .count();
    times.push_back(time_ns);

    double full_approx = time_ns * ROUNDS;
    double percent = (time_ns * (i + 1)) / full_approx;

    progress.update(percent, "Round " + std::to_string(i + 1) + " of " +
                                 std::to_string(ROUNDS));
  }

  /*================ CALCULATE RESULTS ================*/
  std::vector<double> throughput;  // Kbit/s

  for (size_t i = 0; i < ROUNDS; i++) {
    size_t total_kbit = qpkg::bench::test_source_code.size() / 1024 * 8;
    double kbit_per_ns = total_kbit / times[i];
    double kbit_per_s = kbit_per_ns * 1e9;
    throughput.push_back(kbit_per_s);
  }

  write_delta_ir_result_csv(throughput);
  progress.done(RESULT_FILE);

  progress.begin_result(Progress::Result::THROUGHPUT);
  double sum = 0;
  double min = throughput[0], max = throughput[0];
  double std_dev = 0;
  for (size_t i = 0; i < throughput.size(); i++) {
    progress.result("Round " + std::to_string(i + 1) + ":\t" +
                    std::to_string(throughput[i]) + " Kbit/s");
    sum += throughput[i];
    if (throughput[i] < min) min = throughput[i];
    if (throughput[i] > max) max = throughput[i];

    std_dev += (throughput[i] - sum / throughput.size()) *
               (throughput[i] - sum / throughput.size());
  }
  std_dev = std::sqrt(std_dev / throughput.size());
  progress.result("Min:\t" + std::to_string(min) + " Kbit/s");
  progress.result("Max:\t" + std::to_string(max) + " Kbit/s");
  progress.result("Mean:\t" + std::to_string(sum / throughput.size()) +
                  " Kbit/s");
  progress.result("Std. Dev.:\t" + std::to_string(std_dev) + " Kbit/s");

  progress.end_result();

  return 0;
}
