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

#include <quix-core/Error.h>
#include <quix-ir/IR.h>
#include <quix-parser/Parser.h>
#include <setjmp.h>
#include <signal.h>

#include <atomic>
#include <core/Config.hh>
#include <cstring>
#include <quix-ir/Classes.hh>
#include <quix-ir/Format.hh>
#include <quix-ir/IRGraph.hh>
#include <quix-ir/Module.hh>
#include <quix-ir/Report.hh>
#include <stack>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>

#include "core/LibMacro.h"
#include "core/PassManager.hh"

using namespace qxir::diag;

struct ConvState {
  bool inside_function = false;
  std::string ns_prefix;
  std::stack<qparse::String> composite_expanse;
  qxir::AbiTag abi_mode = qxir::AbiTag::Internal;
  qxir::Type *return_type = nullptr;
  std::stack<std::unordered_map<std::string_view, qxir::Local *>> local_scope;

  std::string cur_named(std::string_view suffix) const {
    if (ns_prefix.empty()) {
      return std::string(suffix);
    }
    return ns_prefix + "::" + std::string(suffix);
  }
};

std::string ns_join(std::string_view a, std::string_view b) {
  if (a.empty()) {
    return std::string(b);
  }
  return std::string(a) + "::" + std::string(b);
}

static std::atomic<size_t> sigguard_refcount;
static std::mutex sigguard_lock;
static std::unordered_map<int, sighandler_t> sigguard_old;
static const std::set<int> sigguard_signals = {SIGABRT, SIGBUS, SIGFPE, SIGILL, SIGSEGV, SIGSYS};
static thread_local jmp_buf sigguard_env;

thread_local qmodule_t *qxir::current;

static void _signal_handler(int sig) {
  sigguard_lock.lock();

  DiagMessage diag;
  diag.m_msg = "FATAL Internal Error: Deadly Signal received: " + std::to_string(sig);
  diag.m_start = diag.m_end = qlex_loc_t{};
  diag.m_type = IssueClass::FatalError;
  diag.m_code = IssueCode::SignalReceived;

  qxir::current->getDiag().push(QXIR_AUDIT_CONV, std::move(diag));

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

static qxir::Expr *qconv_one(ConvState &s, qparse::Node *node);
static std::vector<qxir::Expr *> qconv_any(ConvState &s, qparse::Node *node);

LIB_EXPORT bool qxir_lower(qmodule_t *mod, qparse_node_t *base, bool diagnostics) {
  qcore_assert(mod, "qxir_lower: mod == nullptr");

  if (!base) {
    return false;
  }

  std::swap(qxir::qxir_arena.get(), mod->getNodeArena());
  install_sigguard(mod);
  qxir::current = mod;
  mod->setRoot(nullptr);
  mod->enableDiagnostics(diagnostics);

  volatile bool success = false;

  if (setjmp(sigguard_env) == 0) {
    try {
      ConvState s;
      mod->setRoot(qconv_one(s, static_cast<qparse::Node *>(base)));

      success = !mod->getFailbit();

      /* Perform the required transformations and checks
         if the first translation was successful */
      if (success) {
        /* Perform the required transformations */
        success = qxir::pass::PassGroupRegistry::get("ds").run(mod, [&](std::string_view name) {
          /* Track the pass name */
          mod->applyPassLabel(std::string(name));
        });
        if (success) {
          success = qxir::pass::PassGroupRegistry::get("chk").run(mod, [&](std::string_view name) {
            /* Track the analysis pass name */
            mod->applyCheckLabel(std::string(name));
          });
        } else {
          report(IssueCode::CompilerError, IssueClass::Debug, "");
        }

        success = success && !mod->getFailbit();
      }
    } catch (QError &) {
      success = false;
    }
  } else {
    success = false;

    /**
     * A signal (what is usually a fatal error) was caught,
     * the program is pretty much in an undefined state. However,
     * I don't care about the state of the program, I just want to
     * clean up the resources and return to the trusting user code.
     */
  }

  success || report(IssueCode::CompilerError, IssueClass::Error, "failed");

  qxir::current = nullptr;
  uninstall_sigguard();
  std::swap(qxir::qxir_arena.get(), mod->getNodeArena());

  return success;
}

LIB_EXPORT bool qxir_justprint(qparse_node_t *base, FILE *out, qxir_serial_t mode, qxir_node_cb cb,
                               uint32_t argcnt, ...) {
  qxir_conf conf;
  qmodule mod(nullptr, conf.get(), nullptr);

  (void)qxir_lower(mod.get(), base, false);

  if (!mod.get()->getRoot()) {
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

    auto ucb = [cb, userdata](qxir::Expr *, qxir::Expr **c) {
      cb(static_cast<qxir_node_t *>(*c), userdata);
      return qxir::IterOp::Proceed;
    };

    qxir::iterate<qxir::dfs_pre, qxir::IterMP::none>(mod.get()->getRoot(), ucb);
  }

  if (!qxir_write(mod.get()->getRoot(), mode, out, nullptr, 0)) {
    return false;
  }

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

  iterate<dfs_pre, IterMP::none>(n, [&](Expr *p, Expr **c) {
    if (p) {
      std::cout << "ParentType: " << p->getKindName();
    } else {
      std::cout << "ParentType: nullptr";
    }

    if (*c) {
      std::cout << " ChildType: " << (*c)->getKindName();
    } else {
      std::cout << " ChildType: nullptr";
    }

    std::cout << std::endl;

    if (*c) {
      dump_node_as_raw_hex(*c);
    }

    std::cout << std::endl;

    return IterOp::Proceed;
  });
}

///=============================================================================

static std::string_view memorize(std::string_view sv) { return qxir::current->internString(sv); }
static std::string_view memorize(qparse::String sv) {
  return memorize(std::string_view(sv.data(), sv.size()));
}

static qxir::Tmp *create_simple_call(
    ConvState &, std::string_view name,
    std::vector<std::pair<std::string_view, qxir::Expr *>,
                qxir::Arena<std::pair<std::string_view, qxir::Expr *>>>
        args = {}) {
  qxir::CallArgsTmpNodeCradle datapack;

  std::get<0>(datapack) = qxir::create<qxir::Ident>(memorize(name), nullptr);
  std::get<1>(datapack) = std::move(args);

  return create<qxir::Tmp>(qxir::TmpType::CALL, std::move(datapack));
  qcore_implement(__func__);
}

static qxir::List *create_string_literal(std::string_view value) {
  qxir::ListItems items;

  for (char c : value) {
    items.push_back(qxir::create<qxir::BinExpr>(qxir::create<qxir::Int>(c),
                                                qxir::create<qxir::I8Ty>(), qxir::Op::CastAs));
  }

  items.push_back(qxir::create<qxir::BinExpr>(qxir::create<qxir::Int>(0),
                                              qxir::create<qxir::I8Ty>(), qxir::Op::CastAs));

  return qxir::create<qxir::List>(items);
}

qxir::Expr *qconv_lower_binexpr(ConvState &, qxir::Expr *lhs, qxir::Expr *rhs, qlex_op_t op) {
#define STD_BINOP(op) qxir::create<qxir::BinExpr>(lhs, rhs, qxir::Op::op)
#define ASSIGN_BINOP(op)                                                                    \
  qxir::create<qxir::BinExpr>(                                                              \
      lhs,                                                                                  \
      qxir::create<qxir::BinExpr>(static_cast<qxir::Expr *>(qxir_clone(nullptr, lhs)), rhs, \
                                  qxir::Op::op),                                            \
      qxir::Op::Set)

  qxir::Expr *R = nullptr;

  switch (op) {
    case qOpPlus: {
      R = STD_BINOP(Plus);
      break;
    }
    case qOpMinus: {
      R = STD_BINOP(Minus);
      break;
    }
    case qOpTimes: {
      R = STD_BINOP(Times);
      break;
    }
    case qOpSlash: {
      R = STD_BINOP(Slash);
      break;
    }
    case qOpPercent: {
      R = STD_BINOP(Percent);
      break;
    }
    case qOpBitAnd: {
      R = STD_BINOP(BitAnd);
      break;
    }
    case qOpBitOr: {
      R = STD_BINOP(BitOr);
      break;
    }
    case qOpBitXor: {
      R = STD_BINOP(BitXor);
      break;
    }
    case qOpBitNot: {
      R = STD_BINOP(BitNot);
      break;
    }
    case qOpLogicAnd: {
      R = STD_BINOP(LogicAnd);
      break;
    }
    case qOpLogicOr: {
      R = STD_BINOP(LogicOr);
      break;
    }
    case qOpLogicXor: {
      // A ^^ B == (A || B) && !(A && B)
      auto a = qxir::create<qxir::BinExpr>(lhs, rhs, qxir::Op::LogicOr);
      auto b = qxir::create<qxir::BinExpr>(lhs, rhs, qxir::Op::LogicAnd);
      auto not_b = qxir::create<qxir::UnExpr>(b, qxir::Op::LogicNot);
      R = qxir::create<qxir::BinExpr>(a, not_b, qxir::Op::LogicAnd);
      break;
    }
    case qOpLogicNot: {
      R = STD_BINOP(LogicNot);
      break;
    }
    case qOpLShift: {
      R = STD_BINOP(LShift);
      break;
    }
    case qOpRShift: {
      R = STD_BINOP(RShift);
      break;
    }
    case qOpROTR: {
      R = STD_BINOP(ROTR);
      break;
    }
    case qOpROTL: {
      R = STD_BINOP(ROTL);
      break;
    }
    case qOpInc: {
      R = STD_BINOP(Inc);
      break;
    }
    case qOpDec: {
      R = STD_BINOP(Dec);
      break;
    }
    case qOpSet: {
      R = STD_BINOP(Set);
      break;
    }
    case qOpPlusSet: {
      R = ASSIGN_BINOP(Plus);
      break;
    }
    case qOpMinusSet: {
      R = ASSIGN_BINOP(Minus);
      break;
    }
    case qOpTimesSet: {
      R = ASSIGN_BINOP(Times);
      break;
    }
    case qOpSlashSet: {
      R = ASSIGN_BINOP(Slash);
      break;
    }
    case qOpPercentSet: {
      R = ASSIGN_BINOP(Percent);
      break;
    }
    case qOpBitAndSet: {
      R = ASSIGN_BINOP(BitAnd);
      break;
    }
    case qOpBitOrSet: {
      R = ASSIGN_BINOP(BitOr);
      break;
    }
    case qOpBitXorSet: {
      R = ASSIGN_BINOP(BitXor);
      break;
    }
    case qOpLogicAndSet: {
      R = ASSIGN_BINOP(LogicAnd);
      break;
    }
    case qOpLogicOrSet: {
      R = ASSIGN_BINOP(LogicOr);
      break;
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
      R = ASSIGN_BINOP(LShift);
      break;
    }
    case qOpRShiftSet: {
      R = ASSIGN_BINOP(RShift);
      break;
    }
    case qOpROTRSet: {
      R = ASSIGN_BINOP(ROTR);
      break;
    }
    case qOpROTLSet: {
      R = ASSIGN_BINOP(ROTL);
      break;
    }
    case qOpLT: {
      R = STD_BINOP(LT);
      break;
    }
    case qOpGT: {
      R = STD_BINOP(GT);
      break;
    }
    case qOpLE: {
      R = STD_BINOP(LE);
      break;
    }
    case qOpGE: {
      R = STD_BINOP(GE);
      break;
    }
    case qOpEq: {
      R = STD_BINOP(Eq);
      break;
    }
    case qOpNE: {
      R = STD_BINOP(NE);
      break;
    }
    case qOpAs: {
      R = STD_BINOP(CastAs);
      break;
    }
    case qOpIn: {
      auto methname = create_string_literal("has");
      auto method = qxir::create<qxir::Index>(rhs, methname);
      R = qxir::create<qxir::Call>(method, qxir::CallArgs({lhs}));
      break;
    }
    case qOpRange: {
      /// TODO: Implement range operator
      throw QError();
    }
    case qOpBitcastAs: {
      R = STD_BINOP(BitcastAs);
      break;
    }
    default: {
      throw QError();
    }
  }

  return R;
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
      auto arg = qxir::create<qxir::BinExpr>(bits, qxir::create<qxir::Float>(8), qxir::Op::Slash);
      return create_simple_call(s, "std::ceil", {{"0", arg}});
    }
    case qOpAlignof: {
      return STD_UNOP(Alignof);
    }
    case qOpTypeof: {
      auto inferred = rhs->getType();
      if (!inferred.has_value()) {
        badtree(nullptr, "qOpTypeof: rhs->getType() == nullptr");
        throw QError();
      }

      qcore_assert(inferred, "qOpTypeof: inferred == nullptr");
      qxir::SymbolEncoding se;
      auto res = se.mangle_name(inferred.value(), qxir::AbiTag::QUIX);
      if (!res) {
        badtree(nullptr, "Failed to mangle type");
        throw QError();
      }

      return create_string_literal(res.value());
    }
    case qOpBitsizeof: {
      return STD_UNOP(Bitsizeof);
    }
    default: {
      throw QError();
    }
  }
}

