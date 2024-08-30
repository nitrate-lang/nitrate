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

#include <quix-core/Error.h>
#include <quix-parser/Parser.h>
#include <quix-qxir/IR.h>
#include <quix-qxir/Module.h>
#include <quix-qxir/Node.h>
#include <setjmp.h>
#include <signal.h>

#include <atomic>
#include <core/Config.hh>
#include <cstring>
#include <diagnostic/Report.hh>
#include <passes/PassManager.hh>

#include "core/LibMacro.h"

using namespace qxir::diag;

struct ConvState {
  bool inside_function = false;
  std::unordered_map<std::string, qxir::Type *> typedef_map;
  std::string ns_prefix;
  std::unordered_map<std::string, std::vector<std::tuple<std::string, qxir::Type *, qxir::Expr *>>>
      param_map;

  std::string cur_named(std::string_view suffix) const {
    if (ns_prefix.empty()) {
      return std::string(suffix);
    }
    return ns_prefix + "::" + std::string(suffix);
  }
};

static std::atomic<size_t> sigguard_refcount;
static std::mutex sigguard_lock;
static std::unordered_map<int, sighandler_t> sigguard_old;
static const std::set<int> sigguard_signals = {SIGABRT, SIGBUS, SIGFPE, SIGILL, SIGSEGV, SIGSYS};
static thread_local jmp_buf sigguard_env;
static thread_local qmodule_t *qxir_ctx;

static void _signal_handler(int sig) {
  sigguard_lock.lock();

  DiagMessage diag;
  diag.msg = "FATAL Internal Error: Deadly Signal received: " + std::to_string(sig);
  diag.start = diag.end = qlex_loc_t{};
  diag.type = IssueClass::FatalError;
  diag.code = IssueCode::SignalReceived;

  qxir_ctx->getDiag().push(QXIR_AUDIT_CONV, std::move(diag));

  sigguard_lock.unlock();

  longjmp(sigguard_env, sig);
}

static void install_sigguard(qmodule_t *qxir) noexcept {
  if (qxir->getConf()->has(QQK_CRASHGUARD, QQV_OFF)) {
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

static void uninstall_sigguard() noexcept {
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

static qxir::Expr *qconv(ConvState &s, const qparse::Node *node);

LIB_EXPORT bool qxir_lower(qmodule_t *mod, qparse_node_t *base, bool diagnostics) {
  if (!base) {
    return false;
  }

  qcore_assert(mod, "qxir_lower: mod == nullptr");

  std::swap(qxir::qxir_arena.get(), mod->getNodeArena());
  qxir::diag::install_reference(mod);
  install_sigguard(mod);
  qxir_ctx = mod;
  mod->setRoot(nullptr);
  mod->enableDiagnostics(diagnostics);

  volatile bool status = false;  // For sanity

  if (setjmp(sigguard_env) == 0) {
    try {
      ConvState s;
      mod->setRoot(qconv(s, static_cast<const qparse::Node *>(base)));
      status = !mod->getFailbit();
    } catch (QError &e) {
      // QError exception is control flow to abort the recursive lowering

      status = false;  // For sanity

      /**
       * At this point status is false, so we can continue to the cleanup
       */
    }
  } else {
    status = false;  // For sanity

    /**
     * An signal (what is usually a fatal error) was caught,
     * the program is pretty much in an undefined state. However,
     * I don't care about the state of the program, I just want to
     * clean up the resources and return to the trusting user code.
     */
  }

  qxir_ctx = nullptr;
  uninstall_sigguard();
  qxir::diag::install_reference(nullptr);
  std::swap(qxir::qxir_arena.get(), mod->getNodeArena());

  return status;
}

LIB_EXPORT bool qxir_justprint(qparse_node_t *base, FILE *out, qxir_serial_t mode, qxir_node_cb cb,
                               uint32_t argcnt, ...) {
  qxir_conf_t *conf = nullptr;
  qmodule_t *mod = nullptr;

  if ((conf = qxir_conf_new(true)) == nullptr) {
    return false;
  }

  if ((mod = qxir_new(nullptr, conf)) == nullptr) {
    qxir_conf_free(conf);
    return false;
  }

  (void)qxir_lower(mod, base, false);

  if (!mod->getRoot()) {
    qxir_free(mod);
    qxir_conf_free(conf);
    return false;
  }

  if (cb) { /* Callback to user code */
    uintptr_t userdata = 0;

    if (argcnt > 0) { /* Extract userdata variadic parameter */
      va_list args;
      va_start(args, argcnt);
      userdata = va_arg(args, uintptr_t);
      va_end(args);
    }

    auto ucb = [cb, userdata](qxir::Expr *p, qxir::Expr *c) {
      cb(static_cast<qxir_node_t *>(c), userdata);
      return qxir::IterOp::Proceed;
    };

    qxir::Expr *root = static_cast<qxir::Expr *>(mod->getRoot());
    qxir::iterate<qxir::dfs_pre, qxir::IterMP::none>(root, ucb);
  }

  if (!qxir_write(mod, mode, out, nullptr, 0)) {
    qxir_free(mod);
    qxir_conf_free(conf);
    return false;
  }

  qxir_free(mod);
  qxir_conf_free(conf);

  return true;
}

#include <iomanip>

static void dump_node_as_raw_hex(qxir::Expr *node) {
  std::array<uint8_t, sizeof(qxir::Expr)> binbuf;
  memcpy(binbuf.data(), node, sizeof(qxir::Expr));

  // hex print
  std::cout << "------------------------------------------------------------\n";
  for (size_t i = 0; i < binbuf.size(); i++) {
    std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)binbuf[i] << " ";
  }
  std::cout << "\n------------------------------------------------------------" << std::endl;
}

LIB_EXPORT void qmodule_testplug(void *node) {
  using namespace qxir;
  Expr *n = static_cast<Expr *>(node);

  std::cout << "sizeof(Expr): " << sizeof(Expr) << std::endl;

  iterate<dfs_pre, IterMP::none>(n, [&](Expr *p, Expr *c) {
    if (p) {
      std::cout << "ParentType: " << p->thisTypeName();
    } else {
      std::cout << "ParentType: nullptr";
    }

    if (c) {
      std::cout << " ChildType: " << c->thisTypeName();
    } else {
      std::cout << " ChildType: nullptr";
    }

    std::cout << std::endl;

    if (c) {
      dump_node_as_raw_hex(c);
    }

    std::cout << std::endl;

    return IterOp::Proceed;
  });
}

///=============================================================================

static std::string_view memorize(std::string_view sv) { return qxir_ctx->internString(sv); }
static std::string_view memorize(qparse::String sv) {
  return memorize(std::string_view(sv.data(), sv.size()));
}

qxir::Expr *qconv_lower_binexpr(ConvState &s, qxir::Expr *lhs, qxir::Expr *rhs, qlex_op_t op) {
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
      auto fn = qxir::create<qxir::Ident>("__is");
      return qxir::create<qxir::Call>(fn, qxir::CallArgs({lhs, rhs}));
    }
    case qOpIn: {
      auto methname = qxir::create<qxir::String>("has");
      auto method = qxir::create<qxir::Index>(rhs, methname);
      return qxir::create<qxir::Call>(method, qxir::CallArgs({lhs}));
    }
    case qOpRange: {
      /// TODO:
      throw std::runtime_error("qOpRange not implemented");
    }
    case qOpBitcastAs: {
      return STD_BINOP(BitcastAs);
    }
    default: {
      throw QError();
    }
  }
}

