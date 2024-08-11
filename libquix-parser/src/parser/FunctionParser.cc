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

#include "parser/Parse.h"

using namespace qparse;
using namespace qparse::parser;
using namespace qparse::diag;

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

static bool fn_get_property(qlex_t *rd, GetPropState &state) {
  qlex_tok_t tok = qlex_peek(rd);

  if (tok.is(qEofF)) {
    syntax(tok, "Expected a function property but found EOF");
    return false;
  }

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
  auto tok = qlex_next(rd);

  std::string name;
  if (!tok.is(qName)) {
    syntax(tok, "Expected a parameter name before ':'");
  }

  name = tok.as_string(rd);
  tok = qlex_next(rd);
  if (!tok.is<qPuncColn>()) {
    syntax(tok, "Expected ':' after parameter name");
  }

  Type *type = nullptr;

  if (!parse_type(job, rd, &type)) {
    qlex_next(rd);
    syntax(tok, "Expected a type after ':'");
  }

  tok = qlex_peek(rd);

  if (tok.is<qOpSet>()) {
    qlex_next(rd);
    tok = qlex_peek(rd);

    Expr *value = nullptr;
    if (!parse_expr(job, rd, {qlex_tok_t(qPunc, qPuncComa), qlex_tok_t(qPunc, qPuncRPar)},
                    &value) ||
        !value) {
      syntax(tok, "Expected an expression after '='");
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

static FunctionProperties read_function_properties(qlex_t *rd) {
  GetPropState state;

  qlex_tok_t tok = qlex_peek(rd);

  while (fn_get_property(rd, state));

  if (state.noexcept_ctr > 1) {
    syntax(tok, "Multiple 'noexcept' specifiers");
    return FunctionProperties();
  }

  if (state.foreign_ctr > 1) {
    syntax(tok, "Multiple 'foreign' specifiers");
    return FunctionProperties();
  }

  if (state.impure_ctr > 1) {
    syntax(tok, "Multiple 'impure' specifiers");
    return FunctionProperties();
  }

  if (state.tsafe_ctr > 1) {
    syntax(tok, "Multiple 'tsafe' specifiers");
    return FunctionProperties();
  }

  if (state.pure_ctr > 1) {
    syntax(tok, "Multiple 'pure' specifiers");
    return FunctionProperties();
  }

  if (state.quasipure_ctr > 1) {
    syntax(tok, "Multiple 'quasipure' specifiers");
    return FunctionProperties();
  }

  if (state.retropure_ctr > 1) {
    syntax(tok, "Multiple 'retropure' specifiers");
    return FunctionProperties();
  }

  if (state.inline_ctr > 1) {
    syntax(tok, "Multiple 'inline' specifiers");
    return FunctionProperties();
  }

  bool partial_pure = state.pure_ctr || state.quasipure_ctr || state.retropure_ctr;

  if (partial_pure && state.impure_ctr) {
    syntax(tok, "Cannot mix 'pure', 'quasipure', 'retropure' with 'impure'");
    return FunctionProperties();
  }

  if (partial_pure && (state.pure_ctr + state.quasipure_ctr + state.retropure_ctr) != 1) {
    syntax(tok, "Multiple purity specifiers; Illegal combination");
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

  auto prop = read_function_properties(rd);

  qlex_tok_t tok = qlex_next(rd);

  if (tok.is(qName)) {
    fndecl->set_name(tok.as_string(rd));
    tok = qlex_next(rd);
  }

  if (!tok.is<qPuncLPar>()) {
    syntax(tok, "Expected '(' after function name");
  }
  bool is_variadic = false;

  while (1) {
    tok = qlex_peek(rd);
    if (tok.is(qEofF)) {
      syntax(tok, "Unexpected EOF in function signature");
      return false;
    }

    if (tok.is<qPuncRPar>()) {
      qlex_next(rd);
      break;
    }

    if (tok.is<qOpEllipsis>()) {
      is_variadic = true;

      qlex_next(rd);
      tok = qlex_next(rd);
      if (!tok.is<qPuncRPar>()) {
        syntax(tok, "Expected ')' after '...'");
      }

      break;
    }

    FuncParam param;
    if (!parse_fn_parameter(job, rd, param)) {
      syntax(tok, "Expected a parameter");
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

  Type *ret_type = nullptr;

  if (tok.is<qPuncColn>()) {
    qlex_next(rd);

    if (!parse_type(job, rd, &ret_type)) {
      syntax(tok, "Expected a return type after ':'");
    }

    ftype->set_return_ty(ret_type);
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

    Block *fnbody = nullptr;

    if (!parse(job, rd, &fnbody, false, true)) {
      syntax(tok, "Expected a block after '=>'");
      return false;
    }

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
      syntax(tok, "Expected a block after '{'");
    }

    tok = qlex_peek(rd);
    Expr *req_in = nullptr, *req_out = nullptr;
    if (tok.is<qKReq>()) {
      /* Parse constraint block */
      qlex_next(rd);

      tok = qlex_next(rd);
      if (!tok.is<qPuncLCur>()) {
        syntax(tok, "Expected '{' after 'req'");
      }

      while (true) {
        tok = qlex_peek(rd);
        if (tok.is(qEofF)) {
          syntax(tok, "Unexpected EOF in 'req' block");
          break;
        }

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

          if (!parse_expr(job, rd, {qlex_tok_t(qPunc, qPuncSemi)}, &expr) || !expr) {
            syntax(tok, "Expected an expression after 'in'");
          }

          tok = qlex_next(rd);
          if (!tok.is<qPuncSemi>()) {
            syntax(tok, "Expected ';' after expression");
          }

          expr = UnaryExpr::get(qOpLogicNot, expr);
          expr = UnaryExpr::get(qOpLogicNot, expr);

          req_in = BinExpr::get(req_in, qOpLogicAnd, expr);
        } else if (tok.is<qOpOut>()) {
          if (!req_out) {
            req_out = ConstBool::get(true);
          }

          if (!parse_expr(job, rd, {qlex_tok_t(qPunc, qPuncSemi)}, &expr) || !expr) {
            syntax(tok, "Expected an expression after 'out'");
          }

          tok = qlex_next(rd);
          if (!tok.is<qPuncSemi>()) {
            syntax(tok, "Expected ';' after expression");
          }

          expr = UnaryExpr::get(qOpLogicNot, expr);
          expr = UnaryExpr::get(qOpLogicNot, expr);
          req_out = BinExpr::get(req_out, qOpLogicAnd, expr);
        } else {
          syntax(tok, "Expected 'in' or 'out' after 'req'");
        }
      }

      tok = qlex_peek(rd);
    }

    std::set<std::string> implements;

    if (!job.conf->has(QPK_NO_AUTO_IMPL, QPV_FUNCTION)) {
      implements.insert("auto");
    }

    if (tok.is<qKImpl>()) {
      qlex_next(rd);
      tok = qlex_next(rd);
      if (!tok.is<qPuncLBrk>()) {
        syntax(tok, "Expected '[' after 'impl'");
      }

      while (true) {
        tok = qlex_next(rd);
        if (tok.is(qEofF)) {
          syntax(tok, "Unexpected EOF in 'impl' block");
          break;
        }

        if (tok.is<qPuncRBrk>()) break;

        if (!tok.is(qName)) {
          syntax(tok, "Expected a trait name after 'impl'");
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
  } else if (ret_type) {
    syntax(tok, "Expected '{', '=>', or ';' in function declaration");
    return true;
  } else {
    syntax(tok, "Expected ':', '{', '=>', or ';' in function declaration");
    return true;
  }
}