qxir::Expr *qconv_lower_post_unexpr(ConvState &, qxir::Expr *lhs, qlex_op_t op) {
#define STD_POST_OP(op) qxir::create<qxir::PostUnExpr>(lhs, qxir::Op::op)

  switch (op) {
    case qOpInc: {
      return STD_POST_OP(Inc);
    }
    case qOpDec: {
      return STD_POST_OP(Dec);
    }
    default: {
      badtree(nullptr, "Unknown post-unary operator");
      throw QError();
    }
  }
}

namespace qxir {

  static Expr *qconv_cexpr(ConvState &s, qparse::ConstExpr *n) {
    auto c = qconv_one(s, n->get_value());
    if (!c) {
      badtree(n, "qparse::ConstExpr::get_value() == nullptr");
      throw QError();
    }

    return c;
  }

  static Expr *qconv_binexpr(ConvState &s, qparse::BinExpr *n) {
    /**
     * @brief Convert a binary expression to a qxir expression.
     * @details Recursively convert the left and right hand sides of the
     *         binary expression, then convert the operator to a qxir
     *         compatible operator.
     */

    auto lhs = qconv_one(s, n->get_lhs());
    if (!lhs) {
      badtree(n, "qparse::BinExpr::get_lhs() == nullptr");
      throw QError();
    }

    auto rhs = qconv_one(s, n->get_rhs());

    if (!rhs) {
      badtree(n, "qparse::BinExpr::get_rhs() == nullptr");
      throw QError();
    }

    return qconv_lower_binexpr(s, lhs, rhs, n->get_op());
  }

  static Expr *qconv_unexpr(ConvState &s, qparse::UnaryExpr *n) {
    /**
     * @brief Convert a unary expression to a qxir expression.
     * @details Recursively convert the left hand side of the unary
     *         expression, then convert the operator to a qxir compatible
     *         operator.
     */

    auto rhs = qconv_one(s, n->get_rhs());
    if (!rhs) {
      badtree(n, "qparse::UnaryExpr::get_rhs() == nullptr");
      throw QError();
    }

    return qconv_lower_unexpr(s, rhs, n->get_op());
  }

  static Expr *qconv_post_unexpr(ConvState &s, qparse::PostUnaryExpr *n) {
    /**
     * @brief Convert a post-unary expression to a qxir expression.
     * @details Recursively convert the left hand side of the post-unary
     *         expression, then convert the operator to a qxir compatible
     *         operator.
     */

    auto lhs = qconv_one(s, n->get_lhs());
    if (!lhs) {
      badtree(n, "qparse::PostUnaryExpr::get_lhs() == nullptr");
      throw QError();
    }

    return qconv_lower_post_unexpr(s, lhs, n->get_op());
  }

  static Expr *qconv_terexpr(ConvState &s, qparse::TernaryExpr *n) {
    /**
     * @brief Convert a ternary expression to a if-else expression.
     * @details Recursively convert the condition, then the true and false
     *        branches of the ternary expression.
     */

    auto cond = qconv_one(s, n->get_cond());
    if (!cond) {
      badtree(n, "qparse::TernaryExpr::get_cond() == nullptr");
      throw QError();
    }

    auto t = qconv_one(s, n->get_lhs());
    if (!t) {
      badtree(n, "qparse::TernaryExpr::get_lhs() == nullptr");
      throw QError();
    }

    auto f = qconv_one(s, n->get_rhs());
    if (!f) {
      badtree(n, "qparse::TernaryExpr::get_rhs() == nullptr");
      throw QError();
    }

    return create<If>(cond, t, f);
  }

  static Expr *qconv_int(ConvState &, qparse::ConstInt *n) {
    /**
     * @brief Convert an integer constant to a qxir number.
     * @details This is a 1-to-1 conversion of the integer constant.
     */

    return create<Int>(memorize(n->get_value()));
  }

  static Expr *qconv_float(ConvState &, qparse::ConstFloat *n) {
    /**
     * @brief Convert a floating point constant to a qxir number.
     * @details This is a 1-to-1 conversion of the floating point constant.
     */

    return create<Float>(memorize(n->get_value()));
  }

  static Expr *qconv_string(ConvState &, qparse::ConstString *n) {
    /**
     * @brief Convert a string constant to a qxir string.
     * @details This is a 1-to-1 conversion of the string constant.
     */

    return create_string_literal(n->get_value());
  }

  static Expr *qconv_char(ConvState &, qparse::ConstChar *n) {
    /**
     * @brief Convert a character constant to a qxir number.
     * @details Convert the char32 codepoint to a qxir number literal.
     */

    return create<Int>(n->get_value());
  }

