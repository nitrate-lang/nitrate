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

/// TODO: Source location

/// TODO: Cleanup this code; it's a mess from refactoring.

#include <nitrate-lexer/Lexer.h>
#include <nitrate-parser/Node.h>

#include <descent/Recurse.hh>

using namespace npar;

struct GetPropState {
  size_t foreign_ctr = 0;
  size_t impure_ctr = 0;
  size_t tsafe_ctr = 0;
  size_t pure_ctr = 0;
  size_t quasipure_ctr = 0;
  size_t retropure_ctr = 0;
  size_t inline_ctr = 0;
};

static bool fn_get_property(qlex_t &rd, GetPropState &state) {
  qlex_tok_t tok = peek();

  if (tok.is(qEofF)) {
    diagnostic << tok << "Expected a function property but found EOF";
    return false;
  }

  if (tok.is<qKForeign>()) {
    next();
    state.foreign_ctr++;
    return true;
  }

  if (tok.is<qKImpure>()) {
    next();
    state.impure_ctr++;
    return true;
  }

  if (tok.is<qKTsafe>()) {
    next();
    state.tsafe_ctr++;
    return true;
  }

  if (tok.is<qKPure>()) {
    next();
    state.pure_ctr++;
    return true;
  }

  if (tok.is<qKQuasipure>()) {
    next();
    state.quasipure_ctr++;
    return true;
  }

  if (tok.is<qKRetropure>()) {
    next();
    state.retropure_ctr++;
    return true;
  }

  if (tok.is<qKInline>()) {
    next();
    state.inline_ctr++;
    return true;
  }

  return false;
}

static bool recurse_fn_parameter(npar_t &S, qlex_t &rd, FuncParam &param) {
  auto tok = next();

  std::string name;
  Type *type = nullptr;

  if (!tok.is(qName)) {
    diagnostic << tok << "Expected a parameter name before ':'";
  }

  name = tok.as_string(&rd);
  tok = peek();

  if (tok.is<qPuncColn>()) {
    next();

    type = recurse_type(S, rd);

    tok = peek();
  } else {
    type = make<InferTy>();
  }

  if (tok.is<qOpSet>()) {
    next();
    tok = peek();

    Expr *value =
        recurse_expr(S, rd,
                     {qlex_tok_t(qPunc, qPuncComa),
                      qlex_tok_t(qPunc, qPuncRPar), qlex_tok_t(qOper, qOpGT)});

    param = {SaveString(name), type, value};
  } else {
    param = {SaveString(name), type, nullptr};
  }

  return true;
}

enum class Purity { Pure, QuasiPure, RetroPure, Impure };

struct FunctionProperties {
  bool _foreign = false;
  bool _tsafe = false;
  Purity _purity = Purity::Impure;
};

