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

static bool parse_region_field(quixcc_job_t &job, libquixcc::Scanner *scanner,
                               std::shared_ptr<RegionFieldNode> &node) {
  Token tok = scanner->next();
  if (tok.type != TT::Identifier) {
    LOG(ERROR) << feedback[REGION_FIELD_MISSING_IDENTIFIER] << tok << std::endl;
    return false;
  }

  auto name = tok.as<std::string>();

  tok = scanner->next();
  if (!tok.is<Punctor>(Punctor::Colon)) {
    LOG(ERROR) << feedback[REGION_FIELD_MISSING_COLON] << tok << std::endl;
    return false;
  }

  TypeNode *type;
  if (!parse_type(job, scanner, &type)) {
    LOG(ERROR) << feedback[REGION_FIELD_TYPE_ERR] << name << tok << std::endl;
    return false;
  }

  std::shared_ptr<ConstExprNode> value;

  tok = scanner->next();
  if (tok.is<Punctor>(Punctor::Comma)) {
    node = std::make_shared<RegionFieldNode>(name, type);
    return true;
  } else if (tok.is<Operator>(Operator::Assign)) {
    if (!parse_const_expr(job, scanner, Token(TT::Punctor, Punctor::Comma),
                          value)) {
      LOG(ERROR) << feedback[REGION_FIELD_INIT_ERR] << name << tok << std::endl;
      return false;
    }
  } else {
    LOG(ERROR) << feedback[REGION_FIELD_MISSING_PUNCTOR] << name << tok
               << std::endl;
    return false;
  }

  tok = scanner->next();
  if (!tok.is<Punctor>(Punctor::Comma)) {
    LOG(ERROR) << feedback[REGION_FIELD_MISSING_PUNCTOR] << name << tok
               << std::endl;
    return false;
  }

  node = std::make_shared<RegionFieldNode>(name, type, value);

  return true;
}

bool libquixcc::parse_region(quixcc_job_t &job, libquixcc::Scanner *scanner,
                             std::shared_ptr<libquixcc::StmtNode> &node) {
  Token tok = scanner->next();
  if (tok.type != TT::Identifier) {
    LOG(ERROR) << feedback[REGION_DECL_MISSING_IDENTIFIER] << tok << std::endl;
    return false;
  }

  std::string name = tok.as<std::string>();

  tok = scanner->next();
  if (!tok.is<Punctor>(Punctor::OpenBrace)) {
    LOG(ERROR) << feedback[REGION_DEF_EXPECTED_OPEN_BRACE] << tok << std::endl;
    return false;
  }

  std::vector<std::shared_ptr<RegionFieldNode>> fields;
  std::vector<std::shared_ptr<StmtNode>> methods;
  std::vector<std::shared_ptr<StmtNode>> static_methods;

  while (true) {
    tok = scanner->peek();
    if (tok.is<Punctor>(Punctor::CloseBrace)) {
      scanner->next();
      break;
    }

    if (tok.is<Keyword>(Keyword::Fn)) {
      scanner->next();

      std::shared_ptr<StmtNode> method;
      if (!parse_function(job, scanner, method)) return false;

      auto fn_this = std::make_shared<FunctionParamNode>(
          "this", PointerTypeNode::create(UserTypeNode::create(name)), nullptr);

      if (method->is<FunctionDeclNode>()) {
        auto fdecl = std::static_pointer_cast<FunctionDeclNode>(method);
        fdecl->m_params.insert(fdecl->m_params.begin(), fn_this);
      } else {
        auto fdef = std::static_pointer_cast<FunctionDefNode>(method);
        fdef->m_decl->m_params.insert(fdef->m_decl->m_params.begin(), fn_this);
      }

      methods.push_back(method);
    } else if (tok.is<Keyword>(Keyword::Static)) {
      scanner->next();
      tok = scanner->next();
      if (!tok.is<Keyword>(Keyword::Fn)) {
        LOG(ERROR) << feedback[REGION_DEF_EXPECTED_FN] << tok << std::endl;
        return false;
      }

      std::shared_ptr<StmtNode> method;
      if (!parse_function(job, scanner, method)) return false;

      static_methods.push_back(method);
    } else {
      std::shared_ptr<RegionFieldNode> field;
      if (!parse_region_field(job, scanner, field)) return false;
      fields.push_back(field);
    }
  }

  auto sdef = std::make_shared<RegionDefNode>();
  sdef->m_name = name;
  sdef->m_fields = fields;
  sdef->m_methods = methods;
  sdef->m_static_methods = static_methods;
  node = sdef;
  return true;
}