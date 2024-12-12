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
#include <nitrate-ir/IR.h>
#include <nitrate-parser/Node.h>
#include <nitrate-parser/Parser.h>

#include <boost/multiprecision/cpp_dec_float.hpp>
#include <core/Config.hh>
#include <core/Diagnostic.hh>
#include <core/PassManager.hh>
#include <cstdint>
#include <cstring>
#include <nitrate-ir/Classes.hh>
#include <nitrate-ir/IRBuilder.hh>
#include <nitrate-ir/IRGraph.hh>
#include <nitrate-ir/Module.hh>
#include <nitrate-ir/Report.hh>
#include <stack>
#include <string>
#include <string_view>
#include <unordered_map>

using namespace nr;

struct PState {
private:
  size_t scope_ctr = 0;
  std::stack<std::string> old_scopes;

public:
  bool inside_function = false;
  std::string ns_prefix;
  std::stack<std::string_view> composite_expanse;
  nr::AbiTag abi_mode = nr::AbiTag::Internal;
  size_t anon_fn_ctr = 0;

  std::string scope_name(std::string_view suffix) const {
    if (ns_prefix.empty()) {
      return std::string(suffix);
    }
    return ns_prefix + "$$" + std::string(suffix);
  }

  std::string join_scope(std::string_view suffix) const {
    if (ns_prefix.empty()) {
      return std::string(suffix);
    }
    return ns_prefix + "::" + std::string(suffix);
  }

  void inc_scope() {
    old_scopes.push(ns_prefix);
    if (scope_ctr++ != 0) {
      ns_prefix = join_scope("__" + std::to_string(scope_ctr));
    }
  }

  void dec_scope() {
    ns_prefix = old_scopes.top();
    old_scopes.pop();
  }
};

using EResult = std::optional<Expr *>;
using BResult = std::optional<std::vector<nr::Expr *>>;

static std::optional<nr::Expr *> nrgen_one(NRBuilder &b, PState &s, IReport *G,
                                           npar_node_t *node);
static BResult nrgen_any(NRBuilder &b, PState &s, IReport *G,
                         npar_node_t *node);

#define next_one(n) nrgen_one(b, s, G, n)
#define next_any(n) nrgen_any(b, s, G, n)

C_EXPORT bool nr_lower(qmodule_t **mod, npar_node_t *base, const char *name,
                       bool diagnostics) {
  if (!mod || !base) {
    return false;
  }
  if (!name) {
    name = "module";
  }

  qcore_arena scratch_arena;
  std::swap(nr::nr_arena.get(), *scratch_arena.get());

  /// TODO: Get the target platform infoformation
  TargetInfo target_info;

  std::unique_ptr<IReport> G = std::make_unique<DiagnosticManager>();

  PState s;
  NRBuilder builder(name, target_info);

  qmodule_t *R = nullptr;
  bool success = false;

  if (auto root = nrgen_one(builder, s, G.get(), base)) {
    builder.appendToRoot(root.value());
    builder.finish();

    if (builder.verify(diagnostics ? std::make_optional(G.get())
                                   : std::nullopt)) {
      R = builder.get_module();
      success = true;
    } else {
      G->report(nr::CompilerError, IC::Error, "Failed to lower source");
    }
  } else {
    G->report(nr::CompilerError, IC::Error, "Failed to lower source");
  }

  if (!R) {
    R = createModule(name);
  }

  R->getDiag() = std::move(G);

  std::swap(nr::nr_arena.get(), *scratch_arena.get());
  *mod = R;

  return success;
}

///=============================================================================

static std::optional<nr::Expr *> nrgen_lower_binexpr(NRBuilder &b, PState &,
                                                     IReport *, nr::Expr *lhs,
                                                     nr::Expr *rhs,
                                                     qlex_op_t op) {
#define STD_BINOP(op) nr::create<nr::BinExpr>(lhs, rhs, nr::Op::op)
#define ASSIGN_BINOP(op)                                                   \
  nr::create<nr::BinExpr>(                                                 \
      lhs,                                                                 \
      nr::create<nr::BinExpr>(static_cast<nr::Expr *>(nr_clone(lhs)), rhs, \
                              nr::Op::op),                                 \
      nr::Op::Set)

  std::optional<nr::Expr *> R;

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
      auto a = nr::create<nr::BinExpr>(lhs, rhs, nr::Op::LogicOr);
      auto b = nr::create<nr::BinExpr>(lhs, rhs, nr::Op::LogicAnd);
      auto not_b = nr::create<nr::UnExpr>(b, nr::Op::LogicNot);
      R = nr::create<nr::BinExpr>(a, not_b, nr::Op::LogicAnd);
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
      /* TODO: Implement '>>>' operator */
      qcore_implement();
      break;
    }
    case qOpROTL: {
      /* TODO: Implement '<<<' operator */
      qcore_implement();
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

      auto a = nr::create<nr::BinExpr>(lhs, rhs, nr::Op::LogicOr);
      auto b = nr::create<nr::BinExpr>(lhs, rhs, nr::Op::LogicAnd);
      auto not_b = nr::create<nr::UnExpr>(b, nr::Op::LogicNot);
      return nr::create<nr::BinExpr>(
          lhs, nr::create<nr::BinExpr>(a, not_b, nr::Op::LogicAnd),
          nr::Op::Set);
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
      /* TODO: Implement '>>>=' operator */
      qcore_implement();
      break;
    }
    case qOpROTLSet: {
      /* TODO: Implement '<<<=' operator */
      qcore_implement();
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
      auto methname = b.createStringDataArray("has");
      auto method = nr::create<nr::Index>(rhs, methname);
      R = nr::create<nr::Call>(method, nr::CallArgs({lhs}));
      break;
    }
    case qOpRange: {
      /// TODO: Implement range operator
      break;
    }
    case qOpBitcastAs: {
      R = STD_BINOP(BitcastAs);
      break;
    }
    default: {
      break;
    }
  }

  return R;
}

static std::optional<nr::Expr *> nrgen_lower_unexpr(NRBuilder &b, PState &,
                                                    IReport *G, nr::Expr *rhs,
                                                    qlex_op_t op) {
#define STD_UNOP(op) nr::create<nr::UnExpr>(rhs, nr::Op::op)

  EResult R;

  switch (op) {
    case qOpPlus: {
      R = STD_UNOP(Plus);
      break;
    }

    case qOpMinus: {
      R = STD_UNOP(Minus);
      break;
    }

    case qOpTimes: {
      R = STD_UNOP(Times);
      break;
    }

    case qOpBitAnd: {
      R = STD_UNOP(BitAnd);
      break;
    }

    case qOpBitXor: {
      R = STD_UNOP(BitXor);
      break;
    }

    case qOpBitNot: {
      R = STD_UNOP(BitNot);
      break;
    }

    case qOpLogicNot: {
      R = STD_UNOP(LogicNot);
      break;
    }

    case qOpInc: {
      R = STD_UNOP(Inc);
      break;
    }

    case qOpDec: {
      R = STD_UNOP(Dec);
      break;
    }

    case qOpSizeof: {
      auto bits = nr::create<nr::UnExpr>(rhs, nr::Op::Bitsizeof);
      auto arg = nr::create<nr::BinExpr>(
          bits, nr::create<nr::Float>(8, nr::FloatSize::F64), nr::Op::Slash);

      std::array<std::pair<std::string_view, Expr *>, 1> args;
      args[0] = {"_0", arg};
      R = b.createCall(nr::create<nr::Ident>(b.intern("std::ceil"), nullptr),
                       args);

      break;
    }

    case qOpAlignof: {
      R = STD_UNOP(Alignof);
      break;
    }

    case qOpTypeof: {
      auto inferred = rhs->getType();
      if (!inferred.has_value()) {
        break;
      }

      nr::SymbolEncoding se;
      auto res = se.mangle_name(inferred.value(), nr::AbiTag::Nitrate);
      if (!res.has_value()) {
        G->report(CompilerError, IC::Error, "Failed to mangle type name",
                  rhs->getLoc());
        break;
      }

      R = b.createStringDataArray(res.value());
      break;
    }

    case qOpBitsizeof: {
      R = STD_UNOP(Bitsizeof);
      break;
    }

    default: {
      break;
    }
  }

  return R;
}