static FunctionProperties read_function_properties(qlex_t &rd) {
  GetPropState state;

  qlex_tok_t tok = peek();

  while (fn_get_property(rd, state));

  if (state.foreign_ctr > 1) {
    diagnostic << tok << "Multiple 'foreign' specifiers";
    return FunctionProperties();
  }

  if (state.impure_ctr > 1) {
    diagnostic << tok << "Multiple 'impure' specifiers";
    return FunctionProperties();
  }

  if (state.tsafe_ctr > 1) {
    diagnostic << tok << "Multiple 'tsafe' specifiers";
    return FunctionProperties();
  }

  if (state.pure_ctr > 1) {
    diagnostic << tok << "Multiple 'pure' specifiers";
    return FunctionProperties();
  }

  if (state.quasipure_ctr > 1) {
    diagnostic << tok << "Multiple 'quasipure' specifiers";
    return FunctionProperties();
  }

  if (state.retropure_ctr > 1) {
    diagnostic << tok << "Multiple 'retropure' specifiers";
    return FunctionProperties();
  }

  if (state.inline_ctr > 1) {
    diagnostic << tok << "Multiple 'inline' specifiers";
    return FunctionProperties();
  }

  bool partial_pure =
      state.pure_ctr || state.quasipure_ctr || state.retropure_ctr;

  if (partial_pure && state.impure_ctr) {
    diagnostic << tok
               << "Cannot mix 'pure', 'quasipure', 'retropure' with 'impure'";
    return FunctionProperties();
  }

  if (partial_pure &&
      (state.pure_ctr + state.quasipure_ctr + state.retropure_ctr) != 1) {
    diagnostic << tok << "Multiple purity specifiers; Illegal combination";
    return FunctionProperties();
  }

  if (partial_pure) {
    state.tsafe_ctr = 1;
  }

  FunctionProperties props;

  props._foreign = state.foreign_ctr;
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

static bool recurse_captures_and_name(qlex_t &rd, FnDef *fndecl,
                                      FnCaptures &captures) {
  qlex_tok_t c = peek();

  if (c.is<qPuncLBrk>()) {
    next();

    while (!c.is(qEofF)) {
      c = next();

      if (c.is<qPuncRBrk>()) {
        break;
      }

      bool is_mut = false;

      if (c.is<qOpBitAnd>()) {
        is_mut = true;
        c = next();
      }

      if (!c.is(qName)) {
        diagnostic << c << "Expected a capture name";
        return false;
      }

      captures.push_back({SaveString(c.as_string(&rd)), is_mut});
      c = peek();

      if (c.is<qPuncComa>()) {
        next();
      }
    }

    c = peek();
  }

  if (c.is(qName)) {
    next();
    fndecl->set_name(SaveString(c.as_string(&rd)));
  }

  return true;
}

bool recurse_template_parameters(
    npar_t &S, qlex_t &rd, std::optional<TemplateParameters> &template_params) {
  template_params = std::nullopt;

  qlex_tok_t c = peek();

  if (!c.is<qOpLT>()) {
    return true;
  }

  next();

  TemplateParameters params;

  while (1) {
    c = peek();
    if (c.is(qEofF)) {
      diagnostic << c << "Unexpected EOF in signature";
      return false;
    }

    if (c.is<qOpGT>()) {
      next();
      break;
    }

    FuncParam param;
    if (!recurse_fn_parameter(S, rd, param)) {
      diagnostic << c << "Expected a parameter";
      return false;
    }

    params.push_back(
        {std::get<0>(param), std::get<1>(param), std::get<2>(param)});

    c = peek();
    if (c.is<qPuncComa>()) {
      next();
      continue;
    }
  }

  c = peek();

  template_params = std::move(params);

  return true;
}

static bool recurse_parameters(npar_t &S, qlex_t &rd, FuncTy *ftype,
                               bool &is_variadic) {
  qlex_tok_t c = peek();

  if (!c.is<qPuncLPar>()) {
    diagnostic << c << "Expected '(' after function name";
    return false;
  }

  next();

  is_variadic = false;

  while (1) {
    c = peek();
    if (c.is(qEofF)) {
      diagnostic << c << "Unexpected EOF in function signature";
      return false;
    }

    if (c.is<qPuncRPar>()) {
      next();
      break;
    }

    if (c.is<qOpEllipsis>()) {
      is_variadic = true;

      next();
      c = next();
      if (!c.is<qPuncRPar>()) {
        diagnostic << c << "Expected ')' after '...'";
      }

      break;
    }

    FuncParam param;
    if (!recurse_fn_parameter(S, rd, param)) {
      diagnostic << c << "Expected a parameter";
      return false;
    }

    ftype->get_params().push_back(
        {std::get<0>(param), std::get<1>(param), std::get<2>(param)});

    c = peek();
    if (c.is<qPuncComa>()) {
      next();
      continue;
    }
  }

  c = peek();

  return true;
}

static bool translate_purity(FunctionProperties prop, FuncTy *ftype) {
  switch (prop._purity) {
    case Purity::Pure:
      ftype->set_purity(FuncPurity::PURE);
      break;
    case Purity::QuasiPure:
      ftype->set_purity(FuncPurity::QUASIPURE);
      break;
    case Purity::RetroPure:
      ftype->set_purity(FuncPurity::RETROPURE);
      break;
    case Purity::Impure:
      if (prop._tsafe) {
        ftype->set_purity(FuncPurity::IMPURE_THREAD_SAFE);
      } else {
        ftype->set_purity(FuncPurity::IMPURE_THREAD_UNSAFE);
      }
      break;
  }

  return true;
}

static bool recurse_constraints(qlex_tok_t &c, qlex_t &rd, npar_t &S,
                                Expr *&req_in, Expr *&req_out) {
  if (c.is<qKPromise>()) {
    /* Parse constraint block */
    next();

    c = next();
    if (!c.is<qPuncLCur>()) {
      diagnostic << c << "Expected '{' after 'req'";
    }

    while (true) {
      c = peek();
      if (c.is(qEofF)) {
        diagnostic << c << "Unexpected EOF in 'req' block";
        return false;
      }

      if (c.is<qPuncRCur>()) {
        next();
        break;
      }

      next();
      if (c.is<qOpIn>()) {
        Expr *expr = recurse_expr(S, rd, {qlex_tok_t(qPunc, qPuncSemi)});

        c = next();
        if (!c.is<qPuncSemi>()) {
          diagnostic << c << "Expected ';' after expression";
          return false;
        }

        if (req_in) {
          req_in = make<BinExpr>(req_in, qOpLogicAnd, expr);
        } else {
          req_in = expr;
        }
      } else if (c.is<qOpOut>()) {
        Expr *expr = recurse_expr(S, rd, {qlex_tok_t(qPunc, qPuncSemi)});

        c = next();
        if (!c.is<qPuncSemi>()) {
          diagnostic << c << "Expected ';' after expression";
          return false;
        }

        if (req_out) {
          req_out = make<BinExpr>(req_out, qOpLogicAnd, expr);
        } else {
          req_out = expr;
        }
      } else {
        diagnostic << c << "Expected 'in' or 'out' after 'req'";
        return false;
      }
    }

    c = peek();
  }

  return true;
}

Stmt *npar::recurse_function(npar_t &S, qlex_t &rd) {
  FnDef *fndecl = make<FnDef>(SaveString(""), nullptr);
  FuncTy *ftype = make<FuncTy>();
  Type *ret_type = nullptr;
  FnCaptures captures;
  qlex_tok_t tok{};
  FunctionProperties prop;
  bool is_variadic = false;
  std::optional<TemplateParameters> params;

  prop = read_function_properties(rd);

  { /* Parse function name or anonymous function capture list */
    tok = peek();

    if (!recurse_captures_and_name(rd, fndecl, captures)) {
      diagnostic << tok << "Expected a function name or capture list";
      return mock_stmt(QAST_FUNCTION);
    }
  }

  { /* Parse possible template parameters */
    tok = peek();

    if (!recurse_template_parameters(S, rd, fndecl->get_template_params())) {
      diagnostic << tok << "Failed to parse template parameters";
      return mock_stmt(QAST_FUNCTION);
    }
  }

  { /* Parse function parameters */
    tok = peek();

    if (!recurse_parameters(S, rd, ftype, is_variadic)) {
      diagnostic << tok << "Failed to parse function parameters";
      return mock_stmt(QAST_FUNCTION);
    }
  }

  { /* Convert function properties */
    tok = peek();

    if (!translate_purity(prop, ftype)) {
      diagnostic << tok << "Failed to translate purity";
      return mock_stmt(QAST_FUNCTION);
    }
  }

  tok = peek();

  { /* Set function type and assign to function declaration */
    ftype->set_variadic(is_variadic);
    ftype->set_foreign(prop._foreign);
    fndecl->set_type(ftype);
  }

  { /* Function declaration with implicit return type of void */
    if (tok.is<qPuncRPar>() || tok.is<qPuncRBrk>() || tok.is<qPuncRCur>() ||
        tok.is<qPuncSemi>()) {
      ftype->set_return_ty(make<VoidTy>());

      return fndecl;
    }
  }

  { /* Function with explicit return type */
    if (tok.is<qPuncColn>()) {
      next();

      ret_type = recurse_type(S, rd);

      ftype->set_return_ty(ret_type);
      tok = peek();

      { /* Function declaration with explicit return type */
        if (tok.is<qPuncRPar>() || tok.is<qPuncRBrk>() || tok.is<qPuncRCur>() ||
            tok.is<qPuncSemi>()) {
          return fndecl;
        }
      }
    }
  }

  { /* Function definition with arrow syntax */
    if (tok.is<qOpArrow>()) {
      next();

      Stmt *fnbody = nullptr;

      fnbody = recurse_block(S, rd, false, true);

      if (!ftype->get_return_ty()) {
        ftype->set_return_ty(make<VoidTy>());
      }

      while ((tok = peek()).is<qPuncSemi>()) {
        next();
      }

      fndecl->set_body(fnbody);
      fndecl->set_captures(captures);

      return fndecl;
    }
  }

  if (tok.is<qPuncLCur>()) {
    Stmt *fnbody = nullptr;
    Expr *req_in = nullptr, *req_out = nullptr;

    fnbody = recurse_block(S, rd);

    tok = peek();

    if (!recurse_constraints(tok, rd, S, req_in, req_out)) {
      return mock_stmt(QAST_FUNCTION);
    }

    if (!ftype->get_return_ty()) {
      ftype->set_return_ty(make<VoidTy>());
    }

    fndecl->set_body(fnbody);
    fndecl->set_precond(req_in);
    fndecl->set_postcond(req_out);
    fndecl->set_captures(captures);

    return fndecl;
  }

  if (ret_type) {
    diagnostic << tok << "Expected '{', '=>', or ';' in function declaration";
    return mock_stmt(QAST_FUNCTION);
  }

  diagnostic << tok
             << "Expected ':', '{', '=>', or ';' in function declaration";
  return mock_stmt(QAST_FUNCTION);
}