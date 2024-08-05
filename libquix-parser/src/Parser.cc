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

#include <Impl.h>
#include <parser/Parse.h>
#include <quix-core/Error.h>
#include <quix-parser/Node.h>
#include <quix-parser/Parser.h>
#include <setjmp.h>
#include <signal.h>

#include <array>
#include <atomic>
#include <cstring>
#include <mutex>
#include <set>

#include "LibMacro.h"

using namespace qparse::parser;
using namespace qparse::diag;

bool qparse::parser::parse(qparse_t &job, qlex_t *rd, Block **group, bool expect_braces,
                           bool single_stmt) {
  try {
    qlex_tok_t tok;

    *group = Block::get();

    if (expect_braces) {
      tok = qlex_next(rd);
      if (!tok.is<qPuncLCur>()) {
        syntax(tok, "Expected '{'");
      }
    }

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
        if (tok.is<qPuncRBrk>() || tok.is<qPuncRCur>() || tok.is<qPuncRPar>()) {
          syntax(tok, "Unexpected closing brace");
          return false;
        }

        Expr *expr = nullptr;
        if (!parse_expr(job, rd, {qlex_tok_t(qPunc, qPuncSemi)}, &expr)) {
          syntax(tok, "Expected expression");
          return false;
        }

        if (!expr) {
          syntax(tok, "Expected valid expression");
        }

        tok = qlex_next(rd);
        if (!tok.is<qPuncSemi>()) {
          syntax(tok, "Expected ';'");
        }

        (*group)->add_item(ExprStmt::get(expr));
        continue;
      }

      qlex_next(rd);

      Stmt *node = nullptr;

      switch (tok.as<qlex_key_t>()) {
        case qKVar: {
          StmtListItems decls;
          if (!parse_var(job, rd, decls)) {
            return false;
          }
          for (auto &decl : decls) (*group)->add_item(decl);
          break;
        }
        case qKLet: {
          StmtListItems decls;
          if (!parse_let(job, rd, decls)) {
            return false;
          }
          for (auto &decl : decls) (*group)->add_item(decl);
          break;
        }
        case qKConst: {
          StmtListItems decls;
          if (!parse_const(job, rd, decls)) {
            return false;
          }
          for (auto &decl : decls) (*group)->add_item(decl);
          break;
        }
        case qKEnum:
          if (!parse_enum(job, rd, &node)) {
            return false;
          }
          break;
        case qKStruct:
          if (!parse_struct(job, rd, &node)) {
            return false;
          }
          break;
        case qKRegion:
          if (!parse_region(job, rd, &node)) {
            return false;
          }
          break;
        case qKGroup:
          if (!parse_group(job, rd, &node)) {
            return false;
          }
          break;
        case qKUnion:
          if (!parse_union(job, rd, &node)) {
            return false;
          }
          break;
        case qKType:
          if (!parse_typedef(job, rd, &node)) {
            return false;
          }
          break;
        case qKSubsystem:
          if (!parse_subsystem(job, rd, &node)) {
            return false;
          }
          break;
        case qKFn:
          if (!parse_function(job, rd, &node)) {
            return false;
          }
          break;
        case qKPub:
        case qKImport:  // they both declare external functions
          if (!parse_pub(job, rd, &node)) {
            return false;
          }
          break;
        case qKSec:
          break;
        case qKReturn:
          if (!parse_return(job, rd, &node)) {
            return false;
          }
          break;
        case qKRetif:
          if (!parse_retif(job, rd, &node)) {
            return false;
          }
          break;
        case qKRetz:
          if (!parse_retz(job, rd, &node)) {
            return false;
          }
          break;
        case qKRetv:
          if (!parse_retv(job, rd, &node)) {
            return false;
          }
          break;
        case qKBreak:
          node = BreakStmt::get();
          break;
        case qKContinue:
          node = ContinueStmt::get();
          break;
        case qKIf:
          if (!parse_if(job, rd, &node)) {
            return false;
          }
          break;
        case qKWhile:
          if (!parse_while(job, rd, &node)) {
            return false;
          }
          break;
        case qKFor:
          if (!parse_for(job, rd, &node)) {
            return false;
          }
          break;
        case qKForm:
          if (!parse_form(job, rd, &node)) {
            return false;
          }
          break;
        case qKForeach:
          if (!parse_foreach(job, rd, &node)) {
            return false;
          }
          break;
        case qKSwitch:
          if (!parse_switch(job, rd, &node)) {
            return false;
          }
          break;
        case qK__Asm__:
          if (!parse_inline_asm(job, rd, &node)) {
            return false;
          }
          break;
        case qKUnsafe: {
          Block *block = nullptr;
          tok = qlex_peek(rd);
          if (tok.is<qPuncLCur>()) {
            if (!parse(job, rd, &block)) {
              return false;
            }
          } else {
            if (!parse(job, rd, &block, false, true)) {
              return false;
            }
          }

          block->set_unsafe(true);
          (*group)->add_item(block);
          break;
        }
        case qKSafe: {
          Block *block = nullptr;
          tok = qlex_peek(rd);
          if (tok.is<qPuncLCur>()) {
            if (!parse(job, rd, &block)) {
              return false;
            }
          } else {
            if (!parse(job, rd, &block, false, true)) {
              return false;
            }
          }
          block->set_unsafe(false);
          (*group)->add_item(block);
          break;
        }
        case qKVolatile: {
          Block *block = nullptr;
          tok = qlex_peek(rd);
          if (tok.is<qPuncLCur>()) {
            if (!parse(job, rd, &block)) {
              return false;
            }
          } else {
            if (!parse(job, rd, &block, false, true)) {
              return false;
            }
          }

          (*group)->add_item(VolStmt::get(block));
          break;
        }
        default:
          syntax(tok, "Unexpected keyword");
          break;
      }

      if (node) (*group)->add_item(node);
    }

    if (expect_braces) {
      syntax(tok, "Expected '}'");
    }

    return true;
  } catch (SyntaxError &e) {
    return false;
  }
}

