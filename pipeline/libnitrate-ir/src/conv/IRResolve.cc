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
#include <nitrate-ir/TypeDecl.h>

#include <nitrate-ir/IRBuilder.hh>
#include <nitrate-ir/IRGraph.hh>

using namespace nr;

void NRBuilder::try_resolve_types(Expr *root) const noexcept {
  /**
   * @brief Resolve the `TmpType::NAMED_TYPE` nodes by replacing them with the
   * actual types they represent.
   * @note Any nodes that fail to resolve are left alone.
   */

  iterate<dfs_pre>(root, [&](Expr *, Expr **C) -> IterOp {
    Expr *N = *C;

    if (N->is(QIR_NODE_TMP) &&
        N->as<Tmp>()->getTmpType() == TmpType::NAMED_TYPE) {
      /* Get the fully-qualified name */
      std::string_view type_name =
          std::get<std::string_view>(N->as<Tmp>()->getData());

      auto result = resolve_name(type_name, Kind::TypeDef);
      if (result.has_value()) [[likely]] {
        /* Replace the current node */
        *C = result.value();
      } else {
        /* Fallthrough */
      }
    }

    return IterOp::Proceed;
  });
}

void NRBuilder::try_resolve_names(Expr *root) const noexcept {
  /**
   * @brief Resolve identifiers by hooking them to the node they represent. This
   * may create cyclic references, which is okay because these hooks are not
   * enumerated during normal iteration.
   * @note Any nodes that fail to resolve are left alone.
   */

  iterate<dfs_pre>(root, [&](Expr *, Expr **C) -> IterOp {
    Expr *N = *C;

    if (N->is(QIR_NODE_IDENT) && N->as<Ident>()->getWhat() == nullptr) {
      Ident *I = N->as<Ident>();

      auto enum_opt = resolve_name(I->getName(), Kind::ScopedEnum);
      if (enum_opt.has_value()) {
        *C = enum_opt.value();
      } else {
        /// TODO: Resolve identifier reference
      }
    }

    return IterOp::Proceed;
  });
}

void NRBuilder::try_resolve_calls(Expr *root) const noexcept {
  /**
   * @brief Resolve the `TmpType::CALL` nodes by replacing them with function
   * call expression nodes with any default arguments filled in.
   * @note Any nodes that fail to resolve are left alone.
   */

  iterate<dfs_pre>(root, [&](Expr *, Expr **C) -> IterOp {
    Expr *N = *C;

    if (N->is(QIR_NODE_TMP) && N->as<Tmp>()->getTmpType() == TmpType::CALL) {
      Tmp *T = N->as<Tmp>();

      const auto &data = std::get<CallArgsTmpNodeCradle>(T->getData());
      qcore_assert(data.base != nullptr);

      // Skip over indirect function calls
      if (!data.base->is(QIR_NODE_IDENT)) {
        goto end;
      }

      std::string_view target_name = data.base->as<Ident>()->getName();

      // Find the target function by name
      auto result = resolve_name(target_name, Kind::Function);
      if (!result.has_value()) [[unlikely]] {
        goto end;
      }

      qcore_assert(result.value()->is(QIR_NODE_FN));

      /// TODO: Generate Call() expression node with default arguments
    }

  end:
    return IterOp::Proceed;
  });
}

void NRBuilder::connect_nodes(Seq *root) const noexcept {
  /* The order of the following matters */

  try_resolve_types(root);
  try_resolve_names(root);
  try_resolve_calls(root);
}