  static Expr *qconv_bool(ConvState &, qparse::ConstBool *n) {
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

  static Expr *qconv_null(ConvState &, qparse::ConstNull *) {
    /**
     * @brief Convert a null literal to a qxir null literal tmp.
     * @details This is a 1-to-1 conversion of the null literal.
     */

    return create<Tmp>(TmpType::NULL_LITERAL);
  }

  static Expr *qconv_undef(ConvState &, qparse::ConstUndef *) {
    /**
     * @brief Convert an undefined literal to a qxir undefined literal tmp.
     * @details This is a 1-to-1 conversion of the undefined literal.
     */

    return create<Tmp>(TmpType::UNDEF_LITERAL);
  }

  static Expr *qconv_call(ConvState &s, qparse::Call *n) {
    /**
     * @brief Convert a function call to a qxir call.
     * @details Recursively convert the function base and the arguments
     *         of the function call. We currently do not have enough information
     *         to handle the expansion of named / optional arguments. Therefore,
     *         we wrap the data present and we will attempt to lower the construct
     *         later.
     */

    auto target = qconv_one(s, n->get_func());
    if (!target) {
      badtree(n, "qparse::Call::get_func() == nullptr");
      throw QError();
    }

    CallArgsTmpNodeCradle datapack;
    for (auto it = n->get_args().begin(); it != n->get_args().end(); ++it) {
      auto arg = qconv_one(s, it->second);
      if (!arg) {
        badtree(n, "qparse::Call::get_args() vector contains nullptr");
        throw QError();
      }

      // Implicit conversion are done later

      std::get<1>(datapack).push_back({memorize(it->first), arg});
    }
    std::get<0>(datapack) = target;

    return create<Tmp>(TmpType::CALL, std::move(datapack));
  }

  static Expr *qconv_list(ConvState &s, qparse::List *n) {
    /**
     * @brief Convert a list of expressions to a qxir list.
     * @details This is a 1-to-1 conversion of the list of expressions.
     */

    ListItems items;

    for (auto it = n->get_items().begin(); it != n->get_items().end(); ++it) {
      auto item = qconv_one(s, *it);
      if (!item) {
        badtree(n, "qparse::List::get_items() vector contains nullptr");
        throw QError();
      }

      items.push_back(item);
    }

    return create<List>(std::move(items));
  }

  static Expr *qconv_assoc(ConvState &s, qparse::Assoc *n) {
    /**
     * @brief Convert an associative list to a qxir list.
     * @details This is a 1-to-1 conversion of the associative list.
     */

    auto key = qconv_one(s, n->get_key());
    if (!key) {
      badtree(n, "qparse::Assoc::get_key() == nullptr");
      throw QError();
    }

    auto value = qconv_one(s, n->get_value());
    if (!value) {
      badtree(n, "qparse::Assoc::get_value() == nullptr");
      throw QError();
    }

    return create<List>(ListItems({key, value}));
  }

  static Expr *qconv_field(ConvState &s, qparse::Field *n) {
    /**
     * @brief Convert a field access to a qxir expression.
     * @details Store the base and field name in a temporary node cradle
     *          for later lowering.
     */

    auto base = qconv_one(s, n->get_base());
    if (!base) {
      badtree(n, "qparse::Field::get_base() == nullptr");
      throw QError();
    }

    return create<Index>(base, create_string_literal(n->get_field()));
  }

  static Expr *qconv_index(ConvState &s, qparse::Index *n) {
    /**
     * @brief Convert an index expression to a qxir expression.
     * @details Recursively convert the base and index of the index
     *         expression.
     */

    auto base = qconv_one(s, n->get_base());
    if (!base) {
      badtree(n, "qparse::Index::get_base() == nullptr");
      throw QError();
    }

    auto index = qconv_one(s, n->get_index());
    if (!index) {
      badtree(n, "qparse::Index::get_index() == nullptr");
      throw QError();
    }

    return create<Index>(base, index);
  }

  static Expr *qconv_slice(ConvState &s, qparse::Slice *n) {
    /**
     * @brief Convert a slice expression to a qxir expression.
     * @details Recursively convert the base, start, and end of the slice
     *         expression and pass them so the .slice() method which is
     *         assumed to be present in the base object.
     */

    auto base = qconv_one(s, n->get_base());
    if (!base) {
      badtree(n, "qparse::Slice::get_base() == nullptr");
      throw QError();
    }

    auto start = qconv_one(s, n->get_start());
    if (!start) {
      badtree(n, "qparse::Slice::get_start() == nullptr");
      throw QError();
    }

    auto end = qconv_one(s, n->get_end());
    if (!end) {
      badtree(n, "qparse::Slice::get_end() == nullptr");
      throw QError();
    }

    return create<Call>(create<Index>(base, create_string_literal("slice")),
                        CallArgs({start, end}));
  }

  static Expr *qconv_fstring(ConvState &s, qparse::FString *n) {
    /**
     * @brief Convert a formatted string to a qxir string concatenation.
     */

    if (n->get_items().empty()) {
      return create_string_literal("");
    }

    if (n->get_items().size() == 1) {
      auto val = n->get_items().front();
      if (std::holds_alternative<qparse::String>(val)) {
        return create_string_literal(std::get<qparse::String>(val));
      } else if (std::holds_alternative<qparse::Expr *>(val)) {
        auto expr = qconv_one(s, std::get<qparse::Expr *>(val));
        if (!expr) {
          badtree(n, "qparse::FString::get_items() vector contains nullptr");
          throw QError();
        }

        return expr;
      } else {
        qcore_panic("Invalid fstring item type");
      }
    }

    Expr *concated = create_string_literal("");

    for (auto it = n->get_items().begin(); it != n->get_items().end(); ++it) {
      if (std::holds_alternative<qparse::String>(*it)) {
        auto val = std::get<qparse::String>(*it);

        concated = create<BinExpr>(concated, create_string_literal(val), Op::Plus);
      } else if (std::holds_alternative<qparse::Expr *>(*it)) {
        auto val = std::get<qparse::Expr *>(*it);
        auto expr = qconv_one(s, val);
        if (!expr) {
          badtree(n, "qparse::FString::get_items() vector contains nullptr");
          throw QError();
        }

        concated = create<BinExpr>(concated, expr, Op::Plus);
      } else {
        qcore_panic("Invalid fstring item type");
      }
    }

    return concated;
  }

  static Expr *qconv_ident(ConvState &s, qparse::Ident *n) {
    /**
     * @brief Convert an identifier to a qxir expression.
     * @details This is a 1-to-1 conversion of the identifier.
     */

    if (s.inside_function) {
      qcore_assert(!s.local_scope.empty());

      auto find = s.local_scope.top().find(n->get_name());
      if (find != s.local_scope.top().end()) {
        return create<Ident>(memorize(n->get_name()), find->second);
      }
    }

    auto str = s.cur_named(n->get_name());

    return create<Ident>(memorize(std::string_view(str)), nullptr);
  }

  static Expr *qconv_seq_point(ConvState &s, qparse::SeqPoint *n) {
    /**
     * @brief Convert a sequence point to a qxir expression.
     * @details This is a 1-to-1 conversion of the sequence point.
     */

    SeqItems items;
    items.reserve(n->get_items().size());

    for (auto it = n->get_items().begin(); it != n->get_items().end(); ++it) {
      auto item = qconv_one(s, *it);
      if (!item) {
        badtree(n, "qparse::SeqPoint::get_items() vector contains nullptr");
        throw QError();
      }

      items.push_back(item);
    }

    return create<Seq>(std::move(items));
  }

  static Expr *qconv_stmt_expr(ConvState &s, qparse::StmtExpr *n) {
    /**
     * @brief Unwrap a statement inside an expression into a qxir expression.
     * @details This is a 1-to-1 conversion of the statement expression.
     */

    auto stmt = qconv_one(s, n->get_stmt());
    if (!stmt) {
      badtree(n, "qparse::StmtExpr::get_stmt() == nullptr");
      throw QError();
    }

    return stmt;
  }

  static Expr *qconv_type_expr(ConvState &s, qparse::TypeExpr *n) {
    /*
     * @brief Convert a type expression to a qxir expression.
     * @details This is a 1-to-1 conversion of the type expression.
     */

    auto type = qconv_one(s, n->get_type());
    if (!type) {
      badtree(n, "qparse::TypeExpr::get_type() == nullptr");
      throw QError();
    }

    return type;
  }

  static Expr *qconv_templ_call(ConvState &, qparse::TemplCall *) {
    /// TODO: templ_call

    throw QError();
  }

  static Expr *qconv_ref_ty(ConvState &s, qparse::RefTy *n) {
    auto pointee = qconv_one(s, n->get_item());
    if (!pointee) {
      badtree(n, "qparse::RefTy::get_item() == nullptr");
      throw QError();
    }

    return create<PtrTy>(pointee->asType());
  }

  static Expr *qconv_u1_ty(ConvState &, qparse::U1 *) {
    /**
     * @brief Convert a U1 type to a qxir expression type.
     * @details This is a 1-to-1 conversion of the U1 type.
     */

    return create<U1Ty>();
  }

  static Expr *qconv_u8_ty(ConvState &, qparse::U8 *) {
    /**
     * @brief Convert a U8 type to a qxir expression type.
     * @details This is a 1-to-1 conversion of the U8 type.
     */

    return create<U8Ty>();
  }

  static Expr *qconv_u16_ty(ConvState &, qparse::U16 *) {
    /**
     * @brief Convert a U16 type to a qxir expression type.
     * @details This is a 1-to-1 conversion of the U16 type.
     */

    return create<U16Ty>();
  }

  static Expr *qconv_u32_ty(ConvState &, qparse::U32 *) {
    /**
     * @brief Convert a U32 type to a qxir expression type.
     * @details This is a 1-to-1 conversion of the U32 type.
     */

    return create<U32Ty>();
  }

  static Expr *qconv_u64_ty(ConvState &, qparse::U64 *) {
    /**
     * @brief Convert a U64 type to a qxir expression type.
     * @details This is a 1-to-1 conversion of the U64 type.
     */

    return create<U64Ty>();
  }

  static Expr *qconv_u128_ty(ConvState &, qparse::U128 *) {
    /**
     * @brief Convert a U128 type to a qxir expression type.
     * @details This is a 1-to-1 conversion of the U128 type.
     */

    return create<U128Ty>();
  }

  static Expr *qconv_i8_ty(ConvState &, qparse::I8 *) {
    /**
     * @brief Convert a I8 type to a qxir expression type.
     * @details This is a 1-to-1 conversion of the I8 type.
     */

    return create<I8Ty>();
  }

  static Expr *qconv_i16_ty(ConvState &, qparse::I16 *) {
    /**
     * @brief Convert a I16 type to a qxir expression type.
     * @details This is a 1-to-1 conversion of the I16 type.
     */

    return create<I16Ty>();
  }

  static Expr *qconv_i32_ty(ConvState &, qparse::I32 *) {
    /**
     * @brief Convert a I32 type to a qxir expression type.
     * @details This is a 1-to-1 conversion of the I32 type.
     */

    return create<I32Ty>();
  }

  static Expr *qconv_i64_ty(ConvState &, qparse::I64 *) {
    /**
     * @brief Convert a I64 type to a qxir expression type.
     * @details This is a 1-to-1 conversion of the I64 type.
     */

    return create<I64Ty>();
  }

  static Expr *qconv_i128_ty(ConvState &, qparse::I128 *) {
    /**
     * @brief Convert a I128 type to a qxir expression type.
     * @details This is a 1-to-1 conversion of the I128 type.
     */

    return create<I128Ty>();
  }

  static Expr *qconv_f16_ty(ConvState &, qparse::F16 *) {
    /**
     * @brief Convert a F16 type to a qxir expression type.
     * @details This is a 1-to-1 conversion of the F16 type.
     */

    return create<F16Ty>();
  }

  static Expr *qconv_f32_ty(ConvState &, qparse::F32 *) {
    /**
     * @brief Convert a F32 type to a qxir expression type.
     * @details This is a 1-to-1 conversion of the F32 type.
     */

    return create<F32Ty>();
  }

  static Expr *qconv_f64_ty(ConvState &, qparse::F64 *) {
    /**
     * @brief Convert a F64 type to a qxir expression type.
     * @details This is a 1-to-1 conversion of the F64 type.
     */

    return create<F64Ty>();
  }

  static Expr *qconv_f128_ty(ConvState &, qparse::F128 *) {
    /**
     * @brief Convert a F128 type to a qxir expression type.
     * @details This is a 1-to-1 conversion of the F128 type.
     */

    return create<F128Ty>();
  }

  static Expr *qconv_void_ty(ConvState &, qparse::VoidTy *) {
    /**
     * @brief Convert a Void type to a qxir expression type.
     * @details This is a 1-to-1 conversion of the Void type.
     */

    return create<VoidTy>();
  }

  static Expr *qconv_ptr_ty(ConvState &s, qparse::PtrTy *n) {
    /**
     * @brief Convert a pointer type to a qxir pointer type.
     * @details This is a 1-to-1 conversion of the pointer type.
     */

    auto pointee = qconv_one(s, n->get_item());
    if (!pointee) {
      badtree(n, "qparse::PtrTy::get_item() == nullptr");
      throw QError();
    }

    return create<PtrTy>(pointee->asType());
  }

  static Expr *qconv_opaque_ty(ConvState &, qparse::OpaqueTy *n) {
    /**
     * @brief Convert an opaque type to a qxir opaque type.
     * @details This is a 1-to-1 conversion of the opaque type.
     */

    return create<OpaqueTy>(memorize(n->get_name()));
  }

  static Expr *qconv_struct_ty(ConvState &s, qparse::StructTy *n) {
    /**
     * @brief Convert a struct type to a qxir struct type.
     * @details This is a 1-to-1 conversion of the struct type.
     */

    StructFields fields;

    for (auto it = n->get_items().begin(); it != n->get_items().end(); ++it) {
      auto item = qconv_one(s, it->second)->asType();
      if (!item) {
        badtree(n, "qparse::StructTy::get_items() vector contains nullptr");
        throw QError();
      }

      fields.push_back(item);
    }

    return create<StructTy>(std::move(fields));
  }

  static Expr *qconv_group_ty(ConvState &s, qparse::GroupTy *n) {
    /**
     * @brief Convert a group type to a qxir struct type.
     * @details This is a 1-to-1 conversion of the group type.
     */

    StructFields fields;

    for (auto it = n->get_items().begin(); it != n->get_items().end(); ++it) {
      auto item = qconv_one(s, *it)->asType();
      if (!item) {
        badtree(n, "qparse::GroupTy::get_items() vector contains nullptr");
        throw QError();
      }

      fields.push_back(item);
    }

    return create<StructTy>(std::move(fields));
  }

  static Expr *qconv_region_ty(ConvState &s, qparse::RegionTy *n) {
    /**
     * @brief Convert a region type to a qxir struct type.
     * @details This is a 1-to-1 conversion of the region type.
     */

    StructFields fields;

    for (auto it = n->get_items().begin(); it != n->get_items().end(); ++it) {
      auto item = qconv_one(s, *it)->asType();
      if (!item) {
        badtree(n, "qparse::RegionTy::get_items() vector contains nullptr");
        throw QError();
      }

      fields.push_back(item);
    }

    return create<StructTy>(std::move(fields));
  }

  static Expr *qconv_union_ty(ConvState &s, qparse::UnionTy *n) {
    /**
     * @brief Convert a union type to a qxir struct type.
     * @details This is a 1-to-1 conversion of the union type.
     */

    UnionFields fields;

    for (auto it = n->get_items().begin(); it != n->get_items().end(); ++it) {
      auto item = qconv_one(s, *it)->asType();
      if (!item) {
        badtree(n, "qparse::UnionTy::get_items() vector contains nullptr");
        throw QError();
      }

      fields.push_back(item);
    }

    return create<UnionTy>(std::move(fields));
  }

  static Expr *qconv_array_ty(ConvState &s, qparse::ArrayTy *n) {
    /**
     * @brief Convert an array type to a qxir array type.
     * @details This is a 1-to-1 conversion of the array type.
     */

    auto item = qconv_one(s, n->get_item());
    if (!item) {
      badtree(n, "qparse::ArrayTy::get_item() == nullptr");
      throw QError();
    }

    auto count = qconv_one(s, n->get_size());
    if (!count) {
      badtree(n, "qparse::ArrayTy::get_size() == nullptr");
      throw QError();
    }

    /// TODO: Invoke an interpreter to calculate size expression
    if (count->getKind() != QIR_NODE_INT) {
      badtree(n, "Non integer literal array size is not supported");
      throw QError();
    }

    uint64_t size = std::atoi(static_cast<qxir::Int *>(count)->getValue().c_str());

    return create<ArrayTy>(item->asType(), size);
  }

  static Expr *qconv_tuple_ty(ConvState &s, qparse::TupleTy *n) {
    /**
     * @brief Convert a tuple type to a qxir struct type.
     * @details This is a 1-to-1 conversion of the tuple type.
     */

    StructFields fields;
    for (auto it = n->get_items().begin(); it != n->get_items().end(); ++it) {
      auto item = qconv_one(s, *it)->asType();
      if (!item) {
        badtree(n, "qparse::TupleTy::get_items() vector contains nullptr");
        throw QError();
      }

      fields.push_back(item);
    }

    return create<StructTy>(std::move(fields));
  }

  static Expr *qconv_fn_ty(ConvState &s, qparse::FuncTy *n) {
    /**
     * @brief Convert a function type to a qxir function type.
     * @details Order of function parameters is consistent with their order
     * of declaration. Support for `optional` arguments is up to the frontend,
     * for inter-language compatibility, the ABI is not concerned with optional
     * arguments as they have no significance at the binary interface level.
     */

    FnParams params;

    for (auto it = n->get_params().begin(); it != n->get_params().end(); ++it) {
      Type *type = qconv_one(s, std::get<1>(*it))->asType();
      if (!type) {
        badtree(n, "qparse::FnDef::get_type() == nullptr");
        throw QError();
      }

      params.push_back(type);
    }

    Type *ret = qconv_one(s, n->get_return_ty())->asType();
    if (!ret) {
      badtree(n, "qparse::FnDef::get_ret() == nullptr");
      throw QError();
    }

    FnAttrs attrs;

    if (n->is_variadic()) {
      attrs.insert(FnAttr::Variadic);
    }

    return create<FnTy>(std::move(params), ret, std::move(attrs));
  }

  static Expr *qconv_unres_ty(ConvState &s, qparse::UnresolvedType *n) {
    /**
     * @brief Convert an unresolved type to a qxir type.
     * @details This is a 1-to-1 conversion of the unresolved type.
     */

    auto str = s.cur_named(n->get_name());
    auto name = memorize(std::string_view(str));

    return create<Tmp>(TmpType::NAMED_TYPE, name);
  }

  static Expr *qconv_infer_ty(ConvState &, qparse::InferType *) {
    /// TODO: infer_ty
    throw QError();
  }

  static Expr *qconv_templ_ty(ConvState &, qparse::TemplType *) {
    /// TODO: templ_ty
    throw QError();
  }

  static std::vector<Expr *> qconv_typedef(ConvState &s, qparse::TypedefDecl *n) {
    /**
     * @brief Memorize a typedef declaration which will be used later for type resolution.
     * @details This node will resolve to type void.
     */

    auto str = s.cur_named(n->get_name());
    auto name = memorize(std::string_view(str));

    if (current->getTypeMap().contains(name)) {
      report(IssueCode::TypeRedefinition, IssueClass::Error, n->get_name(), n->get_start_pos(),
             n->get_end_pos());
    }

    auto type = qconv_one(s, n->get_type());
    if (!type) {
      badtree(n, "qparse::TypedefDecl::get_type() == nullptr");
      throw QError();
    }

    current->getTypeMap()[name] = type->asType();

    return {};
  }

  static Expr *qconv_fndecl(ConvState &s, qparse::FnDecl *n) {
    Params params;
    qparse::FuncTy *fty = n->get_type();

    std::string name;
    if (s.abi_mode == AbiTag::C) {
      name = n->get_name();
    } else {
      name = s.cur_named(n->get_name());
    }
    auto str = memorize(std::string_view(name));

    current->getParameterMap()[str] = {};

    { /* Produce the function parameters */
      for (auto it = fty->get_params().begin(); it != fty->get_params().end(); ++it) {
        /**
         * Parameter properties:
         * 1. Name - All parameters have a name.
         * 2. Type - All parameters have a type.
         * 3. Default - Optional, if the parameter has a default value.
         * 4. Position - All parameters have a position.
         */

        Type *type = qconv_one(s, std::get<1>(*it))->asType();
        if (!type) {
          badtree(n, "qparse::FnDecl::get_type() == nullptr");
          throw QError();
        }

        Expr *def = nullptr;
        if (std::get<2>(*it)) {
          def = qconv_one(s, std::get<2>(*it));
          if (!def) {
            badtree(n, "qparse::FnDecl::get_type() == nullptr");
            throw QError();
          }
        }

        std::string_view sv = memorize(std::string_view(std::get<0>(*it)));

        params.push_back({type, sv});
        current->getParameterMap()[str].push_back({std::string(std::get<0>(*it)), type, def});
      }
    }

    Expr *fnty = qconv_one(s, fty);
    if (!fnty) {
      badtree(n, "qparse::FnDecl::get_type() == nullptr");
      throw QError();
    }

    Fn *fn = create<Fn>(str, std::move(params), fnty->as<FnTy>()->getReturn(), createIgn(),
                        fty->is_variadic(), s.abi_mode);

    current->getFunctions().insert({str, {fnty->as<FnTy>(), fn}});

    return fn;
  }

#define align(x, a) (((x) + (a) - 1) & ~((a) - 1))

  static std::vector<Expr *> qconv_struct(ConvState &s, qparse::StructDef *n) {
    /**
     * @brief Convert a struct definition to a qxir sequence.
     * @details This is a 1-to-1 conversion of the struct definition.
     */

    std::string name = s.cur_named(n->get_name());
    auto sv = memorize(std::string_view(name));

    if (current->getTypeMap().contains(sv)) {
      report(IssueCode::TypeRedefinition, IssueClass::Error, n->get_name(), n->get_start_pos(),
             n->get_end_pos());
    }

    StructFields fields;
    std::vector<Expr *> items;
    size_t offset = 0;

    for (auto it = n->get_fields().begin(); it != n->get_fields().end(); ++it) {
      if (!*it) {
        badtree(n, "qparse::StructDef::get_fields() vector contains nullptr");
        throw QError();
      }

      s.composite_expanse.push((*it)->get_name());
      auto field = qconv_one(s, *it);
      s.composite_expanse.pop();

      size_t field_align = field->asType()->getAlignBytes();
      size_t padding = align(offset, field_align) - offset;
      if (padding > 0) {
        fields.push_back(create<ArrayTy>(create<U8Ty>(), padding));
      }

      fields.push_back(field->asType());
      offset += field->asType()->getSizeBytes();
    }

    StructTy *st = create<StructTy>(std::move(fields));

    current->getTypeMap()[sv] = st;

    for (auto it = n->get_methods().begin(); it != n->get_methods().end(); ++it) {
      qparse::FnDecl *cur_meth = *it;
      auto old_name = cur_meth->get_name();
      cur_meth->set_name(ns_join(n->get_name(), old_name));

      auto method = qconv_one(s, *it);

      cur_meth->set_name(old_name);

      if (!method) {
        badtree(n, "qparse::StructDef::get_methods() vector contains nullptr");
        throw QError();
      }

      items.push_back(method);
    }

    for (auto it = n->get_static_methods().begin(); it != n->get_static_methods().end(); ++it) {
      qparse::FnDecl *cur_meth = *it;
      auto old_name = cur_meth->get_name();
      cur_meth->set_name(ns_join(n->get_name(), old_name));

      auto method = qconv_one(s, *it);

      cur_meth->set_name(old_name);

      if (!method) {
        badtree(n, "qparse::StructDef::get_static_methods() vector contains nullptr");
        throw QError();
      }

      items.push_back(method);
    }

    return items;
  }

  static std::vector<Expr *> qconv_region(ConvState &s, qparse::RegionDef *n) {
    /**
     * @brief Convert a region definition to a qxir sequence.
     * @details This is a 1-to-1 conversion of the region definition.
     */

    std::string name = s.cur_named(n->get_name());
    auto sv = memorize(std::string_view(name));

    if (current->getTypeMap().contains(sv)) {
      report(IssueCode::TypeRedefinition, IssueClass::Error, n->get_name(), n->get_start_pos(),
             n->get_end_pos());
    }

    StructFields fields;
    std::vector<Expr *> items;

    for (auto it = n->get_fields().begin(); it != n->get_fields().end(); ++it) {
      if (!*it) {
        badtree(n, "qparse::RegionDef::get_fields() vector contains nullptr");
        throw QError();
      }

      s.composite_expanse.push((*it)->get_name());
      auto field = qconv_one(s, *it);
      s.composite_expanse.pop();

      fields.push_back(field->asType());
    }

    StructTy *st = create<StructTy>(std::move(fields));

    current->getTypeMap()[sv] = st;

    for (auto it = n->get_methods().begin(); it != n->get_methods().end(); ++it) {
      qparse::FnDecl *cur_meth = *it;
      auto old_name = cur_meth->get_name();
      cur_meth->set_name(ns_join(n->get_name(), old_name));

      auto method = qconv_one(s, *it);

      cur_meth->set_name(old_name);

      if (!method) {
        badtree(n, "qparse::RegionDef::get_methods() vector contains nullptr");
        throw QError();
      }

      items.push_back(method);
    }

    for (auto it = n->get_static_methods().begin(); it != n->get_static_methods().end(); ++it) {
      qparse::FnDecl *cur_meth = *it;
      auto old_name = cur_meth->get_name();
      cur_meth->set_name(ns_join(n->get_name(), old_name));

      auto method = qconv_one(s, *it);

      cur_meth->set_name(old_name);

      if (!method) {
        badtree(n, "qparse::RegionDef::get_static_methods() vector contains nullptr");
        throw QError();
      }

      items.push_back(method);
    }

    return items;
  }

  static std::vector<Expr *> qconv_group(ConvState &s, qparse::GroupDef *n) {
    /**
     * @brief Convert a group definition to a qxir sequence.
     * @details This is a 1-to-1 conversion of the group definition.
     */

    std::string name = s.cur_named(n->get_name());
    auto sv = memorize(std::string_view(name));

    if (current->getTypeMap().contains(sv)) {
      report(IssueCode::TypeRedefinition, IssueClass::Error, n->get_name(), n->get_start_pos(),
             n->get_end_pos());
    }

    StructFields fields;
    std::vector<Expr *> items;

    { /* Optimize layout with field sorting */
      std::vector<Type *> tmp_fields;
      for (auto it = n->get_fields().begin(); it != n->get_fields().end(); ++it) {
        if (!*it) {
          badtree(n, "qparse::GroupDef::get_fields() vector contains nullptr");
          throw QError();
        }

        s.composite_expanse.push((*it)->get_name());
        auto field = qconv_one(s, *it);
        s.composite_expanse.pop();

        tmp_fields.push_back(field->asType());
      }

      std::sort(tmp_fields.begin(), tmp_fields.end(),
                [](Type *a, Type *b) { return a->getSizeBits() > b->getSizeBits(); });

      size_t offset = 0;
      for (auto it = tmp_fields.begin(); it != tmp_fields.end(); ++it) {
        size_t field_align = (*it)->getAlignBytes();
        size_t padding = align(offset, field_align) - offset;
        if (padding > 0) {
          fields.push_back(create<ArrayTy>(create<U8Ty>(), padding));
        }

        fields.push_back(*it);
        offset += (*it)->getSizeBytes();
      }
    }

    StructTy *st = create<StructTy>(std::move(fields));

    current->getTypeMap()[sv] = st;

    for (auto it = n->get_methods().begin(); it != n->get_methods().end(); ++it) {
      qparse::FnDecl *cur_meth = *it;
      auto old_name = cur_meth->get_name();
      cur_meth->set_name(ns_join(n->get_name(), old_name));

      auto method = qconv_one(s, *it);

      cur_meth->set_name(old_name);

      if (!method) {
        badtree(n, "qparse::GroupDef::get_methods() vector contains nullptr");
        throw QError();
      }

      items.push_back(method);
    }

    for (auto it = n->get_static_methods().begin(); it != n->get_static_methods().end(); ++it) {
      qparse::FnDecl *cur_meth = *it;
      auto old_name = cur_meth->get_name();
      cur_meth->set_name(ns_join(n->get_name(), old_name));

      auto method = qconv_one(s, *it);

      cur_meth->set_name(old_name);

      if (!method) {
        badtree(n, "qparse::GroupDef::get_static_methods() vector contains nullptr");
        throw QError();
      }

      items.push_back(method);
    }

    return items;
  }

  static std::vector<Expr *> qconv_union(ConvState &s, qparse::UnionDef *n) {
    /**
     * @brief Convert a union definition to a qxir sequence.
     * @details This is a 1-to-1 conversion of the union definition.
     */

    std::string name = s.cur_named(n->get_name());
    auto sv = memorize(std::string_view(name));

    if (current->getTypeMap().contains(sv)) {
      report(IssueCode::TypeRedefinition, IssueClass::Error, n->get_name(), n->get_start_pos(),
             n->get_end_pos());
    }

    UnionFields fields;
    std::vector<Expr *> items;

    for (auto it = n->get_fields().begin(); it != n->get_fields().end(); ++it) {
      if (!*it) {
        badtree(n, "qparse::UnionDef::get_fields() vector contains nullptr");
        throw QError();
      }

      s.composite_expanse.push((*it)->get_name());
      auto field = qconv_one(s, *it);
      s.composite_expanse.pop();

      fields.push_back(field->asType());
    }

    UnionTy *st = create<UnionTy>(std::move(fields));

    current->getTypeMap()[sv] = st;

    for (auto it = n->get_methods().begin(); it != n->get_methods().end(); ++it) {
      qparse::FnDecl *cur_meth = *it;
      auto old_name = cur_meth->get_name();
      cur_meth->set_name(ns_join(n->get_name(), old_name));

      auto method = qconv_one(s, *it);

      cur_meth->set_name(old_name);

      if (!method) {
        badtree(n, "qparse::UnionDef::get_methods() vector contains nullptr");
        throw QError();
      }

      items.push_back(method);
    }

    for (auto it = n->get_static_methods().begin(); it != n->get_static_methods().end(); ++it) {
      qparse::FnDecl *cur_meth = *it;
      auto old_name = cur_meth->get_name();
      cur_meth->set_name(ns_join(n->get_name(), old_name));

      auto method = qconv_one(s, *it);

      cur_meth->set_name(old_name);

      if (!method) {
        badtree(n, "qparse::UnionDef::get_static_methods() vector contains nullptr");
        throw QError();
      }

      items.push_back(method);
    }

    return items;
  }

  static std::vector<Expr *> qconv_enum(ConvState &s, qparse::EnumDef *n) {
    /**
     * @brief Convert an enum definition to a qxir sequence.
     * @details Extrapolate the fields by adding 1 to the previous field value.
     */

    std::string name = s.cur_named(n->get_name());
    auto sv = memorize(std::string_view(name));

    if (current->getTypeMap().contains(sv)) {
      report(IssueCode::TypeRedefinition, IssueClass::Error, n->get_name(), n->get_start_pos(),
             n->get_end_pos());
    }

    Type *type = nullptr;
    if (n->get_type()) {
      type = qconv_one(s, n->get_type())->asType();
      if (!type) {
        badtree(n, "qparse::EnumDef::get_type() == nullptr");
        throw QError();
      }
    } else {
      type = create<Tmp>(TmpType::ENUM, sv)->asType();
    }

    current->getTypeMap()[sv] = type->asType();

    Expr *last = nullptr;

    for (auto it = n->get_items().begin(); it != n->get_items().end(); ++it) {
      Expr *cur = nullptr;

      if (it->second) {
        cur = qconv_one(s, it->second);
        if (!cur) {
          badtree(n, "qparse::EnumDef::get_items() vector contains nullptr");
          throw QError();
        }

        last = cur;
      } else {
        if (!last) {
          last = cur = create<Int>(0);
        } else {
          last = cur = create<BinExpr>(last, create<Int>(1), Op::Plus);
        }
      }

      std::string_view field_name =
          memorize(std::string_view(name + "::" + std::string(it->first)));

      current->getNamedConstants().insert({field_name, cur});
    }

    return {};
  }

  static Expr *qconv_fn(ConvState &s, qparse::FnDef *n) {
    bool old_inside_function = s.inside_function;
    s.inside_function = true;

    Expr *precond = nullptr, *postcond = nullptr;
    Seq *body = nullptr;
    Params params;
    qparse::FnDecl *decl = n;
    qparse::FuncTy *fty = decl->get_type();

    FnTy *fnty = static_cast<FnTy *>(qconv_one(s, fty));
    if (!fnty) {
      badtree(n, "qparse::FnDef::get_type() == nullptr");
      throw QError();
    }

    /* Produce the function preconditions */
    if ((precond = qconv_one(s, n->get_precond()))) {
      precond = create<If>(create<UnExpr>(precond, Op::LogicNot),
                           create_simple_call(s, "__detail::precond_fail"), createIgn());
    }

    /* Produce the function postconditions */
    if ((postcond = qconv_one(s, n->get_postcond()))) {
      postcond = create<If>(create<UnExpr>(postcond, Op::LogicNot),
                            create_simple_call(s, "__detail::postcond_fail"), createIgn());
    }

    { /* Produce the function body */
      Type *old_return_ty = s.return_type;
      s.return_type = fnty->getReturn();
      s.local_scope.push({});

      Expr *tmp = qconv_one(s, n->get_body());
      if (!tmp) {
        badtree(n, "qparse::FnDef::get_body() == nullptr");
        throw QError();
      }

      if (tmp->getKind() != QIR_NODE_SEQ) {
        tmp = create<Seq>(SeqItems({tmp}));
      }

      Seq *seq = tmp->as<Seq>();

      { /* Implicit return */
        if (fty->get_return_ty()->is_void()) {
          if (!seq->getItems().empty()) {
            if (seq->getItems().back()->getKind() != QIR_NODE_RET) {
              seq->getItems().push_back(create<Ret>(create<VoidTy>()));
            }
          }
        }
      }

      body = seq;

      if (precond) {
        body->getItems().insert(body->getItems().begin(), precond);
      }
      if (postcond) {
        /// TODO: add postcond at each exit point
      }

      s.local_scope.pop();
      s.return_type = old_return_ty;
    }

    auto name = s.cur_named(n->get_name());
    auto str = memorize(std::string_view(name));

    current->getParameterMap()[str] = {};

    { /* Produce the function parameters */
      for (auto it = fty->get_params().begin(); it != fty->get_params().end(); ++it) {
        /**
         * Parameter properties:
         * 1. Name - All parameters have a name.
         * 2. Type - All parameters have a type.
         * 3. Default - Optional, if the parameter has a default value.
         * 4. Position - All parameters have a position.
         */

        Type *type = qconv_one(s, std::get<1>(*it))->asType();
        if (!type) {
          badtree(n, "qparse::FnDef::get_type() == nullptr");
          throw QError();
        }

        Expr *def = nullptr;
        if (std::get<2>(*it)) {
          def = qconv_one(s, std::get<2>(*it));
          if (!def) {
            badtree(n, "qparse::FnDef::get_type() == nullptr");
            throw QError();
          }
        }

        std::string_view sv = memorize(std::string_view(std::get<0>(*it)));

        params.push_back({type, sv});
        current->getParameterMap()[str].push_back({std::string(std::get<0>(*it)), type, def});
      }
    }

    auto obj =
        create<Fn>(str, std::move(params), fnty->getReturn(), body, fty->is_variadic(), s.abi_mode);

    current->getFunctions().insert({str, {fnty, obj}});

    s.inside_function = old_inside_function;
    return obj;
  }

  static std::vector<Expr *> qconv_subsystem(ConvState &s, qparse::SubsystemDecl *n) {
    /**
     * @brief Convert a subsystem declaration to a qxir sequence with
     * namespace prefixes.
     */

    std::vector<Expr *> items;

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
      auto item = qconv_any(s, *it);

      items.insert(items.end(), item.begin(), item.end());
    }

    s.ns_prefix = old_ns;

    return items;
  }

