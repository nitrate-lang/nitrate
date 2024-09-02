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
        const auto &provided_args = std::get<1>(info);

        if (base->getKind() == QIR_NODE_IDENT) { /* Direct call */
          const std::string_view &funcname = base->as<Ident>()->getName();

          { /* Function does not exist */
            if (mod->getFunctions().left.count(funcname) == 0) {
              NO_MATCHING_FUNCTION(funcname);
              error = true;
              return IterOp::Proceed;
            }
          }

          qcore_assert(mod->getParameterMap().contains(funcname));

          std::unordered_map<std::string_view, std::pair<size_t, Expr *>> named_args_map;
          Fn *target = mod->getFunctions().left.at(funcname);
          const auto &params = mod->getParameterMap().at(funcname);
          DirectCallArgs new_args;
          new_args.resize(std::max(provided_args.size(), target->getParams().size()));
          size_t pos_i = 0;

          qcore_assert(params.size() == target->getParams().size());

          for (size_t i = 0; i < params.size(); i++) {
            named_args_map[std::get<0>(params[i])] = {i, std::get<2>(params[i])};
          }

          { /* Check for too many arguments */
            if (provided_args.size() > params.size() && !target->isVariadic()) {
              TOO_MANY_ARGUMENTS(funcname);
              error = true;
              return IterOp::Proceed;
            }
          }

          { /* Allocate arguments into function parameters */
            for (auto &[name, arg] : provided_args) {
              if (std::isdigit(name.at(0))) { /* positional argument */
                new_args.at(pos_i) = arg;
                pos_i++;
              } else { /* named argument */
                if (!named_args_map.contains(name)) {
                  std::cout << "No matching parameter: " << name << std::endl;
                  NO_MATCHING_PARAMETER(funcname, name);
                  error = true;
                  return IterOp::Proceed;
                }

                new_args[named_args_map[name].first] = arg;
                if (named_args_map[name].first <= pos_i) {
                  pos_i++;
                }
              }
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
                return IterOp::Proceed;
              }
            }
          }

          *_cur = create<DirectCall>(target, std::move(new_args));
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