static std::optional<nr::Expr *> nrgen_lower_post_unexpr(NRBuilder &, PState &,
                                                         IReport *G,
                                                         nr::Expr *lhs,
                                                         qlex_op_t op) {
#define STD_POST_OP(op) nr::create<nr::PostUnExpr>(lhs, nr::Op::op)

  EResult R;

  switch (op) {
    case qOpInc: {
      R = STD_POST_OP(Inc);
      break;
    }
    case qOpDec: {
      R = STD_POST_OP(Dec);
      break;
    }
    default: {
      G->report(nr::CompilerError, IC::Error,
                "Operator is not supported in post-unary expression",
                lhs->getLoc());
      break;
    }
  }

  return R;
}

static EResult nrgen_binexpr(NRBuilder &b, PState &s, IReport *G,
                             npar::BinExpr *n) {
  if (n->get_lhs() && n->get_rhs() && n->get_op() == qOpAs &&
      n->get_rhs()->is(QAST_NODE_TYPE_EXPR)) {
    npar::Type *type = n->get_rhs()->as<npar::TypeExpr>()->get_type();

    bool is_integer_ty = type->is_integral();
    bool is_integer_lit = n->get_lhs()->getKind() == QAST_NODE_INT;

    bool is_float_ty = type->is_floating_point();
    bool is_float_lit = n->get_lhs()->getKind() == QAST_NODE_FLOAT;

    if ((is_integer_lit && is_integer_ty) || (is_float_lit && is_float_ty)) {
      if (is_integer_lit) {
        static const std::unordered_map<npar_ty_t, uint8_t>
            integer_lit_suffixes = {
                {QAST_NODE_U1_TY, 1},   {QAST_NODE_U8_TY, 8},
                {QAST_NODE_U16_TY, 16}, {QAST_NODE_U32_TY, 32},
                {QAST_NODE_U64_TY, 64}, {QAST_NODE_U128_TY, 128},

                /* Signeness is not expressed in the NR_NODE_INT */
                // {QAST_NODE_I8_TY, 8},     {QAST_NODE_I16_TY, 16},
                // {QAST_NODE_I32_TY, 32},   {QAST_NODE_I64_TY, 64},
                // {QAST_NODE_I128_TY, 128},
            };

        auto it = integer_lit_suffixes.find(type->getKind());
        if (it != integer_lit_suffixes.end()) {
          npar::ConstInt *N = n->get_lhs()->as<npar::ConstInt>();

          return b.createFixedInteger(
              boost::multiprecision::cpp_int(N->get_value()->c_str()),
              it->second);
        }
      } else {
        static const std::unordered_map<npar_ty_t, FloatSize>
            float_lit_suffixes = {{
                {QAST_NODE_F16_TY, FloatSize::F16},
                {QAST_NODE_F32_TY, FloatSize::F32},
                {QAST_NODE_F64_TY, FloatSize::F64},
                {QAST_NODE_F128_TY, FloatSize::F128},
            }};

        auto it = float_lit_suffixes.find(type->getKind());
        if (it != float_lit_suffixes.end()) {
          npar::ConstFloat *N = n->get_lhs()->as<npar::ConstFloat>();

          return b.createFixedFloat(
              boost::multiprecision::cpp_dec_float_100(N->get_value()->c_str()),
              it->second);
        }
      }
    }
  }

  auto lhs = next_one(n->get_lhs());
  if (!lhs.has_value()) {
    G->report(CompilerError, IC::Error,
              "Failed to lower LHS of binary expression", n->get_pos());
    return std::nullopt;
  }

  auto rhs = next_one(n->get_rhs());
  if (!rhs.has_value()) {
    G->report(CompilerError, IC::Error,
              "Failed to lower RHS of binary expression", n->get_pos());
    return std::nullopt;
  }

  auto E = nrgen_lower_binexpr(b, s, G, lhs.value(), rhs.value(), n->get_op());
  if (!E.has_value()) {
    G->report(CompilerError, IC::Error, "Failed to lower the binary expression",
              n->get_pos());
    return std::nullopt;
  }

  return E;
}

static EResult nrgen_unexpr(NRBuilder &b, PState &s, IReport *G,
                            npar::UnaryExpr *n) {
  auto rhs = next_one(n->get_rhs());
  if (!rhs.has_value()) {
    G->report(CompilerError, IC::Error,
              "Failed to lower RHS of unary expression", n->get_pos());
    return std::nullopt;
  }

  auto E = nrgen_lower_unexpr(b, s, G, rhs.value(), n->get_op());
  if (!E.has_value()) {
    G->report(CompilerError, IC::Error, "Failed to lower unary expression",
              n->get_pos());
    return std::nullopt;
  }

  return E;
}

static EResult nrgen_post_unexpr(NRBuilder &b, PState &s, IReport *G,
                                 npar::PostUnaryExpr *n) {
  auto lhs = next_one(n->get_lhs());
  if (!lhs.has_value()) {
    G->report(CompilerError, IC::Error,
              "Failed to lower LHS of post-unary expression", n->get_pos());

    return std::nullopt;
  }

  auto E = nrgen_lower_post_unexpr(b, s, G, lhs.value(), n->get_op());
  if (!E.has_value()) {
    G->report(CompilerError, IC::Error, "Failed to lower post-unary expression",
              n->get_pos());
    return std::nullopt;
  }

  return E;
}

static EResult nrgen_terexpr(NRBuilder &b, PState &s, IReport *G,
                             npar::TernaryExpr *n) {
  auto cond = next_one(n->get_cond());
  if (!cond.has_value()) {
    G->report(CompilerError, IC::Error,
              "Failed to lower condition of ternery expression", n->get_pos());
    return std::nullopt;
  }

  auto lhs = next_one(n->get_lhs());
  if (!lhs.has_value()) {
    G->report(CompilerError, IC::Error,
              "Failed to lower LHS of ternery expression", n->get_pos());
    return std::nullopt;
  }

  auto rhs = next_one(n->get_rhs());
  if (!rhs.has_value()) {
    G->report(CompilerError, IC::Error,
              "Failed to lower RHS of ternery expression", n->get_pos());
    return std::nullopt;
  }

  return create<If>(cond.value(), lhs.value(), rhs.value());
}

static EResult nrgen_int(NRBuilder &b, PState &, IReport *G,
                         npar::ConstInt *n) {
  /**
   * Integer types:
   *  i32:  [0 - 2147483647]
   *  i64:  [2147483648 - 9223372036854775807]
   *  u128: [9223372036854775808 - 340282366920938463463374607431768211455]
   *  error: [340282366920938463463374607431768211456 - ...]
   */
  boost::multiprecision::cpp_int num(std::string_view(*n->get_value()));

  if (num < 0) {
    G->report(CompilerError, IC::Error,
              "Integer literal should never be negative");
    return std::nullopt;
  }

  if (num <= 2147483647) {
    return b.createFixedInteger(num, 32);
  } else if (num <= 9223372036854775807) {
    return b.createFixedInteger(num, 64);
  } else if (num <= boost::multiprecision::cpp_int(
                        "340282366920938463463374607431768211455")) {
    return b.createFixedInteger(num, 128);
  } else {
    G->report(CompilerError, IC::Error,
              "Integer literal is not representable in u128 type");
    return std::nullopt;
  }
}

static EResult nrgen_float(NRBuilder &b, PState &, IReport *,
                           npar::ConstFloat *n) {
  boost::multiprecision::cpp_dec_float_100 num(*n->get_value());
  return b.createFixedFloat(num, FloatSize::F64);
}