  static std::vector<Expr *> qconv_export(ConvState &s, qparse::ExportDecl *n) {
    /**
     * @brief Convert an export declaration to a qxir export node.
     * @details Convert a list of statements under a common ABI into a
     * sequence under a common ABI.
     */

    AbiTag old = s.abi_mode;

    if (n->get_abi_name().empty()) {
      s.abi_mode = AbiTag::Default;
    } else if (n->get_abi_name() == "q") {
      s.abi_mode = AbiTag::QUIX;
    } else if (n->get_abi_name() == "c") {
      s.abi_mode = AbiTag::C;
    } else {
      badtree(n, "qparse::ExportDecl abi name is not supported: '" + n->get_abi_name() + "'");
      throw QError();
    }

    if (!n->get_body()) {
      badtree(n, "qparse::ExportDecl::get_body() == nullptr");
      throw QError();
    }

    std::string_view abi_name;

    if (n->get_abi_name().empty()) {
      abi_name = "std";
    } else {
      abi_name = memorize(n->get_abi_name());
    }

    std::vector<qxir::Expr *> items;

    for (auto it = n->get_body()->get_items().begin(); it != n->get_body()->get_items().end();
         ++it) {
      auto result = qconv_any(s, *it);
      for (auto &item : result) {
        items.push_back(create<Extern>(item, abi_name));
      }
    }

    s.abi_mode = old;

    return items;
  }

