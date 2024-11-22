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

#include <cctype>
#include <nitrate-ir/IRBuilder.hh>
#include <nitrate-ir/IRGraph.hh>
#include <unordered_map>
#include <unordered_set>

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

static void resolve_direct_call(
    Expr **C, Fn *target_fn,
    const std::unordered_map<std::string_view, size_t> &name_to_index_map,
    const std::unordered_map<size_t, Expr *> &default_args,
    const CallArgsTmpNodeCradle &data) {
  std::unordered_map<size_t, Expr *> arguments;
  arguments.reserve(data.args.size());

  { /* Allocate the arguments */
    for (const auto &[argument_name, argument_value] : data.args) {
      bool is_positional = std::isdigit(argument_name.at(0));

      if (is_positional) {
        size_t position = std::stoul(std::string(argument_name));

        // Ensure we don't add the same argument twice
        if (!arguments.contains(position)) [[likely]] {
          arguments[position] = argument_value;
          continue;
        }
      } else {
        auto it = name_to_index_map.find(argument_name);
        if (it != name_to_index_map.end()) [[likely]] {
          // Ensure we don't add the same argument twice
          if (!arguments.contains(it->second)) [[likely]] {
            arguments[it->second] = argument_value;
            continue;
          }
        }
      }

      goto end;
    }

    for (size_t i = 0; i < target_fn->getParams().size(); ++i) {
      if (!arguments.contains(i)) {
        auto it = default_args.find(i);
        if (it != default_args.end()) {
          arguments[i] = it->second;
        }
      }
    }
  }

  { /* Validate the arguments vector */
    if (target_fn->isVariadic()) {
      if (arguments.size() < target_fn->getParams().size()) [[unlikely]] {
        goto end;
      }
    } else {
      if (arguments.size() != target_fn->getParams().size()) [[unlikely]] {
        goto end;
      }
    }

    std::unordered_set<size_t> set;
    for (const auto &[k, _] : arguments) {
      set.insert(k);
    }

    // Check if elements are contiguous and start at zero
    for (size_t i = 0; i < arguments.size(); i++) {
      if (set.find(i) == set.end()) [[unlikely]] {
        goto end;
      }
    }
  }

  { /* Emit the Call IR expression */
    CallArgs flattened(arguments.size());
    for (const auto &[index, value] : arguments) {
      flattened[index] = value;
    }

    *C = create<Call>(target_fn, std::move(flattened));
  }

end:
  return;
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
      const auto &data =
          std::get<CallArgsTmpNodeCradle>(N->as<Tmp>()->getData());
      Fn *target_fn = nullptr;
      const std::unordered_map<size_t, Expr *> *default_args = nullptr;

      { /* Get the target function node */
        qcore_assert(data.base != nullptr);

        // Skip over indirect function calls
        if (!data.base->is(QIR_NODE_IDENT)) {
          goto end;
        }

        // Get name of target function
        std::string_view target_name = data.base->as<Ident>()->getName();

        // Find the target function by name
        auto result = resolve_name(target_name, Kind::Function);
        if (!result.has_value()) [[unlikely]] {
          goto end;
        }

        qcore_assert(result.value()->is(QIR_NODE_FN));
        target_fn = result.value()->as<Fn>();
        default_args = &m_function_defaults.at(target_fn);
      }

      std::unordered_map<std::string_view, size_t> name_to_index_map;
      for (size_t i = 0; i < target_fn->getParams().size(); ++i) {
        name_to_index_map[target_fn->getParams()[i].second] = i;
      }

      resolve_direct_call(C, target_fn, name_to_index_map, *default_args, data);
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
