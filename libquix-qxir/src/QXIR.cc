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

static qxir::Expr *qconv(qxir_t *m, const qparse::Node *node);

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
        *out = qconv(qxir, static_cast<const qparse::Node *>(qxir->root));
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

static std::string_view memorize(qxir_t *m, std::string_view sv) {
  return m->impl->push_string(sv);
}

static std::string_view memorize(qxir_t *m, qparse::String sv) {
  return memorize(m, std::string_view(sv.data(), sv.size()));
}

qxir::Op qconv_op(qlex_op_t op) {
  /// TODO: Fix me; lower operators
  (void)op;

  return qxir::Op::Times;

  throw QError();

}

static qxir::Expr *qconv(qxir_t *m, const qparse::Node *node) {
  using namespace qparse;

  if (!node) {
    return nullptr;
  }

  qxir::Expr *out = nullptr;

  switch (node->this_typeid()) {
    case QAST_NODE_CEXPR: {
      auto cur = node->as<ConstExpr>();
      out = qconv(m, cur->get_value());
      out->setConst(true);
      break;
    }
    case QAST_NODE_BINEXPR: {
      auto cur = node->as<BinExpr>();
      auto lhs = qconv(m, cur->get_lhs());
      auto rhs = qconv(m, cur->get_rhs());
      auto op = qconv_op(cur->get_op());
      out = qxir::create<qxir::BinExpr>(lhs, rhs, op);
      break;
    }
    case QAST_NODE_UNEXPR: {
      auto cur = node->as<UnaryExpr>();
      auto op = qconv_op(cur->get_op());
      auto val = qconv(m, cur->get_rhs());
      out = qxir::create<qxir::UnExpr>(val, op);
      break;
    }
    case QAST_NODE_TEREXPR: {
      auto cur = node->as<TernaryExpr>();
      auto cond = qconv(m, cur->get_cond());
      auto then = qconv(m, cur->get_lhs());
      auto els = qconv(m, cur->get_rhs());
      out = qxir::create<qxir::If>(cond, then, els);
      break;
    }
    case QAST_NODE_INT: {
      out = qxir::create<qxir::Int>(node->as<ConstInt>()->get_value().data());
      break;
    }
    case QAST_NODE_FLOAT: {
      out = qxir::create<qxir::Float>(node->as<ConstFloat>()->get_value().data());
      break;
    }
    case QAST_NODE_STRING: {
      out = qxir::create<qxir::String>(node->as<ConstString>()->get_value().data());
      break;
    }
    case QAST_NODE_CHAR: {
      out = qxir::create<qxir::Int>(node->as<ConstChar>()->get_value());
      break;
    }
    case QAST_NODE_BOOL: {
      auto cur = node->as<ConstBool>();
      if (cur->get_value()) {
        out = qxir::create<qxir::Int>(1);
      } else {
        out = qxir::create<qxir::Int>(0);
      }
      break;
    }
    case QAST_NODE_NULL: {
      auto cur = node->as<ConstNull>();
      (void)cur;
      /// TODO:
      break;
    }
    case QAST_NODE_UNDEF: {
      auto cur = node->as<ConstUndef>();
      (void)cur;
      /// TODO:
      break;
    }
    case QAST_NODE_CALL: {
      auto cur = node->as<Call>();
      (void)cur;
      /// TODO:
      break;
    }
    case QAST_NODE_LIST: {
      auto cur = node->as<List>();
      qxir::ListItems items;
      for (auto &i : cur->get_items()) {
        items.push_back(qconv(m, i));
      }
      out = qxir::create<qxir::List>(std::move(items));
      break;
    }
    case QAST_NODE_ASSOC: {
      auto cur = node->as<Assoc>();
      (void)cur;
      /// TODO:
      break;
    }
    case QAST_NODE_FIELD: {
      auto cur = node->as<Field>();
      (void)cur;
      /// TODO:
      break;
    }
    case QAST_NODE_INDEX: {
      auto cur = node->as<Index>();
      out = qxir::create<qxir::Index>(qconv(m, cur->get_base()), qconv(m, cur->get_index()));
      break;
    }
    case QAST_NODE_SLICE: {
      auto cur = node->as<Slice>();
      (void)cur;
      /// TODO:
      break;
    }
    case QAST_NODE_FSTRING: {
      auto cur = node->as<FString>();
      (void)cur;
      /// TODO:
      break;
    }
    case QAST_NODE_IDENT: {
      out = qxir::create<qxir::Ident>(memorize(m, node->as<Ident>()->get_name()));
      break;
    }
    case QAST_NODE_SEQ_POINT: {
      auto cur = node->as<SeqPoint>();
      qxir::SeqItems items;
      for (auto &s : cur->get_items()) {
        items.push_back(qconv(m, s));
      }
      out = qxir::create<qxir::Seq>(std::move(items));
      break;
    }
    case QAST_NODE_POST_UNEXPR: {
      auto cur = node->as<PostUnaryExpr>();
      auto op = qconv_op(cur->get_op());
      auto val = qconv(m, cur->get_lhs());
      out = qxir::create<qxir::PostUnExpr>(val, op);
      break;
    }
    case QAST_NODE_STMT_EXPR: {
      out = qconv(m, node->as<StmtExpr>()->get_stmt());
      break;
    }
    case QAST_NODE_TYPE_EXPR: {
      out = qconv(m, node->as<TypeExpr>()->get_type());
      break;
    }
    case QAST_NODE_TEMPL_CALL: {
      auto cur = node->as<TemplCall>();
      (void)cur;
      /// TODO:
      break;
    }
    case QAST_NODE_MUT_TY: {
      auto cur = node->as<MutTy>();
      (void)cur;
      /// TODO:
      break;
    }
    case QAST_NODE_U1_TY: {
      out = qxir::getType<qxir::U1Ty>();
      break;
    }
    case QAST_NODE_U8_TY: {
      out = qxir::getType<qxir::U8Ty>();
      break;
    }
    case QAST_NODE_U16_TY: {
      out = qxir::getType<qxir::U16Ty>();
      break;
    }
    case QAST_NODE_U32_TY: {
      out = qxir::getType<qxir::U32Ty>();
      break;
    }
    case QAST_NODE_U64_TY: {
      out = qxir::getType<qxir::U64Ty>();
      break;
    }
    case QAST_NODE_U128_TY: {
      out = qxir::getType<qxir::U128Ty>();
      break;
    }
    case QAST_NODE_I8_TY: {
      out = qxir::getType<qxir::I8Ty>();
      break;
    }
    case QAST_NODE_I16_TY: {
      out = qxir::getType<qxir::I16Ty>();
      break;
    }
    case QAST_NODE_I32_TY: {
      out = qxir::getType<qxir::I32Ty>();
      break;
    }
    case QAST_NODE_I64_TY: {
      out = qxir::getType<qxir::I64Ty>();
      break;
    }
    case QAST_NODE_I128_TY: {
      out = qxir::getType<qxir::I128Ty>();
      break;
    }
    case QAST_NODE_F32_TY: {
      out = qxir::getType<qxir::F32Ty>();
      break;
    }
    case QAST_NODE_F64_TY: {
      out = qxir::getType<qxir::F64Ty>();
      break;
    }
    case QAST_NODE_VOID_TY: {
      out = qxir::getType<qxir::VoidTy>();
      break;
    }
    case QAST_NODE_PTR_TY: {
      out = qxir::create<qxir::PtrTy>(qconv(m, node->as<PtrTy>()->get_item())->asType());
      break;
    }
    case QAST_NODE_OPAQUE_TY: {
      out = qxir::create<qxir::OpaqueTy>(node->as<OpaqueTy>()->get_name());
      break;
    }
    case QAST_NODE_STRING_TY: {
      out = qxir::create<qxir::StringTy>();
      break;
    }
    case QAST_NODE_ENUM_TY: {
      out = qconv(m, node->as<EnumTy>()->get_memtype());
      break;
    }
    case QAST_NODE_STRUCT_TY: {
      auto cur = node->as<StructTy>();
      (void)cur;
      /// TODO:
      break;
    }
    case QAST_NODE_GROUP_TY: {
      auto cur = node->as<GroupTy>();
      (void)cur;
      /// TODO:
      break;
    }
    case QAST_NODE_REGION_TY: {
      auto cur = node->as<RegionTy>();
      qxir::StructFields fields;
      for (auto f : cur->get_items()) {
        fields.push_back(qconv(m, f)->asType());
      }
      out = qxir::create<qxir::StructTy>(std::move(fields));
      break;
    }
    case QAST_NODE_UNION_TY: {
      auto cur = node->as<UnionTy>();
      qxir::UnionFields fields;
      for (auto f : cur->get_items()) {
        fields.insert(qconv(m, f)->asType());
      }
      out = qxir::create<qxir::UnionTy>(std::move(fields));
      break;
    }
    case QAST_NODE_ARRAY_TY: {
      auto cur = node->as<ArrayTy>();
      auto item = qconv(m, cur->get_item());
      auto size = qconv(m, cur->get_size());
      out = qxir::create<qxir::ArrayTy>(item->asType(), size);
      break;
    }
    case QAST_NODE_VECTOR_TY: {
      auto cur = node->as<VectorTy>();
      auto inner = qconv(m, cur->get_item());
      out = qxir::create<qxir::ListTy>(inner->asType());
      break;
    }
    case QAST_NODE_MAP_TY: {
      auto cur = node->as<MapTy>();
      (void)cur;
      /// TODO:
      break;
    }
    case QAST_NODE_TUPLE_TY: {
      auto cur = node->as<TupleTy>();
      (void)cur;
      /// TODO:
      break;
    }
    case QAST_NODE_SET_TY: {
      auto cur = node->as<SetTy>();
      (void)cur;
      /// TODO:
      break;
    }
    case QAST_NODE_RESULT_TY: {
      auto cur = node->as<OptionalTy>();
      (void)cur;
      /// TODO:
      break;
    }
    case QAST_NODE_FN_TY: {
      auto cur = node->as<FuncTy>();
      (void)cur;
      /// TODO:
      break;
    }
    case QAST_NODE_UNRES_TY: {
      auto cur = node->as<UnresolvedType>();
      (void)cur;
      /// TODO:
      break;
    }
    case QAST_NODE_TYPEDEF: {
      auto cur = node->as<TypedefDecl>();
      (void)cur;
      /// TODO:
      break;
    }
    case QAST_NODE_FNDECL: {
      auto cur = node->as<FnDecl>();
      (void)cur;
      /// TODO:
      break;
    }
    case QAST_NODE_STRUCT: {
      auto cur = node->as<StructDef>();
      (void)cur;
      /// TODO:
      break;
    }
    case QAST_NODE_REGION: {
      auto cur = node->as<RegionDef>();
      (void)cur;
      /// TODO:
      break;
    }
    case QAST_NODE_GROUP: {
      auto cur = node->as<GroupDef>();
      (void)cur;
      /// TODO:
      break;
    }
    case QAST_NODE_UNION: {
      auto cur = node->as<UnionDef>();
      (void)cur;
      /// TODO:
      break;
    }
    case QAST_NODE_ENUM: {
      auto cur = node->as<EnumDef>();
      (void)cur;
      /// TODO:
      break;
    }
    case QAST_NODE_FN: {
      auto cur = node->as<FnDef>();
      (void)cur;
      /// TODO:
      break;
    }
    case QAST_NODE_SUBSYSTEM: {
      auto cur = node->as<SubsystemDecl>();
      (void)cur;
      /// TODO:
      break;
    }
    case QAST_NODE_EXPORT: {
      auto cur = node->as<ExportDecl>();
      (void)cur;
      /// TODO:
      break;
    }
    case QAST_NODE_COMPOSITE_FIELD: {
      auto cur = node->as<CompositeField>();
      (void)cur;
      /// TODO:
      break;
    }
    case QAST_NODE_BLOCK: {
      auto cur = node->as<Block>();
      qxir::SeqItems items;
      for (auto &s : cur->get_items()) {
        items.push_back(qconv(m, s));
      }
      out = qxir::create<qxir::Seq>(std::move(items));
      break;
    }
    case QAST_NODE_CONST: {
      auto cur = node->as<ConstDecl>();
      auto name = cur->get_name();
      if (cur->get_value() && cur->get_type()) {
        auto data = qconv(m, cur->get_value());
        auto type = qconv(m, cur->get_type());
        auto value = qxir::create<qxir::BinExpr>(data, type, qxir::Op::CastAs);
        out = qxir::create<qxir::BinExpr>(qxir::create<qxir::Ident>(memorize(m, name)), value, qxir::Op::Set);
      } else if (!cur->get_value() && cur->get_type()) {
        auto data = qxir::create<qxir::List>(qxir::ListItems());
        auto type = qconv(m, cur->get_type());
        auto value = qxir::create<qxir::BinExpr>(data, type, qxir::Op::CastAs);
        out = qxir::create<qxir::BinExpr>(qxir::create<qxir::Ident>(memorize(m, name)), value, qxir::Op::Set);
      } else if (cur->get_value() && !cur->get_type()) {
        auto data = qconv(m, cur->get_value());
        out = qxir::create<qxir::BinExpr>(qxir::create<qxir::Ident>(memorize(m, name)), data, qxir::Op::Set);
      } else {
        throw QError();
      }
      break;
    }
    case QAST_NODE_VAR: {
      auto cur = node->as<VarDecl>();
      (void)cur;
      /// TODO:
      break;
    }
    case QAST_NODE_LET: {
      auto cur = node->as<LetDecl>();
      (void)cur;
      /// TODO:
      break;
    }
    case QAST_NODE_INLINE_ASM: {
      auto cur = node->as<InlineAsm>();
      (void)cur;
      /// TODO:
      break;
    }
    case QAST_NODE_RETURN: {
      auto cur = node->as<ReturnStmt>();
      if (cur->get_value()) {
        out = qxir::create<qxir::Ret>(qconv(m, cur->get_value()));
      } else {
        out = qxir::create<qxir::Ret>(nullptr);
      }
      break;
    }
    case QAST_NODE_RETIF: {
      auto cur = node->as<ReturnIfStmt>();
      auto cond = qconv(m, cur->get_cond());
      auto val = qconv(m, cur->get_value());
      auto ret = qxir::create<qxir::Ret>(val);
      out = qxir::create<qxir::If>(cond, ret, nullptr);
      break;
    }
    case QAST_NODE_RETZ: {
      auto cur = node->as<RetZStmt>();
      (void)cur;
      /// TODO:
      break;
    }
    case QAST_NODE_RETV: {
      auto cur = node->as<RetVStmt>();
      auto cond = qconv(m, cur->get_cond());
      auto ret = qxir::create<qxir::Ret>(nullptr);
      out = qxir::create<qxir::If>(cond, ret, nullptr);
      break;
    }
    case QAST_NODE_BREAK: {
      out = qxir::create<qxir::Brk>();
      break;
    }
    case QAST_NODE_CONTINUE: {
      out = qxir::create<qxir::Cont>();
      break;
    }
    case QAST_NODE_IF: {
      auto cur = node->as<IfStmt>();
      auto cond = qconv(m, cur->get_cond());
      auto then = qconv(m, cur->get_then());
      auto els = qconv(m, cur->get_else());
      out = qxir::create<qxir::If>(cond, then, els);
      break;
    }
    case QAST_NODE_WHILE: {
      auto cur = node->as<WhileStmt>();
      auto cond = qconv(m, cur->get_cond());
      auto body = qconv(m, cur->get_body());
      out = qxir::create<qxir::While>(cond, body);
      break;
    }
    case QAST_NODE_FOR: {
      auto cur = node->as<ForStmt>();
      auto init = qconv(m, cur->get_init());
      auto cond = qconv(m, cur->get_cond());
      auto step = qconv(m, cur->get_step());
      auto body = qconv(m, cur->get_body());
      out = qxir::create<qxir::For>(init, cond, step, body);
      break;
    }
    case QAST_NODE_FORM: {
      auto cur = node->as<FormStmt>();
      auto maxjobs = qconv(m, cur->get_maxjobs());
      auto idx_var = memorize(m, cur->get_idx_ident());
      auto val_var = memorize(m, cur->get_val_ident());
      auto expr = qconv(m, cur->get_expr());
      auto stmt = qconv(m, cur->get_body());
      auto body = qxir::create<qxir::Seq>(qxir::SeqItems({stmt}));
      out = qxir::create<qxir::Form>(idx_var, val_var, maxjobs, expr, body);
      break;
    }
    case QAST_NODE_FOREACH: {
      auto cur = node->as<ForeachStmt>();
      auto idx_var = memorize(m, cur->get_idx_ident());
      auto val_var = memorize(m, cur->get_val_ident());
      auto expr = qconv(m, cur->get_expr());
      auto stmt = qconv(m, cur->get_body());
      auto seq = qxir::create<qxir::Seq>(qxir::SeqItems({stmt}));
      out = qxir::create<qxir::Foreach>(idx_var, val_var, expr, seq);
      break;
    }
    case QAST_NODE_CASE: {
      auto cur = node->as<CaseStmt>();
      auto cond = qconv(m, cur->get_cond());
      auto body = qconv(m, cur->get_body());
      out = qxir::create<qxir::Case>(cond, body);
      break;
    }
    case QAST_NODE_SWITCH: {
      auto cur = node->as<SwitchStmt>();
      auto cond = qconv(m, cur->get_cond());
      qxir::SwitchCases cases;
      for (auto &c : cur->get_cases()) {
        cases.push_back(qconv(m, c)->as<qxir::Case>());
      }
      out = qxir::create<qxir::Switch>(cond, std::move(cases));
      break;
    }
    case QAST_NODE_SLIST: {
      auto cur = node->as<StmtList>();
      qxir::SeqItems items;
      for (auto &s : cur->get_items()) {
        items.push_back(qconv(m, s));
      }
      out = qxir::create<qxir::Seq>(std::move(items));
      break;
    }
    case QAST_NODE_EXPR_STMT: {
      out = qconv(m, node->as<ExprStmt>()->get_expr());
      break;
    }
    case QAST_NODE_VOLSTMT: {
      out = qconv(m, node->as<VolStmt>()->get_stmt());;
      out->setVolatile(true);
      break;
    }
    default: {
      qcore_panicf("qxir: unknown node type: %d", static_cast<int>(node->this_typeid()));
    }
  }
  
  out->setStartLoc(node->get_start_pos());
  out->setEndLoc(node->get_end_pos());
  return out;
}
