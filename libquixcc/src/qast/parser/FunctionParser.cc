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

#define QUIXCC_INTERNAL

#include <core/Macro.h>
#include <qast/Parser.h>
#include <quixcc/core/Logger.h>

using namespace libquixcc;
using namespace libquixcc::qast;
using namespace libquixcc::qast::parser;

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

static bool fn_get_property(quixcc_cc_job_t &job, libquixcc::Scanner *scanner,
                            GetPropState &state) {
  Token tok = scanner->peek();

  if (tok.is<Keyword>(Keyword::Noexcept)) {
    scanner->next();
    state.noexcept_ctr++;
    return true;
  }

  if (tok.is<Keyword>(Keyword::Foreign)) {
    scanner->next();
    state.foreign_ctr++;
    return true;
  }

  if (tok.is<Keyword>(Keyword::Impure)) {
    scanner->next();
    state.impure_ctr++;
    return true;
  }

  if (tok.is<Keyword>(Keyword::Tsafe)) {
    scanner->next();
    state.tsafe_ctr++;
    return true;
  }

  if (tok.is<Keyword>(Keyword::Pure)) {
    scanner->next();
    state.pure_ctr++;
    return true;
  }

  if (tok.is<Keyword>(Keyword::Quasipure)) {
    scanner->next();
    state.quasipure_ctr++;
    return true;
  }

  if (tok.is<Keyword>(Keyword::Retropure)) {
    scanner->next();
    state.retropure_ctr++;
    return true;
  }

  if (tok.is<Keyword>(Keyword::Inline)) {
    scanner->next();
    state.inline_ctr++;
    return true;
  }

  return false;
}

