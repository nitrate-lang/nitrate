////////////////////////////////////////////////////////////////////////////////
///                                                                          ///
///     .-----------------.    .----------------.     .----------------.     ///
///    | .--------------. |   | .--------------. |   | .--------------. |    ///
///    | | ____  _____  | |   | |     ____     | |   | |    ______    | |    ///
///    | ||_   _|_   _| | |   | |   .'    `.   | |   | |   / ____ `.  | |    ///
///    | |  |   \ | |   | |   | |  /  .--.  \  | |   | |   `'  __) |  | |    ///
///    | |  | |\ \| |   | |   | |  | |    | |  | |   | |   _  |__ '.  | |    ///
///    | | _| |_\   |_  | |   | |  \  `--'  /  | |   | |  | \____) |  | |    ///
///    | ||_____|\____| | |   | |   `.____.'   | |   | |   \______.'  | |    ///
///    | |              | |   | |              | |   | |              | |    ///
///    | '--------------' |   | '--------------' |   | '--------------' |    ///
///     '----------------'     '----------------'     '----------------'     ///
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

#define IRBUILDER_IMPL

#include <nitrate-core/Error.h>

#include <nitrate-ir/IRBuilder.hh>
#include <nitrate-ir/IRGraph.hh>

using namespace nr;

void NRBuilder::flatten_symbols(Seq *root) noexcept {
  std::unordered_set<Expr *> symbols;

  iterate<dfs_post>(root, [&](Expr *P, Expr **C) -> IterOp {
    if (!P) {
      return IterOp::Proceed;
    }

    bool replace_with_ident = !P->is(NR_NODE_SEQ);

    if ((!P->is(NR_NODE_EXTERN) && (*C)->is(NR_NODE_FN)) ||
        (*C)->is(NR_NODE_EXTERN)) {
      symbols.insert(*C);

      if (replace_with_ident) {
        *C = create<Ident>((*C)->getName(), *C);
      } else {
        *C = createIgn();
      }
    }

    return IterOp::Proceed;
  });

  for (auto ele : symbols) {
    root->getItems().push_back(ele);
  }
}

///=============================================================================

bool NRBuilder::check_duplicates(Seq *root, IReport *I) noexcept {
  bool ok = true;

  flatten_symbols(root);

  for (auto fn : m_duplicate_functions) {
    if (auto fn_type = fn->getType()) {
      std::stringstream ss;
      fn_type.value()->dump(ss);

      I->report(CompilerError, IC::Error,
                {"Duplicate function \"", fn->getName(), "\": ", ss.str()},
                fn->getLoc());

    } else {
      I->report(CompilerError, IC::Error,
                {"Duplicate function \"", fn->getName(), "\""}, fn->getLoc());
    }

    ok = false;
  }

  for (auto var : m_duplicate_variables) {
    if (auto var_type = var->getType()) {
      std::stringstream ss;
      var_type.value()->dump(ss);

      I->report(CompilerError, IC::Error,
                {"Duplicate variable \"", var->getName(), "\": ", ss.str()},
                var->getLoc());

    } else {
      I->report(CompilerError, IC::Error,
                {"Duplicate variable \"", var->getName(), "\""}, var->getLoc());
    }

    ok = false;
  }

  ///=====================================================================

  iterate<dfs_pre>(root, [&](Expr *, Expr **) -> IterOp {
    /// TODO: Warn user about using reserved namespaces.

    return IterOp::Proceed;
  });

  return ok;
}