static EResult nrgen_string(NRBuilder &b, PState &, IReport *,
                            npar::ConstString *n) {
  return b.createStringDataArray(*n->get_value());
}

static EResult nrgen_char(NRBuilder &b, PState &, IReport *,
                          npar::ConstChar *n) {
  return b.createFixedInteger(n->get_value(), 8);
}

static EResult nrgen_bool(NRBuilder &b, PState &, IReport *,
                          npar::ConstBool *n) {
  return b.createBool(n->get_value());
}

static EResult nrgen_null(NRBuilder &b, PState &, IReport *,
                          npar::ConstNull *) {
  return b.getUnknownNamedTy("__builtin_null");
}

static EResult nrgen_undef(NRBuilder &, PState &, IReport *G,
                           npar::ConstUndef *n) {
  G->report(UnexpectedUndefLiteral, IC::Error, "", n->get_pos());
  return std::nullopt;
}

static EResult nrgen_call(NRBuilder &b, PState &s, IReport *G, npar::Call *n) {
  auto target = next_one(n->get_func());
  if (!target.has_value()) {
    G->report(nr::CompilerError, IC::Error, "Failed to lower function target",
              n->get_pos());
    return std::nullopt;
  }

  const auto &args = n->get_args();

  std::vector<std::pair<std::string_view, Expr *>> arguments;
  arguments.resize(args.size());

  for (size_t i = 0; i < args.size(); i++) {
    auto arg = next_one(args[i].second);
    if (!arg.has_value()) {
      G->report(nr::CompilerError, IC::Error,
                "Failed to lower function argument", n->get_pos());
      return std::nullopt;
    }

    arguments[i] = {b.intern(*args[i].first), arg.value()};
  }

  return b.createCall(target.value(), arguments);
}

static EResult nrgen_list(NRBuilder &b, PState &s, IReport *G, npar::List *n) {
  ListItems items;

  for (auto it = n->get_items().begin(); it != n->get_items().end(); ++it) {
    auto item = next_one(*it);
    if (!item.has_value()) {
      G->report(nr::CompilerError, IC::Error, "Failed to lower list element",
                n->get_pos());
      return std::nullopt;
    }

    items.push_back(item.value());
  }

  return b.createList(items, false);
}

static EResult nrgen_assoc(NRBuilder &b, PState &s, IReport *G,
                           npar::Assoc *n) {
  auto key = next_one(n->get_key());
  if (!key.has_value()) {
    G->report(nr::CompilerError, IC::Error, "Failed to lower associative key",
              n->get_pos());
    return std::nullopt;
  }

  auto value = next_one(n->get_value());
  if (!value.has_value()) {
    G->report(nr::CompilerError, IC::Error, "Failed to lower associative value",
              n->get_pos());
    return std::nullopt;
  }

  std::array<Expr *, 2> kv = {key.value(), value.value()};
  return b.createList(kv, false);
}

static EResult nrgen_field(NRBuilder &b, PState &s, IReport *G,
                           npar::Field *n) {
  auto base = next_one(n->get_base());
  if (!base.has_value()) {
    G->report(nr::CompilerError, IC::Error,
              "Failed to lower field-access base expression", n->get_pos());
    return std::nullopt;
  }

  /// TODO: Support for named composite field indexing

  Expr *field = b.createStringDataArray(*n->get_field());
  return create<Index>(base.value(), field);
}

static EResult nrgen_index(NRBuilder &b, PState &s, IReport *G,
                           npar::Index *n) {
  auto base = next_one(n->get_base());
  if (!base.has_value()) {
    G->report(nr::CompilerError, IC::Error,
              "Failed to lower index expression base", n->get_pos());
    return std::nullopt;
  }

  auto index = next_one(n->get_index());
  if (!index.has_value()) {
    G->report(nr::CompilerError, IC::Error,
              "Failed to lower index expression index", n->get_pos());
    return std::nullopt;
  }

  return create<Index>(base.value(), index.value());
}

static EResult nrgen_slice(NRBuilder &b, PState &s, IReport *G,
                           npar::Slice *n) {
  auto base = next_one(n->get_base());
  if (!base.has_value()) {
    G->report(nr::CompilerError, IC::Error,
              "Failed to lower slice expression base", n->get_pos());
    return std::nullopt;
  }

  auto start = next_one(n->get_start());
  if (!start.has_value()) {
    G->report(nr::CompilerError, IC::Error,
              "Failed to lower slice expression start", n->get_pos());
    return std::nullopt;
  }

  auto end = next_one(n->get_end());
  if (!end.has_value()) {
    G->report(nr::CompilerError, IC::Error,
              "Failed to lower slice expression end", n->get_pos());
    return std::nullopt;
  }

  std::array<std::pair<std::string_view, Expr *>, 2> args;
  args[0] = {"0", start.value()};
  args[1] = {"1", end.value()};

  return b.createMethodCall(base.value(), "slice", args);
}

static EResult nrgen_fstring(NRBuilder &b, PState &s, IReport *G,
                             npar::FString *n) {
  /// TODO: Cleanup the fstring implementation

  if (n->get_items().empty()) {
    return b.createStringDataArray("");
  }

  if (n->get_items().size() == 1) {
    auto val = n->get_items().front();

    if (std::holds_alternative<npar::SmallString>(val)) {
      return b.createStringDataArray(*std::get<npar::SmallString>(val));
    } else if (std::holds_alternative<npar::Expr *>(val)) {
      auto expr = next_one(std::get<npar::Expr *>(val));

      if (!expr.has_value()) {
        G->report(CompilerError, IC::Error,
                  "npar::FString::get_items() vector contains std::nullopt",
                  n->get_pos());
        return std::nullopt;
      }

      return expr;
    } else {
      qcore_panic("Invalid fstring item type");
    }
  }

  Expr *concated = b.createStringDataArray("");

  for (auto it = n->get_items().begin(); it != n->get_items().end(); ++it) {
    if (std::holds_alternative<npar::SmallString>(*it)) {
      auto val = *std::get<npar::SmallString>(*it);

      concated =
          create<BinExpr>(concated, b.createStringDataArray(val), Op::Plus);
    } else if (std::holds_alternative<npar::Expr *>(*it)) {
      auto val = std::get<npar::Expr *>(*it);
      auto expr = next_one(val);

      if (!expr.has_value()) {
        G->report(CompilerError, IC::Error,
                  "npar::FString::get_items() vector contains std::nullopt",
                  n->get_pos());
        return std::nullopt;
      }

      concated = create<BinExpr>(concated, expr.value(), Op::Plus);
    } else {
      qcore_panic("Invalid fstring item type");
    }
  }

  return concated;
}

static EResult nrgen_ident(NRBuilder &b, PState &s, IReport *, npar::Ident *n) {
  return create<Ident>(b.intern(s.scope_name(*n->get_name())), nullptr);
}

static EResult nrgen_seq_point(NRBuilder &b, PState &s, IReport *G,
                               npar::SeqPoint *n) {
  SeqItems items(n->get_items().size());

  for (size_t i = 0; i < n->get_items().size(); i++) {
    auto item = next_one(n->get_items()[i]);
    if (!item.has_value()) [[unlikely]] {
      G->report(CompilerError, IC::Error,
                "npar::SeqPoint::get_items() vector contains std::nullopt",
                n->get_pos());
      return std::nullopt;
    }

    items[i] = item.value();
  }

  return create<Seq>(std::move(items));
}

static EResult nrgen_stmt_expr(NRBuilder &b, PState &s, IReport *G,
                               npar::StmtExpr *n) {
  auto stmt = next_one(n->get_stmt());
  if (!stmt.has_value()) {
    G->report(CompilerError, IC::Error, "Failed to lower statement expression",
              n->get_pos());
    return std::nullopt;
  }

  return stmt;
}

static EResult nrgen_type_expr(NRBuilder &b, PState &s, IReport *G,
                               npar::TypeExpr *n) {
  auto type = next_one(n->get_type());
  if (!type.has_value()) {
    G->report(CompilerError, IC::Error, "Failed to lower type expression",
              n->get_pos());
    return std::nullopt;
  }

  return type;
}