static bool parse_fn_parameter(quixcc_cc_job_t &job, libquixcc::Scanner *scanner,
                               FuncParam &param) {
  /*
   <name> : <type> [?] [= <value>]
  */

  auto tok = scanner->next();

  std::string name;
  if (tok.type() != tName) {
    LOG(ERROR) << core::feedback[FN_PARAM_EXPECTED_IDENTIFIER] << tok << std::endl;
    return false;
  }

  name = tok.as_string();
  tok = scanner->next();
  if (!tok.is<Punctor>(Colon)) {
    LOG(ERROR) << core::feedback[FN_PARAM_EXPECTED_COLON] << tok << std::endl;
    return false;
  }

  Type *type = nullptr;

  if (!parse_type(job, scanner, &type)) {
    LOG(ERROR) << core::feedback[FN_PARAM_TYPE_ERR] << tok << std::endl;
    return false;
  }

  tok = scanner->peek();

  if (tok.is<Operator>(OpAssign)) {
    scanner->next();

    Expr *value = nullptr;
    if (!parse_expr(job, scanner, {Token(tPunc, Comma), Token(tPunc, CloseParen)}, &value)) {
      LOG(ERROR) << core::feedback[FN_PARAM_INIT_ERR] << tok << std::endl;
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

static FunctionProperties read_function_properties(quixcc_cc_job_t &job,
                                                   libquixcc::Scanner *scanner) {
  GetPropState state;

  while (fn_get_property(job, scanner, state))
    ;

  if (state.noexcept_ctr > 1) {
    LOG(ERROR) << core::feedback[FN_NOEXCEPT_MULTIPLE] << scanner->peek() << std::endl;
    return FunctionProperties();
  }

  if (state.foreign_ctr > 1) {
    LOG(ERROR) << core::feedback[FN_FOREIGN_MULTIPLE] << scanner->peek() << std::endl;
    return FunctionProperties();
  }

  if (state.impure_ctr > 1) {
    LOG(ERROR) << core::feedback[FN_IMPURE_MULTIPLE] << scanner->peek() << std::endl;
    return FunctionProperties();
  }

  if (state.tsafe_ctr > 1) {
    LOG(ERROR) << core::feedback[FN_TSAFE_MULTIPLE] << scanner->peek() << std::endl;
    return FunctionProperties();
  }

  if (state.pure_ctr > 1) {
    LOG(ERROR) << core::feedback[FN_PURE_MULTIPLE] << scanner->peek() << std::endl;
    return FunctionProperties();
  }

  if (state.quasipure_ctr > 1) {
    LOG(ERROR) << core::feedback[FN_QUASIPURE_MULTIPLE] << scanner->peek() << std::endl;
    return FunctionProperties();
  }

  if (state.retropure_ctr > 1) {
    LOG(ERROR) << core::feedback[FN_RETROPURE_MULTIPLE] << scanner->peek() << std::endl;
    return FunctionProperties();
  }

  if (state.inline_ctr > 1) {
    LOG(ERROR) << core::feedback[FN_INLINE_MULTIPLE] << scanner->peek() << std::endl;
    return FunctionProperties();
  }

  bool partial_pure = state.pure_ctr || state.quasipure_ctr || state.retropure_ctr;

  if (partial_pure && state.impure_ctr) {
    LOG(ERROR) << core::feedback[FN_PURE_IMPURE_MIX] << scanner->peek() << std::endl;
    return FunctionProperties();
  }

  if (partial_pure && (state.pure_ctr + state.quasipure_ctr + state.retropure_ctr) != 1) {
    LOG(ERROR) << core::feedback[FN_PURE_MIX] << scanner->peek() << std::endl;
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

bool libquixcc::qast::parser::parse_function(quixcc_cc_job_t &job, libquixcc::Scanner *scanner,
                                             Stmt **node) {
  FnDecl *fndecl = FnDecl::get();
  FuncTy *ftype = FuncTy::get();

  auto prop = read_function_properties(job, scanner);

  Token tok = scanner->next();

  if (tok.type() == tName) {
    fndecl->set_name(tok.as_string());
    tok = scanner->next();
  }

  if (!tok.is<Punctor>(OpenParen)) {
    LOG(ERROR) << core::feedback[FN_EXPECTED_OPEN_PAREN] << tok << std::endl;
    return false;
  }
  bool is_variadic = false;

  while (1) {
    tok = scanner->peek();
    if (tok.is<Punctor>(CloseParen)) {
      scanner->next();
      break;
    }

    if (tok.is<Operator>(Ellipsis)) {
      is_variadic = true;

      scanner->next();
      tok = scanner->next();
      if (!tok.is<Punctor>(CloseParen)) {
        LOG(ERROR) << core::feedback[FN_EXPECTED_VARARG] << tok << std::endl;
        return false;
      }

      break;
    }

    FuncParam param;
    if (!parse_fn_parameter(job, scanner, param)) {
      LOG(ERROR) << core::feedback[FN_PARAM_PARSE_ERROR] << tok << std::endl;
      return false;
    }

    ftype->add_param(std::get<0>(param), std::get<1>(param), std::get<2>(param));

    tok = scanner->peek();
    if (tok.is<Punctor>(Comma)) {
      scanner->next();
      continue;
    }
  }

  tok = scanner->peek();
  /// TODO: Implement function properties

  if (tok.is<Punctor>(Semicolon)) {
    ftype->set_variadic(is_variadic);
    ftype->set_foreign(prop._foreign);
    ftype->set_noexcept(prop._noexcept);
    fndecl->set_type(ftype);
    scanner->next();
    *node = fndecl;
    return true;
  }

  if (tok.is<Punctor>(Colon)) {
    scanner->next();
    Type *type = nullptr;

    if (!parse_type(job, scanner, &type))
      return false;

    ftype->set_return_ty(type);
    ftype->set_variadic(is_variadic);
    ftype->set_foreign(prop._foreign);
    ftype->set_noexcept(prop._noexcept);
    fndecl->set_type(ftype);

    tok = scanner->peek();
    if (tok.is<Punctor>(Semicolon)) {
      scanner->next();
      *node = fndecl;
      return true;
    }
  }

  if (tok.is<Operator>(Arrow)) {
    scanner->next();

    // auto fnbody = std::make_shared<Block>();
    Block *fnbody = nullptr;

    if (!parse(job, scanner, &fnbody, false, true))
      return false;

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
  } else if (tok.is<Punctor>(OpenBrace)) {
    Block *fnbody = nullptr;

    if (!parse(job, scanner, &fnbody)) {
      return false;
    }

    tok = scanner->peek();
    Expr *req_in = nullptr, *req_out = nullptr;
    if (tok.is<Keyword>(Keyword::Req)) {
      /* Parse constraint block */
      scanner->next();

      tok = scanner->next();
      if (!tok.is<Punctor>(OpenBrace)) {
        LOG(ERROR) << core::feedback[FN_EXPECTED_OPEN_BRACE] << tok << std::endl;
        return false;
      }

      while (true) {
        tok = scanner->peek();
        if (tok.is<Punctor>(CloseBrace)) {
          scanner->next();
          break;
        }

        Expr *expr = nullptr;
        scanner->next();
        if (tok.is<Operator>(Operator::In)) {
          if (!req_in) {
            req_in = ConstBool::get(true);
          }

          if (!parse_expr(job, scanner, {Token(tPunc, Semicolon)}, &expr)) {
            return false;
          }

          tok = scanner->next();
          if (!tok.is<Punctor>(Semicolon)) {
            LOG(ERROR) << core::feedback[FN_EXPECTED_SEMICOLON] << tok << std::endl;
            return false;
          }

          expr = UnaryExpr::get(UnaryOp::LogicalNot, expr);
          expr = UnaryExpr::get(UnaryOp::LogicalNot, expr);

          req_in = BinExpr::get(req_in, BinOp::LogicalAnd, expr);
        } else if (tok.is<Operator>(Operator::Out)) {
          if (!req_out) {
            req_out = ConstBool::get(true);
          }

          if (!parse_expr(job, scanner, {Token(tPunc, Semicolon)}, &expr)) {
            return false;
          }

          tok = scanner->next();
          if (!tok.is<Punctor>(Semicolon)) {
            LOG(ERROR) << core::feedback[FN_EXPECTED_SEMICOLON] << tok << std::endl;
            return false;
          }

          expr = UnaryExpr::get(UnaryOp::LogicalNot, expr);
          expr = UnaryExpr::get(UnaryOp::LogicalNot, expr);
          req_out = BinExpr::get(req_out, BinOp::LogicalAnd, expr);
        } else {
          LOG(ERROR) << core::feedback[FN_EXPECTED_IN_OUT] << tok << std::endl;
          return false;
        }
      }

      tok = scanner->peek();
    }

    std::set<std::string> implements;

    if (!job.has("-fno-auto-impl", "function")) {
      implements.insert("auto");
    }

    if (tok.is<Keyword>(Keyword::Impl)) {
      scanner->next();
      tok = scanner->next();
      if (!tok.is<Punctor>(OpenBracket)) {
        LOG(ERROR) << core::feedback[FN_DEF_EXPECTED_OPEN_BRACKET] << tok << std::endl;
        return false;
      }

      while (true) {
        tok = scanner->next();
        if (tok.is<Punctor>(CloseBracket))
          break;

        if (tok.type() != tName) {
          LOG(ERROR) << core::feedback[FN_DEF_EXPECTED_IDENTIFIER] << tok << std::endl;
          return false;
        }

        implements.insert(tok.as_string());

        tok = scanner->peek();
        if (tok.is<Punctor>(Comma)) {
          scanner->next();
        }
      }

      tok = scanner->peek();
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
    LOG(ERROR) << core::feedback[FN_EXPECTED_OPEN_BRACE] << tok << std::endl;
    return false;
  }
}