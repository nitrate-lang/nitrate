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
///   * NITRATE TOOLCHAIN - The official toolchain for the Nitrate language. ///
///   * Copyright (C) 2024 Wesley C. Jones                                   ///
///                                                                          ///
///   The Nitrate Toolchain is free software; you can redistribute it or     ///
///   modify it under the terms of the GNU Lesser General Public             ///
///   License as published by the Free Software Foundation; either           ///
///   version 2.1 of the License, or (at your option) any later version.     ///
///                                                                          ///
///   The Nitrate Toolcain is distributed in the hope that it will be        ///
///   useful, but WITHOUT ANY WARRANTY; without even the implied warranty of ///
///   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      ///
///   Lesser General Public License for more details.                        ///
///                                                                          ///
///   You should have received a copy of the GNU Lesser General Public       ///
///   License along with the Nitrate Toolchain; if not, see                  ///
///   <https://www.gnu.org/licenses/>.                                       ///
///                                                                          ///
////////////////////////////////////////////////////////////////////////////////

#include <nitrate-ir/IR.h>

#include <boost/bimap.hpp>
#include <nitrate-ir/Format.hh>
#include <nitrate-ir/IRGraph.hh>
#include <nitrate-ir/Report.hh>
#include <passes/PassList.hh>

/**
 * @brief Canonicalize the names of things in the module.
 *
 * @timecomplexity O(n)
 * @spacecomplexity O(1)
 */

using namespace nr;

bool nr::pass::ds_mangle(qmodule_t *mod) {
  SymbolEncoding se;
  bool failed = false;

  iterate<dfs_pre>(mod->getRoot(), [&](Expr *, Expr **cur) -> IterOp {
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
  iterate<dfs_pre>(mod->getRoot(), [](Expr *, Expr **cur) -> IterOp {
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
