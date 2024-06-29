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

static bool parse_union_field(quixcc_job_t &job, libquixcc::Scanner *scanner,
                              std::shared_ptr<UnionFieldNode> &node) {
  Token tok = scanner->next();
  if (tok.type != TT::Identifier) {
    LOG(ERROR) << feedback[UNION_FIELD_MISSING_IDENTIFIER] << tok << std::endl;
    return false;
  }

  node = std::make_shared<UnionFieldNode>();

  node->m_name = tok.as<std::string>();

  tok = scanner->next();
  if (!tok.is<Punctor>(Punctor::Colon)) {
    LOG(ERROR) << feedback[UNION_FIELD_MISSING_COLON] << tok << std::endl;
    return false;
  }

  std::shared_ptr<TypeNode> type;
  if (!parse_type(job, scanner, type)) {
    LOG(ERROR) << feedback[UNION_FIELD_TYPE_ERR] << node->m_name << tok
               << std::endl;
    return false;
  }

  node->m_type = type;

  tok = scanner->next();
  if (!tok.is<Punctor>(Punctor::Comma)) {
    LOG(ERROR) << feedback[UNION_FIELD_MISSING_PUNCTOR] << tok << std::endl;
    return false;
  }

  return true;
}

bool libquixcc::parse_union(quixcc_job_t &job, libquixcc::Scanner *scanner,
                            std::shared_ptr<libquixcc::StmtNode> &node) {
  Token tok = scanner->next();
  if (tok.type != TT::Identifier) {
    LOG(ERROR) << feedback[UNION_DECL_MISSING_IDENTIFIER] << tok << std::endl;
    return false;
  }

  std::string name = tok.as<std::string>();

  tok = scanner->next();
  if (!tok.is<Punctor>(Punctor::OpenBrace)) {
    LOG(ERROR) << feedback[UNION_DEF_EXPECTED_OPEN_BRACE] << tok << std::endl;
    return false;
  }

  std::vector<std::shared_ptr<UnionFieldNode>> fields;

  while (true) {
    tok = scanner->peek();
    if (tok.is<Punctor>(Punctor::CloseBrace)) {
      scanner->next();
      break;
    }

    std::shared_ptr<UnionFieldNode> field;
    if (!parse_union_field(job, scanner, field)) return false;
    fields.push_back(field);
  }

  auto sdef = std::make_shared<UnionDefNode>();
  sdef->m_name = name;
  sdef->m_fields = fields;
  node = sdef;
  return true;
}