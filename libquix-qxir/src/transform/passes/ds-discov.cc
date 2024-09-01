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

#include <boost/bimap.hpp>
#include <transform/passes/Decl.hh>

/**
 * @brief [TODO: Write a short description of this pass]
 *
 * @details [TODO: Write a detailed description of this pass]
 *
 * @note [TODO: Write any additional notes about this pass]
 *
 * @timecomplexity O(n)
 * @spacecomplexity O(n)
 */

bool qxir::passes::impl::ds_resolv(qmodule_t *mod) {
  bool error = false;

  const auto cb = [&](Expr *par, Expr **_cur) -> IterOp {
    if ((*_cur)->getKind() != QIR_NODE_TMP) {
      return IterOp::Proceed;
    }

    Tmp *cur = (*_cur)->as<Tmp>();
    TmpType tt = cur->getTmpType();

    switch (tt) {
      case TmpType::NULL_LITERAL: {
        /// TODO:
        break;
      }

      case TmpType::UNDEF_LITERAL: {
        /// TODO:
        break;
      }

      case TmpType::CALL: {
        const auto &info = std::get<DirectCallArgsTmpNodeCradle>(cur->getData());
        const Expr *base = std::get<0>(info);
        const auto &args = std::get<1>(info);

        if (base->getKind() == QIR_NODE_IDENT) { /* Direct call */
          const std::string_view &name = base->as<Ident>()->getName();

          if (mod->getFunctions().left.count(name) == 0) {
            NO_MATCHING_FUNCTION(name);
            error = true;
            return IterOp::Proceed;
          }

          DirectCallArgs new_args;

          /// TODO: convert args to DirectCallArgs
          (void)args;

          *_cur = create<DirectCall>(mod->getFunctions().left.at(name), std::move(new_args));
        } else { /* Indirect call */
          /// TODO: Learn what an indirect call [actually] is.
          /// Do I need lookup tables? How does this apply to non-native builds?
          qcore_panic("Indirect calls are not yet supported.");
        }
        break;
      }

      case TmpType::ENUM: {
        /// TODO:
        break;
      }

      case TmpType::LET: {
        /// TODO:
        break;
      }

      case TmpType::VAR: {
        /// TODO:
        break;
      }

      case TmpType::CONST: {
        /// TODO:
        break;
      }

      case TmpType::FIELD: {
        /// TODO:
        break;
      }

      case TmpType::NAMED_TYPE: {
        /// TODO:
        break;
      }

      default: {
        qcore_panicf("Invalid temporary type: %d", static_cast<int>(tt));
      }
    }

    return IterOp::Proceed;
  };

  iterate<IterMode::dfs_pre, IterMP::none>(mod->getRoot(), cb);

  for (auto &[name, fn] : mod->getFunctions()) {
    CONV_DEBUG("Found function: " + std::string(name));
  }
  for (auto &[name, var] : mod->getVariables()) {
    CONV_DEBUG("Found variable: " + std::string(name));
  }

  return !error;
}
