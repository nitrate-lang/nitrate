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

#define __QUIX_IMPL__
#define QPARSE_USE_CPP_API
#define QXIR_USE_CPP_API

#include <QXIRImpl.h>
#include <QXIRReport.h>
#include <quix-core/Error.h>
#include <quix-qxir/Node.h>
#include <quix-qxir/QXIR.h>
#include <setjmp.h>
#include <signal.h>

#include <atomic>
#include <cstring>

#include "LibMacro.h"

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

thread_local qxir_t *m = nullptr;

static void _signal_handler(int sig) {
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
  (void)lock;

  if (++sigguard_refcount > 1) {
    return;
  }

  for (int sig : sigguard_signals) {
    sighandler_t old = signal(sig, _signal_handler);
    if (old == SIG_ERR) {
      qcore_panicf("Failed to install signal handler for signal %d", sig);
    }
    sigguard_old[sig] = old;
  }
}

static void uninstall_sigguard() {
  std::lock_guard<std::mutex> lock(sigguard_lock);
  (void)lock;

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

class QError : public std::exception {
public:
  QError() = default;
};

static qxir::Expr *qconv(const qparse::Node *node);

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
      try {
        m = qxir;
        *out = qconv(static_cast<const qparse::Node *>(qxir->root));
        status = true;
      } catch (QError &e) {
        qxir->failed = true;
      }
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
  qxir_node_t *root;

  if (!qxir || !out) {
    return false;
  }

  qcore_arena_open(&arena);

  if (!qxir_do(qxir, &arena, &root)) {
    qcore_arena_close(&arena);
    return false;
  }

  size_t len = 0;
  char *repr = qxir_repr(root, false, 2, &arena, &len);

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

static std::string_view memorize(std::string_view sv) { return m->impl->push_string(sv); }

static std::string_view memorize(qparse::String sv) {
  return memorize(std::string_view(sv.data(), sv.size()));
}

qxir::Expr *qconv_lower_binexpr(qxir::Expr *lhs, qxir::Expr *rhs, qlex_op_t op) {
#define STD_BINOP(op) qxir::create<qxir::BinExpr>(lhs, rhs, qxir::Op::op)
#define ASSIGN_BINOP(op)                                                                \
  qxir::create<qxir::BinExpr>(lhs, qxir::create<qxir::BinExpr>(lhs, rhs, qxir::Op::op), \
                              qxir::Op::Set)

  switch (op) {
    case qOpPlus: {
      return STD_BINOP(Plus);
    }
    case qOpMinus: {
      return STD_BINOP(Minus);
    }
    case qOpTimes: {
      return STD_BINOP(Times);
    }
    case qOpSlash: {
      return STD_BINOP(Slash);
    }
    case qOpPercent: {
      return STD_BINOP(Percent);
    }
    case qOpBitAnd: {
      return STD_BINOP(BitAnd);
    }
    case qOpBitOr: {
      return STD_BINOP(BitOr);
    }
    case qOpBitXor: {
      return STD_BINOP(BitXor);
    }
    case qOpBitNot: {
      return STD_BINOP(BitNot);
    }
    case qOpLogicAnd: {
      return STD_BINOP(LogicAnd);
    }
    case qOpLogicOr: {
      return STD_BINOP(LogicOr);
    }
    case qOpLogicXor: {
      // A ^^ B == (A || B) && !(A && B)
      auto a = qxir::create<qxir::BinExpr>(lhs, rhs, qxir::Op::LogicOr);
      auto b = qxir::create<qxir::BinExpr>(lhs, rhs, qxir::Op::LogicAnd);
      auto not_b = qxir::create<qxir::UnExpr>(b, qxir::Op::LogicNot);
      return qxir::create<qxir::BinExpr>(a, not_b, qxir::Op::LogicAnd);
    }
    case qOpLogicNot: {
      return STD_BINOP(LogicNot);
    }
    case qOpLShift: {
      return STD_BINOP(LShift);
    }
    case qOpRShift: {
      return STD_BINOP(RShift);
    }
    case qOpROTR: {
      return STD_BINOP(ROTR);
    }
    case qOpROTL: {
      return STD_BINOP(ROTL);
    }
    case qOpInc: {
      return STD_BINOP(Inc);
    }
    case qOpDec: {
      return STD_BINOP(Dec);
    }
    case qOpSet: {
      return STD_BINOP(Set);
    }
    case qOpPlusSet: {
      return ASSIGN_BINOP(Plus);
    }
    case qOpMinusSet: {
      return ASSIGN_BINOP(Minus);
    }
    case qOpTimesSet: {
      return ASSIGN_BINOP(Times);
    }
    case qOpSlashSet: {
      return ASSIGN_BINOP(Slash);
    }
    case qOpPercentSet: {
      return ASSIGN_BINOP(Percent);
    }
    case qOpBitAndSet: {
      return ASSIGN_BINOP(BitAnd);
    }
    case qOpBitOrSet: {
      return ASSIGN_BINOP(BitOr);
    }
    case qOpBitXorSet: {
      return ASSIGN_BINOP(BitXor);
    }
    case qOpLogicAndSet: {
      return ASSIGN_BINOP(LogicAnd);
    }
    case qOpLogicOrSet: {
      return ASSIGN_BINOP(LogicOr);
    }
    case qOpLogicXorSet: {
      // a ^^= b == a = (a || b) && !(a && b)

      auto a = qxir::create<qxir::BinExpr>(lhs, rhs, qxir::Op::LogicOr);
      auto b = qxir::create<qxir::BinExpr>(lhs, rhs, qxir::Op::LogicAnd);
      auto not_b = qxir::create<qxir::UnExpr>(b, qxir::Op::LogicNot);
      return qxir::create<qxir::BinExpr>(
          lhs, qxir::create<qxir::BinExpr>(a, not_b, qxir::Op::LogicAnd), qxir::Op::Set);
    }
    case qOpLShiftSet: {
      return ASSIGN_BINOP(LShift);
    }
    case qOpRShiftSet: {
      return ASSIGN_BINOP(RShift);
    }
    case qOpROTRSet: {
      return ASSIGN_BINOP(ROTR);
    }
    case qOpROTLSet: {
      return ASSIGN_BINOP(ROTL);
    }
    case qOpLT: {
      return STD_BINOP(LT);
    }
    case qOpGT: {
      return STD_BINOP(GT);
    }
    case qOpLE: {
      return STD_BINOP(LE);
    }
    case qOpGE: {
      return STD_BINOP(GE);
    }
    case qOpEq: {
      return STD_BINOP(Eq);
    }
    case qOpNE: {
      return STD_BINOP(NE);
    }
    case qOpAs: {
      return STD_BINOP(CastAs);
    }
    case qOpIs: {
      /// TODO:
      throw QError();
    }
    case qOpIn: {
      /// TODO:
      throw QError();
    }
    case qOpRange: {
      /// TODO:
      throw QError();
    }
    case qOpEllipsis: {
      /// TODO:
      throw QError();
    }
    case qOpSpaceship: {
      /// TODO:
      throw QError();
    }
    case qOpBitcastAs: {
      return STD_BINOP(BitcastAs);
    }
    case qOpReinterpretAs: {
      /// TODO:
      throw QError();
    }
    default: {
      throw QError();
    }
  }
}

qxir::Expr *qconv_lower_unexpr(qxir::Expr *rhs, qlex_op_t op) {
#define STD_UNOP(op) qxir::create<qxir::UnExpr>(rhs, qxir::Op::op)

  switch (op) {
    case qOpPlus: {
      return STD_UNOP(Plus);
    }
    case qOpMinus: {
      return STD_UNOP(Minus);
    }
    case qOpTimes: {
      return STD_UNOP(Times);
    }
    case qOpBitAnd: {
      return STD_UNOP(BitAnd);
    }
    case qOpBitXor: {
      return STD_UNOP(BitXor);
    }
    case qOpBitNot: {
      return STD_UNOP(BitNot);
    }
    case qOpLogicNot: {
      return STD_UNOP(LogicNot);
    }
    case qOpInc: {
      return STD_UNOP(Inc);
    }
    case qOpDec: {
      return STD_UNOP(Dec);
    }
    case qOpSizeof: {
      /// TODO:
      throw QError();
    }
    case qOpAlignof: {
      return STD_UNOP(Alignof);
    }
    case qOpTypeof: {
      /// TODO:
      throw QError();
    }
    case qOpOffsetof: {
      return STD_UNOP(Offsetof);
    }
    case qOpBitsizeof: {
      return STD_UNOP(Bitsizeof);
    }
    default: {
      throw QError();
    }
  }
}

qxir::Expr *qconv_lower_post_unexpr(qxir::Expr *lhs, qlex_op_t op) {
#define STD_POST_OP(op) qxir::create<qxir::PostUnExpr>(lhs, qxir::Op::op)

  switch (op) {
    case qOpInc: {
      return STD_POST_OP(Inc);
    }
    case qOpDec: {
      return STD_POST_OP(Dec);
    }
    default: {
      throw QError();
    }
  }
}

namespace qxir {
  static qxir::Expr *qconv_cexpr(const qparse::ConstExpr *n) {
    /// TODO: cexpr

    throw QError();
  }

  static qxir::Expr *qconv_binexpr(const qparse::BinExpr *n) {
    /// TODO: binexpr

    throw QError();
  }

  static qxir::Expr *qconv_unexpr(const qparse::UnaryExpr *n) {
    /// TODO: unexpr

    throw QError();
  }

  static qxir::Expr *qconv_post_unexpr(const qparse::PostUnaryExpr *n) {
    /// TODO: post_unexpr

    throw QError();
  }

  static qxir::Expr *qconv_terexpr(const qparse::TernaryExpr *n) {
    /// TODO: terexpr

    throw QError();
  }

  static qxir::Expr *qconv_int(const qparse::ConstInt *n) {
    /// TODO: int

    throw QError();
  }

  static qxir::Expr *qconv_float(const qparse::ConstFloat *n) {
    /// TODO: float

    throw QError();
  }

  static qxir::Expr *qconv_string(const qparse::ConstString *n) {
    /// TODO: string

    throw QError();
  }

  static qxir::Expr *qconv_char(const qparse::ConstChar *n) {
    /// TODO: char

    throw QError();
  }

  static qxir::Expr *qconv_bool(const qparse::ConstBool *n) {
    /**
     * @brief Convert a boolean constant to a qxir number.
     * @details QXIIR does not have boolean types, so we convert
     *          them to integers.
     */

    if (n->get_value()) {
      return create<Int>(1);
    } else {
      return create<Int>(0);
    }
  }

  static qxir::Expr *qconv_null(const qparse::ConstNull *n) {
    /// TODO: null

    throw QError();
  }

  static qxir::Expr *qconv_undef(const qparse::ConstUndef *n) {
    /// TODO: undef

    throw QError();
  }

  static qxir::Expr *qconv_call(const qparse::Call *n) {
    /// TODO: call

    throw QError();
  }

  static qxir::Expr *qconv_list(const qparse::List *n) {
    /// TODO: list

    throw QError();
  }

  static qxir::Expr *qconv_assoc(const qparse::Assoc *n) {
    /// TODO: assoc

    throw QError();
  }

  static qxir::Expr *qconv_field(const qparse::Field *n) {
    /// TODO: field

    throw QError();
  }

  static qxir::Expr *qconv_index(const qparse::Index *n) {
    /// TODO: index

    throw QError();
  }

  static qxir::Expr *qconv_slice(const qparse::Slice *n) {
    /// TODO: slice

    throw QError();
  }

  static qxir::Expr *qconv_fstring(const qparse::FString *n) {
    /// TODO: fstring

    throw QError();
  }

  static qxir::Expr *qconv_ident(const qparse::Ident *n) {
    /**
     * @brief Convert an identifier to a qxir expression.
     * @details This is a 1-to-1 conversion of the identifier.
     */

    return create<Ident>(memorize(n->get_name()));
  }

  static qxir::Expr *qconv_seq_point(const qparse::SeqPoint *n) {
    /// TODO: seq_point

    throw QError();
  }

  static qxir::Expr *qconv_stmt_expr(const qparse::StmtExpr *n) {
    /// TODO: stmt_expr

    throw QError();
  }

  static qxir::Expr *qconv_type_expr(const qparse::TypeExpr *n) {
    /// TODO: type_expr

    throw QError();
  }

  static qxir::Expr *qconv_templ_call(const qparse::TemplCall *n) {
    /// TODO: templ_call

    throw QError();
  }

  static qxir::Expr *qconv_mut_ty(const qparse::MutTy *n) {
    /// TODO: mut_ty

    throw QError();
  }

  static qxir::Expr *qconv_u1_ty(const qparse::U1 *n) {
    /**
     * @brief Convert a U1 type to a qxir expression type.
     * @details This is a 1-to-1 conversion of the U1 type.
     */

    return create<U1Ty>();
  }

  static qxir::Expr *qconv_u8_ty(const qparse::U8 *n) {
    /**
     * @brief Convert a U8 type to a qxir expression type.
     * @details This is a 1-to-1 conversion of the U8 type.
     */

    return create<U8Ty>();
  }

  static qxir::Expr *qconv_u16_ty(const qparse::U16 *n) {
    /**
     * @brief Convert a U16 type to a qxir expression type.
     * @details This is a 1-to-1 conversion of the U16 type.
     */

    return create<U16Ty>();
  }

  static qxir::Expr *qconv_u32_ty(const qparse::U32 *n) {
    /**
     * @brief Convert a U32 type to a qxir expression type.
     * @details This is a 1-to-1 conversion of the U32 type.
     */

    return create<U32Ty>();
  }

  static qxir::Expr *qconv_u64_ty(const qparse::U64 *n) {
    /**
     * @brief Convert a U64 type to a qxir expression type.
     * @details This is a 1-to-1 conversion of the U64 type.
     */

    return create<U64Ty>();
  }

  static qxir::Expr *qconv_u128_ty(const qparse::U128 *n) {
    /**
     * @brief Convert a U128 type to a qxir expression type.
     * @details This is a 1-to-1 conversion of the U128 type.
     */

    return create<U128Ty>();
  }

  static qxir::Expr *qconv_i8_ty(const qparse::I8 *n) {
    /**
     * @brief Convert a I8 type to a qxir expression type.
     * @details This is a 1-to-1 conversion of the I8 type.
     */

    return create<I8Ty>();
  }

  static qxir::Expr *qconv_i16_ty(const qparse::I16 *n) {
    /**
     * @brief Convert a I16 type to a qxir expression type.
     * @details This is a 1-to-1 conversion of the I16 type.
     */

    return create<I16Ty>();
  }

  static qxir::Expr *qconv_i32_ty(const qparse::I32 *n) {
    /**
     * @brief Convert a I32 type to a qxir expression type.
     * @details This is a 1-to-1 conversion of the I32 type.
     */

    return create<I32Ty>();
  }

  static qxir::Expr *qconv_i64_ty(const qparse::I64 *n) {
    /**
     * @brief Convert a I64 type to a qxir expression type.
     * @details This is a 1-to-1 conversion of the I64 type.
     */

    return create<I64Ty>();
  }

  static qxir::Expr *qconv_i128_ty(const qparse::I128 *n) {
    /**
     * @brief Convert a I128 type to a qxir expression type.
     * @details This is a 1-to-1 conversion of the I128 type.
     */

    return create<I128Ty>();
  }

  static qxir::Expr *qconv_f32_ty(const qparse::F32 *n) {
    /**
     * @brief Convert a F32 type to a qxir expression type.
     * @details This is a 1-to-1 conversion of the F32 type.
     */

    return create<F32Ty>();
  }

  static qxir::Expr *qconv_f64_ty(const qparse::F64 *n) {
    /**
     * @brief Convert a F64 type to a qxir expression type.
     * @details This is a 1-to-1 conversion of the F64 type.
     */

    return create<F64Ty>();
  }

  static qxir::Expr *qconv_void_ty(const qparse::VoidTy *n) {
    /**
     * @brief Convert a Void type to a qxir expression type.
     * @details This is a 1-to-1 conversion of the Void type.
     */

    return create<VoidTy>();
  }

  static qxir::Expr *qconv_ptr_ty(const qparse::PtrTy *n) {
    /// TODO: ptr_ty

    throw QError();
  }

  static qxir::Expr *qconv_opaque_ty(const qparse::OpaqueTy *n) {
    /// TODO: opaque_ty

    throw QError();
  }

  static qxir::Expr *qconv_string_ty(const qparse::StringTy *n) {
    /// TODO: string_ty

    throw QError();
  }

  static qxir::Expr *qconv_enum_ty(const qparse::EnumTy *n) {
    /// TODO: enum_ty

    throw QError();
  }

  static qxir::Expr *qconv_struct_ty(const qparse::StructTy *n) {
    /// TODO: struct_ty

    throw QError();
  }

  static qxir::Expr *qconv_group_ty(const qparse::GroupTy *n) {
    /// TODO: group_ty

    throw QError();
  }

  static qxir::Expr *qconv_region_ty(const qparse::RegionTy *n) {
    /// TODO: region_ty

    throw QError();
  }

  static qxir::Expr *qconv_union_ty(const qparse::UnionTy *n) {
    /// TODO: union_ty

    throw QError();
  }

  static qxir::Expr *qconv_array_ty(const qparse::ArrayTy *n) {
    /// TODO: array_ty

    throw QError();
  }

  static qxir::Expr *qconv_vector_ty(const qparse::VectorTy *n) {
    /// TODO: vector_ty

    throw QError();
  }

  static qxir::Expr *qconv_map_ty(const qparse::MapTy *n) {
    /// TODO: map_ty

    throw QError();
  }

  static qxir::Expr *qconv_tuple_ty(const qparse::TupleTy *n) {
    /// TODO: tuple_ty

    throw QError();
  }

  static qxir::Expr *qconv_set_ty(const qparse::SetTy *n) {
    /// TODO: set_ty

    throw QError();
  }

  static qxir::Expr *qconv_result_ty(const qparse::OptionalTy *n) {
    /// TODO: result_ty

    throw QError();
  }

  static qxir::Expr *qconv_fn_ty(const qparse::FuncTy *n) {
    /// TODO: fn_ty

    throw QError();
  }

  static qxir::Expr *qconv_unres_ty(const qparse::UnresolvedType *n) {
    /// TODO: unres_ty

    throw QError();
  }

  static qxir::Expr *qconv_typedef(const qparse::TypedefDecl *n) {
    /// TODO: typedef

    throw QError();
  }

  static qxir::Expr *qconv_fndecl(const qparse::FnDecl *n) {
    /// TODO: fndecl

    throw QError();
  }

  static qxir::Expr *qconv_struct(const qparse::StructDef *n) {
    /// TODO: struct

    throw QError();
  }

  static qxir::Expr *qconv_region(const qparse::RegionDef *n) {
    /// TODO: region

    throw QError();
  }

  static qxir::Expr *qconv_group(const qparse::GroupDef *n) {
    /// TODO: group

    throw QError();
  }

  static qxir::Expr *qconv_union(const qparse::UnionDef *n) {
    /// TODO: union

    throw QError();
  }

  static qxir::Expr *qconv_enum(const qparse::EnumDef *n) {
    /// TODO: enum

    throw QError();
  }

  static qxir::Expr *qconv_fn(const qparse::FnDef *n) {
    /// TODO: fn

    throw QError();
  }

  static qxir::Expr *qconv_subsystem(const qparse::SubsystemDecl *n) {
    /// TODO: subsystem

    throw QError();
  }

  static qxir::Expr *qconv_export(const qparse::ExportDecl *n) {
    /// TODO: export

    throw QError();
  }

  static qxir::Expr *qconv_composite_field(const qparse::CompositeField *n) {
    /// TODO: composite_field

    throw QError();
  }

  static qxir::Expr *qconv_block(const qparse::Block *n) {
    /// TODO: block

    throw QError();
  }

  static qxir::Expr *qconv_const(const qparse::ConstDecl *n) {
    /// TODO: const

    throw QError();
  }

  static qxir::Expr *qconv_var(const qparse::VarDecl *n) {
    /// TODO: var

    throw QError();
  }

  static qxir::Expr *qconv_let(const qparse::LetDecl *n) {
    /// TODO: let

    throw QError();
  }

  static qxir::Expr *qconv_inline_asm(const qparse::InlineAsm *n) {
    /// TODO: inline_asm

    throw QError();
  }

  static qxir::Expr *qconv_return(const qparse::ReturnStmt *n) {
    /// TODO: return

    throw QError();
  }

  static qxir::Expr *qconv_retif(const qparse::ReturnIfStmt *n) {
    /// TODO: retif

    throw QError();
  }

  static qxir::Expr *qconv_retz(const qparse::RetZStmt *n) {
    /// TODO: retz

    throw QError();
  }

  static qxir::Expr *qconv_retv(const qparse::RetVStmt *n) {
    /// TODO: retv

    throw QError();
  }

  static qxir::Expr *qconv_break(const qparse::BreakStmt *n) {
    /// TODO: break

    throw QError();
  }

  static qxir::Expr *qconv_continue(const qparse::ContinueStmt *n) {
    /// TODO: continue

    throw QError();
  }

  static qxir::Expr *qconv_if(const qparse::IfStmt *n) {
    /// TODO: if

    throw QError();
  }

  static qxir::Expr *qconv_while(const qparse::WhileStmt *n) {
    /// TODO: while

    throw QError();
  }

  static qxir::Expr *qconv_for(const qparse::ForStmt *n) {
    /// TODO: for

    throw QError();
  }

  static qxir::Expr *qconv_form(const qparse::FormStmt *n) {
    /// TODO: form

    throw QError();
  }

  static qxir::Expr *qconv_foreach(const qparse::ForeachStmt *n) {
    /// TODO: foreach

    throw QError();
  }

  static qxir::Expr *qconv_case(const qparse::CaseStmt *n) {
    /// TODO: case

    throw QError();
  }

  static qxir::Expr *qconv_switch(const qparse::SwitchStmt *n) {
    /// TODO: switch

    throw QError();
  }

  static qxir::Expr *qconv_slist(const qparse::StmtList *n) {
    /// TODO: slist

    throw QError();
  }

  static qxir::Expr *qconv_expr_stmt(const qparse::ExprStmt *n) {
    /// TODO: expr_stmt

    throw QError();
  }

  static qxir::Expr *qconv_volstmt(const qparse::VolStmt *n) {
    /// TODO: volstmt

    throw QError();
  }
}  // namespace qxir

static qxir::Expr *qconv(const qparse::Node *n) {
  using namespace qxir;

  if (!n) {
    return nullptr;
  }

  qxir::Expr *out = nullptr;

  switch (n->this_typeid()) {
    case QAST_NODE_CEXPR:
      out = qconv_cexpr(n->as<qparse::ConstExpr>());
      break;

    case QAST_NODE_BINEXPR:
      out = qconv_binexpr(n->as<qparse::BinExpr>());
      break;

    case QAST_NODE_UNEXPR:
      out = qconv_unexpr(n->as<qparse::UnaryExpr>());
      break;

    case QAST_NODE_TEREXPR:
      out = qconv_terexpr(n->as<qparse::TernaryExpr>());
      break;

    case QAST_NODE_INT:
      out = qconv_int(n->as<qparse::ConstInt>());
      break;

    case QAST_NODE_FLOAT:
      out = qconv_float(n->as<qparse::ConstFloat>());
      break;

    case QAST_NODE_STRING:
      out = qconv_string(n->as<qparse::ConstString>());
      break;

    case QAST_NODE_CHAR:
      out = qconv_char(n->as<qparse::ConstChar>());
      break;

    case QAST_NODE_BOOL:
      out = qconv_bool(n->as<qparse::ConstBool>());
      break;

    case QAST_NODE_NULL:
      out = qconv_null(n->as<qparse::ConstNull>());
      break;

    case QAST_NODE_UNDEF:
      out = qconv_undef(n->as<qparse::ConstUndef>());
      break;

    case QAST_NODE_CALL:
      out = qconv_call(n->as<qparse::Call>());
      break;

    case QAST_NODE_LIST:
      out = qconv_list(n->as<qparse::List>());
      break;

    case QAST_NODE_ASSOC:
      out = qconv_assoc(n->as<qparse::Assoc>());
      break;

    case QAST_NODE_FIELD:
      out = qconv_field(n->as<qparse::Field>());
      break;

    case QAST_NODE_INDEX:
      out = qconv_index(n->as<qparse::Index>());
      break;

    case QAST_NODE_SLICE:
      out = qconv_slice(n->as<qparse::Slice>());
      break;

    case QAST_NODE_FSTRING:
      out = qconv_fstring(n->as<qparse::FString>());
      break;

    case QAST_NODE_IDENT:
      out = qconv_ident(n->as<qparse::Ident>());
      break;

    case QAST_NODE_SEQ_POINT:
      out = qconv_seq_point(n->as<qparse::SeqPoint>());
      break;

    case QAST_NODE_POST_UNEXPR:
      out = qconv_post_unexpr(n->as<qparse::PostUnaryExpr>());
      break;

    case QAST_NODE_STMT_EXPR:
      out = qconv_stmt_expr(n->as<qparse::StmtExpr>());
      break;

    case QAST_NODE_TYPE_EXPR:
      out = qconv_type_expr(n->as<qparse::TypeExpr>());
      break;

    case QAST_NODE_TEMPL_CALL:
      out = qconv_templ_call(n->as<qparse::TemplCall>());
      break;

    case QAST_NODE_MUT_TY:
      out = qconv_mut_ty(n->as<qparse::MutTy>());
      break;

    case QAST_NODE_U1_TY:
      out = qconv_u1_ty(n->as<qparse::U1>());
      break;

    case QAST_NODE_U8_TY:
      out = qconv_u8_ty(n->as<qparse::U8>());
      break;

    case QAST_NODE_U16_TY:
      out = qconv_u16_ty(n->as<qparse::U16>());
      break;

    case QAST_NODE_U32_TY:
      out = qconv_u32_ty(n->as<qparse::U32>());
      break;

    case QAST_NODE_U64_TY:
      out = qconv_u64_ty(n->as<qparse::U64>());
      break;

    case QAST_NODE_U128_TY:
      out = qconv_u128_ty(n->as<qparse::U128>());
      break;

    case QAST_NODE_I8_TY:
      out = qconv_i8_ty(n->as<qparse::I8>());
      break;

    case QAST_NODE_I16_TY:
      out = qconv_i16_ty(n->as<qparse::I16>());
      break;

    case QAST_NODE_I32_TY:
      out = qconv_i32_ty(n->as<qparse::I32>());
      break;

    case QAST_NODE_I64_TY:
      out = qconv_i64_ty(n->as<qparse::I64>());
      break;

    case QAST_NODE_I128_TY:
      out = qconv_i128_ty(n->as<qparse::I128>());
      break;

    case QAST_NODE_F32_TY:
      out = qconv_f32_ty(n->as<qparse::F32>());
      break;

    case QAST_NODE_F64_TY:
      out = qconv_f64_ty(n->as<qparse::F64>());
      break;

    case QAST_NODE_VOID_TY:
      out = qconv_void_ty(n->as<qparse::VoidTy>());
      break;

    case QAST_NODE_PTR_TY:
      out = qconv_ptr_ty(n->as<qparse::PtrTy>());
      break;

    case QAST_NODE_OPAQUE_TY:
      out = qconv_opaque_ty(n->as<qparse::OpaqueTy>());
      break;

    case QAST_NODE_STRING_TY:
      out = qconv_string_ty(n->as<qparse::StringTy>());
      break;

    case QAST_NODE_ENUM_TY:
      out = qconv_enum_ty(n->as<qparse::EnumTy>());
      break;

    case QAST_NODE_STRUCT_TY:
      out = qconv_struct_ty(n->as<qparse::StructTy>());
      break;

    case QAST_NODE_GROUP_TY:
      out = qconv_group_ty(n->as<qparse::GroupTy>());
      break;

    case QAST_NODE_REGION_TY:
      out = qconv_region_ty(n->as<qparse::RegionTy>());
      break;

    case QAST_NODE_UNION_TY:
      out = qconv_union_ty(n->as<qparse::UnionTy>());
      break;

    case QAST_NODE_ARRAY_TY:
      out = qconv_array_ty(n->as<qparse::ArrayTy>());
      break;

    case QAST_NODE_VECTOR_TY:
      out = qconv_vector_ty(n->as<qparse::VectorTy>());
      break;

    case QAST_NODE_MAP_TY:
      out = qconv_map_ty(n->as<qparse::MapTy>());
      break;

    case QAST_NODE_TUPLE_TY:
      out = qconv_tuple_ty(n->as<qparse::TupleTy>());
      break;

    case QAST_NODE_SET_TY:
      out = qconv_set_ty(n->as<qparse::SetTy>());
      break;

    case QAST_NODE_RESULT_TY:
      out = qconv_result_ty(n->as<qparse::OptionalTy>());
      break;

    case QAST_NODE_FN_TY:
      out = qconv_fn_ty(n->as<qparse::FuncTy>());
      break;

    case QAST_NODE_UNRES_TY:
      out = qconv_unres_ty(n->as<qparse::UnresolvedType>());
      break;

    case QAST_NODE_TYPEDEF:
      out = qconv_typedef(n->as<qparse::TypedefDecl>());
      break;

    case QAST_NODE_FNDECL:
      out = qconv_fndecl(n->as<qparse::FnDecl>());
      break;

    case QAST_NODE_STRUCT:
      out = qconv_struct(n->as<qparse::StructDef>());
      break;

    case QAST_NODE_REGION:
      out = qconv_region(n->as<qparse::RegionDef>());
      break;

    case QAST_NODE_GROUP:
      out = qconv_group(n->as<qparse::GroupDef>());
      break;

    case QAST_NODE_UNION:
      out = qconv_union(n->as<qparse::UnionDef>());
      break;

    case QAST_NODE_ENUM:
      out = qconv_enum(n->as<qparse::EnumDef>());
      break;

    case QAST_NODE_FN:
      out = qconv_fn(n->as<qparse::FnDef>());
      break;

    case QAST_NODE_SUBSYSTEM:
      out = qconv_subsystem(n->as<qparse::SubsystemDecl>());
      break;

    case QAST_NODE_EXPORT:
      out = qconv_export(n->as<qparse::ExportDecl>());
      break;

    case QAST_NODE_COMPOSITE_FIELD:
      out = qconv_composite_field(n->as<qparse::CompositeField>());
      break;

    case QAST_NODE_BLOCK:
      out = qconv_block(n->as<qparse::Block>());
      break;

    case QAST_NODE_CONST:
      out = qconv_const(n->as<qparse::ConstDecl>());
      break;

    case QAST_NODE_VAR:
      out = qconv_var(n->as<qparse::VarDecl>());
      break;

    case QAST_NODE_LET:
      out = qconv_let(n->as<qparse::LetDecl>());
      break;

    case QAST_NODE_INLINE_ASM:
      out = qconv_inline_asm(n->as<qparse::InlineAsm>());
      break;

    case QAST_NODE_RETURN:
      out = qconv_return(n->as<qparse::ReturnStmt>());
      break;

    case QAST_NODE_RETIF:
      out = qconv_retif(n->as<qparse::ReturnIfStmt>());
      break;

    case QAST_NODE_RETZ:
      out = qconv_retz(n->as<qparse::RetZStmt>());
      break;

    case QAST_NODE_RETV:
      out = qconv_retv(n->as<qparse::RetVStmt>());
      break;

    case QAST_NODE_BREAK:
      out = qconv_break(n->as<qparse::BreakStmt>());
      break;

    case QAST_NODE_CONTINUE:
      out = qconv_continue(n->as<qparse::ContinueStmt>());
      break;

    case QAST_NODE_IF:
      out = qconv_if(n->as<qparse::IfStmt>());
      break;

    case QAST_NODE_WHILE:
      out = qconv_while(n->as<qparse::WhileStmt>());
      break;

    case QAST_NODE_FOR:
      out = qconv_for(n->as<qparse::ForStmt>());
      break;

    case QAST_NODE_FORM:
      out = qconv_form(n->as<qparse::FormStmt>());
      break;

    case QAST_NODE_FOREACH:
      out = qconv_foreach(n->as<qparse::ForeachStmt>());
      break;

    case QAST_NODE_CASE:
      out = qconv_case(n->as<qparse::CaseStmt>());
      break;

    case QAST_NODE_SWITCH:
      out = qconv_switch(n->as<qparse::SwitchStmt>());
      break;

    case QAST_NODE_SLIST:
      out = qconv_slist(n->as<qparse::StmtList>());
      break;

    case QAST_NODE_EXPR_STMT:
      out = qconv_expr_stmt(n->as<qparse::ExprStmt>());
      break;

    case QAST_NODE_VOLSTMT:
      out = qconv_volstmt(n->as<qparse::VolStmt>());
      break;

    default: {
      qcore_panicf("qxir: unknown node type: %d", static_cast<int>(n->this_typeid()));
    }
  }

  if (!out) {
    qcore_panicf("qxir: conversion failed for node type: %d", static_cast<int>(n->this_typeid()));
  }

  out->setStartLoc(n->get_start_pos());
  out->setEndLoc(n->get_end_pos());

  return out;
}
