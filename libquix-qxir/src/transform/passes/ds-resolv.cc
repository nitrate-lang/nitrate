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

#include <string_view>
#define QXIR_USE_CPP_API

#include <quix-qxir/Inference.h>
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

static bool resolve_node(qxir::Expr **_cur) {
  if ((*_cur)->getKind() != QIR_NODE_TMP) {
    return true;
  }

  using namespace qxir;

  Tmp *cur = (*_cur)->as<Tmp>();
  TmpType tt = cur->getTmpType();
  qmodule_t *mod = cur->getModule();
  bool error = false;

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
      const auto &info = std::get<CallArgsTmpNodeCradle>(cur->getData());
      const Expr *base = std::get<0>(info);
      const auto &provided_args = std::get<1>(info);

      if (base->getKind() == QIR_NODE_IDENT) { /* Direct call */
        const std::string_view &funcname = base->as<Ident>()->getName();

        { /* Function does not exist */
          if (mod->getFunctions().left.count(funcname) == 0) {
            NO_MATCHING_FUNCTION(funcname);
            error = true;
            break;
          }
        }

        qcore_assert(mod->getParameterMap().contains(funcname));
        Expr *target = mod->getFunctions().left.at(funcname).second;
        FnTy *ft = mod->getFunctions().left.at(funcname).first;

        std::unordered_map<std::string_view, std::pair<size_t, Expr *>> named_args_map;
        const auto &params = mod->getParameterMap().at(funcname);
        CallArgs new_args;
        new_args.resize(std::max(provided_args.size(), ft->getParams().size()));
        size_t pos_i = 0;

        qcore_assert(params.size() == ft->getParams().size());

        for (size_t i = 0; i < params.size(); i++) {
          named_args_map[std::get<0>(params[i])] = {i, std::get<2>(params[i])};
        }

        { /* Check for too many arguments */
          if (provided_args.size() > params.size() && !ft->getAttrs().contains(FnAttr::Variadic)) {
            TOO_MANY_ARGUMENTS(funcname);
            error = true;
            break;
          }
        }

        { /* Allocate arguments into function parameters */
          for (auto &[name, arg] : provided_args) {
            if (std::isdigit(name.at(0))) { /* positional argument */
              new_args.at(pos_i) = arg;
              pos_i++;
            } else { /* named argument */
              if (!named_args_map.contains(name)) {
                NO_MATCHING_PARAMETER(funcname, name);
                error = true;
                break;
              }

              new_args[named_args_map[name].first] = arg;
              if (named_args_map[name].first <= pos_i) {
                pos_i++;
              }
            }
          }
          if (error) {
            break;
          }

          for (size_t i = 0; i < new_args.size(); i++) {
            if (new_args[i]) {
              continue;
            }

            Expr *_default = std::get<2>(params.at(i));
            if (_default) {
              new_args[i] = _default;
            } else {
              NO_MATCHING_PARAMETER(funcname, std::get<0>(params.at(i)));
              error = true;
              break;
            }
          }

          if (error) {
            break;
          }
        }

        *_cur = create<Call>(target, std::move(new_args));
      }
      break;
    }

    case TmpType::ENUM: {
      std::string_view name = std::get<std::string_view>(cur->as<Tmp>()->getData());

      if (!mod->getTypeMap().contains(name)) {
        NO_MATCHING_TYPE(name);
        error = true;
        break;
      }

      Type *type = mod->getTypeMap().at(name);

      if (type->getKind() != QIR_NODE_TMP) {
        *_cur = type;
        break;
      }

      for (auto &[const_name, val] : mod->getNamedConstants()) {
        if (!const_name.starts_with(name)) {
          CONV_DEBUG("Skipping constant: " + std::string(const_name));
          continue;
        }

        qxir_node_t *inferred = qxir_infer(val);
        if (!inferred) {
          error = true;
          break;
        }

        *_cur = static_cast<Expr *>(inferred);
        break;
      }

      break;
    }

    case TmpType::LET: {
      /// TODO:
      break;
    }

    case TmpType::FIELD: {
      /// TODO:
      break;
    }

    case TmpType::NAMED_TYPE: {
      std::string_view name = std::get<std::string_view>(cur->as<Tmp>()->getData());

      if (!mod->getTypeMap().contains(name)) {
        NO_MATCHING_TYPE(name);
        error = true;
        break;
      }

      *_cur = mod->getTypeMap().at(name);
      break;
    }

    default: {
      qcore_panicf("Invalid temporary type: %d", static_cast<int>(tt));
    }
  }

  return !error;
}

