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
#include <nitrate-lexer/Lexer.h>
#include <nitrate-parser/Node.h>
#include <parser/Parse.h>

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
  Type *type = nullptr;

  if (!tok.is(qName)) {
    syntax(tok, "Expected a parameter name before ':'");
  }

  name = tok.as_string(rd);
  tok = qlex_peek(rd);

  if (tok.is<qPuncColn>()) {
    qlex_next(rd);

    if (!parse_type(job, rd, &type) || !type) {
      qlex_next(rd);
      syntax(tok, "Expected a type after ':'");
    }

    tok = qlex_peek(rd);
  } else {
    type = InferTy::get();
  }

  if (tok.is<qOpSet>()) {
    qlex_next(rd);
    tok = qlex_peek(rd);

    Expr *value = nullptr;
    if (!parse_expr(job, rd,
                    {qlex_tok_t(qPunc, qPuncComa), qlex_tok_t(qPunc, qPuncRPar),
                     qlex_tok_t(qOper, qOpGT)},
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

  bool partial_pure =
      state.pure_ctr || state.quasipure_ctr || state.retropure_ctr;

  if (partial_pure && state.impure_ctr) {
    syntax(tok, "Cannot mix 'pure', 'quasipure', 'retropure' with 'impure'");
    return FunctionProperties();
  }

  if (partial_pure &&
      (state.pure_ctr + state.quasipure_ctr + state.retropure_ctr) != 1) {
    syntax(tok, "Multiple purity specifiers; Illegal combination");
    return FunctionProperties();
  }

  if (partial_pure) {
    state.tsafe_ctr = 1;
    state.noexcept_ctr = 1;
  }

  FunctionProperties props;

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

static bool parse_captures_and_name(qlex_t *rd, FnDecl *fndecl,
                                    FnCaptures &captures) {
  qlex_tok_t c = qlex_peek(rd);

  if (c.is<qPuncLBrk>()) {
    qlex_next(rd);

    while (!c.is(qEofF)) {
      c = qlex_next(rd);

      if (c.is<qPuncRBrk>()) {
        break;
      }

      bool is_mut = false;

      if (c.is<qOpBitAnd>()) {
        is_mut = true;
        c = qlex_next(rd);
      }

      if (!c.is(qName)) {
        syntax(c, "Expected a capture name");
        return false;
      }

      captures.push_back({c.as_string(rd), is_mut});
      c = qlex_peek(rd);

      if (c.is<qPuncComa>()) {
        qlex_next(rd);
      }
    }

    c = qlex_peek(rd);
  }

  if (c.is(qName)) {
    qlex_next(rd);
    fndecl->set_name(c.as_string(rd));
  }

  return true;
}

bool parse_template_parameters(
    qparse_t &job, qlex_t *rd,
    std::optional<TemplateParameters> &template_params) {
  template_params = std::nullopt;

  qlex_tok_t c = qlex_peek(rd);

  if (!c.is<qOpLT>()) {
    return true;
  }

  qlex_next(rd);

  TemplateParameters params;

  while (1) {
    c = qlex_peek(rd);
    if (c.is(qEofF)) {
      syntax(c, "Unexpected EOF in signature");
      return false;
    }

    if (c.is<qOpGT>()) {
      qlex_next(rd);
      break;
    }

    FuncParam param;
    if (!parse_fn_parameter(job, rd, param)) {
      syntax(c, "Expected a parameter");
      return false;
    }

    params.push_back(
        {std::get<0>(param), std::get<1>(param), std::get<2>(param)});

    c = qlex_peek(rd);
    if (c.is<qPuncComa>()) {
      qlex_next(rd);
      continue;
    }
  }

  c = qlex_peek(rd);

  template_params = std::move(params);

  return true;
}

static bool parse_parameters(qparse_t &job, qlex_t *rd, FuncTy *ftype,
                             bool &is_variadic) {
  qlex_tok_t c = qlex_peek(rd);

  if (!c.is<qPuncLPar>()) {
    syntax(c, "Expected '(' after function name");
    return false;
  }

  qlex_next(rd);

  is_variadic = false;

  while (1) {
    c = qlex_peek(rd);
    if (c.is(qEofF)) {
      syntax(c, "Unexpected EOF in function signature");
      return false;
    }

    if (c.is<qPuncRPar>()) {
      qlex_next(rd);
      break;
    }

    if (c.is<qOpEllipsis>()) {
      is_variadic = true;

      qlex_next(rd);
      c = qlex_next(rd);
      if (!c.is<qPuncRPar>()) {
        syntax(c, "Expected ')' after '...'");
      }

      break;
    }

    FuncParam param;
    if (!parse_fn_parameter(job, rd, param)) {
      syntax(c, "Expected a parameter");
      return false;
    }

    ftype->get_params().push_back(
        {std::get<0>(param), std::get<1>(param), std::get<2>(param)});

    c = qlex_peek(rd);
    if (c.is<qPuncComa>()) {
      qlex_next(rd);
      continue;
    }
  }

  c = qlex_peek(rd);

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

static bool parse_constraints(qlex_tok_t &c, qlex_t *rd, qparse_t &job,
                              Expr *&req_in, Expr *&req_out) {
  if (c.is<qKPromise>()) {
    /* Parse constraint block */
    qlex_next(rd);

    c = qlex_next(rd);
    if (!c.is<qPuncLCur>()) {
      syntax(c, "Expected '{' after 'req'");
    }

    while (true) {
      c = qlex_peek(rd);
      if (c.is(qEofF)) {
        syntax(c, "Unexpected EOF in 'req' block");
        return false;
      }

      if (c.is<qPuncRCur>()) {
        qlex_next(rd);
        break;
      }

      qlex_next(rd);
      if (c.is<qOpIn>()) {
        Expr *expr = nullptr;

        if (!parse_expr(job, rd, {qlex_tok_t(qPunc, qPuncSemi)}, &expr) ||
            !expr) {
          syntax(c, "Expected an expression after 'in'");
          return false;
        }

        c = qlex_next(rd);
        if (!c.is<qPuncSemi>()) {
          syntax(c, "Expected ';' after expression");
          return false;
        }

        if (req_in) {
          req_in = BinExpr::get(req_in, qOpLogicAnd, expr);
        } else {
          req_in = expr;
        }
      } else if (c.is<qOpOut>()) {
        Expr *expr = nullptr;

        if (!parse_expr(job, rd, {qlex_tok_t(qPunc, qPuncSemi)}, &expr) ||
            !expr) {
          syntax(c, "Expected an expression after 'out'");
          return false;
        }

        c = qlex_next(rd);
        if (!c.is<qPuncSemi>()) {
          syntax(c, "Expected ';' after expression");
          return false;
        }

        if (req_out) {
          req_out = BinExpr::get(req_out, qOpLogicAnd, expr);
        } else {
          req_out = expr;
        }
      } else {
        syntax(c, "Expected 'in' or 'out' after 'req'");
        return false;
      }
    }

    c = qlex_peek(rd);
  }

  return true;
}

bool qparse::parser::parse_function(qparse_t &job, qlex_t *rd, Stmt **node) {
  FnDecl *fndecl = FnDecl::get();
  FuncTy *ftype = FuncTy::get();
  Type *ret_type = nullptr;
  FnCaptures captures;
  qlex_tok_t tok{};
  FunctionProperties prop;
  bool is_variadic = false;
  std::optional<TemplateParameters> params;

  prop = read_function_properties(rd);

  { /* Parse function name or anonymous function capture list */
    tok = qlex_peek(rd);

    if (!parse_captures_and_name(rd, fndecl, captures)) {
      syntax(tok, "Expected a function name or capture list");
      return false;
    }
  }

  { /* Parse possible template parameters */
    tok = qlex_peek(rd);

    if (!parse_template_parameters(job, rd, fndecl->get_template_params())) {
      syntax(tok, "Failed to parse template parameters");
      return false;
    }
  }

  { /* Parse function parameters */
    tok = qlex_peek(rd);

    if (!parse_parameters(job, rd, ftype, is_variadic)) {
      syntax(tok, "Failed to parse function parameters");
      return false;
    }
  }

  { /* Convert function properties */
    tok = qlex_peek(rd);

    if (!translate_purity(prop, ftype)) {
      syntax(tok, "Failed to translate purity");
      return false;
    }
  }

  tok = qlex_peek(rd);

  { /* Set function type and assign to function declaration */
    ftype->set_variadic(is_variadic);
    ftype->set_foreign(prop._foreign);
    ftype->set_noexcept(prop._noexcept);
    fndecl->set_type(ftype);
  }

  { /* Function declaration with implicit return type of void */
    if (tok.is<qPuncRPar>() || tok.is<qPuncRBrk>() || tok.is<qPuncRCur>() ||
        tok.is<qPuncSemi>()) {
      ftype->set_return_ty(VoidTy::get());

      tok = qlex_peek(rd);
      if (tok.is<qKWith>()) {
        std::set<Expr *> attributes;
        qlex_next(rd);

        if (!parse_attributes(job, rd, attributes)) {
          return true;
        }

        fndecl->get_tags().insert(attributes.begin(), attributes.end());
      }

      *node = fndecl;
      (*node)->set_end_pos(tok.start);
      return true;
    }
  }

  { /* Function with explicit return type */
    if (tok.is<qPuncColn>()) {
      qlex_next(rd);

      if (!parse_type(job, rd, &ret_type)) {
        syntax(tok, "Expected a return type after ':'");
      }

      ftype->set_return_ty(ret_type);
      tok = qlex_peek(rd);

      { /* Function declaration with explicit return type */
        if (tok.is<qPuncRPar>() || tok.is<qPuncRBrk>() || tok.is<qPuncRCur>() ||
            tok.is<qPuncSemi>()) {
          tok = qlex_peek(rd);
          if (tok.is<qKWith>()) {
            std::set<Expr *> attributes;
            qlex_next(rd);

            if (!parse_attributes(job, rd, attributes)) {
              return true;
            }

            fndecl->get_tags().insert(attributes.begin(), attributes.end());
          }
          *node = fndecl;
          (*node)->set_end_pos(tok.start);
          return true;
        }
      }
    }
  }

  { /* Function definition with arrow syntax */
    if (tok.is<qOpArrow>()) {
      qlex_next(rd);

      Block *fnbody = nullptr;

      if (!parse(job, rd, &fnbody, false, true)) {
        syntax(tok, "Expected a block after '=>'");
        return false;
      }

      if (!ftype->get_return_ty()) {
        ftype->set_return_ty(VoidTy::get());
      }

      while ((tok = qlex_peek(rd)).is<qPuncSemi>()) {
        qlex_next(rd);
      }

      FnDef *fndef = FnDef::get(fndecl, fnbody, nullptr, nullptr, captures);

      tok = qlex_peek(rd);
      if (tok.is<qKWith>()) {
        std::set<Expr *> attributes;
        qlex_next(rd);

        if (!parse_attributes(job, rd, attributes)) {
          return true;
        }

        fndef->get_tags().insert(attributes.begin(), attributes.end());
      }

      *node = fndef;
      (*node)->set_end_pos(fnbody->get_end_pos());
      return true;
    }
  }

  if (tok.is<qPuncLCur>()) {
    Block *fnbody = nullptr;
    Expr *req_in = nullptr, *req_out = nullptr;
    std::set<Expr *> attributes;

    if (!parse(job, rd, &fnbody)) {
      syntax(tok, "Expected a block after '{'");
    }

    tok = qlex_peek(rd);

    if (!parse_constraints(tok, rd, job, req_in, req_out)) {
      return false;
    }

    tok = qlex_peek(rd);
    if (tok.is<qKWith>()) {
      qlex_next(rd);

      if (!parse_attributes(job, rd, attributes)) {
        return true;
      }
    }

    if (!ftype->get_return_ty()) {
      ftype->set_return_ty(VoidTy::get());
    }

    FnDef *fndef = FnDef::get(fndecl, fnbody, req_in, req_out, captures);
    fndef->get_tags().insert(attributes.begin(), attributes.end());

    *node = fndef;
    (*node)->set_end_pos(tok.end);
    return true;
  }

  if (ret_type) {
    syntax(tok, "Expected '{', '=>', or ';' in function declaration");
    return true;
  }

  syntax(tok, "Expected ':', '{', '=>', or ';' in function declaration");
  return true;
}