LIB_EXPORT qparse_t *qparse_new(qlex_t *lexer, qparse_conf_t *conf) {
  if (!lexer || !conf) {
    return nullptr;
  }

  qparse_t *parser = new qparse_t();

  parser->impl = new qparse_impl_t();
  parser->lexer = lexer;
  parser->conf = conf;
  parser->failed = false;
  parser->impl->diag.set_ctx(parser);

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

static std::atomic<size_t> sigguard_refcount;
static std::mutex sigguard_lock;
static std::unordered_map<int, sighandler_t> sigguard_old;
static const std::set<int> sigguard_signals = {SIGABRT, SIGBUS, SIGFPE, SIGILL, SIGSEGV, SIGSYS};
static thread_local jmp_buf sigguard_env;
static thread_local qparse_t *parser_ctx;

static void _signal_hander(int sig) {
  sigguard_lock.lock();

  DiagMessage diag;
  diag.msg = "FATAL Internal Error: Deadly Signal received: " + std::to_string(sig);
  diag.tok = qlex_tok_t(qEofF, 0);
  diag.type = MessageType::FatalError;

  parser_ctx->impl->diag.push(std::move(diag));

  sigguard_lock.unlock();

  longjmp(sigguard_env, sig);
}

static void install_sigguard(qparse_t *parser) {
  if (parser->conf->has(QPK_CRASHGUARD, QPV_OFF)) {
    return;
  }

  std::lock_guard<std::mutex> lock(sigguard_lock);

  if (++sigguard_refcount > 1) {
    return;
  }

  for (int sig : sigguard_signals) {
    sighandler_t old = signal(sig, _signal_hander);
    if (old == SIG_ERR) {
      qcore_panicf("Failed to install signal handler for signal %d", sig);
    }
    sigguard_old[sig] = old;
  }
}

static void uninstall_sigguard() {
  std::lock_guard<std::mutex> lock(sigguard_lock);

  if (--sigguard_refcount > 0) {
    return;
  }

  for (int sig : sigguard_signals) {
    sighandler_t old = signal(sig, sigguard_old[sig]);
    if (old == SIG_ERR) {
      qcore_panicf("Failed to uninstall signal handler for signal %d", sig);
    }
  }

  sigguard_old.clear();
}

LIB_EXPORT bool qparse_do(qparse_t *parser, qcore_arena_t *arena, qparse_node_t **out) {
  if (!parser || !arena || !out) {
    return false;
  }
  *out = nullptr;

  try {
    /*=============== Swap in their arena ===============*/
    qparse::qparse_arena.swap(*arena);

    /*== Install thread-local references to the parser ==*/
    qparse::diag::install_reference(parser);

    /*==== Facilitate signal handling for the parser ====*/
    install_sigguard(parser);
    parser_ctx = parser;

    bool status = false;
    if (setjmp(sigguard_env) == 0) {
      status = qparse::parser::parse(*parser, parser->lexer, (qparse::Block **)out, false, false);
    } else {
      parser->failed = true;
    }

    /*==== Clean up signal handling for the parser ====*/
    parser_ctx = nullptr;
    uninstall_sigguard();

    /*== Uninstall thread-local references to the parser ==*/
    qparse::diag::install_reference(nullptr);

    /*=============== Swap out their arena ===============*/
    qparse::qparse_arena.swap(*arena);

    /*==================== Return status ====================*/
    return status && !parser->failed;
  } catch (...) {
    /*== This will be caught iff QPK_CRASHGUARD is QPV_ON ==*/
    abort();
  }
}

LIB_EXPORT bool qparse_and_dump(qparse_t *parser, FILE *out, void *x0, void *x1) {
  (void)x0;
  (void)x1;

  qcore_arena_t arena;
  qparse_node_t *node = nullptr;

  if (!parser || !out) {
    return false;
  }

  qcore_arena_open(&arena);

  if (!qparse_do(parser, &arena, &node)) {
    qcore_arena_close(&arena);
    return false;
  }

  size_t len = 0;
  char *repr = qparse_repr(node, false, 2, &arena, &len);

  fwrite(repr, 1, len, out);

  qcore_arena_close(&arena);

  return true;
}

LIB_EXPORT bool qparse_check(qparse_t *parser, const qparse_node_t *base) {
  if (!parser || !base) {
    return false;
  }

  if (parser->failed) {
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

  auto adapter = [&](const char *msg) { cb(msg, std::strlen(msg), data); };

  if (no_ansi) {
    parser->impl->diag.render(adapter, qparse::diag::FormatStyle::ClangPlain);
  } else {
    parser->impl->diag.render(adapter, qparse::diag::FormatStyle::Clang16Color);
  }
}
