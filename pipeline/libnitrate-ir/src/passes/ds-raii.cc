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

#include <nitrate-ir/IRGraph.hh>
#include <passes/PassList.hh>

/**
 * @brief Insert destructors to stack-allocated local variables
 *
 * @timecomplexity O(n)
 * @spacecomplexity O(1)
 */

using namespace nr;

bool nr::pass::ds_raii(qmodule_t *M, IReport *) {
  for (auto &[k, v] : M->getFunctions()) {
    Expr *F = v.second->getBody().value_or(nullptr);
    if (!F) {
      continue;
    }

    iterate<dfs_pre>(F, [](Expr *, Expr **C) -> IterOp {
      if ((*C)->getKind() != QIR_NODE_SEQ) {
        return IterOp::Proceed;
      }

      ///==========================================================
      /// Foreach scope in function:

      SeqItems &SI = (*C)->as<Seq>()->getItems();

      /// For each unconditional branch instruction call
      /// destructors in reverse order
      auto first_ubr = std::find_if(SI.begin(), SI.end(), [](Expr *E) {
        nr_ty_t ty = E->getKind();
        return ty == QIR_NODE_RET || ty == QIR_NODE_CONT || ty == QIR_NODE_BRK;
      });

      for (size_t i = 0; i < SI.size(); ++i) {
        Expr *E = SI[i];

        if (E->getKind() == QIR_NODE_LOCAL) {
          (void)first_ubr;
          // Local *L = E->as<Local>();
          // Fn *D = createIgn()->as<Fn>();

          /// TODO: Get the destructor function

          // UnExpr *addr_of = create<UnExpr>(L, Op::BitAnd);
          // CallArgs args = CallArgs({addr_of});
          // Call *C = create<Call>(D, std::move(args));

          // SI.insert(first_ubr, C);
        }
      }

      ///==========================================================

      return IterOp::Proceed;
    });
  }

  /// TODO: Implement support for RAII

  return true;
}
