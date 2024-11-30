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

#include <nitrate-core/Error.h>
#include <nitrate-core/Macro.h>
#include <nitrate-parser/Node.h>
#include <nitrate-parser/Parser.h>

#include <atomic>
#include <core/ParserStruct.hh>
#include <cstring>
#include <decent/Recurse.hh>
#include <nitrate-core/Classes.hh>

using namespace qparse;

Stmt *qparse::recurse(qparse_t &S, qlex_t &rd, bool expect_braces,
                      bool single_stmt) {
  qlex_tok_t tok;

  Block *block = Block::get();

  if (expect_braces) {
    tok = next();
    if (!tok.is<qPuncLCur>()) {
      syntax(tok, "Expected '{'");
    }
  }

  while ((tok = peek()).ty != qEofF) {
    if (single_stmt && block->get_items().size() > 0) {
      break;
    }

    if (expect_braces) {
      if (tok.is<qPuncRCur>()) {
        next();
        return block;
      }
    }

    if (tok.is<qPuncSemi>()) /* Skip excessive semicolons */
    {
      next();
      continue;
    }

    if (!tok.is(qKeyW)) {
      if (tok.is<qPuncRBrk>() || tok.is<qPuncRCur>() || tok.is<qPuncRPar>()) {
        syntax(tok, "Unexpected closing brace");
        return mock_stmt(QAST_NODE_BLOCK);
      }

      Expr *expr = nullptr;
      if (!recurse_expr(S, rd, {qlex_tok_t(qPunc, qPuncSemi)}, &expr)) {
        syntax(tok, "Expected expression");
        return mock_stmt(QAST_NODE_BLOCK);
      }

      if (!expr) {
        syntax(tok, "Expected valid expression");
        return mock_stmt(QAST_NODE_BLOCK);
      }

      tok = next();
      if (!tok.is<qPuncSemi>()) {
        syntax(tok, "Expected ';'");
      }

      ExprStmt *stmt = ExprStmt::get(expr);
      stmt->set_start_pos(std::get<0>(expr->get_pos()));
      stmt->set_end_pos(tok.end);

      block->get_items().push_back(stmt);
      continue;
    }

    next();

    Stmt *node = nullptr;

    uint32_t loc_start = tok.start;
    switch (tok.as<qlex_key_t>()) {
      case qKVar: {
        std::vector<Stmt *> items;
        if (!recurse_var(S, rd, items)) {
          return mock_stmt(QAST_NODE_BLOCK);
        }
        for (auto &decl : items) {
          block->get_items().push_back(decl);
        }
        break;
      }

      case qKLet: {
        std::vector<Stmt *> items;
        if (!recurse_let(S, rd, items)) {
          return mock_stmt(QAST_NODE_BLOCK);
        }
        for (auto &decl : items) {
          block->get_items().push_back(decl);
        }
        break;
      }

      case qKConst: {
        std::vector<Stmt *> items;
        if (!recurse_const(S, rd, items)) {
          return mock_stmt(QAST_NODE_BLOCK);
        }
        for (auto &decl : items) {
          block->get_items().push_back(decl);
        }
        break;
      }

      case qKEnum: {
        if (!recurse_enum(S, rd, &node)) {
          return mock_stmt(QAST_NODE_BLOCK);
        }
        break;
      }

      case qKStruct: {
        if (!recurse_struct(S, rd, &node) || !node) {
          return mock_stmt(QAST_NODE_BLOCK);
        }

        node->as<StructDef>()->set_composite_type(CompositeType::Struct);
        break;
      }

      case qKRegion: {
        if (!recurse_struct(S, rd, &node) || !node) {
          return mock_stmt(QAST_NODE_BLOCK);
        }

        node->as<StructDef>()->set_composite_type(CompositeType::Region);
        break;
      }

      case qKGroup: {
        if (!recurse_struct(S, rd, &node) || !node) {
          return mock_stmt(QAST_NODE_BLOCK);
        }

        node->as<StructDef>()->set_composite_type(CompositeType::Group);
        break;
      }

      case qKClass: {
        if (!recurse_struct(S, rd, &node) || !node) {
          return mock_stmt(QAST_NODE_BLOCK);
        }

        node->as<StructDef>()->set_composite_type(CompositeType::Class);
        break;
      }

      case qKUnion: {
        if (!recurse_struct(S, rd, &node) || !node) {
          return mock_stmt(QAST_NODE_BLOCK);
        }

        node->as<StructDef>()->set_composite_type(CompositeType::Union);
        break;
      }

      case qKType: {
        node = recurse_typedef(S, rd);
        break;
      }

      case qKSubsystem: {
        if (!recurse_subsystem(S, rd, &node)) {
          return mock_stmt(QAST_NODE_BLOCK);
        }
        break;
      }

      case qKFn: {
        if (!recurse_function(S, rd, &node)) {
          return mock_stmt(QAST_NODE_BLOCK);
        }
        break;
      }

      case qKPub:
      case qKImport: {  // they both declare external functions
        if (!recurse_pub(S, rd, &node)) {
          return mock_stmt(QAST_NODE_BLOCK);
        }
        break;
      }

      case qKSec: {
        if (!recurse_sec(S, rd, &node)) {
          return mock_stmt(QAST_NODE_BLOCK);
        }
        break;
      }

      case qKPro: {
        if (!recurse_pro(S, rd, &node)) {
          return mock_stmt(QAST_NODE_BLOCK);
        }
        break;
      }

      case qKReturn: {
        node = recurse_return(S, rd);
        break;
      }

      case qKRetif: {
        node = recurse_retif(S, rd);
        break;
      }

      case qKBreak: {
        node = BreakStmt::get();
        break;
      }

      case qKContinue: {
        node = ContinueStmt::get();
        break;
      }

      case qKIf: {
        node = recurse_if(S, rd);
        break;
      }

      case qKWhile: {
        node = recurse_while(S, rd);
        break;
      }

      case qKFor: {
        node = recurse_for(S, rd);
        break;
      }

      case qKForeach: {
        if (!recurse_foreach(S, rd, &node)) {
          return mock_stmt(QAST_NODE_BLOCK);
        }
        break;
      }

      case qKSwitch: {
        if (!recurse_switch(S, rd, &node)) {
          return mock_stmt(QAST_NODE_BLOCK);
        }
        break;
      }

      case qK__Asm__: {
        node = recurse_inline_asm(S, rd);
        break;
      }

      case qKTrue: {
        node = ExprStmt::get(ConstBool::get(true));
        break;
      }

      case qKFalse: {
        node = ExprStmt::get(ConstBool::get(false));
        break;
      }

      case qKUnsafe: {
        Stmt *block = nullptr;
        tok = peek();
        if (tok.is<qPuncLCur>()) {
          block = recurse(S, rd);
        } else {
          block = recurse(S, rd, false, true);
        }

        /// FIXME: Set safety
        // block->set_safety(SafetyMode::Unsafe);
        node = block;
        break;
      }

      case qKSafe: {
        tok = peek();
        if (tok.is<qPuncLCur>()) {
          node = recurse(S, rd);
        } else {
          node = recurse(S, rd, false, true);
        }

        /// FIXME: Set safety
        // block->set_safety(SafetyMode::Safe);
        break;
      }

      case qKVolatile: {
        tok = peek();
        if (tok.is<qPuncLCur>()) {
          node = recurse(S, rd);
        } else {
          node = recurse(S, rd, false, true);
        }

        node = VolStmt::get(block);
        break;
      }

      default:
        syntax(tok, "Unexpected keyword");
        break;
    }

    if (node) {
      node->set_start_pos(loc_start);
      /* End position is supplied by producer */
      block->get_items().push_back(node);
    }
  }

  if (expect_braces) {
    syntax(tok, "Expected '}'");
  }

  return block;
}

