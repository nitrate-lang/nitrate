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
 * @brief [TODO: Write a short description of this pass]
 *
 * @details [TODO: Write a detailed description of this pass]
 *
 * @note [TODO: Write any additional notes about this pass]
 */

bool qxir::passes::impl::ds_chtype(qmodule_t *mod) {
#define EPUT(msg)                                                                      \
  mod->getDiag().push(                                                                 \
      QXIR_AUDIT_CONV,                                                                 \
      diag::DiagMessage(msg, diag::IssueClass::FatalError, diag::IssueCode::DSBadType, \
                        cur->getLoc().first, cur->getLoc().second));

  bool bad_ty = false;

  const auto cb = [&bad_ty, mod](Expr *par, Expr *cur) -> IterOp {
    if (!par || !cur) {
      return IterOp::Proceed;
    }

    (void)bad_ty;  /// TODO: remove voider
    (void)mod;     /// TODO: remove voider

    qxir_ty_t T = cur->getKind();

    switch (par->getKind()) {
      case QIR_NODE_BINEXPR: {
        /// TODO: check child nodes
        break;
      }

      case QIR_NODE_UNEXPR: {
        /// TODO: check child nodes
        break;
      }

      case QIR_NODE_POST_UNEXPR: {
        /// TODO: check child nodes
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
        /// TODO: check child nodes
        break;
      }

      case QIR_NODE_ALLOC: {
        /// TODO: check child nodes
        break;
      }

      case QIR_NODE_CALL: {
        /// TODO: check child nodes
        break;
      }

      case QIR_NODE_SEQ: {
        /// TODO: check child nodes
        break;
      }

      case QIR_NODE_ASYNC: {
        /// TODO: check child nodes
        break;
      }

      case QIR_NODE_INDEX: {
        /// TODO: check child nodes
        break;
      }

      case QIR_NODE_IDENT: {
        break;
      }

      case QIR_NODE_EXPORT: {
        /// TODO: check child nodes
        break;
      }

      case QIR_NODE_LOCAL: {
        /// TODO: check child nodes
        break;
      }

      case QIR_NODE_RET: {
        /// TODO: check child nodes
        break;
      }

      case QIR_NODE_BRK: {
        break;
      }

      case QIR_NODE_CONT: {
        break;
      }

      case QIR_NODE_IF: {
        /// TODO: check child nodes
        break;
      }

      case QIR_NODE_WHILE: {
        /// TODO: check child nodes
        break;
      }

      case QIR_NODE_FOR: {
        /// TODO: check child nodes
        break;
      }

      case QIR_NODE_FORM: {
        /// TODO: check child nodes
        break;
      }

      case QIR_NODE_FOREACH: {
        /// TODO: check child nodes
        break;
      }

      case QIR_NODE_CASE: {
        /// TODO: check child nodes
        break;
      }

      case QIR_NODE_SWITCH: {
        /// TODO: check child nodes
        break;
      }

      case QIR_NODE_FN: {
        /// TODO: check child nodes
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
        /// TODO: check child nodes
        break;
      }

      case QIR_NODE_OPAQUE_TY: {
        break;
      }

      case QIR_NODE_STRING_TY: {
        break;
      }

      case QIR_NODE_STRUCT_TY: {
        /// TODO: check child nodes
        break;
      }

      case QIR_NODE_UNION_TY: {
        /// TODO: check child nodes
        break;
      }

      case QIR_NODE_ARRAY_TY: {
        /// TODO: check child nodes
        break;
      }

      case QIR_NODE_LIST_TY: {
        /// TODO: check child nodes
        break;
      }

      case QIR_NODE_INTRIN_TY: {
        /// TODO: check child nodes
        break;
      }

      case QIR_NODE_FN_TY: {
        /// TODO: check child nodes
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
