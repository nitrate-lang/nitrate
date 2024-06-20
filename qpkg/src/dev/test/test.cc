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
#include <iostream>

int qpkg::dev::test::run_lexer_test(Progress &p) {
  p.result("Lexer", Progress::Result::PASS);

  /// TODO: Implement lexer test
  return 0;
}

int qpkg::dev::test::run_parser_test(Progress &p) {
  p.result("Parser", Progress::Result::PASS);

  /// TODO: Implement parser test
  return 0;
}

int qpkg::dev::test::run_quix_ir_test(Progress &p) {
  p.result("Quix IR", Progress::Result::FAIL);

  /// TODO: Implement Quix IR test
  return 0;
}

int qpkg::dev::test::run_delta_ir_test(Progress &p) {
  p.result("Delta IR", Progress::Result::PASS);

  /// TODO: Implement Delta IR test
  return 0;
}

int qpkg::dev::test::run_llvm_ir_test(Progress &p) {
  p.result("LLVM IR", Progress::Result::PASS);

  /// TODO: Implement LLVM IR test
  return 0;
}

int qpkg::dev::test::run_llvm_codegen_test(Progress &p) {
  p.result("LLVM Codegen", Progress::Result::PASS);

  /// TODO: Implement LLVM codegen test
  return 0;
}

int qpkg::dev::test::run_c11_codegen_test(Progress &p) {
  p.result("C11 Codegen", Progress::Result::PASS);

  /// TODO: Implement C11 codegen test
  return 0;
}

int qpkg::dev::test::run_tests() {
  int ret = 0;

  Progress progress("Quix");

  progress.begin_result();
  ret += run_lexer_test(progress);
  ret += run_parser_test(progress);
  ret += run_quix_ir_test(progress);
  ret += run_delta_ir_test(progress);
  ret += run_llvm_ir_test(progress);
  ret += run_llvm_codegen_test(progress);
  ret += run_c11_codegen_test(progress);
  progress.end_result();

  progress.done();

  return ret;
}

#endif  // QPKG_DEV_TOOLS
