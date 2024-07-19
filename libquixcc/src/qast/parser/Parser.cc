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
using namespace libquixcc::qast::parser;

bool libquixcc::qast::parser::parse(quixcc_cc_job_t &job, Scanner *scanner, Block **group,
                                    bool expect_braces, bool single_stmt) {
  Token tok;

  if (expect_braces) {
    tok = scanner->next();
    if (!tok.is<Punctor>(OpenBrace)) {
      LOG(ERROR) << core::feedback[PARSER_EXPECTED_LEFT_BRACE] << tok.serialize() << tok
                 << std::endl;
      return false;
    }
  }

  *group = Block::get();

  while ((tok = scanner->peek()).type() != tEofF) {
    if (single_stmt && (*group)->get_items().size() > 0)
      break;

    if (expect_braces) {
      if (tok.is<Punctor>(CloseBrace)) {
        scanner->next();
        return true;
      }
    }

    if (tok.is<Punctor>(Semicolon)) /* Skip excessive semicolons */
    {
      scanner->next();
      continue;
    }

    if (tok.type() != tKeyW) {
      Expr *expr = nullptr;
      if (!parse_expr(job, scanner, {Token(tPunc, Semicolon)}, &expr)) {
        LOG(ERROR) << "Error parsing expression in block statement." << tok << std::endl;
        return false;
      }

      if (!expr)
        LOG(ERROR) << "Null expressions are illegal in ExprStmtNode." << tok << std::endl;

      tok = scanner->next();
      if (!tok.is<Punctor>(Semicolon)) {
        LOG(ERROR) << core::feedback[PARSER_EXPECTED_SEMICOLON] << tok.serialize() << tok
                   << std::endl;
        return false;
      }

      (*group)->add_item(ExprStmt::get(expr));
      continue;
    }

    scanner->next();

    Stmt *node = nullptr;

    switch (tok.as<Keyword>()) {
    case Keyword::Var: {
      StmtListItems decls;
      if (!parse_var(job, scanner, decls))
        return false;
      for (auto &decl : decls)
        (*group)->add_item(decl);
      break;
    }
    case Keyword::Let: {
      StmtListItems decls;
      if (!parse_let(job, scanner, decls))
        return false;
      for (auto &decl : decls)
        (*group)->add_item(decl);
      break;
    }
    case Keyword::Const: {
      StmtListItems decls;
      if (!parse_const(job, scanner, decls))
        return false;
      for (auto &decl : decls)
        (*group)->add_item(decl);
      break;
    }
    case Keyword::Enum:
      if (!parse_enum(job, scanner, &node))
        return false;
      break;
    case Keyword::Struct:
      if (!parse_struct(job, scanner, &node))
        return false;
      break;
    case Keyword::Region:
      if (!parse_region(job, scanner, &node))
        return false;
      break;
    case Keyword::Group:
      if (!parse_group(job, scanner, &node))
        return false;
      break;
    case Keyword::Union:
      if (!parse_union(job, scanner, &node))
        return false;
      break;
    case Keyword::Type:
      if (!parse_typedef(job, scanner, &node))
        return false;
      break;
    case Keyword::Subsystem:
      if (!parse_subsystem(job, scanner, &node))
        return false;
      break;
    case Keyword::Fn:
      if (!parse_function(job, scanner, &node))
        return false;
      break;
    case Keyword::Pub:
    case Keyword::Import: // they both declare external functions
      if (!parse_pub(job, scanner, &node))
        return false;
      break;
    case Keyword::Sec:
      break;
    case Keyword::Return:
      if (!parse_return(job, scanner, &node))
        return false;
      break;
    case Keyword::Retif:
      if (!parse_retif(job, scanner, &node))
        return false;
      break;
    case Keyword::Retz:
      if (!parse_retz(job, scanner, &node))
        return false;
      break;
    case Keyword::Retv:
      if (!parse_retv(job, scanner, &node))
        return false;
      break;
    case Keyword::Break:
      node = BreakStmt::get();
      break;
    case Keyword::Continue:
      node = ContinueStmt::get();
      break;
    case Keyword::If:
      if (!parse_if(job, scanner, &node))
        return false;
      break;
    case Keyword::While:
      if (!parse_while(job, scanner, &node))
        return false;
      break;
    case Keyword::For:
      if (!parse_for(job, scanner, &node))
        return false;
      break;
    case Keyword::Form:
      if (!parse_form(job, scanner, &node))
        return false;
      break;
    case Keyword::Foreach:
      if (!parse_foreach(job, scanner, &node))
        return false;
      break;
    case Keyword::Switch:
      if (!parse_switch(job, scanner, &node))
        return false;
      break;
    case Keyword::__Asm__:
      if (!parse_inline_asm(job, scanner, &node))
        return false;
      break;
    case Keyword::Unsafe: {
      Block *block = nullptr;
      Token tok = scanner->peek();
      if (tok.is<Punctor>(OpenBrace)) {
        if (!parse(job, scanner, &block))
          return false;
      } else {
        if (!parse(job, scanner, &block, false, true))
          return false;
      }

      block->set_unsafe(true);
      (*group)->add_item(block);
      break;
    }
    case Keyword::Safe: {
      Block *block = nullptr;
      Token tok = scanner->peek();
      if (tok.is<Punctor>(OpenBrace)) {
        if (!parse(job, scanner, &block))
          return false;
      } else {
        if (!parse(job, scanner, &block, false, true))
          return false;
      }
      block->set_unsafe(false);
      (*group)->add_item(block);
      break;
    }
    default:
      LOG(ERROR) << core::feedback[PARSER_ILLEGAL_KEYWORD] << tok.serialize() << tok << std::endl;
      return false;
    }

    if (node)
      (*group)->add_item(node);
  }

  if (expect_braces) {
    LOG(ERROR) << core::feedback[PARSER_EXPECTED_RIGHT_BRACE] << tok.serialize() << tok
               << std::endl;
    return false;
  }

  return true;
}