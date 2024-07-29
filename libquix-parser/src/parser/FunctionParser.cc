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

#define __QUIX_IMPL__

#include "LibMacro.h"
#include "parser/Parse.h"

using namespace qparse;
using namespace qparse::parser;

struct GetPropState {
  size_t noexcept_ctr = 0;
  size_t foreign_ctr = 0;
  size_t impure_ctr = 0;
  size_t tsafe_ctr = 0;
  size_t pure_ctr = 0;
  size_t quasipure_ctr = 0;
  size_t retropure_ctr = 0;
  size_t inline_ctr = 0;
};

static bool fn_get_property(qparse_t &job, qlex_t *rd, GetPropState &state) {
  qlex_tok_t tok = qlex_peek(rd);

  if (tok.is<qKNoexcept>()) {
    qlex_next(rd);
    state.noexcept_ctr++;
    return true;
  }

  if (tok.is<qKForeign>()) {
    qlex_next(rd);
    state.foreign_ctr++;
    return true;
  }

  if (tok.is<qKImpure>()) {
    qlex_next(rd);
    state.impure_ctr++;
    return true;
  }

  if (tok.is<qKTsafe>()) {
    qlex_next(rd);
    state.tsafe_ctr++;
    return true;
  }

  if (tok.is<qKPure>()) {
    qlex_next(rd);
    state.pure_ctr++;
    return true;
  }

  if (tok.is<qKQuasipure>()) {
    qlex_next(rd);
    state.quasipure_ctr++;
    return true;
  }

  if (tok.is<qKRetropure>()) {
    qlex_next(rd);
    state.retropure_ctr++;
    return true;
  }

  if (tok.is<qKInline>()) {
    qlex_next(rd);
    state.inline_ctr++;
    return true;
  }

  return false;
}

static bool parse_fn_parameter(qparse_t &job, qlex_t *rd, FuncParam &param) {
  /*
   <name> : <type> [?] [= <value>]
  */

  auto tok = qlex_next(rd);

  std::string name;
  if (!tok.is(qName)) {
    /// TODO: Write the ERROR message
    return false;
  }

  name = tok.as_string(rd);
  tok = qlex_next(rd);
  if (!tok.is<qPuncColn>()) {
    /// TODO: Write the ERROR message
    return false;
  }

  Type *type = nullptr;

  if (!parse_type(job, rd, &type)) {
    /// TODO: Write the ERROR message
    return false;
  }

  tok = qlex_peek(rd);

  if (tok.is<qOpSet>()) {
    qlex_next(rd);

    Expr *value = nullptr;
    if (!parse_expr(job, rd, {qlex_tok_t(qPunc, qPuncComa), qlex_tok_t(qPunc, qPuncRPar)},
                    &value)) {
      /// TODO: Write the ERROR message
      return false;
    }

    param = {name, type, value};
  } else {
    param = {name, type, nullptr};
  }

  return true;
}

enum class Purity { Pure, QuasiPure, RetroPure, Impure };

struct FunctionProperties {
  bool _noexcept = false;
  bool _foreign = false;
  bool _tsafe = false;
  Purity _purity = Purity::Impure;
};