static EResult nrgen_templ_call(NRBuilder &, PState &, IReport *G,
                                npar::TemplCall *n) {
  G->report(CompilerError, IC::FatalError,
            "Attempted to lower an unexpected "
            "template function call",
            n->get_pos());

  return std::nullopt;
}

static EResult nrgen_ref_ty(NRBuilder &b, PState &s, IReport *G,
                            npar::RefTy *n) {
  auto pointee = next_one(n->get_item());
  if (!pointee.has_value()) {
    G->report(CompilerError, IC::Error, "Failed to lower reference type",
              n->get_pos());
    return std::nullopt;
  }

  return b.getPtrTy(pointee.value()->asType());
}

static EResult nrgen_u1_ty(NRBuilder &b, PState &, IReport *, npar::U1 *) {
  return b.getU1Ty();
}
static EResult nrgen_u8_ty(NRBuilder &b, PState &, IReport *, npar::U8 *) {
  return b.getU8Ty();
}
static EResult nrgen_u16_ty(NRBuilder &b, PState &, IReport *, npar::U16 *) {
  return b.getU16Ty();
}
static EResult nrgen_u32_ty(NRBuilder &b, PState &, IReport *, npar::U32 *) {
  return b.getU32Ty();
}
static EResult nrgen_u64_ty(NRBuilder &b, PState &, IReport *, npar::U64 *) {
  return b.getU64Ty();
}
static EResult nrgen_u128_ty(NRBuilder &b, PState &, IReport *, npar::U128 *) {
  return b.getU128Ty();
}
static EResult nrgen_i8_ty(NRBuilder &b, PState &, IReport *, npar::I8 *) {
  return b.getI8Ty();
}
static EResult nrgen_i16_ty(NRBuilder &b, PState &, IReport *, npar::I16 *) {
  return b.getI16Ty();
}
static EResult nrgen_i32_ty(NRBuilder &b, PState &, IReport *, npar::I32 *) {
  return b.getI32Ty();
}
static EResult nrgen_i64_ty(NRBuilder &b, PState &, IReport *, npar::I64 *) {
  return b.getI64Ty();
}
static EResult nrgen_i128_ty(NRBuilder &b, PState &, IReport *, npar::I128 *) {
  return b.getI128Ty();
}
static EResult nrgen_f16_ty(NRBuilder &b, PState &, IReport *, npar::F16 *) {
  return b.getF16Ty();
}
static EResult nrgen_f32_ty(NRBuilder &b, PState &, IReport *, npar::F32 *) {
  return b.getF32Ty();
}
static EResult nrgen_f64_ty(NRBuilder &b, PState &, IReport *, npar::F64 *) {
  return b.getF64Ty();
}
static EResult nrgen_f128_ty(NRBuilder &b, PState &, IReport *, npar::F128 *) {
  return b.getF128Ty();
}
static EResult nrgen_void_ty(NRBuilder &b, PState &, IReport *,
                             npar::VoidTy *) {
  return b.getVoidTy();
}

static EResult nrgen_ptr_ty(NRBuilder &b, PState &s, IReport *G,
                            npar::PtrTy *n) {
  auto pointee = next_one(n->get_item());
  if (!pointee.has_value()) {
    G->report(CompilerError, IC::Error, "Failed to lower pointer type",
              n->get_pos());
    return std::nullopt;
  }

  return b.getPtrTy(pointee.value()->asType());
}

static EResult nrgen_opaque_ty(NRBuilder &b, PState &, IReport *,
                               npar::OpaqueTy *n) {
  return b.getOpaqueTy(*n->get_name());
}

static EResult nrgen_array_ty(NRBuilder &b, PState &s, IReport *G,
                              npar::ArrayTy *n) {
  auto item = next_one(n->get_item());
  if (!item.has_value()) {
    G->report(CompilerError, IC::Error, "Failed to lower array item type",
              n->get_pos());
    return std::nullopt;
  }

  auto count_expr = next_one(n->get_size());
  if (!count_expr.has_value()) {
    G->report(CompilerError, IC::Error, "Failed to lower array size",
              n->get_pos());
    return std::nullopt;
  }

  auto eprintn_cb = [&](std::string_view msg) {
    G->report(CompilerError, IC::Error, msg, count_expr.value()->getLoc());
  };

  auto result = nr::comptime_impl(count_expr.value(), eprintn_cb);
  if (!result.has_value()) {
    G->report(CompilerError, IC::Error, "Failed to evaluate array size",
              count_expr.value()->getLoc());
    return std::nullopt;
  }

  if (result.value()->getKind() != NR_NODE_INT) {
    G->report(CompilerError, IC::Error,
              "Non integer literal array size is not supported", n->get_pos());
    return std::nullopt;
  }

  uint128_t size = result.value()->as<Int>()->getValue();

  if (size > UINT64_MAX) {
    G->report(CompilerError, IC::Error, "Array size > UINT64_MAX",
              n->get_pos());
    return std::nullopt;
  }

  return b.getArrayTy(item.value()->asType(), static_cast<uint64_t>(size));
}

static EResult nrgen_tuple_ty(NRBuilder &b, PState &s, IReport *G,
                              npar::TupleTy *n) {
  const auto &items = n->get_items();
  StructFields fields(items.size());

  for (size_t i = 0; i < items.size(); i++) {
    auto item = next_one(items[i]);
    if (!item.has_value()) {
      G->report(CompilerError, IC::Error, "Failed to lower tuple field type",
                n->get_pos());
      return std::nullopt;
    }

    fields[i] = item.value()->asType();
  }

  return b.getStructTy(fields);
}

using IsThreadSafe = bool;

static std::pair<Purity, IsThreadSafe> convert_purity(npar::FuncPurity x) {
  switch (x) {
    case npar::FuncPurity::IMPURE_THREAD_UNSAFE:
      return {Purity::Impure, false};
    case npar::FuncPurity::IMPURE_THREAD_SAFE:
      return {Purity::Impure, true};
    case npar::FuncPurity::PURE:
      return {Purity::Pure, true};
    case npar::FuncPurity::QUASIPURE:
      return {Purity::Quasipure, true};
    case npar::FuncPurity::RETROPURE:
      return {Purity::Retropure, true};
  }
}

static EResult nrgen_fn_ty(NRBuilder &b, PState &s, IReport *G,
                           npar::FuncTy *n) {
  const auto &items = n->get_params();
  FnParams params(items.size());

  for (size_t i = 0; i < items.size(); i++) {
    auto type = next_one(std::get<1>(items[i]));
    if (!type.has_value()) {
      G->report(CompilerError, IC::Error, "Failed to lower function parameter",
                n->get_pos());
      return std::nullopt;
    }

    params[i] = type.value()->asType();
  }

  auto ret = next_one(n->get_return_ty());
  if (!ret.has_value()) {
    G->report(CompilerError, IC::Error, "Failed to lower function return type",
              n->get_pos());
    return std::nullopt;
  }

  auto props = convert_purity(n->get_purity());

  return b.getFnTy(params, ret.value()->asType(), n->is_variadic(), props.first,
                   props.second, n->is_foreign());
}

static EResult nrgen_unres_ty(NRBuilder &b, PState &s, IReport *,
                              npar::NamedTy *n) {
  return b.getUnknownNamedTy(b.intern(s.scope_name(*n->get_name())));
}

static EResult nrgen_infer_ty(NRBuilder &b, PState &, IReport *,
                              npar::InferTy *) {
  return b.getUnknownTy();
}

static EResult nrgen_templ_ty(NRBuilder &, PState &, IReport *G,
                              npar::TemplType *n) {
  G->report(nr::CompilerError, IC::FatalError,
            "Attempted to lower an unexpected npar::TemplType node",
            n->get_pos());
  return std::nullopt;
}

