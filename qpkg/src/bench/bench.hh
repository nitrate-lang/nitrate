#ifndef __QPKG_BENCH_HH__
#define __QPKG_BENCH_HH__

#include <set>
#include <string>

namespace qpkg {
namespace bench {
int run_benchmark_lexer();
int run_benchmark_preprocessor();
int run_benchmark_parser();
int run_benchmark_q_ir();
int run_benchmark_delta_ir();
int run_benchmark_llvm_ir();
int run_benchmark_llvm_codegen();
int run_benchmark_c11_codegen();
int run_benchmark_cxx_codegen();
int run_benchmark_pipeline();
}  // namespace bench
}  // namespace qpkg

#endif  // __QPKG_BENCH_HH__