static FunctionProperties read_function_properties(qparse_t &job, qlex_t *rd) {
  GetPropState state;

  while (fn_get_property(job, rd, state));

  if (state.noexcept_ctr > 1) {
    /// TODO: Write the ERROR message
    return FunctionProperties();
  }

  if (state.foreign_ctr > 1) {
    /// TODO: Write the ERROR message
    return FunctionProperties();
  }

  if (state.impure_ctr > 1) {
    /// TODO: Write the ERROR message
    return FunctionProperties();
  }

  if (state.tsafe_ctr > 1) {
    /// TODO: Write the ERROR message
    return FunctionProperties();
  }

  if (state.pure_ctr > 1) {
    /// TODO: Write the ERROR message
    return FunctionProperties();
  }

  if (state.quasipure_ctr > 1) {
    /// TODO: Write the ERROR message
    return FunctionProperties();
  }

  if (state.retropure_ctr > 1) {
    /// TODO: Write the ERROR message
    return FunctionProperties();
  }

  if (state.inline_ctr > 1) {
    /// TODO: Write the ERROR message
    return FunctionProperties();
  }

  bool partial_pure = state.pure_ctr || state.quasipure_ctr || state.retropure_ctr;

  if (partial_pure && state.impure_ctr) {
    /// TODO: Write the ERROR message
    return FunctionProperties();
  }

  if (partial_pure && (state.pure_ctr + state.quasipure_ctr + state.retropure_ctr) != 1) {
    /// TODO: Write the ERROR message
    return FunctionProperties();
  }

  if (partial_pure) {
    state.tsafe_ctr = 1;
    state.noexcept_ctr = 1;
  }

  FunctionProperties props;

  /// TODO: verify that this is what i want

  props._foreign = state.foreign_ctr;
  props._noexcept = state.noexcept_ctr;
  props._tsafe = state.tsafe_ctr;

  if (state.pure_ctr) {
    props._purity = Purity::Pure;
  } else if (state.quasipure_ctr) {
    props._purity = Purity::QuasiPure;
  } else if (state.retropure_ctr) {
    props._purity = Purity::RetroPure;
  } else {
    props._purity = Purity::Impure;
  }

  return props;
}