static BResult nrgen_typedef(NRBuilder &b, PState &s, IReport *G,
                             npar::TypedefStmt *n) {
  auto type = next_one(n->get_type());
  if (!type.has_value()) {
    G->report(nr::CompilerError, IC::Error,
              "Failed to lower type in typedef statement", n->get_pos());
    return std::nullopt;
  }

  b.createNamedTypeAlias(type.value()->asType(),
                         b.intern(s.join_scope(*n->get_name())));

  return std::vector<Expr *>();
}

#define align(x, a) (((x) + (a) - 1) & ~((a) - 1))

static BResult nrgen_struct(NRBuilder &b, PState &s, IReport *G,
                            npar::StructDef *n) {
  bool is_template = n->get_template_params().has_value();
  if (is_template) {
    G->report(nr::CompilerError, IC::FatalError,
              "Attempted to lower an unexpected template struct node",
              n->get_pos());
    return std::nullopt;
  }

  std::vector<std::tuple<std::string_view, Type *, Expr *>> fields(
      n->get_fields().size());

  std::string old_ns = s.ns_prefix;
  s.ns_prefix = s.join_scope(*n->get_name());

  for (size_t i = 0; i < n->get_fields().size(); i++) {
    auto field_raw = n->get_fields()[i];
    if (!field_raw->is(QAST_NODE_STRUCT_FIELD)) {
      return std::nullopt;
    }
    npar::StructField *field = field_raw->as<npar::StructField>();

    auto field_type = next_one(field->get_type());
    if (!field_type.has_value()) {
      G->report(nr::CompilerError, IC::Error,
                "Failed to lower struct field type", n->get_pos());
      s.ns_prefix = old_ns;
      return std::nullopt;
    }

    auto field_name = b.intern(*field->get_name());

    Expr *field_default = nullptr;
    if (field->get_value() == nullptr) {
      auto val = b.getDefaultValue(field_type.value()->asType());
      if (!val.has_value()) {
        G->report(nr::CompilerError, IC::Error,
                  "Failed to lower struct field default value", n->get_pos());
        s.ns_prefix = old_ns;
        return std::nullopt;
      }

      field_default = val.value();
    } else {
      auto val = next_one(field->get_value());
      if (!val.has_value()) {
        G->report(nr::CompilerError, IC::Error,
                  "Failed to lower struct field default value", n->get_pos());
        s.ns_prefix = old_ns;
        return std::nullopt;
      }

      field_default = val.value();
    }

    fields[i] = {field_name, field_type.value()->asType(), field_default};
  }

  std::swap(s.ns_prefix, old_ns);
  b.createNamedTypeAlias(b.getStructTy(fields),
                         b.intern(s.join_scope(*n->get_name())));
  std::swap(s.ns_prefix, old_ns);

  BResult R;
  R = std::vector<Expr *>();

  for (const auto &method : n->get_methods()) {
    auto val = next_one(method);
    if (!val.has_value()) {
      G->report(nr::CompilerError, IC::Error, "Failed to lower struct method",
                n->get_pos());
      s.ns_prefix = old_ns;
      return std::nullopt;
    }

    R->push_back(val.value());
  }

  for (const auto &method : n->get_static_methods()) {
    auto val = next_one(method);
    if (!val.has_value()) {
      G->report(nr::CompilerError, IC::Error,
                "Failed to lower struct static method", n->get_pos());
      s.ns_prefix = old_ns;
      return std::nullopt;
    }

    R->push_back(val.value());
  }

  s.ns_prefix = old_ns;

  return R;
}

static BResult nrgen_enum(NRBuilder &b, PState &s, IReport *G,
                          npar::EnumDef *n) {
  std::unordered_map<std::string_view, Expr *> values;

  std::optional<Expr *> last;

  for (auto it = n->get_items().begin(); it != n->get_items().end(); ++it) {
    Expr *field_value = nullptr;

    if (it->second != nullptr) {
      auto val = next_one(it->second);
      if (!val.has_value()) {
        G->report(nr::CompilerError, IC::Error, "Failed to lower enum field",
                  n->get_pos());
        return std::nullopt;
      }
      last = field_value = val.value();
    } else {
      if (last.has_value()) {
        last = field_value = create<BinExpr>(
            last.value(), b.createFixedInteger(1, 32), Op::Plus);
      } else {
        last = field_value = b.createFixedInteger(0, 32);
      }
    }

    auto field_name = b.intern(*it->first);

    if (values.contains(field_name)) [[unlikely]] {
      G->report(CompilerError, IC::Error,
                {"Enum field named '", field_name, "' is redefined"});
    } else {
      values[field_name] = field_value;
    }
  }

  auto name = b.intern(s.join_scope(*n->get_name()));
  b.createNamedConstantDefinition(name, values);

  /* FIXME: Allow for first class enum types */
  b.createNamedTypeAlias(b.getI32Ty(), name);

  return std::vector<Expr *>();
}

static EResult nrgen_fndecl(NRBuilder &b, PState &s, IReport *G,
                            npar::FnDecl *n) {
  npar::FuncTy *func_ty = n->get_type();
  const npar::FuncParams &params = func_ty->get_params();

  std::vector<NRBuilder::FnParam> parameters(params.size());

  for (size_t i = 0; i < params.size(); i++) {
    const auto &param = params[i];
    NRBuilder::FnParam p;

    { /* Set function parameter name */
      std::get<0>(p) = b.intern(*std::get<0>(param));
    }

    { /* Set function parameter type */
      auto tmp = next_one(std::get<1>(param));
      if (!tmp.has_value()) {
        G->report(CompilerError, nr::IC::Error,
                  "Failed to convert function declaration parameter type");
        return std::nullopt;
      }

      std::get<1>(p) = tmp.value()->asType();
    }

    { /* Set function parameter default value if it exists */
      if (std::get<2>(param) != nullptr) {
        auto val = next_one(std::get<2>(param));
        if (!val.has_value()) {
          G->report(
              CompilerError, nr::IC::Error,
              "Failed to convert function declaration parameter default value");
          return std::nullopt;
        }

        std::get<2>(p) = val.value();
      }
    }

    parameters[i] = std::move(p);
  }

  auto ret_type = next_one(func_ty->get_return_ty());
  if (!ret_type.has_value()) {
    G->report(CompilerError, nr::IC::Error,
              "Failed to convert function declaration return type");
    return std::nullopt;
  }

  auto props = convert_purity(func_ty->get_purity());

  Fn *fndecl = b.createFunctionDeclaration(
      b.intern(s.join_scope(*n->get_name())), parameters,
      ret_type.value()->asType(), func_ty->is_variadic(), Vis::Pub, props.first,
      props.second, func_ty->is_foreign());

  fndecl->setAbiTag(s.abi_mode);

  return fndecl;
}

static EResult nrgen_block(NRBuilder &b, PState &s, IReport *G, npar::Block *n,
                           bool insert_scope_id);

