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

#include <quix-qxir/Module.hh>
#include <transform/PassManager.hh>
#include <transform/Transform.hh>
#include <transform/passes/Decl.hh>

bool qxir::transform::std_transform(qmodule_t* M, std::ostream& err) {
#define RUN_PASS(name, fn)                                        \
  {                                                               \
    if (!fn(M)) {                                                 \
      err << "Error: Pass '" << name << "' failed." << std::endl; \
      return false;                                               \
    }                                                             \
    M->applyPassLabel(name);                                      \
    if (M->getFailbit()) {                                        \
      return false;                                               \
    }                                                             \
  }

  RUN_PASS("ds-acyclic", impl::ds_acyclic);     /* Verify that the module is acyclic */
  RUN_PASS("ds-nullchk", impl::ds_nullchk);     /* Verify that the module is null-safe */
  RUN_PASS("ds-resolv", impl::ds_resolv);       /* Resolve all symbols */
  RUN_PASS("ds-verify", impl::ds_verify);       /* Verify the module */
  RUN_PASS("ds-flatten", impl::ds_flatten);     /* Flatten all nested functions */
  RUN_PASS("tyinfer", impl::tyinfer);           /* Do type inference */
  RUN_PASS("nm-premangle", impl::nm_premangle); /* Mangle all names */
  RUN_PASS("ds-clean", impl::ds_clean);         /* Cleanup IR */

  return true;
}

void qxir::transform::do_semantic_analysis(qmodule_t* M) {
  for (const auto& [_, val] : diag::PassRegistry::the().get_passes()) {
    val.second(M);

    M->applyCheckLabel(val.first);
  }
}
