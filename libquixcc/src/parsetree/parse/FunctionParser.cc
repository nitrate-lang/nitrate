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

#include <LibMacro.h>
#include <core/Logger.h>
#include <parsetree/Parser.h>

using namespace libquixcc;

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

static bool fn_get_property(quixcc_job_t &job, libquixcc::Scanner *scanner,
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

static bool parse_fn_parameter(quixcc_job_t &job, libquixcc::Scanner *scanner,
                               std::shared_ptr<FunctionParamNode> &param) {
  /*
   <name> : <type> [?] [= <value>]
  */

  auto tok = scanner->next();

  std::string name;
  if (tok.type != TT::Identifier) {
    LOG(ERROR) << feedback[FN_PARAM_EXPECTED_IDENTIFIER] << tok << std::endl;
    return false;
  }

  name = tok.as<std::string>();
  tok = scanner->next();
  if (!tok.is<Punctor>(Punctor::Colon)) {
    LOG(ERROR) << feedback[FN_PARAM_EXPECTED_COLON] << tok << std::endl;
    return false;
  }

  TypeNode *type;

  if (!parse_type(job, scanner, &type)) {
    LOG(ERROR) << feedback[FN_PARAM_TYPE_ERR] << tok << std::endl;
    return false;
  }

  tok = scanner->peek();

  if (tok.is<Operator>(Operator::Assign)) {
    scanner->next();

    std::shared_ptr<ExprNode> value;
    if (!parse_expr(job, scanner,
                    {Token(TT::Punctor, Punctor::Comma),
                     Token(TT::Punctor, Punctor::CloseParen)},
                    value)) {
      LOG(ERROR) << feedback[FN_PARAM_INIT_ERR] << tok << std::endl;
      return false;
    }

    param = std::make_shared<FunctionParamNode>(name, type, value);
  } else {
    param = std::make_shared<FunctionParamNode>(name, type, nullptr);
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

static FunctionProperties read_function_properties(
    quixcc_job_t &job, libquixcc::Scanner *scanner) {
  GetPropState state;

  while (fn_get_property(job, scanner, state));

  if (!(state.noexcept_ctr || state.foreign_ctr || state.impure_ctr ||
        state.tsafe_ctr || state.pure_ctr || state.quasipure_ctr ||
        state.retropure_ctr))
    return FunctionProperties();

  LOG(WARN) << "Function designators are not supported yet. Ignoring..."
            << std::endl;

  FunctionProperties props;

  /// TODO: validate properties

  /// TODO: dedude missing properties: i.e. purity implies thread-safety

  return props;
}

bool libquixcc::parse_function(quixcc_job_t &job, libquixcc::Scanner *scanner,
                               std::shared_ptr<libquixcc::StmtNode> &node) {
  // fn [noexcept] [foreign] [pure] [tsafe] <name> ( [param]... ) [: <type>]; or
  // {}
  auto fndecl = std::make_shared<FunctionDeclNode>();

  auto prop = read_function_properties(job, scanner);

  (void)prop;

  Token tok = scanner->next();

  if (tok.type == TT::Identifier) {
    fndecl->m_name = tok.as<std::string>();
    tok = scanner->next();
  }

  if (!tok.is<Punctor>(Punctor::OpenParen)) {
    LOG(ERROR) << feedback[FN_EXPECTED_OPEN_PAREN] << tok << std::endl;
    return false;
  }
  bool is_variadic = false;

  while (1) {
    tok = scanner->peek();
    if (tok.is<Punctor>(Punctor::CloseParen)) {
      scanner->next();
      break;
    }

    if (tok.is<Punctor>(Punctor::Dot)) {
      scanner->next();
      tok = scanner->next();
      if (!tok.is<Punctor>(Punctor::Dot)) {
        LOG(ERROR) << feedback[FN_EXPECTED_VARARG] << tok << std::endl;
        return false;
      }
      tok = scanner->next();
      if (!tok.is<Punctor>(Punctor::Dot)) {
        LOG(ERROR) << feedback[FN_EXPECTED_VARARG] << tok << std::endl;
        return false;
      }

      is_variadic = true;

      tok = scanner->next();
      if (!tok.is<Punctor>(Punctor::CloseParen)) {
        LOG(ERROR) << feedback[FN_EXPECTED_VARARG] << tok << std::endl;
        return false;
      }

      break;
    }

    std::shared_ptr<FunctionParamNode> param;
    if (!parse_fn_parameter(job, scanner, param)) {
      LOG(ERROR) << feedback[FN_PARAM_PARSE_ERROR] << tok << std::endl;
      return false;
    }
    fndecl->m_params.push_back(param);

    tok = scanner->peek();
    if (tok.is<Punctor>(Punctor::Comma)) {
      scanner->next();
      continue;
    }
  }

  std::vector<std::pair<std::string, TypeNode *>> params;
  for (auto &param : fndecl->m_params)
    params.push_back({param->m_name, param->m_type});

  tok = scanner->peek();

  if (tok.is<Punctor>(Punctor::Semicolon)) {
    /// TODO: Implement function properties
    fndecl->m_type =
        FunctionTypeNode::create(VoidTypeNode::create(), params, is_variadic,
                                 false, false, false, false);

    scanner->next();
    node = fndecl;
    return true;
  }

  if (tok.is<Punctor>(Punctor::Colon)) {
    scanner->next();
    TypeNode *type;

    if (!parse_type(job, scanner, &type)) return false;

    /// TODO: Implement function properties
    fndecl->m_type = FunctionTypeNode::create(type, params, is_variadic, false,
                                              false, false, false);

    tok = scanner->peek();
    if (tok.is<Punctor>(Punctor::Semicolon)) {
      scanner->next();
      node = fndecl;
      return true;
    }
  }

  if (tok.is<Operator>(Operator::Arrow)) {
    scanner->next();
    /// TODO: Implement arrow function
    // throw std::runtime_error("Arrow functions are not yet implemented.");

    auto fnbody = std::make_shared<BlockNode>();

    if (!parse(job, scanner, fnbody, false, true)) return false;

    /// TODO: Implement function properties
    if (!fndecl->m_type)
      fndecl->m_type =
          FunctionTypeNode::create(VoidTypeNode::create(), params, is_variadic,
                                   false, false, false, false);

    node = std::make_shared<FunctionDefNode>(fndecl, fnbody);
    return true;
  } else if (tok.is<Punctor>(Punctor::OpenBrace)) {
    auto fnbody = std::make_shared<BlockNode>();

    if (!parse(job, scanner, fnbody)) return false;

    /// TODO: Implement function properties
    if (!fndecl->m_type)
      fndecl->m_type =
          FunctionTypeNode::create(VoidTypeNode::create(), params, is_variadic,
                                   false, false, false, false);

    node = std::make_shared<FunctionDefNode>(fndecl, fnbody);
    return true;
  } else {
    LOG(ERROR) << feedback[FN_EXPECTED_OPEN_BRACE] << tok << std::endl;
    return false;
  }
}