static EResult nrgen_fn(NRBuilder &b, PState &s, IReport *G, npar::FnDef *n) {
  bool failed = false;

  {
    if (!n->get_captures().empty()) {
      G->report(nr::CompilerError, IC::Error,
                "Function capture groups are not currently supported");
      failed = true;
    }

    if (n->get_precond() != nullptr) {
      G->report(nr::CompilerError, IC::Error,
                "Function pre-conditions are not currently supported");
      failed = true;
    }

    if (n->get_postcond() != nullptr) {
      G->report(nr::CompilerError, IC::Error,
                "Function post-conditions are not currently supported");
      failed = true;
    }

    if (failed) {
      return std::nullopt;
    }
  }

  {
    npar::FuncTy *func_ty = n->get_type();
    const npar::FuncParams &params = func_ty->get_params();

    std::vector<NRBuilder::FnParam> parameters;
    parameters.resize(params.size());

    for (size_t i = 0; i < params.size(); i++) {
      const auto &param = params[i];
      NRBuilder::FnParam p;

      { /* Set function parameter name */
        std::get<0>(p) = b.intern(*std::get<0>(param));
      }

      { /* Set function parameter type */
        auto tmp = next_one(std::get<1>(param));
        if (!tmp.has_value()) {
          G->report(CompilerError, nr::IC::Error,
                    "Failed to convert function declaration parameter type",
                    n->get_pos());
          return std::nullopt;
        }

        std::get<1>(p) = tmp.value()->asType();
      }

      { /* Set function parameter default value if it exists */
        if (std::get<2>(param) != nullptr) {
          auto val = next_one(std::get<2>(param));
          if (!val.has_value()) {
            G->report(CompilerError, nr::IC::Error,
                      "Failed to convert function declaration parameter "
                      "default value",
                      n->get_pos());
            return std::nullopt;
          }

          std::get<2>(p) = val.value();
        }
      }

      parameters[i] = std::move(p);
    }

    auto ret_type = next_one(func_ty->get_return_ty());
    if (!ret_type.has_value()) {
      G->report(CompilerError, nr::IC::Error,
                "Failed to convert function declaration return type",
                n->get_pos());
      return std::nullopt;
    }

    auto props = convert_purity(func_ty->get_purity());

    std::string_view name;

    if (n->get_name()->empty()) {
      name = b.intern(s.join_scope("_A$" + std::to_string(s.anon_fn_ctr++)));
    } else {
      name = b.intern(s.join_scope(*n->get_name()));
    }

    Fn *fndef = b.createFunctionDefintion(
        name, parameters, ret_type.value()->asType(), func_ty->is_variadic(),
        Vis::Pub, props.first, props.second, func_ty->is_foreign());

    fndef->setAbiTag(s.abi_mode);

    { /* Function body */

      if (!n->get_body()->is(QAST_NODE_BLOCK)) {
        return std::nullopt;
      }

      std::string old_ns = s.ns_prefix;
      s.ns_prefix = name;

      auto body = nrgen_block(b, s, G, n->get_body()->as<npar::Block>(), false);
      if (!body.has_value()) {
        G->report(CompilerError, nr::IC::Error,
                  "Failed to convert function body", n->get_pos());
        return std::nullopt;
      }

      s.ns_prefix = old_ns;

      fndef->setBody(body.value()->as<Seq>());
    }

    return fndef;
  }
}

static BResult nrgen_scope(NRBuilder &b, PState &s, IReport *G,
                           npar::ScopeStmt *n) {
  if (!n->get_body()->is(QAST_NODE_BLOCK)) {
    return std::nullopt;
  }

  std::string old_ns = s.ns_prefix;
  s.ns_prefix = s.join_scope(*n->get_name());

  auto body = nrgen_block(b, s, G, n->get_body()->as<npar::Block>(), false);
  if (!body.has_value()) {
    G->report(nr::CompilerError, IC::Error, "Failed to lower scope body",
              n->get_pos());
    return std::nullopt;
  }

  s.ns_prefix = old_ns;

  return BResult({body.value()});
}

static BResult nrgen_export(NRBuilder &b, PState &s, IReport *G,
                            npar::ExportStmt *n) {
  static const std::unordered_map<std::string_view,
                                  std::pair<std::string_view, AbiTag>>
      abi_name_map = {
          /* Default ABI */
          {"", {"n", AbiTag::Default}},
          {"std", {"n", AbiTag::Default}},

          /* Nitrate standard ABI */
          {"n", {"n", AbiTag::Nitrate}},

          /* C ABI variant is dictated by the LLVM target */
          {"c", {"c", AbiTag::C}},
      };

  if (!n->get_body()) {
    G->report(CompilerError, IC::Error,
              "Failed to lower extern node; body is null", n->get_pos());
    return std::nullopt;
  }

  auto it = abi_name_map.find(*n->get_abi_name());
  if (it == abi_name_map.end()) {
    G->report(
        CompilerError, IC::Error,
        {"The requested ABI name '", *n->get_abi_name(), "' is not supported"},
        n->get_pos());
    return std::nullopt;
  }

  if (!n->get_body()->is(QAST_NODE_BLOCK)) {
    return std::nullopt;
  }

  AbiTag old = s.abi_mode;
  s.abi_mode = it->second.second;

  const auto &body = n->get_body()->as<npar::Block>()->get_items();
  std::vector<nr::Expr *> items;

  for (size_t i = 0; i < body.size(); i++) {
    auto result = next_any(body[i]);
    if (!result.has_value()) {
      G->report(CompilerError, IC::Error,
                "Failed to lower element in external declaration",
                n->get_pos());
      s.abi_mode = old;
      return std::nullopt;
    }

    for (auto &item : result.value()) {
      items.push_back(create<Extern>(item, it->second.first));
    }
  }

  s.abi_mode = old;

  return items;
}

static EResult nrgen_composite_field(NRBuilder &, PState &, IReport *,
                                     npar::StructField *) {
  qcore_panic("Unreachable");
}

static EResult nrgen_block(NRBuilder &b, PState &s, IReport *G, npar::Block *n,
                           bool insert_scope_id) {
  SeqItems items;
  items.reserve(n->get_items().size());

  std::string old_ns = s.ns_prefix;

  if (insert_scope_id) {
    s.inc_scope();
  }

  for (auto it = n->get_items().begin(); it != n->get_items().end(); ++it) {
    auto item = next_any(*it);
    if (!item.has_value()) {
      G->report(nr::CompilerError, IC::Error,
                "Failed to lower element in statement block", n->get_pos());
      return std::nullopt;
    }

    if ((*it)->getKind() == QAST_NODE_BLOCK) {
      /* Reduce unneeded nesting in the IR */
      qcore_assert(item->size() == 1);
      Seq *inner = item->at(0)->as<Seq>();

      items.insert(items.end(), inner->getItems().begin(),
                   inner->getItems().end());
    } else {
      items.insert(items.end(), item.value().begin(), item.value().end());
    }
  }

  if (insert_scope_id) {
    s.dec_scope();
  }

  return create<Seq>(std::move(items));
}

static EResult nrgen_var(NRBuilder &b, PState &s, IReport *G,
                         npar::VarDecl *n) {
  auto init = next_one(n->get_value());
  auto type = next_one(n->get_type());

  if (init.has_value() && type.has_value()) { /* Do implicit cast */
    init = create<BinExpr>(init.value(), type.value(), Op::CastAs);
  } else if (init.has_value() && !type.has_value()) {
    type = b.getUnknownTy();
  } else if (type.has_value() && !init.has_value()) {
    init = b.getDefaultValue(type.value()->asType());
    if (!init.has_value()) {
      G->report(TypeInference, IC::Error,
                "Failed to get default value for type in let declaration");
      return std::nullopt;
    }
  } else {
    G->report(TypeInference, IC::Error,
              "Expected a type specifier or initial value in let declaration");
    return std::nullopt;
  }

  qcore_assert(init.has_value() && type.has_value());

  StorageClass storage = s.inside_function ? StorageClass::LLVM_StackAlloa
                                           : StorageClass::LLVM_Static;
  Vis visibility = s.abi_mode == AbiTag::Internal ? Vis::Sec : Vis::Pub;

  Local *local =
      b.createVariable(b.intern(s.join_scope(*n->get_name())),
                       type.value()->asType(), visibility, storage, false);

  local->setValue(init.value());
  local->setAbiTag(s.abi_mode);

  return local;
}

static EResult nrgen_inline_asm(NRBuilder &, PState &, IReport *G,
                                npar::InlineAsm *) {
  /// TODO: Decide whether or not to support inline assembly
  G->report(nr::CompilerError, IC::Error,
            "Inline assembly is not currently supported");
  return std::nullopt;
}

static EResult nrgen_return(NRBuilder &b, PState &s, IReport *G,
                            npar::ReturnStmt *n) {
  if (n->get_value()) {
    auto val = next_one(n->get_value().value_or(nullptr));
    if (!val.has_value()) {
      G->report(nr::CompilerError, IC::Error,
                "Failed to lower return statement value", n->get_pos());
      return std::nullopt;
    }

    return create<Ret>(val.value());

  } else {
    return create<Ret>(create<VoidTy>());
  }
}

