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

#include "LibMacro.h"
#include "parser/Parse.h"
#include <quixcc/core/Logger.h>

using namespace libquixcc;
using namespace qparse;
using namespace qparse::parser;

static bool parse_enum_field(quixcc_cc_job_t &job, libquixcc::Scanner *scanner,
                             EnumDefItems &fields) {
  Token tok = scanner->next();
  if (!tok.is(tName)) {
    LOG(ERROR) << core::feedback[ENUM_FIELD_EXPECTED_IDENTIFIER] << tok << std::endl;
    return false;
  }

  EnumItem item;

  item.first = tok.as_string();

  tok = scanner->peek();
  if (tok.is<Operator>(OpAssign)) {
    scanner->next();
    Expr *expr = nullptr;
    if (!parse_expr(job, scanner, {Token(tPunc, Comma)}, &expr)) {
      LOG(ERROR) << core::feedback[ENUM_FIELD_EXPECTED_CONST_EXPR] << item.first << tok
                 << std::endl;
      return false;
    }

    item.second = ConstExpr::get(expr);

    tok = scanner->peek();
  }

  fields.push_back(item);

  if (tok.is<Punctor>(Comma)) {
    scanner->next();
    return true;
  }

  if (!tok.is<Punctor>(CloseBrace)) {
    LOG(ERROR) << core::feedback[ENUM_FIELD_EXPECTED_SEMICOLON] << tok << std::endl;
    return false;
  }

  return true;
}

bool qparse::parser::parse_enum(quixcc_cc_job_t &job, libquixcc::Scanner *scanner,
                                         Stmt **node) {
  Token tok = scanner->next();
  if (!tok.is(tName)) {
    LOG(ERROR) << core::feedback[ENUM_EXPECTED_IDENTIFIER] << tok << std::endl;
    return false;
  }

  std::string name = tok.as_string();

  tok = scanner->peek();
  Type *type = nullptr;
  if (tok.is<Punctor>(Colon)) {
    scanner->next();
    if (!parse_type(job, scanner, &type)) return false;
  }

  tok = scanner->next();
  if (!tok.is<Punctor>(OpenBrace)) {
    LOG(ERROR) << core::feedback[ENUM_EXPECTED_LEFT_BRACE] << tok << std::endl;
    return false;
  }

  EnumDefItems fields;

  while (true) {
    tok = scanner->peek();
    if (tok.is<Punctor>(CloseBrace)) {
      scanner->next();
      break;
    }

    if (!parse_enum_field(job, scanner, fields)) {
      return false;
    }
  }

  *node = EnumDef::get(name, static_cast<EnumTy *>(type), fields);
  return true;
}