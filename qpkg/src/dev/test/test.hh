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

#include <core/Config.hh>
#include <map>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#if QPKG_DEV_TOOLS

#ifndef __QPKG_DEV_TEST_HH__
#define __QPKG_DEV_TEST_HH__

#include <chrono>
#include <cstdint>
#include <queue>
#include <set>
#include <sstream>
#include <string>
#include <string_view>

namespace qpkg::dev::test {
class Progress {
  std::string m_title;
  bool m_all_okay;

 public:
  Progress(const std::string &title);

  enum class Result {
    PASS,
    FAIL,
  };

  void begin_result();
  void result(const std::string &msg, Result type);
  void end_result();

  void begin_detail();
  void detail(std::string_view test_id, std::string_view name, Result type);
  void end_detail();

  void done(const std::string &outfile_name = "");
};

int run_tests();

int run_lexer_test(
    Progress &progress,
    std::map<std::string_view, std::map<std::string_view, bool>> &tests);
int run_parser_test(
    Progress &progress,
    std::map<std::string_view, std::map<std::string_view, bool>> &tests);
int run_quix_ir_test(
    Progress &progress,
    std::map<std::string_view, std::map<std::string_view, bool>> &tests);
int run_delta_ir_test(
    Progress &progress,
    std::map<std::string_view, std::map<std::string_view, bool>> &tests);
int run_llvm_ir_test(
    Progress &progress,
    std::map<std::string_view, std::map<std::string_view, bool>> &tests);
int run_llvm_codegen_test(
    Progress &progress,
    std::map<std::string_view, std::map<std::string_view, bool>> &tests);
int run_c11_codegen_test(
    Progress &progress,
    std::map<std::string_view, std::map<std::string_view, bool>> &tests);

extern const std::map<std::pair<std::string_view, std::string_view>,
                      std::pair<std::string_view, std::vector<quixcc_tok_t>>>
    g_lexer_test_vectors;

}  // namespace qpkg::dev::test

#endif

#endif  // QPKG_DEV_TOOLS
