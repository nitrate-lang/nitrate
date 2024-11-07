////////////////////////////////////////////////////////////////////////////////
///                                                                          ///
///  ░▒▓██████▓▒░░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░ ░▒▓██████▓▒░  ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░        ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓██████▓▒░░▒▓█▓▒░      ░▒▓█▓▒░        ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░        ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ///
///  ░▒▓██████▓▒░ ░▒▓██████▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░ ░▒▓██████▓▒░  ///
///    ░▒▓█▓▒░                                                               ///
///     ░▒▓██▓▒░                                                             ///
///                                                                          ///
///   * QUIX LANG COMPILER - The official compiler for the Quix language.    ///
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

#include <quix-core/Error.h>

#include <core/PassManager.hh>
#include <passes/PassList.hh>
#include <quix-qxir/Module.hh>

void qxir::pass::PassRegistry::link_builtin() {
  addPass("ds-acyclic", ds_acyclic);
  addPass("ds-nullchk", ds_nullchk);
  addPass("ds-resolv", ds_resolv);
  addPass("ds-verify", ds_verify);
  addPass("ds-flatten", ds_flatten);
  addPass("ds-tyinfer", ds_tyinfer);
  addPass("ds-mangle", ds_mangle);
  addPass("ds-clean", ds_clean);
  addPass("ds-raii", ds_raii);

  addPass("chk-missing-return", chk_missing_return);
  addPass("chk-bad-cast", chk_bad_cast);
}

void qxir::pass::PassGroupRegistry::RegisterBuiltinGroups() {
  PassGroupBuilder()
      .addPass("ds-clean") /* Cleanup IR */
      /* Add more cleanup passes: [dead-code removal, ?] */
      .build("reduce");

  PassGroupBuilder()
      .addPass("ds-acyclic") /* Verify that the module is acyclic */
      .addPass("ds-nullchk") /* Verify that the module is null-safe */
      .addPass("ds-resolv")  /* Resolve all symbols */
      .addPass("ds-verify")  /* Verify the module */
      .addPass("ds-flatten") /* Flatten all nested functions */
      .addPass("ds-tyinfer") /* Do type inference */
      .addPass("ds-mangle")  /* Mangle all names */
      .addPass("ds-raii")    /* Insert destructors */
      .addGroup("reduce")
      .build("ds");

  PassGroupBuilder()
      .addPass("chk-missing-return") /* Check for missing return statements */
      .addPass("chk-bad-cast")       /* Check for bad casts */
      .build("chk");
}