static bool recursive_resolve(qxir::Expr **base) {
  using namespace qxir;

  bool error = false;

  const auto cb = [&error](Expr *, Expr **_cur) -> IterOp {
    if (!resolve_node(_cur)) {
      error = true;
    }

    return IterOp::Proceed;
  };

  iterate<IterMode::dfs_pre, IterMP::none>(*base, cb);

  return !error;
}

static bool alpha_pass(qmodule_t *mod) {
  using namespace qxir;

  bool error = false;

  { /* Phase 1 */
    for (auto it = mod->getParameterMap().begin(); it != mod->getParameterMap().end(); it++) {
      for (auto &[name, type, expr] : it->second) {
        Expr *tmp = type;
        if (!recursive_resolve(&tmp)) {
          error = true;
        }
        type = tmp->asType();

        if (expr) {
          tmp = expr;
          if (!recursive_resolve(&tmp)) {
            error = true;
          }

          expr = tmp;
        }
      }
    }
  }

  { /* Phase 2 */
    std::vector<std::pair<std::string_view, std::pair<FnTy *, Expr *>>> simped;

    for (auto it = mod->getFunctions().begin(); it != mod->getFunctions().end(); it++) {
      simped.push_back({(*it).left, {(*it).right.first, (*it).right.second}});
    }

    for (auto &[name, func] : simped) {
      Expr *fn = func.second;
      Expr *ft = func.first;

      error |= !recursive_resolve(&fn) || !recursive_resolve(&ft);
    }

    mod->getFunctions().clear();
    for (auto &[name, func] : simped) {
      mod->getFunctions().insert({name, func});
    }
  }

  { /* Phase 3 */
    for (auto it = mod->getCompositeFields().begin(); it != mod->getCompositeFields().end(); it++) {
      for (auto &[name, type, expr] : it->second) {
        Expr *tmp = type;
        if (!recursive_resolve(&tmp)) {
          error = true;
        }
        type = tmp->asType();

        if (expr) {
          tmp = expr;
          if (!recursive_resolve(&tmp)) {
            error = true;
          }

          expr = tmp;
        }
      }
    }
  }

  { /* Phase 4 */
    for (auto &[name, expr] : mod->getNamedConstants()) {
      Expr *tmp = expr;
      if (!recursive_resolve(&tmp)) {
        error = true;
      }
      expr = tmp;
    }
  }

  { /* Phase 5 */
    for (auto &[name, type] : mod->getTypeMap()) {
      Expr *tmp = type;
      if (!recursive_resolve(&tmp)) {
        error = true;
      }
      type = tmp->asType();
    }
  }

  { /* Phase 6 */
    if (!recursive_resolve(&mod->getRoot())) {
      error = true;
    }
  }

  return !error;
}

static bool beta_pass(qmodule_t *mod) {
  using namespace qxir;

  bool error = false;

  auto cb = [&](Expr *, Expr **_cur) -> IterOp {
    if ((*_cur)->getKind() != QIR_NODE_IDENT) {
      return IterOp::Proceed;
    }

    Ident *cur = (*_cur)->as<Ident>();
    std::string_view name = cur->getName();

    if (mod->getFunctions().left.count(name) > 0) {
      *_cur = mod->getFunctions().left.at(name).second;
    } else if (mod->getVariables().left.count(name) > 0) {
      *_cur = mod->getVariables().left.at(name);
    } else if (mod->getNamedConstants().count(name) > 0) {
      *_cur = mod->getNamedConstants().at(name);
    } else {
      UNRESOLVED_IDENTIFIER(name);
      error = true;
    }
    return IterOp::Proceed;
  };

  iterate<IterMode::dfs_pre, IterMP::none>(mod->getRoot(), cb);

  return !error;
}

bool qxir::passes::impl::ds_resolv(qmodule_t *mod) {
  bool error = false;

  error |= !alpha_pass(mod);
  error |= !beta_pass(mod);

  for (auto &[name, fn] : mod->getFunctions()) {
    CONV_DEBUG("Found function: " + std::string(name));
  }
  for (auto &[name, var] : mod->getVariables()) {
    CONV_DEBUG("Found variable: " + std::string(name));
  }

  return !error;
}