  static Expr *qconv_composite_field(ConvState &s, qparse::CompositeField *n) {
    auto type = qconv_one(s, n->get_type());
    if (!type) {
      badtree(n, "qparse::CompositeField::get_type() == nullptr");
      throw QError();
    }

    Expr *_def = nullptr;
    if (n->get_value()) {
      _def = qconv_one(s, n->get_value());
      if (!_def) {
        badtree(n, "qparse::CompositeField::get_value() == nullptr");
        throw QError();
      }
    }

    if (s.composite_expanse.empty()) {
      badtree(n, "state.composite_expanse.empty()");
      throw QError();
    }

    std::string_view dt_name = memorize(s.composite_expanse.top());

    current->getCompositeFields()[dt_name].push_back(
        {std::string(n->get_name()), type->asType(), _def});

    return type;
  }

  static Expr *qconv_block(ConvState &s, qparse::Block *n) {
    /**
     * @brief Convert a scope block into an expression sequence.
     * @details A QXIR sequence is a list of expressions (a sequence point).
     *          This is equivalent to a scope block.
     */

    SeqItems items;
    items.reserve(n->get_items().size());

    s.local_scope.push({});

    for (auto it = n->get_items().begin(); it != n->get_items().end(); ++it) {
      auto item = qconv_any(s, *it);

      items.insert(items.end(), item.begin(), item.end());
    }

    s.local_scope.pop();

    return create<Seq>(std::move(items));
  }