static EResult nrgen_retif(NRBuilder &b, PState &s, IReport *G,
                           npar::ReturnIfStmt *n) {
  auto cond = next_one(n->get_cond());
  if (!cond.has_value()) {
    return std::nullopt;
  }

  cond = create<BinExpr>(cond.value(), create<U1Ty>(), Op::CastAs);

  auto val = next_one(n->get_value());
  if (!val.has_value()) {
    return std::nullopt;
  }

  return create<If>(cond.value(), create<Ret>(val.value()), createIgn());
}

static EResult nrgen_break(NRBuilder &, PState &, IReport *,
                           npar::BreakStmt *) {
  return create<Brk>();
}

static EResult nrgen_continue(NRBuilder &, PState &, IReport *,
                              npar::ContinueStmt *) {
  return create<Cont>();
}

static EResult nrgen_if(NRBuilder &b, PState &s, IReport *G, npar::IfStmt *n) {
  auto cond = next_one(n->get_cond());
  auto then = next_one(n->get_then());
  auto els = next_one(n->get_else());

  if (!cond.has_value()) {
    return std::nullopt;
  }

  cond = create<BinExpr>(cond.value(), create<U1Ty>(), Op::CastAs);

  if (!then.has_value()) {
    return std::nullopt;
  }

  if (!els.has_value()) {
    els = createIgn();
  }

  return create<If>(cond.value(), then.value(), els.value());
}

static EResult nrgen_while(NRBuilder &b, PState &s, IReport *G,
                           npar::WhileStmt *n) {
  auto cond = next_one(n->get_cond());
  auto body = next_one(n->get_body());

  if (!cond.has_value()) {
    cond = create<Int>(1, 1);
  }

  cond = create<BinExpr>(cond.value(), create<U1Ty>(), Op::CastAs);

  if (!body.has_value()) {
    body = create<Seq>(SeqItems({}));
  } else if (body.value()->getKind() != NR_NODE_SEQ) {
    body = create<Seq>(SeqItems({body.value()}));
  }

  return create<While>(cond.value(), body.value()->as<Seq>());
}

static EResult nrgen_for(NRBuilder &b, PState &s, IReport *G,
                         npar::ForStmt *n) {
  s.inc_scope();

  auto init = next_one(n->get_init().value_or(nullptr));
  auto cond = next_one(n->get_cond().value_or(nullptr));
  auto step = next_one(n->get_step().value_or(nullptr));
  auto body = next_one(n->get_body());

  if (!init.has_value()) {
    init = create<Int>(1, 32);
  }

  if (!cond.has_value()) {
    cond = create<Int>(1, 32);  // infinite loop like 'for (;;) {}'
    cond = create<BinExpr>(cond.value(), create<U1Ty>(), Op::CastAs);
  }

  if (!step.has_value()) {
    step = create<Int>(1, 32);
  }

  if (!body.has_value()) {
    body = create<Int>(1, 32);
  }

  s.dec_scope();

  return create<For>(init.value(), cond.value(), step.value(), body.value());
}

static EResult nrgen_foreach(NRBuilder &, PState &, IReport *,
                             npar::ForeachStmt *) {
  /**
   * @brief Convert a foreach loop to a nr expression.
   * @details This is a 1-to-1 conversion of the foreach loop.
   */

  // auto idx_name = b.intern(n->get_idx_ident());
  // auto val_name = b.intern(n->get_val_ident());

  // auto iter = nrgen_one(b, s,X, n->get_expr());
  // if (!iter) {
  //   G->report(CompilerError, IC::Error, "npar::ForeachStmt::get_expr() ==
  //   std::nullopt",n->get_offset(),n->get_pos()); return std::nullopt;
  // }

  // auto body = nrgen_one(b, s,X, n->get_body());
  // if (!body) {
  //   G->report(CompilerError, IC::Error, "npar::ForeachStmt::get_body() ==
  //   std::nullopt",n->get_offset(),n->get_pos()); return std::nullopt;
  // }

  // return create<Foreach>(idx_name, val_name, iter,
  // create<Seq>(SeqItems({body})));
  qcore_implement();
}

static EResult nrgen_case(NRBuilder &b, PState &s, IReport *G,
                          npar::CaseStmt *n) {
  auto cond = next_one(n->get_cond());
  if (!cond.has_value()) {
    return std::nullopt;
  }

  auto body = next_one(n->get_body());
  if (!body.has_value()) {
    return std::nullopt;
  }

  return create<Case>(cond.value(), body.value());
}

static EResult nrgen_switch(NRBuilder &b, PState &s, IReport *G,
                            npar::SwitchStmt *n) {
  auto cond = next_one(n->get_cond());
  if (!cond.has_value()) {
    return std::nullopt;
  }

  SwitchCases cases;
  for (auto it = n->get_cases().begin(); it != n->get_cases().end(); ++it) {
    auto item = next_one(*it);
    if (!item.has_value()) {
      G->report(CompilerError, IC::Error,
                "npar::SwitchStmt::get_cases() vector contains std::nullopt",
                n->get_pos());
      return std::nullopt;
    }

    cases.push_back(item.value()->as<Case>());
  }

  EResult def;
  if (n->get_default()) {
    def = next_one(n->get_default());
    if (!def.has_value()) {
      return std::nullopt;
    }
  } else {
    def = createIgn();
  }

  return create<Switch>(cond.value(), std::move(cases), def.value());
}

static EResult nrgen_expr_stmt(NRBuilder &b, PState &s, IReport *G,
                               npar::ExprStmt *n) {
  return next_one(n->get_expr());
}

