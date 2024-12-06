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

#include <nitrate-ir/IRGraph.hh>
#include <nitrate-ir/Report.hh>
#include <passes/PassList.hh>
#include <string>

/**
 * @brief Cleanup the IR
 *
 * @timecomplexity O(n)
 * @spacecomplexity O(1)
 */

using namespace nr;

static void seq_mark_non_functional(Expr* P, Expr** C) {
  if (!P || P->getKind() != NR_NODE_SEQ) {
    return;
  }

  Expr* N = *C;

#define IGNORE_NODE() *C = createIgn()

  switch (N->getKind()) {
    case NR_NODE_BINEXPR: {
      break;
    }

    case NR_NODE_UNEXPR: {
      break;
    }

    case NR_NODE_POST_UNEXPR: {
      break;
    }

    case NR_NODE_INT: {
      IGNORE_NODE();
      break;
    }

    case NR_NODE_FLOAT: {
      IGNORE_NODE();
      break;
    }

    case NR_NODE_LIST: {
      IGNORE_NODE();
      break;
    }

    case NR_NODE_CALL: {
      break;
    }

    case NR_NODE_SEQ: {
      break;
    }

    case NR_NODE_INDEX: {
      break;
    }

    case NR_NODE_IDENT: {
      IGNORE_NODE();
      break;
    }

    case NR_NODE_EXTERN: {
      break;
    }

    case NR_NODE_LOCAL: {
      break;
    }

    case NR_NODE_RET: {
      break;
    }

    case NR_NODE_BRK: {
      break;
    }

    case NR_NODE_CONT: {
      break;
    }

    case NR_NODE_IF: {
      break;
    }

    case NR_NODE_WHILE: {
      break;
    }

    case NR_NODE_FOR: {
      break;
    }

    case NR_NODE_CASE: {
      break;
    }

    case NR_NODE_SWITCH: {
      break;
    }

    case NR_NODE_FN: {
      break;
    }

    case NR_NODE_ASM: {
      break;
    }

    case NR_NODE_IGN: {
      break;
    }

    case NR_NODE_U1_TY: {
      IGNORE_NODE();
      break;
    }

    case NR_NODE_U8_TY: {
      IGNORE_NODE();
      break;
    }

    case NR_NODE_U16_TY: {
      IGNORE_NODE();
      break;
    }

    case NR_NODE_U32_TY: {
      IGNORE_NODE();
      break;
    }

    case NR_NODE_U64_TY: {
      IGNORE_NODE();
      break;
    }

    case NR_NODE_U128_TY: {
      IGNORE_NODE();
      break;
    }

    case NR_NODE_I8_TY: {
      IGNORE_NODE();
      break;
    }

    case NR_NODE_I16_TY: {
      IGNORE_NODE();
      break;
    }

    case NR_NODE_I32_TY: {
      IGNORE_NODE();
      break;
    }

    case NR_NODE_I64_TY: {
      IGNORE_NODE();
      break;
    }

    case NR_NODE_I128_TY: {
      IGNORE_NODE();
      break;
    }

    case NR_NODE_F16_TY: {
      IGNORE_NODE();
      break;
    }

    case NR_NODE_F32_TY: {
      IGNORE_NODE();
      break;
    }

    case NR_NODE_F64_TY: {
      IGNORE_NODE();
      break;
    }

    case NR_NODE_F128_TY: {
      IGNORE_NODE();
      break;
    }

    case NR_NODE_VOID_TY: {
      IGNORE_NODE();
      break;
    }

    case NR_NODE_PTR_TY: {
      IGNORE_NODE();
      break;
    }

    case NR_NODE_CONST_TY: {
      IGNORE_NODE();
      break;
    }

    case NR_NODE_OPAQUE_TY: {
      IGNORE_NODE();
      break;
    }

    case NR_NODE_STRUCT_TY: {
      IGNORE_NODE();
      break;
    }

    case NR_NODE_UNION_TY: {
      IGNORE_NODE();
      break;
    }

    case NR_NODE_ARRAY_TY: {
      IGNORE_NODE();
      break;
    }

    case NR_NODE_FN_TY: {
      IGNORE_NODE();
      break;
    }

    case NR_NODE_TMP: {
      qcore_assert(false);
      break;
    }
  }

#undef IGNORE_NODE
}

static void seq_sweep_ign_nodes(Expr* P, Expr** C) {
  if (P && P->getKind() == NR_NODE_SEQ && (*C)->getKind() == NR_NODE_IGN) {
    SeqItems& V = P->as<Seq>()->getItems();

    auto it = std::find(V.begin(), V.end(), *C);
    if (it != V.end()) {
      V.erase(it);
    }
  }
}

static void ignore_empty_seq(Expr* P, Expr** C) {
  /* Don't ignore the root node! */
  if (P && P->getKind() == NR_NODE_SEQ && (*C)->getKind() == NR_NODE_SEQ) {
    Seq* S = (*C)->as<Seq>();

    if (S->getItems().empty()) {
      *C = createIgn();
    }
  }
}

static void ignore_empty_extern(Expr*, Expr** C) {
  if ((*C)->getKind() == NR_NODE_EXTERN) {
    Extern* E = (*C)->as<Extern>();

    if (E->getValue()->getKind() == NR_NODE_IGN) {
      *C = createIgn();
    }
  }
}

static void remove_unneeded_cast(Expr*, Expr** C) {
  if ((*C)->getKind() == NR_NODE_BINEXPR) {
    BinExpr* E = (*C)->as<BinExpr>();

    if (E->getOp() == Op::CastAs || E->getOp() == Op::BitcastAs) {
      Type* LHT = E->getLHS()->getType().value_or(nullptr);

      if (LHT && LHT->isSame(E->getRHS())) {
        *C = E->getLHS();
      }
    }
  }
}

using NodeCount = size_t;
static size_t garbage_collect_round(qmodule_t* M, size_t& iteration,
                                    IReport* log) {
  log->report(Info, IC::Debug,
              "Running IR GC pass " + std::to_string(iteration));

  { /* Erase garbage in IR sequence nodes */
    /* Mark non-functional sequence nodes as ignored */
    iterate<dfs_post>(M->getRoot(), [](Expr* P, Expr** C) -> IterOp {
      seq_mark_non_functional(P, C);
      return IterOp ::Proceed;
    });

    /* Erase ignore nodes from sequence structures */
    iterate<dfs_pre>(M->getRoot(), [](Expr* P, Expr** C) -> IterOp {
      seq_sweep_ign_nodes(P, C);
      return IterOp::Proceed;
    });
  }

  { /* Simplify nodes / ignore garbage */
    iterate<dfs_post>(M->getRoot(), [](Expr* P, Expr** C) -> IterOp {
      ignore_empty_seq(P, C);
      ignore_empty_extern(P, C);
      remove_unneeded_cast(P, C);
      return IterOp ::Proceed;
    });
  }

  size_t node_count = 0;
  iterate<dfs_post>(M->getRoot(), [&node_count](Expr*, Expr**) -> IterOp {
    node_count++;
    return IterOp::Proceed;
  });

  return node_count;
}

bool nr::pass::ds_clean(qmodule_t* M, IReport* log) {
  /* Run garbage collection until there is no more removable garbage */
  NodeCount last_count = -1, cur_count = 0;
  size_t gc_iter = 0;
  while (last_count != cur_count) {
    last_count = cur_count;
    cur_count = garbage_collect_round(M, gc_iter, log);
    gc_iter++;
  }

  return true;
}