  static Expr *qconv_const(ConvState &s, qparse::ConstDecl *n) {
    Expr *init = qconv_one(s, n->get_value());
    Type *type = nullptr;
    if (n->get_type()) {
      Expr *tmp = qconv_one(s, n->get_type());
      if (tmp) {
        type = tmp->asType();
      }
    }

    if (init && type) {
      init = create<BinExpr>(init, type, Op::CastAs);
    } else if (!init && type) {
      init = type;
    } else if (!init && !type) {
      badtree(
          n, "parse::ConstDecl::get_value() == nullptr && parse::ConstDecl::get_type() == nullptr");
      throw QError();
    }

    if (s.inside_function) {
      std::string_view name = memorize(n->get_name());
      Local *local = create<Local>(name, init, s.abi_mode);
      local->setMutable(false);

      qcore_assert(!s.local_scope.empty());
      if (s.local_scope.top().contains(name)) {
        report(IssueCode::VariableRedefinition, IssueClass::Error, n->get_name(),
               n->get_start_pos(), n->get_end_pos());
      }
      s.local_scope.top()[name] = local;
      return local;
    } else {
      std::string_view name = memorize(std::string_view(s.cur_named(n->get_name())));
      auto g = create<Local>(name, init, s.abi_mode);
      g->setMutable(false);
      current->getGlobalVariables().insert({name, g});
      return g;
    }
  }

