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

#include <quix-qxir/IR.h>

#include <boost/bimap.hpp>
#include <quix-qxir/Format.hh>
#include <quix-qxir/IRGraph.hh>
#include <quix-qxir/Report.hh>
#include <transform/passes/Decl.hh>

/**
 * @brief Canonicalize the names of things in the module.
 *
 * @timecomplexity O(n)
 * @spacecomplexity O(1)
 */

using namespace qxir;
using namespace qxir::diag;

bool qxir::transform::impl::nm_premangle(qmodule_t *mod) {
  SymbolEncoding se;
  bool failed = false;

  iterate<dfs_pre, IterMP::none>(mod->getRoot(), [&](Expr *, Expr **cur) -> IterOp {
    if ((*cur)->getKind() == QIR_NODE_FN) {
      Fn *fn = (*cur)->as<Fn>();
      auto name = se.mangle_name(fn, fn->getAbiTag());
      if (name) [[likely]] {
        fn->setName(mod->internString(*name));
      } else {
        failed = true;
        report(IssueCode::NameManglingTypeInfer, IssueClass::Error, fn->getName(), fn->locBeg(),
               fn->locEnd());
      }
    } else if ((*cur)->getKind() == QIR_NODE_LOCAL) {
      Local *local = (*cur)->as<Local>();
      auto name = se.mangle_name(local, local->getAbiTag());
      if (name) [[likely]] {
        qcore_assert(!name->empty());
        local->setName(mod->internString(*name));
      } else {
        failed = true;
        report(IssueCode::NameManglingTypeInfer, IssueClass::Error, local->getName(),
               local->locBeg(), local->locEnd());
      }
    }

    return IterOp::Proceed;
  });

  /* Update identifiers to use the new names */
  iterate<dfs_pre, IterMP::none>(mod->getRoot(), [](Expr *, Expr **cur) -> IterOp {
    if ((*cur)->getKind() != QIR_NODE_IDENT) {
      return IterOp::Proceed;
    }

    Ident *ident = (*cur)->as<Ident>();
    if (!ident->getWhat()) {
      return IterOp::Proceed;
    }

    qcore_assert(!ident->getWhat()->getName().empty());
    ident->setName(ident->getWhat()->getName());

    return IterOp::Proceed;
  });

  return !failed;
}