static EResult nrgen_one(NRBuilder &b, PState &s, IReport *G, npar_node_t *n) {
  using namespace nr;

  if (!n) {
    return std::nullopt;
  }

  std::optional<nr::Expr *> out;

  switch (n->getKind()) {
    case QAST_NODE_NODE: {
      break;
    }

    case QAST_NODE_BINEXPR:
      out = nrgen_binexpr(b, s, G, n->as<npar::BinExpr>());
      break;

    case QAST_NODE_UNEXPR:
      out = nrgen_unexpr(b, s, G, n->as<npar::UnaryExpr>());
      break;

    case QAST_NODE_TEREXPR:
      out = nrgen_terexpr(b, s, G, n->as<npar::TernaryExpr>());
      break;

    case QAST_NODE_INT:
      out = nrgen_int(b, s, G, n->as<npar::ConstInt>());
      break;

    case QAST_NODE_FLOAT:
      out = nrgen_float(b, s, G, n->as<npar::ConstFloat>());
      break;

    case QAST_NODE_STRING:
      out = nrgen_string(b, s, G, n->as<npar::ConstString>());
      break;

    case QAST_NODE_CHAR:
      out = nrgen_char(b, s, G, n->as<npar::ConstChar>());
      break;

    case QAST_NODE_BOOL:
      out = nrgen_bool(b, s, G, n->as<npar::ConstBool>());
      break;

    case QAST_NODE_NULL:
      out = nrgen_null(b, s, G, n->as<npar::ConstNull>());
      break;

    case QAST_NODE_UNDEF:
      out = nrgen_undef(b, s, G, n->as<npar::ConstUndef>());
      break;

    case QAST_NODE_CALL:
      out = nrgen_call(b, s, G, n->as<npar::Call>());
      break;

    case QAST_NODE_LIST:
      out = nrgen_list(b, s, G, n->as<npar::List>());
      break;

    case QAST_NODE_ASSOC:
      out = nrgen_assoc(b, s, G, n->as<npar::Assoc>());
      break;

    case QAST_NODE_FIELD:
      out = nrgen_field(b, s, G, n->as<npar::Field>());
      break;

    case QAST_NODE_INDEX:
      out = nrgen_index(b, s, G, n->as<npar::Index>());
      break;

    case QAST_NODE_SLICE:
      out = nrgen_slice(b, s, G, n->as<npar::Slice>());
      break;

    case QAST_NODE_FSTRING:
      out = nrgen_fstring(b, s, G, n->as<npar::FString>());
      break;

    case QAST_NODE_IDENT:
      out = nrgen_ident(b, s, G, n->as<npar::Ident>());
      break;

    case QAST_NODE_SEQ:
      out = nrgen_seq_point(b, s, G, n->as<npar::SeqPoint>());
      break;

    case QAST_NODE_POST_UNEXPR:
      out = nrgen_post_unexpr(b, s, G, n->as<npar::PostUnaryExpr>());
      break;

    case QAST_NODE_STMT_EXPR:
      out = nrgen_stmt_expr(b, s, G, n->as<npar::StmtExpr>());
      break;

    case QAST_NODE_TYPE_EXPR:
      out = nrgen_type_expr(b, s, G, n->as<npar::TypeExpr>());
      break;

    case QAST_NODE_TEMPL_CALL:
      out = nrgen_templ_call(b, s, G, n->as<npar::TemplCall>());
      break;

    case QAST_NODE_REF_TY:
      out = nrgen_ref_ty(b, s, G, n->as<npar::RefTy>());
      break;

    case QAST_NODE_U1_TY:
      out = nrgen_u1_ty(b, s, G, n->as<npar::U1>());
      break;

    case QAST_NODE_U8_TY:
      out = nrgen_u8_ty(b, s, G, n->as<npar::U8>());
      break;

    case QAST_NODE_U16_TY:
      out = nrgen_u16_ty(b, s, G, n->as<npar::U16>());
      break;

    case QAST_NODE_U32_TY:
      out = nrgen_u32_ty(b, s, G, n->as<npar::U32>());
      break;

    case QAST_NODE_U64_TY:
      out = nrgen_u64_ty(b, s, G, n->as<npar::U64>());
      break;

    case QAST_NODE_U128_TY:
      out = nrgen_u128_ty(b, s, G, n->as<npar::U128>());
      break;

    case QAST_NODE_I8_TY:
      out = nrgen_i8_ty(b, s, G, n->as<npar::I8>());
      break;

    case QAST_NODE_I16_TY:
      out = nrgen_i16_ty(b, s, G, n->as<npar::I16>());
      break;

    case QAST_NODE_I32_TY:
      out = nrgen_i32_ty(b, s, G, n->as<npar::I32>());
      break;

    case QAST_NODE_I64_TY:
      out = nrgen_i64_ty(b, s, G, n->as<npar::I64>());
      break;

    case QAST_NODE_I128_TY:
      out = nrgen_i128_ty(b, s, G, n->as<npar::I128>());
      break;

    case QAST_NODE_F16_TY:
      out = nrgen_f16_ty(b, s, G, n->as<npar::F16>());
      break;

    case QAST_NODE_F32_TY:
      out = nrgen_f32_ty(b, s, G, n->as<npar::F32>());
      break;

    case QAST_NODE_F64_TY:
      out = nrgen_f64_ty(b, s, G, n->as<npar::F64>());
      break;

    case QAST_NODE_F128_TY:
      out = nrgen_f128_ty(b, s, G, n->as<npar::F128>());
      break;

    case QAST_NODE_VOID_TY:
      out = nrgen_void_ty(b, s, G, n->as<npar::VoidTy>());
      break;

    case QAST_NODE_PTR_TY:
      out = nrgen_ptr_ty(b, s, G, n->as<npar::PtrTy>());
      break;

    case QAST_NODE_OPAQUE_TY:
      out = nrgen_opaque_ty(b, s, G, n->as<npar::OpaqueTy>());
      break;

    case QAST_NODE_ARRAY_TY:
      out = nrgen_array_ty(b, s, G, n->as<npar::ArrayTy>());
      break;

    case QAST_NODE_TUPLE_TY:
      out = nrgen_tuple_ty(b, s, G, n->as<npar::TupleTy>());
      break;

    case QAST_NODE_FN_TY:
      out = nrgen_fn_ty(b, s, G, n->as<npar::FuncTy>());
      break;

    case QAST_NODE_UNRES_TY:
      out = nrgen_unres_ty(b, s, G, n->as<npar::NamedTy>());
      break;

    case QAST_NODE_INFER_TY:
      out = nrgen_infer_ty(b, s, G, n->as<npar::InferTy>());
      break;

    case QAST_NODE_TEMPL_TY:
      out = nrgen_templ_ty(b, s, G, n->as<npar::TemplType>());
      break;

    case QAST_NODE_FNDECL:
      out = nrgen_fndecl(b, s, G, n->as<npar::FnDecl>());
      break;

    case QAST_NODE_FN:
      out = nrgen_fn(b, s, G, n->as<npar::FnDef>());
      break;

    case QAST_NODE_STRUCT_FIELD:
      out = nrgen_composite_field(b, s, G, n->as<npar::StructField>());
      break;

    case QAST_NODE_BLOCK:
      out = nrgen_block(b, s, G, n->as<npar::Block>(), true);
      break;

    case QAST_NODE_VAR:
      out = nrgen_var(b, s, G, n->as<npar::VarDecl>());
      break;

    case QAST_NODE_INLINE_ASM:
      out = nrgen_inline_asm(b, s, G, n->as<npar::InlineAsm>());
      break;

    case QAST_NODE_RETURN:
      out = nrgen_return(b, s, G, n->as<npar::ReturnStmt>());
      break;

    case QAST_NODE_RETIF:
      out = nrgen_retif(b, s, G, n->as<npar::ReturnIfStmt>());
      break;

    case QAST_NODE_BREAK:
      out = nrgen_break(b, s, G, n->as<npar::BreakStmt>());
      break;

    case QAST_NODE_CONTINUE:
      out = nrgen_continue(b, s, G, n->as<npar::ContinueStmt>());
      break;

    case QAST_NODE_IF:
      out = nrgen_if(b, s, G, n->as<npar::IfStmt>());
      break;

    case QAST_NODE_WHILE:
      out = nrgen_while(b, s, G, n->as<npar::WhileStmt>());
      break;

    case QAST_NODE_FOR:
      out = nrgen_for(b, s, G, n->as<npar::ForStmt>());
      break;

    case QAST_NODE_FOREACH:
      out = nrgen_foreach(b, s, G, n->as<npar::ForeachStmt>());
      break;

    case QAST_NODE_CASE:
      out = nrgen_case(b, s, G, n->as<npar::CaseStmt>());
      break;

    case QAST_NODE_SWITCH:
      out = nrgen_switch(b, s, G, n->as<npar::SwitchStmt>());
      break;

    case QAST_NODE_EXPR_STMT:
      out = nrgen_expr_stmt(b, s, G, n->as<npar::ExprStmt>());
      break;

    default: {
      break;
    }
  }

  return out;
}

static BResult nrgen_any(NRBuilder &b, PState &s, IReport *G, npar_node_t *n) {
  using namespace nr;

  if (!n) {
    return std::nullopt;
  }

  BResult out;

  switch (n->getKind()) {
    case QAST_NODE_TYPEDEF:
      out = nrgen_typedef(b, s, G, n->as<npar::TypedefStmt>());
      break;

    case QAST_NODE_ENUM:
      out = nrgen_enum(b, s, G, n->as<npar::EnumDef>());
      break;

    case QAST_NODE_STRUCT:
      out = nrgen_struct(b, s, G, n->as<npar::StructDef>());
      break;

    case QAST_NODE_SCOPE:
      out = nrgen_scope(b, s, G, n->as<npar::ScopeStmt>());
      break;

    case QAST_NODE_EXPORT:
      out = nrgen_export(b, s, G, n->as<npar::ExportStmt>());
      break;

    default: {
      auto expr = next_one(n);
      if (expr.has_value()) {
        out = {expr.value()};
      } else {
        return std::nullopt;
      }
    }
  }

  return out;
}