  static Expr *qconv_var(ConvState &, qparse::VarDecl *) {
    /// TODO: var

    throw QError();
  }

  static Expr *qconv_let(ConvState &s, qparse::LetDecl *n) {
    Expr *init = qconv_one(s, n->get_value());
    Type *type = nullptr;
    if (n->get_type()) {
      Expr *tmp = qconv_one(s, n->get_type());
      if (tmp) {
        type = tmp->asType();
      }
    }

    if (init && type) {
      init = create<BinExpr>(init, type, Op::CastAs);
    } else if (!init && type) {
      init = type;
    } else if (!init && !type) {
      badtree(
          n, "parse::ConstDecl::get_value() == nullptr && parse::ConstDecl::get_type() == nullptr");
      throw QError();
    }

    if (s.inside_function) {
      std::string_view name = memorize(n->get_name());
      Local *local = create<Local>(name, init, s.abi_mode);

      qcore_assert(!s.local_scope.empty());
      if (s.local_scope.top().contains(name)) {
        report(IssueCode::VariableRedefinition, IssueClass::Error, n->get_name(),
               n->get_start_pos(), n->get_end_pos());
      }
      s.local_scope.top()[name] = local;
      return local;
    } else {
      std::string_view name = memorize(std::string_view(s.cur_named(n->get_name())));
      auto g = create<Local>(name, init, s.abi_mode);
      current->getGlobalVariables().insert({name, g});
      return g;
    }
  }

  static Expr *qconv_inline_asm(ConvState &, qparse::InlineAsm *) {
    qcore_implement("qconv_inline_asm");
  }

  static Expr *qconv_return(ConvState &s, qparse::ReturnStmt *n) {
    /**
     * @brief Convert a return statement to a qxir expression.
     * @details This is a 1-to-1 conversion of the return statement.
     */

    auto val = qconv_one(s, n->get_value());
    if (!val) {
      val = create<VoidTy>();
    }
    val = create<BinExpr>(val, s.return_type, Op::CastAs);

    return create<Ret>(val);
  }

  static Expr *qconv_retif(ConvState &s, qparse::ReturnIfStmt *n) {
    /**
     * @brief Convert a return statement to a qxir expression.
     * @details Lower into an 'if (cond) {return val}' expression.
     */

    auto cond = qconv_one(s, n->get_cond());
    if (!cond) {
      badtree(n, "qparse::ReturnIfStmt::get_cond() == nullptr");
      throw QError();
    }

    cond = create<BinExpr>(cond, create<U1Ty>(), Op::CastAs);

    auto val = qconv_one(s, n->get_value());
    if (!val) {
      badtree(n, "qparse::ReturnIfStmt::get_value() == nullptr");
      throw QError();
    }
    val = create<BinExpr>(val, s.return_type, Op::CastAs);

    return create<If>(cond, create<Ret>(val), createIgn());
  }

  static Expr *qconv_retz(ConvState &s, qparse::RetZStmt *n) {
    /**
     * @brief Convert a return statement to a qxir expression.
     * @details Lower into an 'if (!cond) {return val}' expression.
     */

    auto cond = qconv_one(s, n->get_cond());
    if (!cond) {
      badtree(n, "qparse::RetZStmt::get_cond() == nullptr");
      throw QError();
    }
    cond = create<BinExpr>(cond, create<U1Ty>(), Op::CastAs);

    auto inv_cond = create<UnExpr>(cond, Op::LogicNot);

    auto val = qconv_one(s, n->get_value());
    if (!val) {
      badtree(n, "qparse::RetZStmt::get_value() == nullptr");
      throw QError();
    }
    val = create<BinExpr>(val, s.return_type, Op::CastAs);

    return create<If>(inv_cond, create<Ret>(val), createIgn());
  }

  static Expr *qconv_retv(ConvState &s, qparse::RetVStmt *n) {
    /**
     * @brief Convert a return statement to a qxir expression.
     * @details Lower into an 'if (cond) {return void}' expression.
     */

    auto cond = qconv_one(s, n->get_cond());
    if (!cond) {
      badtree(n, "qparse::RetVStmt::get_cond() == nullptr");
      throw QError();
    }
    cond = create<BinExpr>(cond, create<U1Ty>(), Op::CastAs);

    return create<If>(cond, create<Ret>(createIgn()), createIgn());
  }

  static Expr *qconv_break(ConvState &, qparse::BreakStmt *) {
    /**
     * @brief Convert a break statement to a qxir expression.
     * @details This is a 1-to-1 conversion of the break statement.
     */

    return create<Brk>();
  }

  static Expr *qconv_continue(ConvState &, qparse::ContinueStmt *) {
    /**
     * @brief Convert a continue statement to a qxir expression.
     * @details This is a 1-to-1 conversion of the continue statement.
     */

    return create<Cont>();
  }

  static Expr *qconv_if(ConvState &s, qparse::IfStmt *n) {
    /**
     * @brief Convert an if statement to a qxir expression.
     * @details The else branch is optional, and if it is missing, it is
     *        replaced with a void expression.
     */

    auto cond = qconv_one(s, n->get_cond());
    auto then = qconv_one(s, n->get_then());
    auto els = qconv_one(s, n->get_else());

    if (!cond) {
      badtree(n, "qparse::IfStmt::get_cond() == nullptr");
      throw QError();
    }
    cond = create<BinExpr>(cond, create<U1Ty>(), Op::CastAs);

    if (!then) {
      badtree(n, "qparse::IfStmt::get_then() == nullptr");
      throw QError();
    }

    if (!els) {
      els = createIgn();
    }

    return create<If>(cond, then, els);
  }

  static Expr *qconv_while(ConvState &s, qparse::WhileStmt *n) {
    /**
     * @brief Convert a while loop to a qxir expression.
     * @details If any of the sub-expressions are missing, they are replaced
     *         with a default value of 1.
     */

    auto cond = qconv_one(s, n->get_cond());
    auto body = qconv_one(s, n->get_body());

    if (!cond) {
      cond = create<Int>(1);
    }

    cond = create<BinExpr>(cond, create<U1Ty>(), Op::CastAs);

    if (!body) {
      body = create<Seq>(SeqItems({}));
    } else if (body->getKind() != QIR_NODE_SEQ) {
      body = create<Seq>(SeqItems({body}));
    }

    return create<While>(cond, body->as<Seq>());
  }

  static Expr *qconv_for(ConvState &s, qparse::ForStmt *n) {
    /**
     * @brief Convert a for loop to a qxir expression.
     * @details If any of the sub-expressions are missing, they are replaced
     *         with a default value of 1.
     */

    auto init = qconv_one(s, n->get_init());
    auto cond = qconv_one(s, n->get_cond());
    auto step = qconv_one(s, n->get_step());
    auto body = qconv_one(s, n->get_body());

    if (!init) {
      init = create<Int>(1);
    }

    if (!cond) {
      cond = create<Int>(1);  // infinite loop like 'for (;;) {}'
      cond = create<BinExpr>(cond, create<U1Ty>(), Op::CastAs);
    }

    if (!step) {
      step = create<Int>(1);
    }

    if (!body) {
      body = create<Int>(1);
    }

    return create<For>(init, cond, step, body);
  }

  static Expr *qconv_form(ConvState &s, qparse::FormStmt *n) {
    /**
     * @brief Convert a form loop to a qxir expression.
     * @details This is a 1-to-1 conversion of the form loop.
     */

    auto maxjobs = qconv_one(s, n->get_maxjobs());
    if (!maxjobs) {
      badtree(n, "qparse::FormStmt::get_maxjobs() == nullptr");
      throw QError();
    }

    auto idx_name = memorize(n->get_idx_ident());
    auto val_name = memorize(n->get_val_ident());

    auto iter = qconv_one(s, n->get_expr());
    if (!iter) {
      badtree(n, "qparse::FormStmt::get_expr() == nullptr");
      throw QError();
    }

    auto body = qconv_one(s, n->get_body());
    if (!body) {
      badtree(n, "qparse::FormStmt::get_body() == nullptr");
      throw QError();
    }

    return create<Form>(idx_name, val_name, maxjobs, iter, create<Seq>(SeqItems({body})));
  }

  static Expr *qconv_foreach(ConvState &, qparse::ForeachStmt *) {
    /**
     * @brief Convert a foreach loop to a qxir expression.
     * @details This is a 1-to-1 conversion of the foreach loop.
     */

    // auto idx_name = memorize(n->get_idx_ident());
    // auto val_name = memorize(n->get_val_ident());

    // auto iter = qconv_one(s, n->get_expr());
    // if (!iter) {
    //   badtree(n, "qparse::ForeachStmt::get_expr() == nullptr");
    //   throw QError();
    // }

    // auto body = qconv_one(s, n->get_body());
    // if (!body) {
    //   badtree(n, "qparse::ForeachStmt::get_body() == nullptr");
    //   throw QError();
    // }

    // return create<Foreach>(idx_name, val_name, iter, create<Seq>(SeqItems({body})));
    qcore_implement(__func__);
  }

  static Expr *qconv_case(ConvState &s, qparse::CaseStmt *n) {
    /**
     * @brief Convert a case statement to a qxir expression.
     * @details This is a 1-to-1 conversion of the case statement.
     */

    auto cond = qconv_one(s, n->get_cond());
    if (!cond) {
      badtree(n, "qparse::CaseStmt::get_cond() == nullptr");
      throw QError();
    }

    auto body = qconv_one(s, n->get_body());
    if (!body) {
      badtree(n, "qparse::CaseStmt::get_body() == nullptr");
      throw QError();
    }

    return create<Case>(cond, body);
  }

