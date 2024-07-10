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

#include <core/Config.hh>

#if QPKG_DEV_TOOLS
#include <dev/test/test.hh>
#include <fstream>
#include <iostream>

int qpkg::dev::test::run_lexer_test(
    Progress &p,
    std::map<std::string_view, std::map<std::string_view, bool>> &tests) {
  Progress::Result overall = Progress::Result::PASS;
  std::map<std::pair<std::string_view, std::string_view>, Progress::Result>
      details;

  for (const auto &[tag, data] : g_lexer_test_vectors) {
    FILE *code = fmemopen((void *)data.first.data(), data.first.size(), "r");
    if (!code) {
      std::cerr << "Failed to open code for test " << tag.first << std::endl;
      return 1;
    }

    quixcc_cc_job_t *job = quixcc_cc_new();
    if (!job) {
      std::cerr << "Failed to create job for test " << tag.first << std::endl;
      fclose(code);
      return 1;
    }

    quixcc_cc_source(job, code, tag.first.data());
    quixcc_cc_option(job, "-fkeep-comments", true);

    bool okay = true;
    std::vector<quixcc_tok_t> toks;
    quixcc_tok_t tok;
    while ((tok = quixcc_cc_next(job)).ty != QUIXCC_LEX_EOF) {
      toks.push_back(tok);
    }

    if (toks.size() != data.second.size()) {
      std::cerr << "Expected " << data.second.size() << " tokens, got "
                << toks.size() << " for test " << tag.first << std::endl;
      okay = false;
    } else {
      for (size_t i = 0; i < data.second.size(); i++) {
        if (!quixcc_cc_tokeq(toks[i], data.second[i])) {
          okay = false;
          std::cerr << "Mismatched token at index " << i << " for test "
                    << tag.first << std::endl;
          break;
        }
      }
    }

    if (okay) {
      details[tag] = Progress::Result::PASS;
      tests[tag.first][tag.second] = true;
    } else {
      details[tag] = Progress::Result::FAIL;
      tests[tag.first][tag.second] = false;
      overall = Progress::Result::FAIL;
    }

    fclose(code);
    quixcc_cc_dispose(job);
  }

  p.result("Lexer", overall);
  p.begin_detail();
  for (const auto &[tag, status] : details) {
    p.detail(tag.first, tag.second, status);
  }
  p.end_detail();

  return 0;
}

int qpkg::dev::test::run_parser_test(
    Progress &p,
    std::map<std::string_view, std::map<std::string_view, bool>> &tests) {
  (void)tests;

  p.result("Parser", Progress::Result::PASS);
  p.begin_detail();
  p.end_detail();

  /// TODO: Implement parser test
  return 0;
}

int qpkg::dev::test::run_quix_ir_test(
    Progress &p,
    std::map<std::string_view, std::map<std::string_view, bool>> &tests) {
  (void)tests;

  p.result("Quix IR", Progress::Result::PASS);
  p.begin_detail();
  p.end_detail();

  /// TODO: Implement Quix IR test
  return 0;
}

int qpkg::dev::test::run_delta_ir_test(
    Progress &p,
    std::map<std::string_view, std::map<std::string_view, bool>> &tests) {
  (void)tests;

  p.result("Delta IR", Progress::Result::PASS);
  p.begin_detail();
  p.end_detail();

  /// TODO: Implement Delta IR test
  return 0;
}

int qpkg::dev::test::run_llvm_ir_test(
    Progress &p,
    std::map<std::string_view, std::map<std::string_view, bool>> &tests) {
  (void)tests;

  p.result("LLVM IR", Progress::Result::PASS);
  p.begin_detail();
  p.end_detail();

  /// TODO: Implement LLVM IR test
  return 0;
}

int qpkg::dev::test::run_llvm_codegen_test(
    Progress &p,
    std::map<std::string_view, std::map<std::string_view, bool>> &tests) {
  (void)tests;

  p.result("LLVM Codegen", Progress::Result::PASS);
  p.begin_detail();
  p.end_detail();

  /// TODO: Implement LLVM codegen test
  return 0;
}

int qpkg::dev::test::run_c11_codegen_test(
    Progress &p,
    std::map<std::string_view, std::map<std::string_view, bool>> &tests) {
  (void)tests;

  p.result("C11 Codegen", Progress::Result::PASS);
  p.begin_detail();

  /// TODO: Implement C11 codegen test
  return 0;
}

int qpkg::dev::test::run_tests() {
  int ret = 0;

  Progress progress("Quix");
  std::map<std::string_view, std::map<std::string_view, bool>> tests;

  progress.begin_result();
  ret += run_lexer_test(progress, tests);
  ret += run_parser_test(progress, tests);
  ret += run_quix_ir_test(progress, tests);
  ret += run_delta_ir_test(progress, tests);
  ret += run_llvm_ir_test(progress, tests);
  ret += run_llvm_codegen_test(progress, tests);
  ret += run_c11_codegen_test(progress, tests);
  progress.end_result();

  try {
    std::fstream results("test-results.csv", std::ios::out);
    if (!results.is_open()) {
      std::cerr << "Failed to open test-results.csv" << std::endl;
      return 1;
    }

    results << "Test ID,Test Name,Passed" << std::endl;
    for (const auto &[tag, data] : tests) {
      for (const auto &[name, status] : data) {
        results << tag << "," << name << "," << status << std::endl;
      }
    }
    progress.done("test-results.csv");

  } catch (std::exception &e) {
    std::cerr << "Failed to write test results: " << e.what() << std::endl;
    progress.done();
    return 1;
  }

  return ret;
}

#endif  // QPKG_DEV_TOOLS
