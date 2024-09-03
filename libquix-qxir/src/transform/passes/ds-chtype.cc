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
 * @brief Verify runtime polymorphic nodes are of suitable types.
 * @details Ensure that a pointer to a concrete node is actually a pointer to that concrete node
 * type.
 * @note Optimize this code to not do extra work when checking.
 *
 * @timecomplexity O(n)
 * @spacecomplexity O(1)
 */

bool qxir::passes::impl::ds_chtype(qmodule_t *mod) {
#define EPUT(msg)                                                                        \
  {                                                                                      \
    bad_ty = true;                                                                       \
    mod->getDiag().push(                                                                 \
        QXIR_AUDIT_CONV,                                                                 \
        diag::DiagMessage(msg, diag::IssueClass::FatalError, diag::IssueCode::DSBadType, \
                          cur->getLoc().first, cur->getLoc().second));                   \
  }

  bool bad_ty = false;

  const auto cb = [&bad_ty, mod](Expr *par, Expr **_cur) -> IterOp {
    if (!par) {
      return IterOp::Proceed;
    }

    Expr *cur = *_cur;

    if (cur->is(QIR_NODE_TMP)) {
      return IterOp::Proceed;
    }

    switch (par->getKind()) {
      case QIR_NODE_BINEXPR: {
        break;
      }

      case QIR_NODE_UNEXPR: {
        break;
      }

      case QIR_NODE_POST_UNEXPR: {
        break;
      }

      case QIR_NODE_INT: {
        break;
      }

      case QIR_NODE_FLOAT: {
        break;
      }

      case QIR_NODE_STRING: {
        break;
      }

      case QIR_NODE_LIST: {
        if (cur->isType()) {
          EPUT("List child node is a type node");
        }
        break;
      }

      case QIR_NODE_ALLOC: {
        if (!cur->isType()) {
          EPUT("Allocation child node is not a type node");
        }
        break;
      }

      case QIR_NODE_CALL: {
        if (cur->isType()) {
          EPUT("Call child node is a type node");
        }
        Call *calln = par->as<Call>();
        auto tkind = calln->getTarget()->getKind();

        if (tkind != QIR_NODE_LOCAL && tkind != QIR_NODE_FN) {
          EPUT("Call target is not an ident, local, or function node");
        }

        break;
      }

      case QIR_NODE_SEQ: {
        break;
      }

      case QIR_NODE_ASYNC: {
        break;
      }

      case QIR_NODE_INDEX: {
        if (cur->isType()) {
          EPUT("Index child node is a type node");
        }
        break;
      }

      case QIR_NODE_IDENT: {
        break;
      }

      case QIR_NODE_EXTERN: {
        if (cur->isType()) {
          EPUT("Extern child node is a type node");
        }
        break;
      }

      case QIR_NODE_LOCAL: {
        if (cur->isType() && !cur->is(QIR_NODE_FN_TY)) {
          EPUT("Local child node is a type node");
        }
        break;
      }

      case QIR_NODE_RET: {
        if (cur->isType() && cur->getKind() != QIR_NODE_VOID_TY) {
          EPUT("Return child node value is a type node");
        }
        break;
      }

      case QIR_NODE_BRK: {
        break;
      }

      case QIR_NODE_CONT: {
        break;
      }

      case QIR_NODE_IF: {
        If *ifn = par->as<If>();

        if (ifn->getCond()->isType()) {
          EPUT("If condition is a type node");
        }

        if (ifn->getThen()->isType()) {
          EPUT("If then branch is a type node");
        }

        if (ifn->getElse()->isType() && ifn->getElse()->getKind() != QIR_NODE_VOID_TY) {
          EPUT("If else branch is a type node");
        }

        break;
      }

      case QIR_NODE_WHILE: {
        While *whilen = par->as<While>();

        if (whilen->getCond()->isType()) {
          EPUT("While condition is a type node");
        }

        if (whilen->getBody()->isType() && whilen->getBody()->getKind() != QIR_NODE_VOID_TY) {
          EPUT("While body is a type node");
        }
        break;
      }

      case QIR_NODE_FOR: {
        For *forn = par->as<For>();

        if (forn->getInit()->isType()) {
          EPUT("For init is a type node");
        }

        if (forn->getCond()->isType()) {
          EPUT("For condition is a type node");
        }

        if (forn->getStep()->isType()) {
          EPUT("For step is a type node");
        }

        if (forn->getBody()->isType() && forn->getBody()->getKind() != QIR_NODE_VOID_TY) {
          EPUT("For body is a type node");
        }
        break;
      }

      case QIR_NODE_FORM: {
        Form *formn = par->as<Form>();

        if (formn->getMaxJobs()->isType()) {
          EPUT("Form maxjobs is a type node");
        }

        if (formn->getExpr()->isType()) {
          EPUT("Form expr is a type node");
        }

        if (formn->getBody()->isType() && formn->getBody()->getKind() != QIR_NODE_VOID_TY) {
          EPUT("Form body is a type node");
        }
        break;
      }

      case QIR_NODE_FOREACH: {
        Foreach *forn = par->as<Foreach>();

        if (forn->getExpr()->isType()) {
          EPUT("Foreach expr is a type node");
        }

        if (forn->getBody()->isType() && forn->getBody()->getKind() != QIR_NODE_VOID_TY) {
          EPUT("Foreach body is a type node");
        }
        break;
      }

      case QIR_NODE_CASE: {
        Case *casen = par->as<Case>();

        if (casen->getCond()->isType()) {
          EPUT("Case condition is a type node");
        }

        if (casen->getBody()->isType() && casen->getBody()->getKind() != QIR_NODE_VOID_TY) {
          EPUT("Case body is a type node");
        }
        break;
      }

      case QIR_NODE_SWITCH: {
        Switch *switn = par->as<Switch>();

        if (switn->getCond()->isType()) {
          EPUT("Switch condition is a type node");
        }

        if (switn->getDefault()->isType() && switn->getDefault()->getKind() != QIR_NODE_VOID_TY) {
          EPUT("Switch default is a type node");
        }
        break;
      }

      case QIR_NODE_FN: {
        Fn *fnn = par->as<Fn>();

        if (fnn->getBody()->isType() && fnn->getBody()->getKind() != QIR_NODE_VOID_TY) {
          EPUT("Function body is a type node");
        }
        break;
      }

      case QIR_NODE_ASM: {
        qcore_implement("asm chtype");
        break;
      }

      case QIR_NODE_U1_TY: {
        break;
      }

      case QIR_NODE_U8_TY: {
        break;
      }

      case QIR_NODE_U16_TY: {
        break;
      }

      case QIR_NODE_U32_TY: {
        break;
      }

      case QIR_NODE_U64_TY: {
        break;
      }

      case QIR_NODE_U128_TY: {
        break;
      }

      case QIR_NODE_I8_TY: {
        break;
      }

      case QIR_NODE_I16_TY: {
        break;
      }

      case QIR_NODE_I32_TY: {
        break;
      }

      case QIR_NODE_I64_TY: {
        break;
      }

      case QIR_NODE_I128_TY: {
        break;
      }

      case QIR_NODE_F16_TY: {
        break;
      }

      case QIR_NODE_F32_TY: {
        break;
      }

      case QIR_NODE_F64_TY: {
        break;
      }

      case QIR_NODE_F128_TY: {
        break;
      }

      case QIR_NODE_VOID_TY: {
        break;
      }

      case QIR_NODE_PTR_TY: {
        if (!cur->isType()) {
          EPUT("Pointer is not a type node");
        }
        break;
      }

      case QIR_NODE_OPAQUE_TY: {
        break;
      }

      case QIR_NODE_STRING_TY: {
        break;
      }

      case QIR_NODE_STRUCT_TY: {
        if (!cur->isType()) {
          EPUT("Struct is not a type node");
        }
        break;
      }

      case QIR_NODE_UNION_TY: {
        if (!cur->isType()) {
          EPUT("Union is not a type node");
        }
        break;
      }

      case QIR_NODE_ARRAY_TY: {
        if (!par->as<ArrayTy>()->getElement()->isType()) {
          EPUT("Array element is not a type node");
        }
        if (par->as<ArrayTy>()->getCount()->isType()) {
          EPUT("Array size is a type node");
        }
        break;
      }

      case QIR_NODE_LIST_TY: {
        if (!cur->isType()) {
          EPUT("List is not a type node");
        }
        break;
      }

      case QIR_NODE_INTRIN_TY: {
        break;
      }

      case QIR_NODE_FN_TY: {
        FnTy *fnty = par->as<FnTy>();

        if (!fnty->getReturn()->isType()) {
          EPUT("Function return type is not a type node");
        }

        for (const auto &p : fnty->getParams()) {
          if (!p->isType()) {
            EPUT("Function parameter is not a type node");
          }
        }
        break;
      }

      case QIR_NODE_TMP: {
        break;
      }

      default: {
        qcore_panicf("Unexpected parent node kind: %s", par->getKindName());
      }
    }

    return IterOp::Proceed;
  };

  iterate<IterMode::dfs_pre, IterMP::none>(mod->getRoot(), cb);

  if (bad_ty) {
    mod->getDiag().push(
        QXIR_AUDIT_CONV,
        diag::DiagMessage("Module does not conform to child-type invariants",
                          diag::IssueClass::FatalError, diag::IssueCode::DSBadType));
  }

  return !bad_ty;
}
