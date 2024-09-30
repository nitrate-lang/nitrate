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

#define __QUIX_IMPL__

#define QXIR_USE_CPP_API

#include <quix-qxir/Inference.h>
#include <quix-qxir/Node.h>

#include <atomic>
#include <boost/bimap.hpp>
#include <transform/passes/Decl.hh>

/**
 * @brief Move nested functions to the top level.
 *
 * @timecomplexity O(n)
 * @spacecomplexity O(n)
 */

using namespace qxir;

static void do_pass(qmodule_t *mod, Expr *&base, std::string cur_scope,
                    std::vector<Fn **> &functions) {
  IterCallback cb = [mod, cur_scope, &functions](Expr *par, Expr **cur) -> IterOp {
    if ((*cur)->getKind() != QIR_NODE_FN) {
      return IterOp::Proceed;
    }

    std::string new_scope;

    { /* Handle name change */
      static thread_local std::atomic<uint64_t> counter = 0;
      std::string orig_name = std::string((*cur)->as<Fn>()->getName());
      if (orig_name.empty()) {
        orig_name = std::to_string(counter++);
      }

      new_scope = cur_scope.empty() ? orig_name : cur_scope + "::" + orig_name;

      (*cur)->as<Fn>()->setName(mod->internString(new_scope));
      functions.push_back(reinterpret_cast<Fn **>(cur));
    }

    Expr *body = (*cur)->as<Fn>()->getBody();
    do_pass(mod, body, new_scope, functions);

    return IterOp::SkipChildren;
  };

  iterate<dfs_pre, IterMP::none>(base, cb);
}

bool qxir::passes::impl::fnflatten(qmodule_t *mod) {
  return true;
  /**
   * This pass in infallible.
   */

  std::vector<Fn **> functions;

  /** Iterate over the module and:
   *  - Treat function names like namespaces and rename them to be unique
   *  - Add each function to the list
   */
  do_pass(mod, mod->getRoot(), "", functions);

  /**
   * Erase the function definitions from the original location
   * (replace with a void type which is a NOP).
   *
   * Add the functions to the root node. eg flatten the functions.
   */
  Seq *root = mod->getRoot()->as<Seq>();
  for (Fn **fn_member : functions) {
    Fn *fn = *fn_member;

    Expr **generic = reinterpret_cast<Expr **>(fn_member);
    *generic = getType<VoidTy>();

    root->addItem(fn);
  }

  return true;
}
