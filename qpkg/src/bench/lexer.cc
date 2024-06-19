#include <quixcc/Quix.h>

#include <bench/bench.hh>
#include <chrono>
#include <cmath>
#include <fstream>
#include <iostream>

#define ROUNDS 100

#define RESULT_FILE "lexer_benchmark.csv"

bool do_bench(size_t *total_toks, std::chrono::system_clock::time_point &start,
              std::chrono::system_clock::time_point &end) {
  size_t outlen = 0, tokens = 0;
  quixcc_tok_t tok;
  char *outbuf = nullptr;
  FILE *outf = nullptr, *code = nullptr;
  quixcc_job_t *job = nullptr;

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

  job = quixcc_new();
  if (!job) {
    std::cerr << "do_bench (internal error): Failed to create job."
              << std::endl;
    fclose(outf);
    fclose(code);
    return false;
  }

  quixcc_output(job, outf, nullptr);
  quixcc_source(job, code, "bench");

  start = std::chrono::system_clock::now();
  while (1) {
    tok = quixcc_next(job);
    if (quixcc_lex_is(&tok, QUIXCC_LEX_EOF)) break;

    quixcc_tok_release(job, &tok);

    tokens++;
  }
  end = std::chrono::system_clock::now();

  if (!quixcc_dispose(job)) {
    std::cerr << "do_bench (internal error): Failed to dispose job."
              << std::endl;
    fclose(outf);
    fclose(code);
    free(outbuf);
    return false;
  }

  fclose(outf);
  fclose(code);
  free(outbuf);

  *total_toks = tokens;

  return true;
}

static void write_lexer_result_csv(const std::vector<double> &throughput) {
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

int qpkg::bench::run_benchmark_lexer() {
  Progress progress("Lexer");
  std::vector<double> times;

  /*=================== DO BENCHMARK ===================*/
  for (size_t i = 0; i < ROUNDS; i++) {
    size_t total_toks = 0;

    std::chrono::system_clock::time_point start, end;
    if (!do_bench(&total_toks, start, end)) {
      std::cerr << "Failed to run benchmark for lexer." << std::endl;
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

  write_lexer_result_csv(throughput);
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
