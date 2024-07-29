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

#define __QPARSE_IMPL__
#define __QUIX_IMPL__

//
#include <Impl.h>
#include <parser/Parse.h>
#include <quix-core/Error.h>
#include <quix-parser/Node.h>
#include <quix-parser/Parser.h>

#include <cstring>

#include "LibMacro.h"

using namespace qparse::parser;

bool qparse::parser::parse(qparse_t &job, qlex_t *rd, Block **group, bool expect_braces,
                           bool single_stmt) {
  qlex_tok_t tok;

  if (expect_braces) {
    tok = qlex_next(rd);
    if (!tok.is<qPuncLCur>()) {
      /// TODO: Write the ERROR message
      return false;
    }
  }

  *group = Block::get();

  while ((tok = qlex_peek(rd)).ty != qEofF) {
    if (single_stmt && (*group)->get_items().size() > 0) break;

    if (expect_braces) {
      if (tok.is<qPuncRCur>()) {
        qlex_next(rd);
        return true;
      }
    }

    if (tok.is<qPuncSemi>()) /* Skip excessive semicolons */
    {
      qlex_next(rd);
      continue;
    }

    if (!tok.is(qKeyW)) {
      Expr *expr = nullptr;
      if (!parse_expr(job, rd, {qlex_tok_t(qPunc, qPuncSemi)}, &expr)) {
        /// TODO: Write the ERROR message
        return false;
      }

      if (!expr) {
        /// TODO: Write the ERROR message
        return false;
      }

      tok = qlex_next(rd);
      if (!tok.is<qPuncSemi>()) {
        /// TODO: Write the ERROR message
        return false;
      }

      (*group)->add_item(ExprStmt::get(expr));
      continue;
    }

    qlex_next(rd);

    Stmt *node = nullptr;

    switch (tok.as<qlex_key_t>()) {
      case qKVar: {
        StmtListItems decls;
        if (!parse_var(job, rd, decls)) return false;
        for (auto &decl : decls) (*group)->add_item(decl);
        break;
      }
      case qKLet: {
        StmtListItems decls;
        if (!parse_let(job, rd, decls)) return false;
        for (auto &decl : decls) (*group)->add_item(decl);
        break;
      }
      case qKConst: {
        StmtListItems decls;
        if (!parse_const(job, rd, decls)) return false;
        for (auto &decl : decls) (*group)->add_item(decl);
        break;
      }
      case qKEnum:
        if (!parse_enum(job, rd, &node)) return false;
        break;
      case qKStruct:
        if (!parse_struct(job, rd, &node)) return false;
        break;
      case qKRegion:
        if (!parse_region(job, rd, &node)) return false;
        break;
      case qKGroup:
        if (!parse_group(job, rd, &node)) return false;
        break;
      case qKUnion:
        if (!parse_union(job, rd, &node)) return false;
        break;
      case qKType:
        if (!parse_typedef(job, rd, &node)) return false;
        break;
      case qKSubsystem:
        if (!parse_subsystem(job, rd, &node)) return false;
        break;
      case qKFn:
        if (!parse_function(job, rd, &node)) return false;
        break;
      case qKPub:
      case qKImport:  // they both declare external functions
        if (!parse_pub(job, rd, &node)) return false;
        break;
      case qKSec:
        break;
      case qKReturn:
        if (!parse_return(job, rd, &node)) return false;
        break;
      case qKRetif:
        if (!parse_retif(job, rd, &node)) return false;
        break;
      case qKRetz:
        if (!parse_retz(job, rd, &node)) return false;
        break;
      case qKRetv:
        if (!parse_retv(job, rd, &node)) return false;
        break;
      case qKBreak:
        node = BreakStmt::get();
        break;
      case qKContinue:
        node = ContinueStmt::get();
        break;
      case qKIf:
        if (!parse_if(job, rd, &node)) return false;
        break;
      case qKWhile:
        if (!parse_while(job, rd, &node)) return false;
        break;
      case qKFor:
        if (!parse_for(job, rd, &node)) return false;
        break;
      case qKForm:
        if (!parse_form(job, rd, &node)) return false;
        break;
      case qKForeach:
        if (!parse_foreach(job, rd, &node)) return false;
        break;
      case qKSwitch:
        if (!parse_switch(job, rd, &node)) return false;
        break;
      case qK__Asm__:
        if (!parse_inline_asm(job, rd, &node)) return false;
        break;
      case qKUnsafe: {
        Block *block = nullptr;
        qlex_tok_t tok = qlex_peek(rd);
        if (tok.is<qPuncLCur>()) {
          if (!parse(job, rd, &block)) return false;
        } else {
          if (!parse(job, rd, &block, false, true)) return false;
        }

        block->set_unsafe(true);
        (*group)->add_item(block);
        break;
      }
      case qKSafe: {
        Block *block = nullptr;
        qlex_tok_t tok = qlex_peek(rd);
        if (tok.is<qPuncLCur>()) {
          if (!parse(job, rd, &block)) return false;
        } else {
          if (!parse(job, rd, &block, false, true)) return false;
        }
        block->set_unsafe(false);
        (*group)->add_item(block);
        break;
      }
      default:
        /// TODO: Write the ERROR message
        return false;
    }

    if (node) (*group)->add_item(node);
  }

  if (expect_braces) {
    /// TODO: Write the ERROR message
    return false;
  }

  return true;
}

LIB_EXPORT qparse_t *qparse_new(qlex_t *lexer, qparse_conf_t *conf) {
  if (!lexer || !conf) {
    return nullptr;
  }

  qparse_t *parser = new qparse_t();

  parser->impl = new qparse_impl_t();
  parser->lexer = lexer;
  parser->conf = conf;

  qlex_set_flags(lexer, qlex_get_flags(lexer) | QLEX_NO_COMMENTS);

  return parser;
}

LIB_EXPORT void qparse_free(qparse_t *parser) {
  if (!parser) {
    return;
  }

  delete parser->impl;

  parser->impl = nullptr;
  parser->lexer = nullptr;
  parser->conf = nullptr;

  delete parser;
}

LIB_EXPORT bool qparse_do(qparse_t *parser, qcore_arena_t *arena, qparse_node_t **out) {
  if (!parser || !arena || !out) {
    return false;
  }

  qparse::qparse_ast_arena.swap(*arena);

  qparse::diag::install_reference(&parser->impl->diag);
  bool status = qparse::parser::parse(*parser, parser->lexer, (qparse::Block **)out, false, false);
  qparse::diag::install_reference(nullptr);

  qparse::qparse_ast_arena.swap(*arena);

  return status;
}

LIB_EXPORT bool qparse_check(qparse_t *parser, const qparse_node_t *base) {
  if (!parser || !base) {
    return false;
  }

  if (!parser->impl) {
    qcore_panic("qpase_check: invariant violation: parser->impl is NULL");
  }

  /* Safety is overrated */
  return static_cast<const qparse::Node *>(base)->verify();
}

LIB_EXPORT void qparse_dumps(qparse_t *parser, bool no_ansi, qparse_dump_cb cb, uintptr_t data) {
  if (!parser || !cb) {
    return;
  }

  qparse::diag::FormatOptions opts;

  opts.use_ansi_modes = !no_ansi;
  opts.use_color = !no_ansi;
  opts.use_unicode = !no_ansi;

  auto adapter = [&](const char *msg) { cb(msg, std::strlen(msg), data); };

  parser->impl->diag.render(adapter, opts);
}
