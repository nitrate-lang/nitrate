#include <quixcc.h>

#include <chrono>
#include <cstdio>
#include <cstdlib>

#define ROUNDS 1000000.0

void benchmark() {
  // get time in nanoseconds
  auto start = std::chrono::high_resolution_clock::now();
  for (int i = 0; i < ROUNDS; i++) {
    free(quixcc_demangle("_ZJ0g5hello4f1v00"));
  }
  auto end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double, std::nano> elapsed_ns = end - start;

  double perf = elapsed_ns.count() / ROUNDS;
  printf("Performance: %f ns / call\n", perf);
  printf("Performance: %f calls / s\n", ROUNDS / (elapsed_ns.count() / 1e9));
  printf("Elapsed time: %f s\n", elapsed_ns.count() / 1e9);
}

int main() {
  if (!quixcc_init()) {
    printf("quixcc_init failed\n");
    return 1;
  }

  benchmark();
}