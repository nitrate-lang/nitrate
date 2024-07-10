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

static bool parse_enum_field(quixcc_job_t &job, libquixcc::Scanner *scanner,
                             std::shared_ptr<EnumFieldNode> &node) {
  // <name> [ = <value> ] [,]

  Token tok = scanner->next();
  if (tok.type() != tName) {
    LOG(ERROR) << core::feedback[ENUM_FIELD_EXPECTED_IDENTIFIER] << tok
               << std::endl;
    return false;
  }

  node = std::make_shared<EnumFieldNode>();

  node->m_name = tok.as<std::string>();

  tok = scanner->peek();
  if (tok.is<Operator>(Operator::Assign)) {
    scanner->next();
    if (!parse_const_expr(job, scanner, Token(tPunc, Comma),
                          node->m_value)) {
      LOG(ERROR) << core::feedback[ENUM_FIELD_EXPECTED_CONST_EXPR]
                 << node->m_name << tok << std::endl;
      return false;
    }

    tok = scanner->peek();
  }

  if (tok.is<Punctor>(Comma)) {
    scanner->next();
    return true;
  }

  if (!tok.is<Punctor>(CloseBrace)) {
    LOG(ERROR) << core::feedback[ENUM_FIELD_EXPECTED_SEMICOLON] << tok
               << std::endl;
    return false;
  }

  return true;
}

bool libquixcc::parse_enum(quixcc_job_t &job, libquixcc::Scanner *scanner,
                           std::shared_ptr<libquixcc::StmtNode> &node) {
  Token tok = scanner->next();
  if (tok.type() != tName) {
    LOG(ERROR) << core::feedback[ENUM_EXPECTED_IDENTIFIER] << tok << std::endl;
    return false;
  }

  std::string name = tok.as<std::string>();

  tok = scanner->peek();
  std::shared_ptr<TypeNode> type = nullptr;
  if (tok.is<Punctor>(Colon)) {
    scanner->next();
    if (!parse_type(job, scanner, type)) return false;
  }

  tok = scanner->next();
  if (!tok.is<Punctor>(OpenBrace)) {
    LOG(ERROR) << core::feedback[ENUM_EXPECTED_LEFT_BRACE] << tok << std::endl;
    return false;
  }

  std::vector<std::shared_ptr<EnumFieldNode>> fields;

  while (true) {
    tok = scanner->peek();
    if (tok.is<Punctor>(CloseBrace)) {
      scanner->next();
      break;
    }

    std::shared_ptr<EnumFieldNode> field;
    if (!parse_enum_field(job, scanner, field)) return false;
    fields.push_back(field);
  }

  node = std::make_shared<EnumDefNode>(
      std::make_shared<EnumTypeNode>(name, type), fields);
  return true;
}