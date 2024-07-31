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

#ifndef __QPKG_BENCH_HH__
#define __QPKG_BENCH_HH__

#include <chrono>
#include <cstdint>
#include <queue>
#include <set>
#include <sstream>
#include <string>
#include <string_view>

namespace qpkg {
  namespace dev::bench {
    int run_benchmark_lexer();
    int run_benchmark_parser();
    int run_benchmark_quix_ir();
    int run_benchmark_delta_ir();
    int run_benchmark_llvm_ir();
    int run_benchmark_llvm_codegen();
    int run_benchmark_c11_codegen();
    int run_benchmark_pipeline();

    extern const std::string_view test_source_code;

    class Progress {
      std::string m_title;
      std::string m_last_msg;
      std::queue<std::pair<std::string, double>> m_queue;
      std::chrono::high_resolution_clock::time_point m_last_print;
      constexpr static std::chrono::milliseconds m_print_interval = std::chrono::milliseconds(1000);

      void print(double percent, const std::string &msg);

  public:
      Progress(const std::string &title);

      void update(double percent, const std::string &msg = "");
      void done(const std::string &outfile_name = "");

      enum class Result {
        THROUGHPUT,
        MEMORY,
        TIME,
      };

      void begin_result(Result type);
      void result(const std::string &msg);
      void end_result();
    };
  } // namespace dev::bench

} // namespace qpkg

#endif // __QPKG_BENCH_HH__