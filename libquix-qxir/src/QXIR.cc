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

#include <QXIRImpl.h>
#include <QXIRReport.h>
#include <quix-core/Error.h>
#include <quix-qxir/Node.h>
#include <quix-qxir/QXIR.h>
#include <setjmp.h>
#include <signal.h>

#include <array>
#include <atomic>
#include <cstring>
#include <mutex>
#include <set>

#include "LibMacro.h"

using namespace qxir;
using namespace qxir::diag;

LIB_EXPORT qxir_t *qxir_new(qparse_node_t *root, qxir_conf_t *conf) {
  if (!root || !conf) {
    return nullptr;
  }

  qxir_t *qxir = new qxir_t();

  qxir->impl = new qxir_impl_t();
  qxir->conf = conf;
  qxir->root = root;
  qxir->failed = false;
  qxir->impl->diag.set_ctx(qxir);

  return qxir;
}

LIB_EXPORT void qxir_free(qxir_t *qxir) {
  if (!qxir) {
    return;
  }

  delete qxir->impl;

  qxir->impl = nullptr;
  qxir->conf = nullptr;
  qxir->root = nullptr;

  delete qxir;
}

static std::atomic<size_t> sigguard_refcount;
static std::mutex sigguard_lock;
static std::unordered_map<int, sighandler_t> sigguard_old;
static const std::set<int> sigguard_signals = {SIGABRT, SIGBUS, SIGFPE, SIGILL, SIGSEGV, SIGSYS};
static thread_local jmp_buf sigguard_env;
static thread_local qxir_t *qxir_ctx;

static void _signal_hander(int sig) {
  sigguard_lock.lock();

  DiagMessage diag;
  diag.msg = "FATAL Internal Error: Deadly Signal received: " + std::to_string(sig);
  diag.tok = qlex_tok_t(qEofF, 0);
  diag.type = MessageType::FatalError;

  qxir_ctx->impl->diag.push(std::move(diag));

  sigguard_lock.unlock();

  longjmp(sigguard_env, sig);
}

static void install_sigguard(qxir_t *qxir) {
  if (qxir->conf->has(QQK_CRASHGUARD, QQV_OFF)) {
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

LIB_EXPORT bool qxir_do(qxir_t *qxir, qcore_arena_t *arena, qxir_node_t **out) {
  if (!qxir || !arena || !out) {
    return false;
  }
  *out = nullptr;

  try {
    /*=============== Swap in their arena ===============*/
    qxir::qxir_arena.swap(*arena);

    /*== Install thread-local references to the qxir ==*/
    qxir::diag::install_reference(qxir);

    /*==== Facilitate signal handling for the qxir ====*/
    install_sigguard(qxir);
    qxir_ctx = qxir;

    bool status = false;
    if (setjmp(sigguard_env) == 0) {
      /// TODO: Implement qxir conv logic.
      // status = qxir::qxir(*qxir, (qxir::Block **)out, false, false);
      syntax(qlex_tok_t(qEofF, 0), "qxir_do: qxir not implemented");
    } else {
      qxir->failed = true;
    }

    /*==== Clean up signal handling for the qxir ====*/
    qxir_ctx = nullptr;
    uninstall_sigguard();

    /*== Uninstall thread-local references to the qxir ==*/
    qxir::diag::install_reference(nullptr);

    /*=============== Swap out their arena ===============*/
    qxir::qxir_arena.swap(*arena);

    /*==================== Return status ====================*/
    return status && !qxir->failed;
  } catch (...) {
    /*== This will be caught iff QQK_CRASHGUARD is QQV_ON ==*/
    abort();
  }
}

LIB_EXPORT bool qxir_and_dump(qxir_t *qxir, FILE *out, void *x0, void *x1) {
  (void)x0;
  (void)x1;

  qcore_arena_t arena;
  qxir_node_t *node;

  if (!qxir || !out) {
    return false;
  }

  qcore_arena_open(&arena);

  if (!qxir_do(qxir, &arena, &node)) {
    qcore_arena_close(&arena);
    return false;
  }

  size_t len = 0;
  char *repr = qxir_repr(node, false, 2, &arena, &len);

  fwrite(repr, 1, len, out);

  qcore_arena_close(&arena);

  return true;
}

LIB_EXPORT bool qxir_check(qxir_t *qxir, const qxir_node_t *base) {
  if (!qxir || !base) {
    return false;
  }

  if (qxir->failed) {
    return false;
  }

  if (!qxir->impl) {
    qcore_panic("qpase_check: invariant violation: qxir->impl is NULL");
  }

  /* Safety is overrated */
  // return static_cast<const qxir::Expr *>(base)->verify();
  /// TODO:
  qcore_panic("qxir_check: qxir not implemented");
}

LIB_EXPORT void qxir_dumps(qxir_t *qxir, bool no_ansi, qxir_dump_cb cb, uintptr_t data) {
  if (!qxir || !cb) {
    return;
  }

  auto adapter = [&](const char *msg) { cb(msg, std::strlen(msg), data); };

  if (no_ansi) {
    qxir->impl->diag.render(adapter, qxir::diag::FormatStyle::ClangPlain);
  } else {
    qxir->impl->diag.render(adapter, qxir::diag::FormatStyle::Clang16Color);
  }
}
