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

#include <nitrate-ir/IR.h>

#include <atomic>
#include <boost/bimap.hpp>
#include <nitrate-ir/IRGraph.hh>
#include <passes/PassList.hh>
#include <unordered_set>

/**
 * @brief Move nested linkable symbols to the top level.
 *
 * @timecomplexity O(n)
 * @spacecomplexity O(n)
 */

using namespace qxir;

static void flatten_externs(qmodule_t *mod) {
  /**
   * This one is simple. We don't manipulate any names, just move the extern
   * nodes.
   */
  std::unordered_set<Expr **> externs;

  IterCallback cb = [&externs](Expr *, Expr **cur) -> IterOp {
    if ((*cur)->getKind() == QIR_NODE_EXTERN) {
      externs.insert(cur);
    }

    return IterOp::Proceed;
  };

  iterate<dfs_pre, IterMP::none>(mod->getRoot(), cb);

  Seq *root = mod->getRoot()->as<Seq>();
  std::unordered_set<Expr *> global_scope;
  for (auto ele : root->getItems()) {
    global_scope.insert(ele);
  }

  for (auto ele : externs) {
    Expr *obj = *ele;

    if (!global_scope.contains(obj)) {
      *reinterpret_cast<Expr **>(ele) = createIgn();
      root->addItem(obj);
    }
  }
}

static void flatten_functions_recurse(qmodule_t *mod, Expr *&base, std::string cur_scope,
                                      std::unordered_set<Expr **> &functions) {
  IterCallback cb = [mod, cur_scope, &functions](Expr *par, Expr **cur) -> IterOp {
    if ((*cur)->getKind() != QIR_NODE_FN) {
      return IterOp::Proceed;
    }

    bool is_extern = par ? par->getKind() == QIR_NODE_EXTERN : false;

    static thread_local std::atomic<uint64_t> counter = 0;
    std::string orig_name = std::string((*cur)->as<Fn>()->getName());
    if (orig_name.empty()) {
      orig_name = "$_" + std::to_string(counter++);
    }

    std::string new_scope = cur_scope.empty() ? orig_name : cur_scope + "::" + orig_name;

    (*cur)->as<Fn>()->setName(mod->internString(new_scope));

    if (!is_extern) { /* Handle name change */
      functions.insert(cur);
    }

    Expr *body = (*cur)->as<Fn>()->getBody();
    flatten_functions_recurse(mod, body, new_scope, functions);

    return IterOp::SkipChildren;
  };

  iterate<dfs_pre, IterMP::none>(base, cb);
}

static void flatten_functions(qmodule_t *mod) {
  std::unordered_set<Expr **> functions;

  flatten_functions_recurse(mod, mod->getRoot(), "", functions);

  /**
   * Replace nested functions with identifier aliases
   * only if they are not at the global scope.
   */
  Seq *root = mod->getRoot()->as<Seq>();
  std::unordered_set<Expr *> global_scope;
  for (auto ele : root->getItems()) {
    global_scope.insert(ele);
  }

  for (auto ele : functions) {
    Expr *obj = *ele;

    if (!global_scope.contains(obj)) {
      *reinterpret_cast<Expr **>(ele) = create<Ident>(obj->getName(), obj);
      root->addItem(obj);
    }
  }
}

bool qxir::pass::ds_flatten(qmodule_t *mod) {
  /**
   * This pass in infallible.
   */

  flatten_externs(mod);
  flatten_functions(mod);

  return true;
}