bool qparse::parser::parse_function(qparse_t &job, qlex_t *rd, Stmt **node) {
  FnDecl *fndecl = FnDecl::get();
  FuncTy *ftype = FuncTy::get();

  auto prop = read_function_properties(job, rd);

  qlex_tok_t tok = qlex_next(rd);

  if (tok.is(qName)) {
    fndecl->set_name(tok.as_string(rd));
    tok = qlex_next(rd);
  }

  if (!tok.is<qPuncLPar>()) {
    /// TODO: Write the ERROR message
    return false;
  }
  bool is_variadic = false;

  while (1) {
    tok = qlex_peek(rd);
    if (tok.is<qPuncRPar>()) {
      qlex_next(rd);
      break;
    }

    if (tok.is<qOpEllipsis>()) {
      is_variadic = true;

      qlex_next(rd);
      tok = qlex_next(rd);
      if (!tok.is<qPuncRPar>()) {
        /// TODO: Write the ERROR message
        return false;
      }

      break;
    }

    FuncParam param;
    if (!parse_fn_parameter(job, rd, param)) {
      /// TODO: Write the ERROR message
      return false;
    }

    ftype->add_param(std::get<0>(param), std::get<1>(param), std::get<2>(param));

    tok = qlex_peek(rd);
    if (tok.is<qPuncComa>()) {
      qlex_next(rd);
      continue;
    }
  }

  tok = qlex_peek(rd);
  /// TODO: Implement function properties

  if (tok.is<qPuncSemi>()) {
    ftype->set_variadic(is_variadic);
    ftype->set_foreign(prop._foreign);
    ftype->set_noexcept(prop._noexcept);
    fndecl->set_type(ftype);
    qlex_next(rd);
    *node = fndecl;
    return true;
  }

  if (tok.is<qPuncColn>()) {
    qlex_next(rd);
    Type *type = nullptr;

    if (!parse_type(job, rd, &type)) return false;

    ftype->set_return_ty(type);
    ftype->set_variadic(is_variadic);
    ftype->set_foreign(prop._foreign);
    ftype->set_noexcept(prop._noexcept);
    fndecl->set_type(ftype);

    tok = qlex_peek(rd);
    if (tok.is<qPuncSemi>()) {
      qlex_next(rd);
      *node = fndecl;
      return true;
    }
  }

  if (tok.is<qOpArrow>()) {
    qlex_next(rd);

    // auto fnbody = std::make_shared<Block>();
    Block *fnbody = nullptr;

    if (!parse(job, rd, &fnbody, false, true)) return false;

    if (!fndecl->get_type()) {
      ftype->set_return_ty(VoidTy::get());
      ftype->set_variadic(is_variadic);
      ftype->set_foreign(prop._foreign);
      ftype->set_noexcept(prop._noexcept);
      fndecl->set_type(ftype);
    }

    fndecl = FnDef::get(fndecl, fnbody);

    *node = fndecl;
    return true;
  } else if (tok.is<qPuncLCur>()) {
    Block *fnbody = nullptr;

    if (!parse(job, rd, &fnbody)) {
      return false;
    }

    tok = qlex_peek(rd);
    Expr *req_in = nullptr, *req_out = nullptr;
    if (tok.is<qKReq>()) {
      /* Parse constraint block */
      qlex_next(rd);

      tok = qlex_next(rd);
      if (!tok.is<qPuncLCur>()) {
        /// TODO: Write the ERROR message
        return false;
      }

      while (true) {
        tok = qlex_peek(rd);
        if (tok.is<qPuncRCur>()) {
          qlex_next(rd);
          break;
        }

        Expr *expr = nullptr;
        qlex_next(rd);
        if (tok.is<qOpIn>()) {
          if (!req_in) {
            req_in = ConstBool::get(true);
          }

          if (!parse_expr(job, rd, {qlex_tok_t(qPunc, qPuncSemi)}, &expr)) {
            return false;
          }

          tok = qlex_next(rd);
          if (!tok.is<qPuncSemi>()) {
            /// TODO: Write the ERROR message
            return false;
          }

          expr = UnaryExpr::get(UnaryOp::LogicalNot, expr);
          expr = UnaryExpr::get(UnaryOp::LogicalNot, expr);

          req_in = BinExpr::get(req_in, BinOp::LogicalAnd, expr);
        } else if (tok.is<qOpOut>()) {
          if (!req_out) {
            req_out = ConstBool::get(true);
          }

          if (!parse_expr(job, rd, {qlex_tok_t(qPunc, qPuncSemi)}, &expr)) {
            return false;
          }

          tok = qlex_next(rd);
          if (!tok.is<qPuncSemi>()) {
            /// TODO: Write the ERROR message
            return false;
          }

          expr = UnaryExpr::get(UnaryOp::LogicalNot, expr);
          expr = UnaryExpr::get(UnaryOp::LogicalNot, expr);
          req_out = BinExpr::get(req_out, BinOp::LogicalAnd, expr);
        } else {
          /// TODO: Write the ERROR message
          return false;
        }
      }

      tok = qlex_peek(rd);
    }

    std::set<std::string> implements;

    if (!job.conf->has("-fno-auto-impl", "function")) {
      implements.insert("auto");
    }

    if (tok.is<qKImpl>()) {
      qlex_next(rd);
      tok = qlex_next(rd);
      if (!tok.is<qPuncLBrk>()) {
        /// TODO: Write the ERROR message
        return false;
      }

      while (true) {
        tok = qlex_next(rd);
        if (tok.is<qPuncRBrk>()) break;

        if (!tok.is(qName)) {
          /// TODO: Write the ERROR message
          return false;
        }

        implements.insert(tok.as_string(rd));

        tok = qlex_peek(rd);
        if (tok.is<qPuncComa>()) {
          qlex_next(rd);
        }
      }

      tok = qlex_peek(rd);
    }

    if (!fndecl->get_type()) {
      ftype->set_return_ty(VoidTy::get());
      ftype->set_variadic(is_variadic);
      ftype->set_foreign(prop._foreign);
      ftype->set_noexcept(prop._noexcept);
      fndecl->set_type(ftype);
    }

    fndecl = FnDef::get(fndecl, fnbody, req_in, req_out);
    fndecl->add_tags(implements);

    *node = fndecl;
    return true;
  } else {
    /// TODO: Write the ERROR message
    return false;
  }
}