  static Expr *qconv_switch(ConvState &s, qparse::SwitchStmt *n) {
    /**
     * @brief Convert a switch statement to a qxir expression.
     * @details If the default case is missing, it is replaced with a void
     *        expression.
     */

    auto cond = qconv_one(s, n->get_cond());
    if (!cond) {
      badtree(n, "qparse::SwitchStmt::get_cond() == nullptr");
      throw QError();
    }

    SwitchCases cases;
    for (auto it = n->get_cases().begin(); it != n->get_cases().end(); ++it) {
      auto item = qconv_one(s, *it);
      if (!item) {
        badtree(n, "qparse::SwitchStmt::get_cases() vector contains nullptr");
        throw QError();
      }

      cases.push_back(item->as<Case>());
    }

    Expr *def = nullptr;
    if (n->get_default()) {
      def = qconv_one(s, n->get_default());
      if (!def) {
        badtree(n, "qparse::SwitchStmt::get_default() == nullptr");
        throw QError();
      }
    } else {
      def = createIgn();
    }

    return create<Switch>(cond, std::move(cases), def);
  }

  static Expr *qconv_expr_stmt(ConvState &s, qparse::ExprStmt *n) {
    /**
     * @brief Convert an expression inside a statement to a qxir expression.
     * @details This is a 1-to-1 conversion of the expression statement.
     */

    return qconv_one(s, n->get_expr());
  }

  static Expr *qconv_volstmt(ConvState &, qparse::VolStmt *) {
    /**
     * @brief Convert a volatile statement to a qxir volatile expression.
     * @details This is a 1-to-1 conversion of the volatile statement.
     */

    // auto expr = qconv_one(s, n->get_stmt());
    // expr->setVolatile(true);

    // return expr;

    qcore_implement(__func__);
  }
}  // namespace qxir

static qxir::Expr *qconv_one(ConvState &s, qparse::Node *n) {
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

    case QAST_NODE_REF_TY:
      out = qconv_ref_ty(s, n->as<qparse::RefTy>());
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

    case QAST_NODE_F16_TY:
      out = qconv_f16_ty(s, n->as<qparse::F16>());
      break;

    case QAST_NODE_F32_TY:
      out = qconv_f32_ty(s, n->as<qparse::F32>());
      break;

    case QAST_NODE_F64_TY:
      out = qconv_f64_ty(s, n->as<qparse::F64>());
      break;

    case QAST_NODE_F128_TY:
      out = qconv_f128_ty(s, n->as<qparse::F128>());
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

    case QAST_NODE_TUPLE_TY:
      out = qconv_tuple_ty(s, n->as<qparse::TupleTy>());
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

    case QAST_NODE_TEMPL_TY:
      out = qconv_templ_ty(s, n->as<qparse::TemplType>());
      break;

    case QAST_NODE_FNDECL:
      out = qconv_fndecl(s, n->as<qparse::FnDecl>());
      break;

    case QAST_NODE_FN:
      out = qconv_fn(s, n->as<qparse::FnDef>());
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

  out->setLocDangerous({n->get_start_pos(), n->get_end_pos()});

  return out;
}

static std::vector<qxir::Expr *> qconv_any(ConvState &s, qparse::Node *n) {
  using namespace qxir;

  if (!n) {
    return {};
  }

  std::vector<qxir::Expr *> out;

  switch (n->this_typeid()) {
    case QAST_NODE_TYPEDEF:
      out = qconv_typedef(s, n->as<qparse::TypedefDecl>());
      break;

    case QAST_NODE_ENUM:
      out = qconv_enum(s, n->as<qparse::EnumDef>());
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

    case QAST_NODE_SUBSYSTEM:
      out = qconv_subsystem(s, n->as<qparse::SubsystemDecl>());
      break;

    case QAST_NODE_EXPORT:
      out = qconv_export(s, n->as<qparse::ExportDecl>());
      break;

    default: {
      auto expr = qconv_one(s, n);
      if (expr) {
        out.push_back(expr);
      } else {
        badtree(n, "qxir::qconv_any() failed to convert node");
        throw QError();
      }
    }
  }

  return out;
}

///=============================================================================

static qxir_node_t *qxir_clone_impl(const qxir_node_t *_node,
                                    std::unordered_map<const qxir_node_t *, qxir_node_t *> &map,
                                    std::unordered_set<qxir_node_t *> &in_visited) {
#define clone(X) static_cast<Expr *>(qxir_clone_impl(X, map, in_visited))

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
    case QIR_NODE_LIST: {
      ListItems items;
      items.reserve(static_cast<List *>(in)->getItems().size());
      for (auto item : static_cast<List *>(in)->getItems()) {
        items.push_back(clone(item));
      }
      out = create<List>(std::move(items));
      break;
    }
    case QIR_NODE_CALL: {
      Call *n = static_cast<Call *>(in);
      CallArgs args;
      for (auto arg : n->getArgs()) {
        args.push_back(clone(arg));
      }
      out = create<Call>(clone(n->getTarget()), std::move(args));
      break;
    }
    case QIR_NODE_SEQ: {
      SeqItems items;
      items.reserve(static_cast<Seq *>(in)->getItems().size());
      for (auto item : static_cast<Seq *>(in)->getItems()) {
        items.push_back(clone(item));
      }
      out = create<Seq>(std::move(items));
      break;
    }
    case QIR_NODE_INDEX: {
      Index *n = static_cast<Index *>(in);
      out = create<Index>(clone(n->getExpr()), clone(n->getIndex()));
      break;
    }
    case QIR_NODE_IDENT: {
      qxir::Expr *bad_tmp_ref = static_cast<Ident *>(in)->getWhat();
      out = create<Ident>(memorize(static_cast<Ident *>(in)->getName()), bad_tmp_ref);
      break;
    }
    case QIR_NODE_EXTERN: {
      Extern *n = static_cast<Extern *>(in);
      out = create<Extern>(clone(n->getValue()), memorize(n->getAbiName()));
      break;
    }
    case QIR_NODE_LOCAL: {
      Local *n = static_cast<Local *>(in);
      out = create<Local>(memorize(n->getName()), clone(n->getValue()), n->getAbiTag());
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
      out = create<While>(clone(n->getCond()), clone(n->getBody())->as<Seq>());
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
    case QIR_NODE_CASE: {
      Case *n = static_cast<Case *>(in);
      out = create<Case>(clone(n->getCond()), clone(n->getBody()));
      break;
    }
    case QIR_NODE_SWITCH: {
      Switch *n = static_cast<Switch *>(in);
      SwitchCases cases;
      cases.reserve(n->getCases().size());
      for (auto item : n->getCases()) {
        cases.push_back(clone(item)->as<Case>());
      }
      out = create<Switch>(clone(n->getCond()), std::move(cases), clone(n->getDefault()));
      break;
    }
    case QIR_NODE_FN: {
      Fn *n = static_cast<Fn *>(in);
      Params params;
      params.reserve(n->getParams().size());
      for (auto param : n->getParams()) {
        params.push_back({clone(param.first)->asType(), memorize(param.second)});
      }

      out = create<Fn>(memorize(n->getName()), std::move(params), clone(n->getReturn())->asType(),
                       clone(n->getBody())->as<Seq>(), n->isVariadic(), n->getAbiTag());
      break;
    }
    case QIR_NODE_ASM: {
      qcore_implement("QIR_NODE_ASM cloning");
      break;
    }
    case QIR_NODE_IGN: {
      out = createIgn();
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
    case QIR_NODE_STRUCT_TY: {
      StructFields fields;
      fields.reserve(static_cast<StructTy *>(in)->getFields().size());
      for (auto field : static_cast<StructTy *>(in)->getFields()) {
        fields.push_back(clone(field)->asType());
      }
      out = create<StructTy>(std::move(fields));
      break;
    }
    case QIR_NODE_UNION_TY: {
      UnionFields fields;
      fields.reserve(static_cast<UnionTy *>(in)->getFields().size());
      for (auto field : static_cast<UnionTy *>(in)->getFields()) {
        fields.push_back(clone(field)->asType());
      }
      out = create<UnionTy>(std::move(fields));
      break;
    }
    case QIR_NODE_ARRAY_TY: {
      ArrayTy *n = static_cast<ArrayTy *>(in);
      out = create<ArrayTy>(clone(n->getElement())->asType(), n->getCount());
      break;
    }
    case QIR_NODE_FN_TY: {
      FnTy *n = static_cast<FnTy *>(in);
      FnParams params;
      params.reserve(n->getParams().size());
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
  }

  qcore_assert(out != nullptr, "qxir_clone: failed to clone node");

  out->setLocDangerous(in->getLoc());
  out->setMutable(in->isMutable());

  map[in] = out;
  in_visited.insert(in);

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
  std::swap(qxir::current, dst);

  out = nullptr;

  try {
    std::unordered_map<const qxir_node_t *, qxir_node_t *> map;
    std::unordered_set<qxir_node_t *> in_visited;
    out = qxir_clone_impl(node, map, in_visited);

    { /* Resolve Directed Acyclic* Graph Internal References */
      using namespace qxir;

      Expr *out_expr = static_cast<Expr *>(out);
      iterate<dfs_pre>(out_expr, [&](Expr *, Expr **_cur) -> IterOp {
        Expr *cur = *_cur;

        if (in_visited.contains(cur)) {
          *_cur = static_cast<Expr *>(map[cur]);
        }

        return IterOp::Proceed;
      });
      out = out_expr;
    }
  } catch (...) {
    return nullptr;
  }

  std::swap(qxir::current, dst);
  std::swap(qxir::qxir_arena.get(), dst->getNodeArena());

  return static_cast<qxir_node_t *>(out);
}