qxir::Expr *qconv_lower_unexpr(ConvState &s, qxir::Expr *rhs, qlex_op_t op) {
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
      auto bits = qxir::create<qxir::UnExpr>(rhs, qxir::Op::Bitsizeof);
      auto ceilfn = qxir::create<qxir::Ident>("__min");
      auto arg = qxir::create<qxir::BinExpr>(bits, qxir::create<qxir::Int>(8), qxir::Op::Slash);
      auto bytes = qxir::create<qxir::Call>(ceilfn, qxir::CallArgs({arg}));
      return bytes;
    }
    case qOpAlignof: {
      return STD_UNOP(Alignof);
    }
    case qOpTypeof: {
      auto fn = qxir::create<qxir::Ident>("__typeof");
      return qxir::create<qxir::Call>(fn, qxir::CallArgs({rhs}));
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

qxir::Expr *qconv_lower_post_unexpr(ConvState &s, qxir::Expr *lhs, qlex_op_t op) {
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
  static Expr *qconv_cexpr(ConvState &s, const qparse::ConstExpr *n) {
    auto c = qconv(s, n->get_value());
    if (!c) {
      badtree(n, "qparse::ConstExpr::get_value() == nullptr");
      throw QError();
    }

    c->setConst(true);

    return c;
  }

  static Expr *qconv_binexpr(ConvState &s, const qparse::BinExpr *n) {
    /**
     * @brief Convert a binary expression to a qxir expression.
     * @details Recursively convert the left and right hand sides of the
     *         binary expression, then convert the operator to a qxir
     *         compatible operator.
     */

    auto lhs = qconv(s, n->get_lhs());
    if (!lhs) {
      badtree(n, "qparse::BinExpr::get_lhs() == nullptr");
      throw QError();
    }

    auto rhs = qconv(s, n->get_rhs());

    if (!rhs) {
      badtree(n, "qparse::BinExpr::get_rhs() == nullptr");
      throw QError();
    }

    return qconv_lower_binexpr(s, lhs, rhs, n->get_op());
  }

  static Expr *qconv_unexpr(ConvState &s, const qparse::UnaryExpr *n) {
    /**
     * @brief Convert a unary expression to a qxir expression.
     * @details Recursively convert the left hand side of the unary
     *         expression, then convert the operator to a qxir compatible
     *         operator.
     */

    auto rhs = qconv(s, n->get_rhs());
    if (!rhs) {
      badtree(n, "qparse::UnaryExpr::get_rhs() == nullptr");
      throw QError();
    }

    return qconv_lower_unexpr(s, rhs, n->get_op());
  }

  static Expr *qconv_post_unexpr(ConvState &s, const qparse::PostUnaryExpr *n) {
    /**
     * @brief Convert a post-unary expression to a qxir expression.
     * @details Recursively convert the left hand side of the post-unary
     *         expression, then convert the operator to a qxir compatible
     *         operator.
     */

    auto lhs = qconv(s, n->get_lhs());
    if (!lhs) {
      badtree(n, "qparse::PostUnaryExpr::get_lhs() == nullptr");
      throw QError();
    }

    return qconv_lower_post_unexpr(s, lhs, n->get_op());
  }

  static Expr *qconv_terexpr(ConvState &s, const qparse::TernaryExpr *n) {
    /**
     * @brief Convert a ternary expression to a if-else expression.
     * @details Recursively convert the condition, then the true and false
     *        branches of the ternary expression.
     */

    auto cond = qconv(s, n->get_cond());
    if (!cond) {
      badtree(n, "qparse::TernaryExpr::get_cond() == nullptr");
      throw QError();
    }

    auto t = qconv(s, n->get_lhs());
    if (!t) {
      badtree(n, "qparse::TernaryExpr::get_lhs() == nullptr");
      throw QError();
    }

    auto f = qconv(s, n->get_rhs());
    if (!f) {
      badtree(n, "qparse::TernaryExpr::get_rhs() == nullptr");
      throw QError();
    }

    return create<If>(cond, t, f);
  }

  static Expr *qconv_int(ConvState &s, const qparse::ConstInt *n) {
    /**
     * @brief Convert an integer constant to a qxir number.
     * @details This is a 1-to-1 conversion of the integer constant.
     */

    return create<Int>(memorize(n->get_value()));
  }

  static Expr *qconv_float(ConvState &s, const qparse::ConstFloat *n) {
    /**
     * @brief Convert a floating point constant to a qxir number.
     * @details This is a 1-to-1 conversion of the floating point constant.
     */

    return create<Float>(memorize(n->get_value()));
  }

  static Expr *qconv_string(ConvState &s, const qparse::ConstString *n) {
    /**
     * @brief Convert a string constant to a qxir string.
     * @details This is a 1-to-1 conversion of the string constant.
     */

    return create<String>(memorize(n->get_value()));
  }

  static Expr *qconv_char(ConvState &s, const qparse::ConstChar *n) {
    /**
     * @brief Convert a character constant to a qxir number.
     * @details Convert the char32 codepoint to a qxir number literal.
     */

    return create<Int>(n->get_value());
  }

  static Expr *qconv_bool(ConvState &s, const qparse::ConstBool *n) {
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

  static Expr *qconv_null(ConvState &s, const qparse::ConstNull *n) {
    /**
     * @brief Convert a null literal to a qxir null literal tmp.
     * @details This is a 1-to-1 conversion of the null literal.
     */

    return create<Tmp>(TmpType::NULL_LITERAL);
  }

  static Expr *qconv_undef(ConvState &s, const qparse::ConstUndef *n) {
    /**
     * @brief Convert an undefined literal to a qxir undefined literal tmp.
     * @details This is a 1-to-1 conversion of the undefined literal.
     */

    return create<Tmp>(TmpType::UNDEF_LITERAL);
  }

  static Expr *qconv_call(ConvState &s, const qparse::Call *n) {
    /**
     * @brief Convert a function call to a qxir call.
     * @details Recursively convert the function base and the arguments
     *         of the function call. We currently do not have enough information
     *         to handle the expansion of named / optional arguments. Therefore,
     *         we wrap the data present and we will attempt to lower the construct
     *         later.
     */

    auto base = qconv(s, n->get_func());
    if (!base) {
      badtree(n, "qparse::Call::get_func() == nullptr");
      throw QError();
    }

    CallArgsTmpNodeCradle datapack;
    for (auto it = n->get_args().begin(); it != n->get_args().end(); ++it) {
      auto arg = qconv(s, it->second);
      if (!arg) {
        badtree(n, "qparse::Call::get_args() vector contains nullptr");
        throw QError();
      }

      std::get<1>(datapack).push_back({memorize(it->first), arg});
    }

    return create<Tmp>(TmpType::CALL, std::move(datapack));
  }

  static Expr *qconv_list(ConvState &s, const qparse::List *n) {
    /**
     * @brief Convert a list of expressions to a qxir list.
     * @details This is a 1-to-1 conversion of the list of expressions.
     */

    ListItems items;

    for (auto it = n->get_items().begin(); it != n->get_items().end(); ++it) {
      auto item = qconv(s, *it);
      if (!item) {
        badtree(n, "qparse::List::get_items() vector contains nullptr");
        throw QError();
      }

      items.push_back(item);
    }

    return create<List>(std::move(items));
  }

  static Expr *qconv_assoc(ConvState &s, const qparse::Assoc *n) {
    /**
     * @brief Convert an associative list to a qxir list.
     * @details This is a 1-to-1 conversion of the associative list.
     */

    auto key = qconv(s, n->get_key());
    if (!key) {
      badtree(n, "qparse::Assoc::get_key() == nullptr");
      throw QError();
    }

    auto value = qconv(s, n->get_value());
    if (!value) {
      badtree(n, "qparse::Assoc::get_value() == nullptr");
      throw QError();
    }

    return create<List>(ListItems({key, value}));
  }

  static Expr *qconv_field(ConvState &s, const qparse::Field *n) {
    /**
     * @brief Convert a field access to a qxir expression.
     * @details Store the base and field name in a temporary node cradle
     *          for later lowering.
     */

    auto base = qconv(s, n->get_base());
    if (!base) {
      badtree(n, "qparse::Field::get_base() == nullptr");
      throw QError();
    }

    FieldTmpNodeCradle datapack;
    std::get<0>(datapack) = base;
    std::get<1>(datapack) = memorize(n->get_field());

    return create<Tmp>(TmpType::FIELD, std::move(datapack));
  }

  static Expr *qconv_index(ConvState &s, const qparse::Index *n) {
    /**
     * @brief Convert an index expression to a qxir expression.
     * @details Recursively convert the base and index of the index
     *         expression.
     */

    auto base = qconv(s, n->get_base());
    if (!base) {
      badtree(n, "qparse::Index::get_base() == nullptr");
      throw QError();
    }

    auto index = qconv(s, n->get_index());
    if (!index) {
      badtree(n, "qparse::Index::get_index() == nullptr");
      throw QError();
    }

    return create<Index>(base, index);
  }

  static Expr *qconv_slice(ConvState &s, const qparse::Slice *n) {
    /// TODO: slice

    throw QError();
  }

  static Expr *qconv_fstring(ConvState &s, const qparse::FString *n) {
    /**
     * @brief Convert a formatted string to a qxir string concatenation.
     */

    if (n->get_items().empty()) {
      return create<String>("");
    }

    if (n->get_items().size() == 1) {
      auto val = n->get_items().front();
      if (std::holds_alternative<qparse::String>(val)) {
        return create<String>(memorize(std::get<qparse::String>(val)));
      } else if (std::holds_alternative<qparse::Expr *>(val)) {
        auto expr = qconv(s, std::get<qparse::Expr *>(val));
        if (!expr) {
          badtree(n, "qparse::FString::get_items() vector contains nullptr");
          throw QError();
        }

        return create<BinExpr>(expr, getType<StringTy>(), Op::CastAs);
      } else {
        qcore_panic("Invalid fstring item type");
      }
    }

    Expr *concated = create<String>("");

    for (auto it = n->get_items().begin(); it != n->get_items().end(); ++it) {
      if (std::holds_alternative<qparse::String>(*it)) {
        auto val = std::get<qparse::String>(*it);

        concated = create<BinExpr>(concated, create<String>(memorize(val)), Op::Plus);
      } else if (std::holds_alternative<qparse::Expr *>(*it)) {
        auto val = std::get<qparse::Expr *>(*it);
        auto expr = qconv(s, val);
        if (!expr) {
          badtree(n, "qparse::FString::get_items() vector contains nullptr");
          throw QError();
        }

        expr = create<BinExpr>(expr, getType<StringTy>(), Op::CastAs);
        concated = create<BinExpr>(concated, expr, Op::Plus);
      } else {
        qcore_panic("Invalid fstring item type");
      }
    }

    return concated;
  }

  static Expr *qconv_ident(ConvState &s, const qparse::Ident *n) {
    /**
     * @brief Convert an identifier to a qxir expression.
     * @details This is a 1-to-1 conversion of the identifier.
     */

    return create<Ident>(memorize(n->get_name()));
  }

  static Expr *qconv_seq_point(ConvState &s, const qparse::SeqPoint *n) {
    /**
     * @brief Convert a sequence point to a qxir expression.
     * @details This is a 1-to-1 conversion of the sequence point.
     */

    SeqItems items;

    for (auto it = n->get_items().begin(); it != n->get_items().end(); ++it) {
      auto item = qconv(s, *it);
      if (!item) {
        badtree(n, "qparse::SeqPoint::get_items() vector contains nullptr");
        throw QError();
      }

      items.push_back(item);
    }

    return create<Seq>(std::move(items));
  }

  static Expr *qconv_stmt_expr(ConvState &s, const qparse::StmtExpr *n) {
    /**
     * @brief Unwrap a statement inside an expression into a qxir expression.
     * @details This is a 1-to-1 conversion of the statement expression.
     */

    auto stmt = qconv(s, n->get_stmt());
    if (!stmt) {
      badtree(n, "qparse::StmtExpr::get_stmt() == nullptr");
      throw QError();
    }

    return stmt;
  }

  static Expr *qconv_type_expr(ConvState &s, const qparse::TypeExpr *n) {
    /*
     * @brief Convert a type expression to a qxir expression.
     * @details This is a 1-to-1 conversion of the type expression.
     */

    auto type = qconv(s, n->get_type());
    if (!type) {
      badtree(n, "qparse::TypeExpr::get_type() == nullptr");
      throw QError();
    }

    return type;
  }

  static Expr *qconv_templ_call(ConvState &s, const qparse::TemplCall *n) {
    /// TODO: templ_call

    throw QError();
  }

  static Expr *qconv_mut_ty(ConvState &s, const qparse::MutTy *n) {
    /// TODO: mut_ty

    throw QError();
  }

  static Expr *qconv_u1_ty(ConvState &s, const qparse::U1 *n) {
    /**
     * @brief Convert a U1 type to a qxir expression type.
     * @details This is a 1-to-1 conversion of the U1 type.
     */

    (void)n;

    return create<U1Ty>();
  }

  static Expr *qconv_u8_ty(ConvState &s, const qparse::U8 *n) {
    /**
     * @brief Convert a U8 type to a qxir expression type.
     * @details This is a 1-to-1 conversion of the U8 type.
     */

    (void)n;

    return create<U8Ty>();
  }

  static Expr *qconv_u16_ty(ConvState &s, const qparse::U16 *n) {
    /**
     * @brief Convert a U16 type to a qxir expression type.
     * @details This is a 1-to-1 conversion of the U16 type.
     */

    (void)n;

    return create<U16Ty>();
  }

  static Expr *qconv_u32_ty(ConvState &s, const qparse::U32 *n) {
    /**
     * @brief Convert a U32 type to a qxir expression type.
     * @details This is a 1-to-1 conversion of the U32 type.
     */

    (void)n;

    return create<U32Ty>();
  }

  static Expr *qconv_u64_ty(ConvState &s, const qparse::U64 *n) {
    /**
     * @brief Convert a U64 type to a qxir expression type.
     * @details This is a 1-to-1 conversion of the U64 type.
     */

    (void)n;

    return create<U64Ty>();
  }

  static Expr *qconv_u128_ty(ConvState &s, const qparse::U128 *n) {
    /**
     * @brief Convert a U128 type to a qxir expression type.
     * @details This is a 1-to-1 conversion of the U128 type.
     */

    (void)n;

    return create<U128Ty>();
  }

  static Expr *qconv_i8_ty(ConvState &s, const qparse::I8 *n) {
    /**
     * @brief Convert a I8 type to a qxir expression type.
     * @details This is a 1-to-1 conversion of the I8 type.
     */

    (void)n;

    return create<I8Ty>();
  }

  static Expr *qconv_i16_ty(ConvState &s, const qparse::I16 *n) {
    /**
     * @brief Convert a I16 type to a qxir expression type.
     * @details This is a 1-to-1 conversion of the I16 type.
     */

    (void)n;

    return create<I16Ty>();
  }

  static Expr *qconv_i32_ty(ConvState &s, const qparse::I32 *n) {
    /**
     * @brief Convert a I32 type to a qxir expression type.
     * @details This is a 1-to-1 conversion of the I32 type.
     */

    (void)n;

    return create<I32Ty>();
  }

  static Expr *qconv_i64_ty(ConvState &s, const qparse::I64 *n) {
    /**
     * @brief Convert a I64 type to a qxir expression type.
     * @details This is a 1-to-1 conversion of the I64 type.
     */

    (void)n;

    return create<I64Ty>();
  }

  static Expr *qconv_i128_ty(ConvState &s, const qparse::I128 *n) {
    /**
     * @brief Convert a I128 type to a qxir expression type.
     * @details This is a 1-to-1 conversion of the I128 type.
     */

    (void)n;

    return create<I128Ty>();
  }

  static Expr *qconv_f32_ty(ConvState &s, const qparse::F32 *n) {
    /**
     * @brief Convert a F32 type to a qxir expression type.
     * @details This is a 1-to-1 conversion of the F32 type.
     */

    (void)n;

    return create<F32Ty>();
  }

  static Expr *qconv_f64_ty(ConvState &s, const qparse::F64 *n) {
    /**
     * @brief Convert a F64 type to a qxir expression type.
     * @details This is a 1-to-1 conversion of the F64 type.
     */

    (void)n;

    return create<F64Ty>();
  }

  static Expr *qconv_void_ty(ConvState &s, const qparse::VoidTy *n) {
    /**
     * @brief Convert a Void type to a qxir expression type.
     * @details This is a 1-to-1 conversion of the Void type.
     */

    (void)n;

    return create<VoidTy>();
  }

  static Expr *qconv_ptr_ty(ConvState &s, const qparse::PtrTy *n) {
    /**
     * @brief Convert a pointer type to a qxir pointer type.
     * @details This is a 1-to-1 conversion of the pointer type.
     */

    auto pointee = qconv(s, n->get_item());
    if (!pointee) {
      badtree(n, "qparse::PtrTy::get_item() == nullptr");
      throw QError();
    }

    return create<PtrTy>(pointee->asType());
  }

  static Expr *qconv_opaque_ty(ConvState &s, const qparse::OpaqueTy *n) {
    /**
     * @brief Convert an opaque type to a qxir opaque type.
     * @details This is a 1-to-1 conversion of the opaque type.
     */

    return create<OpaqueTy>(memorize(n->get_name()));
  }

  static Expr *qconv_string_ty(ConvState &s, const qparse::StringTy *n) {
    /**
     * @brief Convert a string type to a qxir string type.
     * @details This is a 1-to-1 conversion of the string type intrinsic.
     */

    (void)n;

    return create<StringTy>();
  }

  static Expr *qconv_enum_ty(ConvState &s, const qparse::EnumTy *n) {
    /**
     * @brief Convert an enum type to a qxir enum type.
     * @details If the enum type has a member type, we convert it to a qxir
     *        type. Otherwise, we wrap it for later conversion.
     */

    auto memtype = qconv(s, n->get_memtype());
    if (memtype) {
      return memtype;
    }

    return create<Tmp>(TmpType::ENUM, memorize(n->get_name()));
  }

  static Expr *qconv_struct_ty(ConvState &s, const qparse::StructTy *n) {
    /**
     * @brief Convert a struct type to a qxir struct type.
     * @details This is a 1-to-1 conversion of the struct type.
     */

    StructFields fields;

    for (auto it = n->get_items().begin(); it != n->get_items().end(); ++it) {
      auto item = qconv(s, it->second)->asType();
      if (!item) {
        badtree(n, "qparse::StructTy::get_items() vector contains nullptr");
        throw QError();
      }

      fields.push_back(item);
    }

    return create<StructTy>(std::move(fields));
  }

  static Expr *qconv_group_ty(ConvState &s, const qparse::GroupTy *n) {
    /**
     * @brief Convert a group type to a qxir struct type.
     * @details This is a 1-to-1 conversion of the group type.
     */

    StructFields fields;

    for (auto it = n->get_items().begin(); it != n->get_items().end(); ++it) {
      auto item = qconv(s, *it)->asType();
      if (!item) {
        badtree(n, "qparse::GroupTy::get_items() vector contains nullptr");
        throw QError();
      }

      fields.push_back(item);
    }

    return create<StructTy>(std::move(fields));
  }

  static Expr *qconv_region_ty(ConvState &s, const qparse::RegionTy *n) {
    /**
     * @brief Convert a region type to a qxir struct type.
     * @details This is a 1-to-1 conversion of the region type.
     */

    StructFields fields;

    for (auto it = n->get_items().begin(); it != n->get_items().end(); ++it) {
      auto item = qconv(s, *it)->asType();
      if (!item) {
        badtree(n, "qparse::RegionTy::get_items() vector contains nullptr");
        throw QError();
      }

      fields.push_back(item);
    }

    return create<StructTy>(std::move(fields));
  }

  static Expr *qconv_union_ty(ConvState &s, const qparse::UnionTy *n) {
    /**
     * @brief Convert a union type to a qxir struct type.
     * @details This is a 1-to-1 conversion of the union type.
     */

    UnionFields fields;

    for (auto it = n->get_items().begin(); it != n->get_items().end(); ++it) {
      auto item = qconv(s, *it)->asType();
      if (!item) {
        badtree(n, "qparse::UnionTy::get_items() vector contains nullptr");
        throw QError();
      }

      fields.push_back(item);
    }

    return create<UnionTy>(std::move(fields));
  }

  static Expr *qconv_array_ty(ConvState &s, const qparse::ArrayTy *n) {
    /**
     * @brief Convert an array type to a qxir array type.
     * @details This is a 1-to-1 conversion of the array type.
     */

    auto item = qconv(s, n->get_item());
    if (!item) {
      badtree(n, "qparse::ArrayTy::get_item() == nullptr");
      throw QError();
    }

    auto count = qconv(s, n->get_size());
    if (!count) {
      badtree(n, "qparse::ArrayTy::get_size() == nullptr");
      throw QError();
    }

    return create<ArrayTy>(item->asType(), count);
  }

  static Expr *qconv_vector_ty(ConvState &s, const qparse::VectorTy *n) {
    /**
     * @brief Convert a vector type to a qxir vector type.
     * @details This is a 1-to-1 conversion of the vector type.
     */

    auto item = qconv(s, n->get_item());
    if (!item) {
      badtree(n, "qparse::VectorTy::get_item() == nullptr");
      throw QError();
    }

    return create<ListTy>(item->asType());
  }

  static Expr *qconv_map_ty(ConvState &s, const qparse::MapTy *n) {
    /// TODO: map_ty

    throw QError();
  }

  static Expr *qconv_tuple_ty(ConvState &s, const qparse::TupleTy *n) {
    /**
     * @brief Convert a tuple type to a qxir struct type.
     * @details This is a 1-to-1 conversion of the tuple type.
     */

    StructFields fields;
    for (auto it = n->get_items().begin(); it != n->get_items().end(); ++it) {
      auto item = qconv(s, *it)->asType();
      if (!item) {
        badtree(n, "qparse::TupleTy::get_items() vector contains nullptr");
        throw QError();
      }

      fields.push_back(item);
    }

    return create<StructTy>(std::move(fields));
  }

  static Expr *qconv_set_ty(ConvState &s, const qparse::SetTy *n) {
    /// TODO: set_ty

    throw QError();
  }

  static Expr *qconv_result_ty(ConvState &s, const qparse::OptionalTy *n) {
    /// TODO: result_ty

    throw QError();
  }

  static Expr *qconv_fn_ty(ConvState &s, const qparse::FuncTy *n) {
    /**
     * @brief Convert a function type to a qxir function type.
     * @details Order of function parameters is consistent with their order
     * of declaration. Support for `optional` arguments is up to the frontend,
     * for inter-language compatibility, the ABI is not concerned with optional
     * arguments as they have no significance at the binary interface level.
     */

    FnParams params;

    for (auto it = n->get_params().begin(); it != n->get_params().end(); ++it) {
      Type *type = qconv(s, std::get<1>(*it))->asType();
      if (!type) {
        badtree(n, "qparse::FnDef::get_type() == nullptr");
        throw QError();
      }

      params.push_back(type);
    }

    Type *ret = qconv(s, n->get_return_ty())->asType();
    if (!ret) {
      badtree(n, "qparse::FnDef::get_ret() == nullptr");
      throw QError();
    }

    FnAttrs attrs;
    /// TODO: Function attributes

    return create<FnTy>(std::move(params), ret, std::move(attrs));
  }

  static Expr *qconv_unres_ty(ConvState &s, const qparse::UnresolvedType *n) {
    /**
     * @brief Convert an unresolved type to a qxir type.
     * @details This is a 1-to-1 conversion of the unresolved type.
     */

    return create<Tmp>(TmpType::NAMED_TYPE, memorize(n->get_name()));
  }

  static Expr *qconv_infer_ty(ConvState &s, const qparse::InferType *n) {
    /// TODO:
    throw QError();
  }

  static Expr *qconv_typedef(ConvState &s, const qparse::TypedefDecl *n) {
    /**
     * @brief Memorize a typedef declaration which will be used later for type resolution.
     * @details This node will resolve to type void.
     */

    auto name = s.cur_named(n->get_name());

    auto type = qconv(s, n->get_type());
    if (!type) {
      badtree(n, "qparse::TypedefDecl::get_type() == nullptr");
      throw QError();
    }

    s.typedef_map[std::move(name)] = type->asType();

    return create<VoidTy>();
  }

  static Expr *qconv_fndecl(ConvState &s, const qparse::FnDecl *n) {
    /// TODO: fndecl

    throw QError();
  }

  static Expr *qconv_struct(ConvState &s, const qparse::StructDef *n) {
    /// TODO: struct

    throw QError();
  }

  static Expr *qconv_region(ConvState &s, const qparse::RegionDef *n) {
    /// TODO: region

    throw QError();

    // IR struct type is a bit-packed structure composite; fields are nameless
    // and are accessed by index. The QXIR region definition has named fields
    // as well as embedded methods / metadata.

    /**
     * 1. Create a vector of QXIR type fields.
     * 2. Create a IR sequence items vector.
     * 3. Iterate over the fields of the region definition.
     * 3.1. Convert the field type to a qxir type.
     * 3.2. Public the (field_name, field_index) to the global context
     * 3.3. Add the field to the vector of fields.
     * 4. Create a QXIR struct type with the vector of fields.
     * 5. Iterate over the methods of the region definition.
     * 5.1. Convert the method to a qxir function.
     * 5.2. Append the IR function to the sequence items vector.
     * 6. Iterate over the static methods of the region definition.
     * 6.1. Convert the static method to a qxir function.
     * 6.2. Append the IR function to the sequence items vector.
     * 7. Create a QXIR SEQ node with the sequence items vector as well as the struct type.
     * 8. Return the SEQ node.
     */
  }

  static Expr *qconv_group(ConvState &s, const qparse::GroupDef *n) {
    /// TODO: group

    throw QError();
  }

  static Expr *qconv_union(ConvState &s, const qparse::UnionDef *n) {
    /// TODO: union

    throw QError();
  }

  static Expr *qconv_enum(ConvState &s, const qparse::EnumDef *n) {
    /// TODO: enum

    throw QError();
  }

  static Expr *qconv_fn(ConvState &s, const qparse::FnDef *n) {
    /**
     * /// TODO: Write a detailed description of the function conversion process.
     */

    Expr *precond = nullptr, *postcond = nullptr;
    Seq *body = nullptr;
    Params params;
    const qparse::FnDecl *decl = n;
    qparse::FuncTy *fty = decl->get_type();

    /* Produce the function preconditions */
    if ((precond = qconv(s, n->get_precond()))) {
      Ident *abort_name = create<Ident>("__qprecond_fail");
      Call *abort_call = create<Call>(abort_name, CallArgs());

      precond = create<If>(create<UnExpr>(precond, Op::LogicNot), abort_call, create<VoidTy>());
    }

    /* Produce the function postconditions */
    if ((postcond = qconv(s, n->get_postcond()))) {
      Ident *abort_name = create<Ident>("__qpostcond_fail");
      Call *abort_call = create<Call>(abort_name, CallArgs());

      postcond = create<If>(create<UnExpr>(postcond, Op::LogicNot), abort_call, create<VoidTy>());
    }

    { /* Produce the function body */
      Expr *tmp = qconv(s, n->get_body());
      if (!tmp) {
        badtree(n, "qparse::FnDef::get_body() == nullptr");
        throw QError();
      }

      if (tmp->getKind() != QIR_NODE_SEQ) {
        tmp = create<Seq>(SeqItems({tmp}));
      }

      body = tmp->as<Seq>();

      if (precond) {
        body->getItems().insert(body->getItems().begin(), precond);
      }
      if (postcond) {
        /// TODO: add postcond at each exit point
      }
    }

    auto name = s.cur_named(n->get_name());

    { /* Produce the function parameters */
      for (auto it = fty->get_params().begin(); it != fty->get_params().end(); ++it) {
        /**
         * Parameter properties:
         * 1. Name - All parameters have a name.
         * 2. Type - All parameters have a type.
         * 3. Default - Optional, if the parameter has a default value.
         * 4. Position - All parameters have a position.
         */

        Type *type = qconv(s, std::get<1>(*it))->asType();
        if (!type) {
          badtree(n, "qparse::FnDef::get_type() == nullptr");
          throw QError();
        }

        Expr *def = nullptr;
        if (std::get<2>(*it)) {
          def = qconv(s, std::get<2>(*it));
          if (!def) {
            badtree(n, "qparse::FnDef::get_type() == nullptr");
            throw QError();
          }
        }

        params.push_back(type);
        s.param_map[name].push_back({s.cur_named(std::get<0>(*it)), type, def});
      }
    }

    return create<Fn>(memorize(std::string_view(name)), std::move(params), body);
  }

  static Expr *qconv_subsystem(ConvState &s, const qparse::SubsystemDecl *n) {
    /**
     * @brief Convert a subsystem declaration to a qxir sequence with
     * namespace prefixes.
     */

    SeqItems items;

    if (!n->get_body()) {
      badtree(n, "qparse::SubsystemDecl::get_body() == nullptr");
      throw QError();
    }

    std::string old_ns = s.ns_prefix;

    if (s.ns_prefix.empty()) {
      s.ns_prefix = std::string(n->get_name());
    } else {
      s.ns_prefix += "::" + std::string(n->get_name());
    }

    for (auto it = n->get_body()->get_items().begin(); it != n->get_body()->get_items().end();
         ++it) {
      auto item = qconv(s, *it);
      if (!item) {
        badtree(n, "qparse::SubsystemDecl::get_body() vector contains nullptr");
        throw QError();
      }

      items.push_back(item);
    }

    s.ns_prefix = old_ns;

    return create<Seq>(std::move(items));
  }

  static Expr *qconv_export(ConvState &s, const qparse::ExportDecl *n) {
    /**
     * @brief Convert an export declaration to a qxir export node.
     * @details Convert a list of statements under a common ABI into a
     * sequence under a common ABI.
     */

    static thread_local size_t name_ctr = 0;

    SeqItems items;

    if (!n->get_body()) {
      badtree(n, "qparse::ExportDecl::get_body() == nullptr");
      throw QError();
    }

    for (auto it = n->get_body()->get_items().begin(); it != n->get_body()->get_items().end();
         ++it) {
      auto item = qconv(s, *it);
      if (!item) {
        badtree(n, "qparse::ExportDecl::get_body() vector contains nullptr");
        throw QError();
      }

      items.push_back(item);
    }

    Seq *seq = create<Seq>(std::move(items));
    std::string name = std::string("__pub") + std::to_string(name_ctr++);

    return create<Export>(memorize(std::string_view(name)), seq, memorize(n->get_abi_name()));
  }

  static Expr *qconv_composite_field(ConvState &s, const qparse::CompositeField *n) {
    /// TODO: composite_field

    throw QError();
  }

  static Expr *qconv_block(ConvState &s, const qparse::Block *n) {
    /**
     * @brief Convert a scope block into an expression sequence.
     * @details A QXIR sequence is a list of expressions (a sequence point).
     *          This is equivalent to a scope block.
     */

    SeqItems items;

    for (auto it = n->get_items().begin(); it != n->get_items().end(); ++it) {
      auto item = qconv(s, *it);
      if (!item) {
        badtree(n, "qparse::Block::get_items() vector contains nullptr");
        throw QError();
      }

      items.push_back(item);
    }

    return create<Seq>(std::move(items));
  }

  static Expr *qconv_const(ConvState &s, const qparse::ConstDecl *n) {
    /// TODO: Write explanation

    if (!n->get_value()) {
      badtree(n, "parse::ConstDecl::get_value() == nullptr");
      throw QError();
    }

    auto val = qconv(s, n->get_value());
    if (!val) {
      badtree(n, "qparse::ConstDecl::get_value() == nullptr");
      throw QError();
    }

    Type *type = nullptr;
    if (n->get_type()) {
      type = qconv(s, n->get_type())->asType();
      if (!type) {
        badtree(n, "qparse::ConstDecl::get_type() == nullptr");
        throw QError();
      }

      val = create<BinExpr>(val, type, Op::CastAs);
    }

    std::string_view name;

    if (s.inside_function) {
      name = memorize(n->get_name());
    } else {
      name = memorize(std::string_view(s.cur_named(n->get_name())));
    }

    auto g = create<Local>(name, val);
    g->setConst(true);
    return g;
  }

  static Expr *qconv_var(ConvState &s, const qparse::VarDecl *n) {
    /// TODO: var

    throw QError();
  }

  static Expr *qconv_let(ConvState &s, const qparse::LetDecl *n) {
    /// TODO: Write explanation

    if (!n->get_type() && !n->get_value()) {
      badtree(n,
              "qparse::LetDecl::get_type() == nullptr && qparse::LetDecl::get_value() == nullptr");
      throw QError();
    }

    if (s.inside_function) {
      if (!n->get_type()) {
        auto val = qconv(s, n->get_value());
        if (!val) {
          badtree(n, "qparse::LetDecl::get_value() == nullptr");
          throw QError();
        }

        LetTmpNodeCradle datapack;
        std::get<0>(datapack) = memorize(n->get_name());
        std::get<1>(datapack) = val;

        return create<Tmp>(TmpType::LET, std::move(datapack));
      } else {
        auto type = qconv(s, n->get_type());
        if (!type) {
          badtree(n, "qparse::LetDecl::get_type() == nullptr");
          throw QError();
        }

        auto alloc = create<Alloc>(type->asType());

        if (n->get_value()) {
          auto val = qconv(s, n->get_value());
          if (!val) {
            badtree(n, "qparse::LetDecl::get_value() == nullptr");
            throw QError();
          }

          auto assign = create<BinExpr>(alloc, val, Op::Set);

          return create<Seq>(SeqItems({alloc, assign}));
        } else {
          return alloc;
        }
      }
    } else {
      Type *type = nullptr;
      if (n->get_type()) {
        type = qconv(s, n->get_type())->asType();
        if (!type) {
          badtree(n, "qparse::LetDecl::get_type() == nullptr");
          throw QError();
        }
      }

      Expr *val = nullptr;
      if (n->get_value()) {
        val = qconv(s, n->get_value());
        if (!val) {
          badtree(n, "qparse::LetDecl::get_value() == nullptr");
          throw QError();
        }
      }

      if (!val) {
        val = create<Tmp>(TmpType::UNDEF_LITERAL);
      }

      if (type) {
        val = create<BinExpr>(val, type, Op::CastAs);
      }

      std::string_view name = memorize(std::string_view(s.cur_named(n->get_name())));

      return create<Local>(name, val);
    }
  }

  static Expr *qconv_inline_asm(ConvState &s, const qparse::InlineAsm *n) {
    qcore_implement("qconv_inline_asm");
  }

  static Expr *qconv_return(ConvState &s, const qparse::ReturnStmt *n) {
    /**
     * @brief Convert a return statement to a qxir expression.
     * @details This is a 1-to-1 conversion of the return statement.
     */

    auto val = qconv(s, n->get_value());
    if (!val) {
      val = create<VoidTy>();
    }

    return create<Ret>(val);
  }

  static Expr *qconv_retif(ConvState &s, const qparse::ReturnIfStmt *n) {
    /**
     * @brief Convert a return statement to a qxir expression.
     * @details Lower into an 'if (cond) {return val}' expression.
     */

    auto cond = qconv(s, n->get_cond());
    if (!cond) {
      badtree(n, "qparse::ReturnIfStmt::get_cond() == nullptr");
      throw QError();
    }

    auto val = qconv(s, n->get_value());
    if (!val) {
      badtree(n, "qparse::ReturnIfStmt::get_value() == nullptr");
      throw QError();
    }

    return create<If>(cond, create<Ret>(val), create<VoidTy>());
  }

  static Expr *qconv_retz(ConvState &s, const qparse::RetZStmt *n) {
    /**
     * @brief Convert a return statement to a qxir expression.
     * @details Lower into an 'if (!cond) {return val}' expression.
     */

    auto cond = qconv(s, n->get_cond());
    if (!cond) {
      badtree(n, "qparse::RetZStmt::get_cond() == nullptr");
      throw QError();
    }

    auto inv_cond = create<UnExpr>(cond, Op::LogicNot);

    auto val = qconv(s, n->get_value());
    if (!val) {
      badtree(n, "qparse::RetZStmt::get_value() == nullptr");
      throw QError();
    }

    return create<If>(inv_cond, create<Ret>(val), create<VoidTy>());
  }

  static Expr *qconv_retv(ConvState &s, const qparse::RetVStmt *n) {
    /**
     * @brief Convert a return statement to a qxir expression.
     * @details Lower into an 'if (cond) {return void}' expression.
     */

    auto cond = qconv(s, n->get_cond());
    if (!cond) {
      badtree(n, "qparse::RetVStmt::get_cond() == nullptr");
      throw QError();
    }

    return create<If>(cond, create<Ret>(create<VoidTy>()), create<VoidTy>());
  }

  static Expr *qconv_break(ConvState &s, const qparse::BreakStmt *n) {
    /**
     * @brief Convert a break statement to a qxir expression.
     * @details This is a 1-to-1 conversion of the break statement.
     */

    return create<Brk>();
  }

  static Expr *qconv_continue(ConvState &s, const qparse::ContinueStmt *n) {
    /**
     * @brief Convert a continue statement to a qxir expression.
     * @details This is a 1-to-1 conversion of the continue statement.
     */

    return create<Cont>();
  }

  static Expr *qconv_if(ConvState &s, const qparse::IfStmt *n) {
    /**
     * @brief Convert an if statement to a qxir expression.
     * @details The else branch is optional, and if it is missing, it is
     *        replaced with a void expression.
     */

    auto cond = qconv(s, n->get_cond());
    auto then = qconv(s, n->get_then());
    auto els = qconv(s, n->get_else());

    if (!cond) {
      badtree(n, "qparse::IfStmt::get_cond() == nullptr");
      throw QError();
    }

    if (!then) {
      badtree(n, "qparse::IfStmt::get_then() == nullptr");
      throw QError();
    }

    if (!els) {
      els = create<VoidTy>();
    }

    return create<If>(cond, then, els);
  }

  static Expr *qconv_while(ConvState &s, const qparse::WhileStmt *n) {
    /**
     * @brief Convert a while loop to a qxir expression.
     * @details If any of the sub-expressions are missing, they are replaced
     *         with a default value of 1.
     */

    auto cond = qconv(s, n->get_cond());
    auto body = qconv(s, n->get_body());

    if (!cond) {
      cond = create<Int>(1);
    }

    if (!body) {
      body = create<Int>(1);
    }

    return create<While>(cond, body);
  }

  static Expr *qconv_for(ConvState &s, const qparse::ForStmt *n) {
    /**
     * @brief Convert a for loop to a qxir expression.
     * @details If any of the sub-expressions are missing, they are replaced
     *         with a default value of 1.
     */

    auto init = qconv(s, n->get_init());
    auto cond = qconv(s, n->get_cond());
    auto step = qconv(s, n->get_step());
    auto body = qconv(s, n->get_body());

    if (!init) {
      init = create<Int>(1);
    }

    if (!cond) {
      cond = create<Int>(1);  // infinite loop like 'for (;;) {}'
    }

    if (!step) {
      step = create<Int>(1);
    }

    if (!body) {
      body = create<Int>(1);
    }

    return create<For>(init, cond, step, body);
  }

  static Expr *qconv_form(ConvState &s, const qparse::FormStmt *n) {
    /**
     * @brief Convert a form loop to a qxir expression.
     * @details This is a 1-to-1 conversion of the form loop.
     */

    auto maxjobs = qconv(s, n->get_maxjobs());
    if (!maxjobs) {
      badtree(n, "qparse::FormStmt::get_maxjobs() == nullptr");
      throw QError();
    }

    auto idx_name = memorize(n->get_idx_ident());
    auto val_name = memorize(n->get_val_ident());

    auto iter = qconv(s, n->get_expr());
    if (!iter) {
      badtree(n, "qparse::FormStmt::get_expr() == nullptr");
      throw QError();
    }

    auto body = qconv(s, n->get_body());
    if (!body) {
      badtree(n, "qparse::FormStmt::get_body() == nullptr");
      throw QError();
    }

    return create<Form>(idx_name, val_name, maxjobs, iter, create<Seq>(SeqItems({body})));
  }

  static Expr *qconv_foreach(ConvState &s, const qparse::ForeachStmt *n) {
    /**
     * @brief Convert a foreach loop to a qxir expression.
     * @details This is a 1-to-1 conversion of the foreach loop.
     */

    auto idx_name = memorize(n->get_idx_ident());
    auto val_name = memorize(n->get_val_ident());

    auto iter = qconv(s, n->get_expr());
    if (!iter) {
      badtree(n, "qparse::ForeachStmt::get_expr() == nullptr");
      throw QError();
    }

    auto body = qconv(s, n->get_body());
    if (!body) {
      badtree(n, "qparse::ForeachStmt::get_body() == nullptr");
      throw QError();
    }

    return create<Foreach>(idx_name, val_name, iter, create<Seq>(SeqItems({body})));
  }

  static Expr *qconv_case(ConvState &s, const qparse::CaseStmt *n) {
    /**
     * @brief Convert a case statement to a qxir expression.
     * @details This is a 1-to-1 conversion of the case statement.
     */

    auto cond = qconv(s, n->get_cond());
    if (!cond) {
      badtree(n, "qparse::CaseStmt::get_cond() == nullptr");
      throw QError();
    }

    auto body = qconv(s, n->get_body());
    if (!body) {
      badtree(n, "qparse::CaseStmt::get_body() == nullptr");
      throw QError();
    }

    return create<Case>(cond, body);
  }

  static Expr *qconv_switch(ConvState &s, const qparse::SwitchStmt *n) {
    /**
     * @brief Convert a switch statement to a qxir expression.
     * @details If the default case is missing, it is replaced with a void
     *        expression.
     */

    auto cond = qconv(s, n->get_cond());
    if (!cond) {
      badtree(n, "qparse::SwitchStmt::get_cond() == nullptr");
      throw QError();
    }

    SwitchCases cases;
    for (auto it = n->get_cases().begin(); it != n->get_cases().end(); ++it) {
      auto item = qconv(s, *it);
      if (!item) {
        badtree(n, "qparse::SwitchStmt::get_cases() vector contains nullptr");
        throw QError();
      }

      cases.push_back(item->as<Case>());
    }

    Expr *def = nullptr;
    if (n->get_default()) {
      def = qconv(s, n->get_default());
      if (!def) {
        badtree(n, "qparse::SwitchStmt::get_default() == nullptr");
        throw QError();
      }
    } else {
      def = create<VoidTy>();
    }

    return create<Switch>(cond, std::move(cases), def);
  }

  static Expr *qconv_expr_stmt(ConvState &s, const qparse::ExprStmt *n) {
    /**
     * @brief Convert an expression inside a statement to a qxir expression.
     * @details This is a 1-to-1 conversion of the expression statement.
     */

    return qconv(s, n->get_expr());
  }

  static Expr *qconv_volstmt(ConvState &s, const qparse::VolStmt *n) {
    /**
     * @brief Convert a volatile statement to a qxir volatile expression.
     * @details This is a 1-to-1 conversion of the volatile statement.
     */

    auto expr = qconv(s, n->get_stmt());
    expr->setVolatile(true);

    return expr;
  }
}  // namespace qxir

static qxir::Expr *qconv(ConvState &s, const qparse::Node *n) {
  using namespace qxir;

  if (!n) {
    return nullptr;
  }

  qxir::Expr *out = nullptr;

  switch (n->this_typeid()) {
    case QAST_NODE_CEXPR:
      out = qconv_cexpr(s, n->as<qparse::ConstExpr>());
      break;

    case QAST_NODE_BINEXPR:
      out = qconv_binexpr(s, n->as<qparse::BinExpr>());
      break;

    case QAST_NODE_UNEXPR:
      out = qconv_unexpr(s, n->as<qparse::UnaryExpr>());
      break;

    case QAST_NODE_TEREXPR:
      out = qconv_terexpr(s, n->as<qparse::TernaryExpr>());
      break;

    case QAST_NODE_INT:
      out = qconv_int(s, n->as<qparse::ConstInt>());
      break;

    case QAST_NODE_FLOAT:
      out = qconv_float(s, n->as<qparse::ConstFloat>());
      break;

    case QAST_NODE_STRING:
      out = qconv_string(s, n->as<qparse::ConstString>());
      break;

    case QAST_NODE_CHAR:
      out = qconv_char(s, n->as<qparse::ConstChar>());
      break;

    case QAST_NODE_BOOL:
      out = qconv_bool(s, n->as<qparse::ConstBool>());
      break;

    case QAST_NODE_NULL:
      out = qconv_null(s, n->as<qparse::ConstNull>());
      break;

    case QAST_NODE_UNDEF:
      out = qconv_undef(s, n->as<qparse::ConstUndef>());
      break;

    case QAST_NODE_CALL:
      out = qconv_call(s, n->as<qparse::Call>());
      break;

    case QAST_NODE_LIST:
      out = qconv_list(s, n->as<qparse::List>());
      break;

    case QAST_NODE_ASSOC:
      out = qconv_assoc(s, n->as<qparse::Assoc>());
      break;

    case QAST_NODE_FIELD:
      out = qconv_field(s, n->as<qparse::Field>());
      break;

    case QAST_NODE_INDEX:
      out = qconv_index(s, n->as<qparse::Index>());
      break;

    case QAST_NODE_SLICE:
      out = qconv_slice(s, n->as<qparse::Slice>());
      break;

    case QAST_NODE_FSTRING:
      out = qconv_fstring(s, n->as<qparse::FString>());
      break;

    case QAST_NODE_IDENT:
      out = qconv_ident(s, n->as<qparse::Ident>());
      break;

    case QAST_NODE_SEQ_POINT:
      out = qconv_seq_point(s, n->as<qparse::SeqPoint>());
      break;

    case QAST_NODE_POST_UNEXPR:
      out = qconv_post_unexpr(s, n->as<qparse::PostUnaryExpr>());
      break;

    case QAST_NODE_STMT_EXPR:
      out = qconv_stmt_expr(s, n->as<qparse::StmtExpr>());
      break;

    case QAST_NODE_TYPE_EXPR:
      out = qconv_type_expr(s, n->as<qparse::TypeExpr>());
      break;

    case QAST_NODE_TEMPL_CALL:
      out = qconv_templ_call(s, n->as<qparse::TemplCall>());
      break;

    case QAST_NODE_MUT_TY:
      out = qconv_mut_ty(s, n->as<qparse::MutTy>());
      break;

    case QAST_NODE_U1_TY:
      out = qconv_u1_ty(s, n->as<qparse::U1>());
      break;

    case QAST_NODE_U8_TY:
      out = qconv_u8_ty(s, n->as<qparse::U8>());
      break;

    case QAST_NODE_U16_TY:
      out = qconv_u16_ty(s, n->as<qparse::U16>());
      break;

    case QAST_NODE_U32_TY:
      out = qconv_u32_ty(s, n->as<qparse::U32>());
      break;

    case QAST_NODE_U64_TY:
      out = qconv_u64_ty(s, n->as<qparse::U64>());
      break;

    case QAST_NODE_U128_TY:
      out = qconv_u128_ty(s, n->as<qparse::U128>());
      break;

    case QAST_NODE_I8_TY:
      out = qconv_i8_ty(s, n->as<qparse::I8>());
      break;

    case QAST_NODE_I16_TY:
      out = qconv_i16_ty(s, n->as<qparse::I16>());
      break;

    case QAST_NODE_I32_TY:
      out = qconv_i32_ty(s, n->as<qparse::I32>());
      break;

    case QAST_NODE_I64_TY:
      out = qconv_i64_ty(s, n->as<qparse::I64>());
      break;

    case QAST_NODE_I128_TY:
      out = qconv_i128_ty(s, n->as<qparse::I128>());
      break;

    case QAST_NODE_F32_TY:
      out = qconv_f32_ty(s, n->as<qparse::F32>());
      break;

    case QAST_NODE_F64_TY:
      out = qconv_f64_ty(s, n->as<qparse::F64>());
      break;

    case QAST_NODE_VOID_TY:
      out = qconv_void_ty(s, n->as<qparse::VoidTy>());
      break;

    case QAST_NODE_PTR_TY:
      out = qconv_ptr_ty(s, n->as<qparse::PtrTy>());
      break;

    case QAST_NODE_OPAQUE_TY:
      out = qconv_opaque_ty(s, n->as<qparse::OpaqueTy>());
      break;

    case QAST_NODE_STRING_TY:
      out = qconv_string_ty(s, n->as<qparse::StringTy>());
      break;

    case QAST_NODE_ENUM_TY:
      out = qconv_enum_ty(s, n->as<qparse::EnumTy>());
      break;

    case QAST_NODE_STRUCT_TY:
      out = qconv_struct_ty(s, n->as<qparse::StructTy>());
      break;

    case QAST_NODE_GROUP_TY:
      out = qconv_group_ty(s, n->as<qparse::GroupTy>());
      break;

    case QAST_NODE_REGION_TY:
      out = qconv_region_ty(s, n->as<qparse::RegionTy>());
      break;

    case QAST_NODE_UNION_TY:
      out = qconv_union_ty(s, n->as<qparse::UnionTy>());
      break;

    case QAST_NODE_ARRAY_TY:
      out = qconv_array_ty(s, n->as<qparse::ArrayTy>());
      break;

    case QAST_NODE_VECTOR_TY:
      out = qconv_vector_ty(s, n->as<qparse::VectorTy>());
      break;

    case QAST_NODE_MAP_TY:
      out = qconv_map_ty(s, n->as<qparse::MapTy>());
      break;

    case QAST_NODE_TUPLE_TY:
      out = qconv_tuple_ty(s, n->as<qparse::TupleTy>());
      break;

    case QAST_NODE_SET_TY:
      out = qconv_set_ty(s, n->as<qparse::SetTy>());
      break;

    case QAST_NODE_RESULT_TY:
      out = qconv_result_ty(s, n->as<qparse::OptionalTy>());
      break;

    case QAST_NODE_FN_TY:
      out = qconv_fn_ty(s, n->as<qparse::FuncTy>());
      break;

    case QAST_NODE_UNRES_TY:
      out = qconv_unres_ty(s, n->as<qparse::UnresolvedType>());
      break;

    case QAST_NODE_INFER_TY:
      out = qconv_infer_ty(s, n->as<qparse::InferType>());
      break;

    case QAST_NODE_TYPEDEF:
      out = qconv_typedef(s, n->as<qparse::TypedefDecl>());
      break;

    case QAST_NODE_FNDECL:
      out = qconv_fndecl(s, n->as<qparse::FnDecl>());
      break;

    case QAST_NODE_STRUCT:
      out = qconv_struct(s, n->as<qparse::StructDef>());
      break;

    case QAST_NODE_REGION:
      out = qconv_region(s, n->as<qparse::RegionDef>());
      break;

    case QAST_NODE_GROUP:
      out = qconv_group(s, n->as<qparse::GroupDef>());
      break;

    case QAST_NODE_UNION:
      out = qconv_union(s, n->as<qparse::UnionDef>());
      break;

    case QAST_NODE_ENUM:
      out = qconv_enum(s, n->as<qparse::EnumDef>());
      break;

    case QAST_NODE_FN:
      out = qconv_fn(s, n->as<qparse::FnDef>());
      break;

    case QAST_NODE_SUBSYSTEM:
      out = qconv_subsystem(s, n->as<qparse::SubsystemDecl>());
      break;

    case QAST_NODE_EXPORT:
      out = qconv_export(s, n->as<qparse::ExportDecl>());
      break;

    case QAST_NODE_COMPOSITE_FIELD:
      out = qconv_composite_field(s, n->as<qparse::CompositeField>());
      break;

    case QAST_NODE_BLOCK:
      out = qconv_block(s, n->as<qparse::Block>());
      break;

    case QAST_NODE_CONST:
      out = qconv_const(s, n->as<qparse::ConstDecl>());
      break;

    case QAST_NODE_VAR:
      out = qconv_var(s, n->as<qparse::VarDecl>());
      break;

    case QAST_NODE_LET:
      out = qconv_let(s, n->as<qparse::LetDecl>());
      break;

    case QAST_NODE_INLINE_ASM:
      out = qconv_inline_asm(s, n->as<qparse::InlineAsm>());
      break;

    case QAST_NODE_RETURN:
      out = qconv_return(s, n->as<qparse::ReturnStmt>());
      break;

    case QAST_NODE_RETIF:
      out = qconv_retif(s, n->as<qparse::ReturnIfStmt>());
      break;

    case QAST_NODE_RETZ:
      out = qconv_retz(s, n->as<qparse::RetZStmt>());
      break;

    case QAST_NODE_RETV:
      out = qconv_retv(s, n->as<qparse::RetVStmt>());
      break;

    case QAST_NODE_BREAK:
      out = qconv_break(s, n->as<qparse::BreakStmt>());
      break;

    case QAST_NODE_CONTINUE:
      out = qconv_continue(s, n->as<qparse::ContinueStmt>());
      break;

    case QAST_NODE_IF:
      out = qconv_if(s, n->as<qparse::IfStmt>());
      break;

    case QAST_NODE_WHILE:
      out = qconv_while(s, n->as<qparse::WhileStmt>());
      break;

    case QAST_NODE_FOR:
      out = qconv_for(s, n->as<qparse::ForStmt>());
      break;

    case QAST_NODE_FORM:
      out = qconv_form(s, n->as<qparse::FormStmt>());
      break;

    case QAST_NODE_FOREACH:
      out = qconv_foreach(s, n->as<qparse::ForeachStmt>());
      break;

    case QAST_NODE_CASE:
      out = qconv_case(s, n->as<qparse::CaseStmt>());
      break;

    case QAST_NODE_SWITCH:
      out = qconv_switch(s, n->as<qparse::SwitchStmt>());
      break;

    case QAST_NODE_EXPR_STMT:
      out = qconv_expr_stmt(s, n->as<qparse::ExprStmt>());
      break;

    case QAST_NODE_VOLSTMT:
      out = qconv_volstmt(s, n->as<qparse::VolStmt>());
      break;

    default: {
      qcore_panicf("qxir: unknown node type: %d", static_cast<int>(n->this_typeid()));
    }
  }

  if (!out) {
    qcore_panicf("qxir: conversion failed for node type: %d", static_cast<int>(n->this_typeid()));
  }

  // Module context id must be assigned before location information
  // is set.
  out->setModule(qxir_ctx);

  out->setLoc({n->get_start_pos(), n->get_end_pos()});

  return out;
}

///=============================================================================

static qxir_node_t *qxir_clone_impl(const qxir_node_t *_node) {
#define clone(X) static_cast<Expr *>(qxir_clone_impl(X))

  using namespace qxir;

  Expr *in;
  Expr *out;

  if (!_node) {
    return nullptr;
  }

  in = static_cast<Expr *>(const_cast<qxir_node_t *>(_node));
  out = nullptr;

  switch (in->getKind()) {
    case QIR_NODE_BINEXPR: {
      BinExpr *n = static_cast<BinExpr *>(in);
      out = create<BinExpr>(clone(n->getLHS()), clone(n->getRHS()), n->getOp());
      break;
    }
    case QIR_NODE_UNEXPR: {
      UnExpr *n = static_cast<UnExpr *>(in);
      out = create<UnExpr>(clone(n->getExpr()), n->getOp());
      break;
    }
    case QIR_NODE_POST_UNEXPR: {
      PostUnExpr *n = static_cast<PostUnExpr *>(in);
      out = create<PostUnExpr>(clone(n->getExpr()), n->getOp());
      break;
    }
    case QIR_NODE_INT: {
      Int *n = static_cast<Int *>(in);
      if (n->isNativeRepresentation()) {
        out = create<Int>(n->getNativeRepresentation());
      } else {
        out = create<Int>(memorize(n->getStringRepresentation()));
      }
      break;
    }
    case QIR_NODE_FLOAT: {
      Float *n = static_cast<Float *>(in);
      if (n->isNativeRepresentation()) {
        out = create<Float>(n->getNativeRepresentation());
      } else {
        out = create<Float>(memorize(n->getStringRepresentation()));
      }
      break;
    }
    case QIR_NODE_STRING: {
      out = create<String>(memorize(static_cast<String *>(in)->getValue()));
      break;
    }
    case QIR_NODE_LIST: {
      ListItems items;
      for (auto item : static_cast<List *>(in)->getItems()) {
        items.push_back(clone(item));
      }
      out = create<List>(std::move(items));
      break;
    }
    case QIR_NODE_ALLOC: {
      out = create<Alloc>(clone(static_cast<Alloc *>(in)->getAllocType())->asType());
      break;
    }
    case QIR_NODE_CALL: {
      Call *n = static_cast<Call *>(in);
      CallArgs args;
      for (auto arg : n->getArgs()) {
        args.push_back(clone(arg));
      }
      out = create<Call>(clone(n->getFn()), std::move(args));
      break;
    }
    case QIR_NODE_SEQ: {
      SeqItems items;
      for (auto item : static_cast<Seq *>(in)->getItems()) {
        items.push_back(clone(item));
      }
      out = create<Seq>(std::move(items));
      break;
    }
    case QIR_NODE_ASYNC: {
      AsyncItems items;
      for (auto item : static_cast<Async *>(in)->getItems()) {
        items.push_back(clone(item));
      }
      out = create<Async>(std::move(items));
      break;
    }
    case QIR_NODE_INDEX: {
      Index *n = static_cast<Index *>(in);
      out = create<Index>(clone(n->getExpr()), clone(n->getIndex()));
      break;
    }
    case QIR_NODE_IDENT: {
      out = create<Ident>(memorize(static_cast<Ident *>(in)->getName()));
      break;
    }
    case QIR_NODE_EXPORT: {
      Export *n = static_cast<Export *>(in);
      out = create<Export>(memorize(n->getName()), clone(n->getValue()));
      break;
    }
    case QIR_NODE_LOCAL: {
      Local *n = static_cast<Local *>(in);
      out = create<Local>(memorize(n->getName()), clone(n->getValue()));
      break;
    }
    case QIR_NODE_RET: {
      out = create<Ret>(clone(static_cast<Ret *>(in)->getExpr()));
      break;
    }
    case QIR_NODE_BRK: {
      out = create<Brk>();
      break;
    }
    case QIR_NODE_CONT: {
      out = create<Cont>();
      break;
    }
    case QIR_NODE_IF: {
      If *n = static_cast<If *>(in);
      out = create<If>(clone(n->getCond()), clone(n->getThen()), clone(n->getElse()));
      break;
    }
    case QIR_NODE_WHILE: {
      While *n = static_cast<While *>(in);
      out = create<While>(clone(n->getCond()), clone(n->getBody()));
      break;
    }
    case QIR_NODE_FOR: {
      For *n = static_cast<For *>(in);
      out = create<For>(clone(n->getInit()), clone(n->getCond()), clone(n->getStep()),
                        clone(n->getBody()));
      break;
    }
    case QIR_NODE_FORM: {
      Form *n = static_cast<Form *>(in);
      out =
          create<Form>(memorize(n->getIdxIdent()), memorize(n->getValIdent()),
                       clone(n->getMaxJobs()), clone(n->getExpr()), clone(n->getBody())->as<Seq>());
      break;
    }
    case QIR_NODE_FOREACH: {
      Foreach *n = static_cast<Foreach *>(in);
      out = create<Foreach>(memorize(n->getIdxIdent()), memorize(n->getValIdent()),
                            clone(n->getExpr()), clone(n->getBody())->as<Seq>());
      break;
    }
    case QIR_NODE_CASE: {
      Case *n = static_cast<Case *>(in);
      out = create<Case>(clone(n->getCond()), clone(n->getBody()));
      break;
    }
    case QIR_NODE_SWITCH: {
      Switch *n = static_cast<Switch *>(in);
      SwitchCases cases;
      for (auto item : n->getCases()) {
        cases.push_back(clone(item)->as<Case>());
      }
      out = create<Switch>(clone(n->getCond()), std::move(cases), clone(n->getDefault()));
      break;
    }
    case QIR_NODE_FN: {
      Fn *n = static_cast<Fn *>(in);
      Params params;
      for (auto param : n->getParams()) {
        params.push_back(clone(param)->asType());
      }

      out = create<Fn>(memorize(n->getName()), std::move(params), clone(n->getBody())->as<Seq>());
      break;
    }
    case QIR_NODE_ASM: {
      qcore_implement("QIR_NODE_ASM cloning");
      break;
    }
    case QIR_NODE_U1_TY: {
      out = create<U1Ty>();
      break;
    }
    case QIR_NODE_U8_TY: {
      out = create<U8Ty>();
      break;
    }
    case QIR_NODE_U16_TY: {
      out = create<U16Ty>();
      break;
    }
    case QIR_NODE_U32_TY: {
      out = create<U32Ty>();
      break;
    }
    case QIR_NODE_U64_TY: {
      out = create<U64Ty>();
      break;
    }
    case QIR_NODE_U128_TY: {
      out = create<U128Ty>();
      break;
    }
    case QIR_NODE_I8_TY: {
      out = create<I8Ty>();
      break;
    }
    case QIR_NODE_I16_TY: {
      out = create<I16Ty>();
      break;
    }
    case QIR_NODE_I32_TY: {
      out = create<I32Ty>();
      break;
    }
    case QIR_NODE_I64_TY: {
      out = create<I64Ty>();
      break;
    }
    case QIR_NODE_I128_TY: {
      out = create<I128Ty>();
      break;
    }
    case QIR_NODE_F16_TY: {
      out = create<F16Ty>();
      break;
    }
    case QIR_NODE_F32_TY: {
      out = create<F32Ty>();
      break;
    }
    case QIR_NODE_F64_TY: {
      out = create<F64Ty>();
      break;
    }
    case QIR_NODE_F128_TY: {
      out = create<F128Ty>();
      break;
    }
    case QIR_NODE_VOID_TY: {
      out = create<VoidTy>();
      break;
    }
    case QIR_NODE_PTR_TY: {
      PtrTy *n = static_cast<PtrTy *>(in);
      out = create<PtrTy>(clone(n->getPointee())->asType());
      break;
    }
    case QIR_NODE_OPAQUE_TY: {
      OpaqueTy *n = static_cast<OpaqueTy *>(in);
      out = create<OpaqueTy>(memorize(n->getName()));
      break;
    }
    case QIR_NODE_STRING_TY: {
      out = create<StringTy>();
      break;
    }
    case QIR_NODE_STRUCT_TY: {
      StructFields fields;
      for (auto field : static_cast<StructTy *>(in)->getFields()) {
        fields.push_back(clone(field)->asType());
      }
      out = create<StructTy>(std::move(fields));
      break;
    }
    case QIR_NODE_UNION_TY: {
      UnionFields fields;
      for (auto field : static_cast<UnionTy *>(in)->getFields()) {
        fields.push_back(clone(field)->asType());
      }
      out = create<UnionTy>(std::move(fields));
      break;
    }
    case QIR_NODE_ARRAY_TY: {
      ArrayTy *n = static_cast<ArrayTy *>(in);
      out = create<ArrayTy>(clone(n->getElement())->asType(), clone(n->getCount()));
      break;
    }
    case QIR_NODE_LIST_TY: {
      ListTy *n = static_cast<ListTy *>(in);
      out = create<ListTy>(clone(n->getElement())->asType());
      break;
    }
    case QIR_NODE_INTRIN_TY: {
      out = create<IntrinTy>(memorize(static_cast<IntrinTy *>(in)->getName()));
      break;
    }
    case QIR_NODE_FN_TY: {
      FnTy *n = static_cast<FnTy *>(in);
      FnParams params;
      for (auto param : n->getParams()) {
        params.push_back(clone(param)->asType());
      }
      out = create<FnTy>(std::move(params), clone(n->getReturn())->asType(), n->getAttrs());
      break;
    }
    case QIR_NODE_TMP: {
      Tmp *n = static_cast<Tmp *>(in);
      out = create<Tmp>(n->getTmpType(), n->getData());
      break;
    }
    case QIR_NODE_BAD: {
      out = create<Expr>(QIR_NODE_BAD);
      break;
    }
  }

  qcore_assert(out != nullptr, "qxir_clone: failed to clone node");

  // Module context id must be assigned before location information
  // is set.
  out->setModule(qxir_ctx);

  out->setLoc(in->getLoc());
  out->setConst(in->isConst());
  out->setVolatile(in->isVolatile());

  return static_cast<qxir_node_t *>(out);
}

LIB_EXPORT qxir_node_t *qxir_clone(qmodule_t *dst, const qxir_node_t *node) {
  qxir_node_t *out;

  if (!node) {
    return nullptr;
  }

  if (!dst) {
    dst = static_cast<const qxir::Expr *>(node)->getModule();
  }

  std::swap(qxir::qxir_arena.get(), dst->getNodeArena());
  std::swap(qxir_ctx, dst);

  out = nullptr;

  try {
    out = qxir_clone_impl(node);
  } catch (...) {
    return nullptr;
  }

  std::swap(qxir_ctx, dst);
  std::swap(qxir::qxir_arena.get(), dst->getNodeArena());

  return static_cast<qxir_node_t *>(out);
}
