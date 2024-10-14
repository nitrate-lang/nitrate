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

#define QXIR_USE_CPP_API

#include <quix-qxir/Node.h>

#include <transform/passes/Decl.hh>

/**
 * @brief Ensure that no nodes are nullptr or have missing module pointers.
 *
 * @timecomplexity O(n)
 * @spacecomplexity O(1)
 */

bool qxir::transform::impl::ds_nullchk(qmodule_t *mod) {
  bool has_bad_null = false;
  bool missing_mod = false;

  const auto cb = [&has_bad_null, &missing_mod, mod](Expr *, Expr **_cur) -> IterOp {
    if (*_cur == nullptr) [[unlikely]] {
      has_bad_null = true;

      mod->getDiag().push(
          QXIR_AUDIT_CONV,
          diag::DiagMessage("Illegal nullptr pointer found in module IR data structure.",
                            diag::IssueClass::FatalError, diag::IssueCode::DSNullPtr));
      return IterOp::Abort;
    }

    Expr *cur = *_cur;

    if (cur->getModule() == nullptr) [[unlikely]] {
      missing_mod = true;

      mod->getDiag().push(
          QXIR_AUDIT_CONV,
          diag::DiagMessage(
              "Module pointer missing in node of type " + std::string(cur->getKindName()),
              diag::IssueClass::FatalError, diag::IssueCode::DSMissingMod, cur->getLoc().first,
              cur->getLoc().second));
      return IterOp::Abort;
    }

    return IterOp::Proceed;
  };

  iterate<IterMode::dfs_pre, IterMP::none>(mod->getRoot(), cb);

  return !has_bad_null && !missing_mod;
}