C_EXPORT qparse_t *qparse_new(qlex_t *lexer, qcore_env_t env) {
  if (!lexer) {
    return nullptr;
  }
  static std::atomic<uint64_t> job_id = 1;  // 0 is reserved for invalid job

  qparse_t *parser = new qparse_t();

  parser->env = env;
  parser->id = job_id++;
  parser->lexer = lexer;
  parser->failed = false;
  parser->diag.set_ctx(parser);

  qlex_set_flags(lexer, qlex_get_flags(lexer) | QLEX_NO_COMMENTS);

  return parser;
}

C_EXPORT void qparse_free(qparse_t *parser) {
  if (!parser) {
    return;
  }

  parser->lexer = nullptr;

  delete parser;
}

static thread_local qparse_t *parser_ctx;

C_EXPORT bool qparse_do(qparse_t *L, qparse_node_t **out) {
  if (!L || !out) {
    return false;
  }
  *out = nullptr;

  /*=============== Swap in their arena  ===============*/
  qparse::qparse_arena.swap(*L->arena.get());

  /*== Install thread-local references to the parser ==*/
  qparse::install_reference(L);

  parser_ctx = L;
  *out = qparse::recurse(*L, *L->lexer, false, false);
  parser_ctx = nullptr;

  /*== Uninstall thread-local references to the parser ==*/
  qparse::install_reference(nullptr);

  /*=============== Swap out their arena ===============*/
  qparse::qparse_arena.swap(*L->arena.get());

  /*==================== Return status ====================*/
  return !L->failed;
}

C_EXPORT bool qparse_and_dump(qparse_t *L, FILE *out, void *x0, void *x1) {
  (void)x0;
  (void)x1;

  qparse_node_t *node = nullptr;

  if (!L || !out) {
    return false;
  }

  if (!qparse_do(L, &node)) {
    return false;
  }

  size_t len = 0;
  char *repr = qparse_repr(node, false, 2, &len);

  fwrite(repr, 1, len, out);

  free(repr);

  return true;
}

C_EXPORT bool qparse_check(qparse_t *parser, const qparse_node_t *base) {
  if (!parser || !base) {
    return false;
  }

  if (parser->failed) {
    return false;
  }

  /* TODO: Implement checks */
  return true;
}

C_EXPORT void qparse_dumps(qparse_t *parser, bool no_ansi, qparse_dump_cb cb,
                           uintptr_t data) {
  if (!parser || !cb) {
    return;
  }

  auto adapter = [&](const char *msg) { cb(msg, std::strlen(msg), data); };

  if (no_ansi) {
    parser->diag.render(adapter, qparse::FormatStyle::ClangPlain);
  } else {
    parser->diag.render(adapter, qparse::FormatStyle::Clang16Color);
  }
}
