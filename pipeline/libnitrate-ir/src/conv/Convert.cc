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

#include <core/LibMacro.h>
#include <nitrate-core/Error.h>
#include <nitrate-ir/IR.h>
#include <nitrate-parser/Node.h>
#include <nitrate-parser/Parser.h>

#include <core/Config.hh>
#include <core/Diagnostic.hh>
#include <core/PassManager.hh>
#include <cstdint>
#include <cstring>
#include <limits>
#include <nitrate-ir/Classes.hh>
#include <nitrate-ir/Format.hh>
#include <nitrate-ir/IRBuilder.hh>
#include <nitrate-ir/IRGraph.hh>
#include <nitrate-ir/Module.hh>
#include <stack>
#include <string>
#include <string_view>
#include <unordered_map>

#include "nitrate-ir/Report.hh"

using namespace nr;

struct PState {
  bool inside_function = false;
  std::string ns_prefix;
  std::stack<qparse::String> composite_expanse;
  nr::AbiTag abi_mode = nr::AbiTag::Internal;
  nr::Type *return_type = nullptr;
  std::stack<std::unordered_map<std::string_view, nr::Local *>> local_scope;

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

using EResult = std::optional<Expr *>;
using BResult = std::optional<std::vector<nr::Expr *>>;

static std::optional<nr::Expr *> nrgen_one(NRBuilder &b, PState &s, IReport *G, qparse::Node *node);
static BResult nrgen_any(NRBuilder &b, PState &s, IReport *G, qparse::Node *node);

#define next_one(n) nrgen_one(b, s, G, n)
#define next_any(n) nrgen_any(b, s, G, n)

LIB_EXPORT bool nr_lower(qmodule_t **mod, qparse_node_t *base, const char *name, bool diagnostics) {
  if (!mod || !base) {
    return false;
  }
  if (!name) {
    name = "module";
  }

  qcore_arena scratch_arena;
  std::swap(nr::nr_arena.get(), *scratch_arena.get());

  /// TODO: Get target info
  TargetInfo target_info;

  std::unique_ptr<DiagnosticManager> provider = std::make_unique<DiagnosticManager>();

  PState s;
  NRBuilder builder(name, target_info);

  qmodule_t *R = nullptr;
  bool success = false;

  if (auto root = nrgen_one(builder, s, provider.get(), static_cast<qparse::Node *>(base))) {
    builder.finish();

    if (builder.verify(diagnostics ? std::make_optional(provider.get()) : std::nullopt)) {
      R = builder.get_module();
      success = true;
    }
  }

  if (!R) {
    R = createModule(name);
    R->getDiag() = std::move(provider);
  }

  std::swap(nr::nr_arena.get(), *scratch_arena.get());
  *mod = R;

  return success;
}

///=============================================================================

static nr::Tmp *create_simple_call(NRBuilder &b, PState &, IReport *G, std::string_view name,
                                   std::vector<std::pair<std::string_view, nr::Expr *>,
                                               nr::Arena<std::pair<std::string_view, nr::Expr *>>>
                                       args = {}) {
  nr::CallArgsTmpNodeCradle datapack;

  std::get<0>(datapack) = nr::create<nr::Ident>(b.intern(name), nullptr);
  std::get<1>(datapack) = std::move(args);

  return create<nr::Tmp>(nr::TmpType::CALL, std::move(datapack));
}

nr::List *nr::createStringLiteral(std::string_view value) noexcept {
  ListItems items;
  items.resize(value.size() + 1);

  for (size_t i = 0; i < value.size(); i++) {
    items[i] = create<Int>(value[i], IntSize::U8);
  }

  /* Add null byte at end */
  items[value.size()] = create<Int>(0, IntSize::U8);

  return create<List>(items, true);
}

std::optional<nr::Expr *> nrgen_lower_binexpr(NRBuilder &, PState &, IReport *G, nr::Expr *lhs,
                                              nr::Expr *rhs, qlex_op_t op) {
#define STD_BINOP(op) nr::create<nr::BinExpr>(lhs, rhs, nr::Op::op)
#define ASSIGN_BINOP(op)                                                                     \
  nr::create<nr::BinExpr>(                                                                   \
      lhs, nr::create<nr::BinExpr>(static_cast<nr::Expr *>(nr_clone(lhs)), rhs, nr::Op::op), \
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

      auto a = nr::create<nr::BinExpr>(lhs, rhs, nr::Op::LogicOr);
      auto b = nr::create<nr::BinExpr>(lhs, rhs, nr::Op::LogicAnd);
      auto not_b = nr::create<nr::UnExpr>(b, nr::Op::LogicNot);
      return nr::create<nr::BinExpr>(lhs, nr::create<nr::BinExpr>(a, not_b, nr::Op::LogicAnd),
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
      auto methname = createStringLiteral("has");
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

std::optional<nr::Expr *> nrgen_lower_unexpr(NRBuilder &b, PState &s, IReport *G, nr::Expr *rhs,
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
      auto arg = nr::create<nr::BinExpr>(bits, nr::create<nr::Float>(8, nr::FloatSize::F64),
                                         nr::Op::Slash);
      R = create_simple_call(b, s, G, "std::ceil", {{"0", arg}});
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
        G->report(CompilerError, IC::Error, "Failed to mangle type name", rhs->getLoc());
        break;
      }

      R = createStringLiteral(res.value());
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

std::optional<nr::Expr *> nrgen_lower_post_unexpr(NRBuilder &, PState &, IReport *G, nr::Expr *lhs,
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
      /// FIXME: Message
      break;
    }
  }

  return R;
}

static EResult nrgen_cexpr(NRBuilder &b, PState &s, IReport *G, qparse::ConstExpr *n) {
  auto c = next_one(n->get_value());
  if (!c.has_value()) {
    G->report(CompilerError, IC::Error, "Failed to lower constant expression", n->get_pos());
    return std::nullopt;
  }

  return c;
}

static EResult nrgen_binexpr(NRBuilder &b, PState &s, IReport *G, qparse::BinExpr *n) {
  auto lhs = next_one(n->get_lhs());
  if (!lhs.has_value()) {
    return std::nullopt;
  }

  auto rhs = next_one(n->get_rhs());
  if (!rhs.has_value()) {
    return std::nullopt;
  }

  return nrgen_lower_binexpr(b, s, G, lhs.value(), rhs.value(), n->get_op());
}

static EResult nrgen_unexpr(NRBuilder &b, PState &s, IReport *G, qparse::UnaryExpr *n) {
  auto rhs = next_one(n->get_rhs());
  if (!rhs.has_value()) {
    return std::nullopt;
  }

  return nrgen_lower_unexpr(b, s, G, rhs.value(), n->get_op());
}

static EResult nrgen_post_unexpr(NRBuilder &b, PState &s, IReport *G, qparse::PostUnaryExpr *n) {
  auto lhs = next_one(n->get_lhs());
  if (!lhs.has_value()) {
    return std::nullopt;
  }

  return nrgen_lower_post_unexpr(b, s, G, lhs.value(), n->get_op());
}

static EResult nrgen_terexpr(NRBuilder &b, PState &s, IReport *G, qparse::TernaryExpr *n) {
  auto cond = next_one(n->get_cond());
  if (!cond.has_value()) {
    return std::nullopt;
  }

  auto lhs = next_one(n->get_lhs());
  if (!lhs.has_value()) {
    return std::nullopt;
  }

  auto rhs = next_one(n->get_rhs());
  if (!rhs.has_value()) {
    return std::nullopt;
  }

  return create<If>(cond.value(), lhs.value(), rhs.value());
}

static EResult nrgen_int(NRBuilder &b, PState &, IReport *G, qparse::ConstInt *n) {
  boost::multiprecision::cpp_int num(std::string_view(n->get_value()));

  if (num > std::numeric_limits<uint128_t>::max()) {
    G->report(nr::CompilerError, IC::Error, "Integer literal is not representable in uint128 type");
    return std::nullopt;
  } else if (num > UINT64_MAX) {
    return b.createFixedInteger(num.convert_to<uint128_t>(), IntSize::U128);
  } else if (num > UINT32_MAX) {
    return b.createFixedInteger(num.convert_to<uint128_t>(), IntSize::U64);
  } else {
    return b.createFixedInteger(num.convert_to<uint128_t>(), IntSize::U32);
  }
}

static EResult nrgen_float(NRBuilder &b, PState &, IReport *G, qparse::ConstFloat *n) {
  /// FIXME: Do floating-point literal range checking
  return create<Float>(b.intern(n->get_value()));
}

static EResult nrgen_string(NRBuilder &b, PState &, IReport *G, qparse::ConstString *n) {
  return b.createStringDataArray(n->get_value());
}

static EResult nrgen_char(NRBuilder &b, PState &, IReport *G, qparse::ConstChar *n) {
  if (n->get_value() > UINT8_MAX) {
    G->report(CompilerError, IC::Error,
              "Character literal value is outside the expected range of UINT8_MAX", n->get_pos());
    return std::nullopt;
  }

  return b.createFixedInteger(n->get_value(), IntSize::U8);
}

static EResult nrgen_bool(NRBuilder &b, PState &, IReport *G, qparse::ConstBool *n) {
  return b.createBool(n->get_value());
}

static EResult nrgen_null(NRBuilder &, PState &, IReport *G, qparse::ConstNull *) {
  return std::nullopt;

  /// TODO: This will be a special global variable for the __builtin_optional class
}

static EResult nrgen_undef(NRBuilder &, PState &, IReport *G, qparse::ConstUndef *n) {
  G->report(UnexpectedUndefLiteral, IC::Error, "", n->get_pos());
  return std::nullopt;
}

static EResult nrgen_call(NRBuilder &b, PState &s, IReport *G, qparse::Call *n) {
  auto target = next_one(n->get_func());
  if (!target) {
    return std::nullopt;
  }

  CallArgsTmpNodeCradle datapack;
  for (auto it = n->get_args().begin(); it != n->get_args().end(); ++it) {
    auto arg = next_one(it->second);
    if (!arg) {
      return std::nullopt;
    }

    // Implicit conversion are done later

    std::get<1>(datapack).push_back({b.intern(it->first), arg.value()});
  }
  std::get<0>(datapack) = target.value();

  return create<Tmp>(TmpType::CALL, std::move(datapack));
}

static EResult nrgen_list(NRBuilder &b, PState &s, IReport *G, qparse::List *n) {
  ListItems items;

  for (auto it = n->get_items().begin(); it != n->get_items().end(); ++it) {
    auto item = next_one(*it);
    if (!item.has_value()) {
      return std::nullopt;
    }

    items.push_back(item.value());
  }

  return b.createList(items, false);
}

static EResult nrgen_assoc(NRBuilder &b, PState &s, IReport *G, qparse::Assoc *n) {
  auto key = next_one(n->get_key());
  if (!key.has_value()) {
    return std::nullopt;
  }

  auto value = next_one(n->get_value());
  if (!value.has_value()) {
    return std::nullopt;
  }

  std::array<Expr *, 2> kv = {key.value(), value.value()};
  return b.createList(kv, false);
}

static EResult nrgen_field(NRBuilder &b, PState &s, IReport *G, qparse::Field *n) {
  auto base = next_one(n->get_base());
  if (!base.has_value()) {
    return std::nullopt;
  }

  Expr *field = createStringLiteral(n->get_field());
  return create<Index>(base.value(), field);
}

static EResult nrgen_index(NRBuilder &b, PState &s, IReport *G, qparse::Index *n) {
  auto base = next_one(n->get_base());
  if (!base.has_value()) {
    return std::nullopt;
  }

  auto index = next_one(n->get_index());
  if (!index.has_value()) {
    return std::nullopt;
  }

  return create<Index>(base.value(), index.value());
}

static EResult nrgen_slice(NRBuilder &b, PState &s, IReport *G, qparse::Slice *n) {
  auto base = next_one(n->get_base());
  if (!base.has_value()) {
    return std::nullopt;
  }

  auto start = next_one(n->get_start());
  if (!start.has_value()) {
    return std::nullopt;
  }

  auto end = next_one(n->get_end());
  if (!end.has_value()) {
    return std::nullopt;
  }

  return create<Call>(create<Index>(base.value(), createStringLiteral("slice")),
                      CallArgs({start.value(), end.value()}));
}

static EResult nrgen_fstring(NRBuilder &b, PState &s, IReport *G, qparse::FString *n) {
  if (n->get_items().empty()) {
    return b.createStringDataArray("");
  }

  if (n->get_items().size() == 1) {
    auto val = n->get_items().front();

    if (std::holds_alternative<qparse::String>(val)) {
      return createStringLiteral(std::get<qparse::String>(val));
    } else if (std::holds_alternative<qparse::Expr *>(val)) {
      auto expr = next_one(std::get<qparse::Expr *>(val));

      if (!expr.has_value()) {
        G->report(CompilerError, IC::Error,
                  "qparse::FString::get_items() vector contains std::nullopt", n->get_pos());
        return std::nullopt;
      }

      return expr;
    } else {
      qcore_panic("Invalid fstring item type");
    }
  }

  Expr *concated = createStringLiteral("");

  for (auto it = n->get_items().begin(); it != n->get_items().end(); ++it) {
    if (std::holds_alternative<qparse::String>(*it)) {
      auto val = std::get<qparse::String>(*it);

      concated = create<BinExpr>(concated, createStringLiteral(val), Op::Plus);
    } else if (std::holds_alternative<qparse::Expr *>(*it)) {
      auto val = std::get<qparse::Expr *>(*it);
      auto expr = next_one(val);

      if (!expr.has_value()) {
        G->report(CompilerError, IC::Error,
                  "qparse::FString::get_items() vector contains std::nullopt", n->get_pos());
        return std::nullopt;
      }

      concated = create<BinExpr>(concated, expr.value(), Op::Plus);
    } else {
      qcore_panic("Invalid fstring item type");
    }
  }

  return concated;
}

static EResult nrgen_ident(NRBuilder &b, PState &s, IReport *G, qparse::Ident *n) {
  if (s.inside_function) {
    qcore_assert(!s.local_scope.empty());

    auto find = s.local_scope.top().find(n->get_name());

    if (find != s.local_scope.top().end()) {
      return create<Ident>(b.intern(n->get_name()), find->second);
    }
  }

  auto str = s.cur_named(n->get_name());

  return create<Ident>(b.intern(std::string_view(str)), nullptr);
}

static EResult nrgen_seq_point(NRBuilder &b, PState &s, IReport *G, qparse::SeqPoint *n) {
  SeqItems items;
  items.reserve(n->get_items().size());

  for (auto it = n->get_items().begin(); it != n->get_items().end(); ++it) {
    auto item = next_one(*it);
    if (!item.has_value()) {
      G->report(CompilerError, IC::Error,
                "qparse::SeqPoint::get_items() vector contains std::nullopt", n->get_pos());
      return std::nullopt;
    }

    items.push_back(item.value());
  }

  return create<Seq>(std::move(items));
}

static EResult nrgen_stmt_expr(NRBuilder &b, PState &s, IReport *G, qparse::StmtExpr *n) {
  auto stmt = next_one(n->get_stmt());
  if (!stmt.has_value()) {
    return std::nullopt;
  }

  return stmt;
}

static EResult nrgen_type_expr(NRBuilder &b, PState &s, IReport *G, qparse::TypeExpr *n) {
  auto type = next_one(n->get_type());
  if (!type.has_value()) {
    return std::nullopt;
  }

  return type;
}

static EResult nrgen_templ_call(NRBuilder &, PState &, IReport *G, qparse::TemplCall *n) {
  /// TODO: Implement template function calls

  G->report(CompilerError, IC::Error, "Template call not implemented", n->get_pos());

  return std::nullopt;
}

static EResult nrgen_ref_ty(NRBuilder &b, PState &s, IReport *G, qparse::RefTy *n) {
  auto pointee = next_one(n->get_item());
  if (!pointee.has_value()) {
    return std::nullopt;
  }

  return b.getPtrTy(pointee.value()->asType());
}

static EResult nrgen_u1_ty(NRBuilder &b, PState &, IReport *G, qparse::U1 *) { return b.getU1Ty(); }
static EResult nrgen_u8_ty(NRBuilder &b, PState &, IReport *G, qparse::U8 *) { return b.getU8Ty(); }
static EResult nrgen_u16_ty(NRBuilder &b, PState &, IReport *G, qparse::U16 *) {
  return b.getU16Ty();
}
static EResult nrgen_u32_ty(NRBuilder &b, PState &, IReport *G, qparse::U32 *) {
  return b.getU32Ty();
}
static EResult nrgen_u64_ty(NRBuilder &b, PState &, IReport *G, qparse::U64 *) {
  return b.getU64Ty();
}
static EResult nrgen_u128_ty(NRBuilder &b, PState &, IReport *G, qparse::U128 *) {
  return b.getU128Ty();
}
static EResult nrgen_i8_ty(NRBuilder &b, PState &, IReport *G, qparse::I8 *) { return b.getI8Ty(); }
static EResult nrgen_i16_ty(NRBuilder &b, PState &, IReport *G, qparse::I16 *) {
  return b.getI16Ty();
}
static EResult nrgen_i32_ty(NRBuilder &b, PState &, IReport *G, qparse::I32 *) {
  return b.getI32Ty();
}
static EResult nrgen_i64_ty(NRBuilder &b, PState &, IReport *G, qparse::I64 *) {
  return b.getI64Ty();
}
static EResult nrgen_i128_ty(NRBuilder &b, PState &, IReport *G, qparse::I128 *) {
  return b.getI128Ty();
}
static EResult nrgen_f16_ty(NRBuilder &b, PState &, IReport *G, qparse::F16 *) {
  return b.getF16Ty();
}
static EResult nrgen_f32_ty(NRBuilder &b, PState &, IReport *G, qparse::F32 *) {
  return b.getF32Ty();
}
static EResult nrgen_f64_ty(NRBuilder &b, PState &, IReport *G, qparse::F64 *) {
  return b.getF64Ty();
}
static EResult nrgen_f128_ty(NRBuilder &b, PState &, IReport *G, qparse::F128 *) {
  return b.getF128Ty();
}
static EResult nrgen_void_ty(NRBuilder &b, PState &, IReport *G, qparse::VoidTy *) {
  return b.getVoidTy();
}

static EResult nrgen_ptr_ty(NRBuilder &b, PState &s, IReport *G, qparse::PtrTy *n) {
  auto pointee = next_one(n->get_item());
  if (!pointee.has_value()) {
    return std::nullopt;
  }

  return b.getPtrTy(pointee.value()->asType());
}

static EResult nrgen_opaque_ty(NRBuilder &b, PState &, IReport *G, qparse::OpaqueTy *n) {
  return b.getOpaqueTy(n->get_name());
}

static EResult nrgen_struct_ty(NRBuilder &b, PState &s, IReport *G, qparse::StructTy *n) {
  const qparse::StructItems &fields = n->get_items();

  std::vector<Type *> the_fields;
  the_fields.resize(fields.size());

  for (size_t i = 0; i < the_fields.size(); i++) {
    auto item = next_one(fields[i].second);
    if (!item.has_value()) {
      G->report(CompilerError, IC::Error,
                "qparse::StructTy::get_items() vector contains std::nullopt", n->get_pos());
      return std::nullopt;
    }

    the_fields.push_back(item.value()->asType());
  }

  return b.getStructTy(the_fields);
}

static EResult nrgen_array_ty(NRBuilder &b, PState &s, IReport *G, qparse::ArrayTy *n) {
  auto item = next_one(n->get_item());
  if (!item.has_value()) {
    return std::nullopt;
  }

  auto count_expr = next_one(n->get_size());
  if (!count_expr.has_value()) {
    return std::nullopt;
  }

  auto eprintn_cb = [&](std::string_view msg) {
    G->report(CompilerError, IC::Error, msg, count_expr.value()->getLoc());
  };

  auto result = nr::comptime_impl(count_expr.value(), eprintn_cb);
  if (!result.has_value()) {
    return std::nullopt;
  }

  if (result.value()->getKind() != QIR_NODE_INT) {
    G->report(CompilerError, IC::Error, "Non integer literal array size is not supported",
              n->get_pos());
    return std::nullopt;
  }

  uint128_t size = result.value()->as<Int>()->getValue();

  if (size > UINT64_MAX) {
    G->report(CompilerError, IC::Error, "Array size > UINT64_MAX", n->get_pos());
    return std::nullopt;
  }

  return b.getArrayTy(item.value()->asType(), static_cast<uint64_t>(size));
}

static EResult nrgen_tuple_ty(NRBuilder &b, PState &s, IReport *G, qparse::TupleTy *n) {
  StructFields fields;
  for (auto it = n->get_items().begin(); it != n->get_items().end(); ++it) {
    auto item = next_one(*it);
    if (!item.has_value()) {
      G->report(CompilerError, IC::Error,
                "qparse::TupleTy::get_items() vector contains std::nullopt", n->get_pos());
      return std::nullopt;
    }

    fields.push_back(item.value()->asType());
  }

  return create<StructTy>(std::move(fields));
}

using IsThreadSafe = bool;

static std::pair<Purity, IsThreadSafe> convert_purity(qparse::FuncPurity x) {
  switch (x) {
    case qparse::FuncPurity::IMPURE_THREAD_UNSAFE:
      return {Purity::Impure, false};
    case qparse::FuncPurity::IMPURE_THREAD_SAFE:
      return {Purity::Impure, true};
    case qparse::FuncPurity::PURE:
      return {Purity::Pure, true};
    case qparse::FuncPurity::QUASIPURE:
      return {Purity::Quasipure, true};
    case qparse::FuncPurity::RETROPURE:
      return {Purity::Retropure, true};
  }
}

static EResult nrgen_fn_ty(NRBuilder &b, PState &s, IReport *G, qparse::FuncTy *n) {
  FnParams params;

  for (auto it = n->get_params().begin(); it != n->get_params().end(); ++it) {
    auto type = next_one(std::get<1>(*it));
    if (!type.has_value()) {
      return std::nullopt;
    }

    params.push_back(type.value()->asType());
  }

  auto ret = next_one(n->get_return_ty());
  if (!ret.has_value()) {
    return std::nullopt;
  }

  auto props = convert_purity(n->get_purity());

  return b.getFnTy(params, ret.value()->asType(), n->is_variadic(), props.first, props.second,
                   n->is_noexcept(), n->is_foreign());
}

static EResult nrgen_unres_ty(NRBuilder &b, PState &s, IReport *G, qparse::UnresolvedType *n) {
  auto str = s.cur_named(n->get_name());

  return create<Tmp>(TmpType::NAMED_TYPE, b.intern(str));
}

static EResult nrgen_infer_ty(NRBuilder &b, PState &, IReport *G, qparse::InferType *) {
  return b.getUnknownTy();
}

static EResult nrgen_templ_ty(NRBuilder &b, PState &s, IReport *G, qparse::TemplType *n) {
  auto base_template = next_one(n->get_template());
  if (!base_template.has_value()) {
    return std::nullopt;
  }

  const qparse::TemplTypeArgs &templ_args = n->get_args();
  std::vector<Type *> template_args;
  template_args.resize(templ_args.size());

  for (size_t i = 0; i < template_args.size(); i++) {
    auto tmp = next_one(templ_args[i]);
    if (!tmp.has_value()) {
      G->report(CompilerError, IC::Error, "Failed to generate template instance argument",
                n->get_pos());
      return std::nullopt;
    }

    if (!tmp.value()->isType()) {
      G->report(CompilerError, IC::Error, "The template instance argument is not a type",
                n->get_pos());
      return std::nullopt;
    }

    template_args[i] = tmp.value()->asType();
  }

  return b.getTemplateInstance(base_template.value()->asType(), template_args);
}

static std::optional<std::vector<Expr *>> nrgen_typedef(NRBuilder &b, PState &s, IReport *G,
                                                        qparse::TypedefDecl *n) {
  // auto str = s.cur_named(n->get_name());
  // auto name = b.intern(std::string_view(str));

  // if (current->getTypeMap().contains(name)) {
  //   G->report(TypeRedefinition, IC::Error, n->get_name(),n->get_pos(),
  //         n->get_pos());
  // }

  // auto type = nrgen_one(b, s,X, n->get_type());
  // if (!type) {
  //   G->report(CompilerError, IC::Error,
  //          "qparse::TypedefDecl::get_type() == std::nullopt",n->get_pos(),
  //         n->get_pos());
  //   return std::nullopt;
  // }

  // current->getTypeMap()[name] = type.value()->asType();

  // return std::vector<Expr *>();

  return std::nullopt;
}

#define align(x, a) (((x) + (a) - 1) & ~((a) - 1))

static std::optional<std::vector<Expr *>> nrgen_struct(NRBuilder &b, PState &s, IReport *G,
                                                       qparse::StructDef *n) {
  // std::string name = s.cur_named(n->get_name());
  // auto sv = b.intern(std::string_view(name));

  // if (current->getTypeMap().contains(sv)) {
  //   G->report(TypeRedefinition, IC::Error, n->get_name(),n->get_pos(),
  //         n->get_pos());
  // }

  // StructFields fields;
  // std::optional<std::vector<Expr *>> items;
  // size_t offset = 0;

  // for (auto it = n->get_fields().begin(); it != n->get_fields().end(); ++it) {
  //   if (!*it) {
  //     G->report(CompilerError, IC::Error,
  //            "qparse::StructDef::get_fields() vector contains std::nullopt",n->get_pos(),
  //           n->get_pos());
  //     return std::nullopt;
  //   }

  //   s.composite_expanse.push((*it)->get_name());
  //   auto field = nrgen_one(b, s,X, *it);
  //   s.composite_expanse.pop();
  //   if (!field.has_value()) {
  //     G->report(CompilerError, IC::Error,
  //            "qparse::StructDef::get_fields() vector contains issue",n->get_pos(),
  //           n->get_pos());
  //     return std::nullopt;
  //   }

  //   size_t field_align = field.value()->asType()->getAlignBytes();
  //   size_t padding = align(offset, field_align) - offset;
  //   if (padding > 0) {
  //     fields.push_back(create<ArrayTy>(create<U8Ty>(), padding));
  //   }

  //   fields.push_back(field.value()->asType());
  //   offset += field.value()->asType()->getSizeBytes();
  // }

  // StructTy *st = create<StructTy>(std::move(fields));

  // current->getTypeMap()[sv] = st;

  // items = std::vector<Expr *>();
  // for (auto it = n->get_methods().begin(); it != n->get_methods().end(); ++it) {
  //   qparse::FnDecl *cur_meth = *it;
  //   auto old_name = cur_meth->get_name();
  //   cur_meth->set_name(ns_join(n->get_name(), old_name));

  //   auto method = nrgen_one(b, s,X, *it);

  //   cur_meth->set_name(old_name);

  //   if (!method) {
  //     G->report(CompilerError, IC::Error,
  //            "qparse::StructDef::get_methods() vector contains std::nullopt",n->get_pos(),
  //           n->get_pos());
  //     return std::nullopt;
  //   }

  //   items->push_back(method.value());
  // }

  // for (auto it = n->get_static_methods().begin(); it != n->get_static_methods().end(); ++it) {
  //   qparse::FnDecl *cur_meth = *it;
  //   auto old_name = cur_meth->get_name();
  //   cur_meth->set_name(ns_join(n->get_name(), old_name));

  //   auto method = nrgen_one(b, s,X, *it);

  //   cur_meth->set_name(old_name);

  //   if (!method) {
  //     G->report(CompilerError, IC::Error,
  //            "qparse::StructDef::get_static_methods() vector contains std::nullopt",
  //           n->get_pos(),n->get_pos());
  //     return std::nullopt;
  //   }

  //   items->push_back(method.value());
  // }

  // return items;

  return std::nullopt;
}

static std::optional<std::vector<Expr *>> nrgen_region(NRBuilder &b, PState &s, IReport *G,
                                                       qparse::RegionDef *n) {
  // std::string name = s.cur_named(n->get_name());
  // auto sv = b.intern(std::string_view(name));

  // if (current->getTypeMap().contains(sv)) {
  //   G->report(TypeRedefinition, IC::Error, n->get_name(),n->get_pos(),
  //         n->get_pos());
  // }

  // StructFields fields;
  // std::optional<std::vector<Expr *>> items;

  // for (auto it = n->get_fields().begin(); it != n->get_fields().end(); ++it) {
  //   if (!*it) {
  //     G->report(CompilerError, IC::Error,
  //            "qparse::RegionDef::get_fields() vector contains std::nullopt",n->get_pos(),
  //           n->get_pos());
  //     return std::nullopt;
  //   }

  //   s.composite_expanse.push((*it)->get_name());
  //   auto field = nrgen_one(b, s,X, *it);
  //   s.composite_expanse.pop();

  //   fields.push_back(field.value()->asType());
  // }

  // StructTy *st = create<StructTy>(std::move(fields));

  // current->getTypeMap()[sv] = st;

  // items = std::vector<Expr *>();
  // for (auto it = n->get_methods().begin(); it != n->get_methods().end(); ++it) {
  //   qparse::FnDecl *cur_meth = *it;
  //   auto old_name = cur_meth->get_name();
  //   cur_meth->set_name(ns_join(n->get_name(), old_name));

  //   auto method = nrgen_one(b, s,X, *it);

  //   cur_meth->set_name(old_name);

  //   if (!method) {
  //     G->report(CompilerError, IC::Error,
  //            "qparse::RegionDef::get_methods() vector contains std::nullopt",n->get_pos(),
  //           n->get_pos());
  //     return std::nullopt;
  //   }

  //   items->push_back(method.value());
  // }

  // for (auto it = n->get_static_methods().begin(); it != n->get_static_methods().end(); ++it) {
  //   qparse::FnDecl *cur_meth = *it;
  //   auto old_name = cur_meth->get_name();
  //   cur_meth->set_name(ns_join(n->get_name(), old_name));

  //   auto method = nrgen_one(b, s,X, *it);

  //   cur_meth->set_name(old_name);

  //   if (!method) {
  //     G->report(CompilerError, IC::Error,
  //            "qparse::RegionDef::get_static_methods() vector contains std::nullopt",
  //           n->get_pos(),n->get_pos());
  //     return std::nullopt;
  //   }

  //   items->push_back(method.value());
  // }

  // return items;

  return std::nullopt;
}

static std::optional<std::vector<Expr *>> nrgen_group(NRBuilder &b, PState &s, IReport *G,
                                                      qparse::GroupDef *n) {
  // std::string name = s.cur_named(n->get_name());
  // auto sv = b.intern(std::string_view(name));

  // if (current->getTypeMap().contains(sv)) {
  //   G->report(TypeRedefinition, IC::Error, n->get_name(),n->get_pos(),
  //         n->get_pos());
  // }

  // StructFields fields;
  // std::optional<std::vector<Expr *>> items;

  // { /* Optimize layout with field sorting */
  //   std::vector<Type *> tmp_fields;
  //   for (auto it = n->get_fields().begin(); it != n->get_fields().end(); ++it) {
  //     if (!*it) {
  //       G->report(CompilerError, IC::Error,
  //              "qparse::GroupDef::get_fields() vector contains std::nullopt",n->get_pos(),
  //             n->get_pos());
  //       return std::nullopt;
  //     }

  //     s.composite_expanse.push((*it)->get_name());
  //     auto field = nrgen_one(b, s,X, *it);
  //     s.composite_expanse.pop();

  //     tmp_fields.push_back(field.value()->asType());
  //   }

  //   std::sort(tmp_fields.begin(), tmp_fields.end(),
  //             [](Type *a, Type *b) { return a->getSizeBits() > b->getSizeBits(); });

  //   size_t offset = 0;
  //   for (auto it = tmp_fields.begin(); it != tmp_fields.end(); ++it) {
  //     size_t field_align = (*it)->getAlignBytes();
  //     size_t padding = align(offset, field_align) - offset;
  //     if (padding > 0) {
  //       fields.push_back(create<ArrayTy>(create<U8Ty>(), padding));
  //     }

  //     fields.push_back(*it);
  //     offset += (*it)->getSizeBytes();
  //   }
  // }

  // StructTy *st = create<StructTy>(std::move(fields));

  // current->getTypeMap()[sv] = st;

  // items = std::vector<Expr *>();
  // for (auto it = n->get_methods().begin(); it != n->get_methods().end(); ++it) {
  //   qparse::FnDecl *cur_meth = *it;
  //   auto old_name = cur_meth->get_name();
  //   cur_meth->set_name(ns_join(n->get_name(), old_name));

  //   auto method = nrgen_one(b, s,X, *it);

  //   cur_meth->set_name(old_name);

  //   if (!method) {
  //     G->report(CompilerError, IC::Error,
  //            "qparse::GroupDef::get_methods() vector contains std::nullopt",n->get_pos(),
  //           n->get_pos());
  //     return std::nullopt;
  //   }

  //   items->push_back(method.value());
  // }

  // for (auto it = n->get_static_methods().begin(); it != n->get_static_methods().end(); ++it) {
  //   qparse::FnDecl *cur_meth = *it;
  //   auto old_name = cur_meth->get_name();
  //   cur_meth->set_name(ns_join(n->get_name(), old_name));

  //   auto method = nrgen_one(b, s,X, *it);

  //   cur_meth->set_name(old_name);

  //   if (!method) {
  //     G->report(CompilerError, IC::Error,
  //            "qparse::GroupDef::get_static_methods() vector contains std::nullopt",
  //           n->get_pos(),n->get_pos());
  //     return std::nullopt;
  //   }

  //   items->push_back(method.value());
  // }

  // return items;

  return std::nullopt;
}

static std::optional<std::vector<Expr *>> nrgen_union(NRBuilder &b, PState &s, IReport *G,
                                                      qparse::UnionDef *n) {
  // std::string name = s.cur_named(n->get_name());
  // auto sv = b.intern(std::string_view(name));

  // if (current->getTypeMap().contains(sv)) {
  //   G->report(TypeRedefinition, IC::Error, n->get_name(),n->get_pos(),
  //         n->get_pos());
  // }

  // UnionFields fields;
  // std::optional<std::vector<Expr *>> items;

  // for (auto it = n->get_fields().begin(); it != n->get_fields().end(); ++it) {
  //   if (!*it) {
  //     G->report(CompilerError, IC::Error,
  //            "qparse::UnionDef::get_fields() vector contains std::nullopt",n->get_pos(),
  //           n->get_pos());
  //     return std::nullopt;
  //   }

  //   s.composite_expanse.push((*it)->get_name());
  //   auto field = nrgen_one(b, s,X, *it);
  //   s.composite_expanse.pop();

  //   fields.push_back(field.value()->asType());
  // }

  // UnionTy *st = create<UnionTy>(std::move(fields));

  // current->getTypeMap()[sv] = st;

  // items = std::vector<Expr *>();
  // for (auto it = n->get_methods().begin(); it != n->get_methods().end(); ++it) {
  //   qparse::FnDecl *cur_meth = *it;
  //   auto old_name = cur_meth->get_name();
  //   cur_meth->set_name(ns_join(n->get_name(), old_name));

  //   auto method = nrgen_one(b, s,X, *it);

  //   cur_meth->set_name(old_name);

  //   if (!method) {
  //     G->report(CompilerError, IC::Error,
  //            "qparse::UnionDef::get_methods() vector contains std::nullopt",n->get_pos(),
  //           n->get_pos());
  //     return std::nullopt;
  //   }

  //   items->push_back(method.value());
  // }

  // for (auto it = n->get_static_methods().begin(); it != n->get_static_methods().end(); ++it) {
  //   qparse::FnDecl *cur_meth = *it;
  //   auto old_name = cur_meth->get_name();
  //   cur_meth->set_name(ns_join(n->get_name(), old_name));

  //   auto method = nrgen_one(b, s,X, *it);

  //   cur_meth->set_name(old_name);

  //   if (!method) {
  //     G->report(CompilerError, IC::Error,
  //            "qparse::UnionDef::get_static_methods() vector contains std::nullopt",
  //           n->get_pos(),n->get_pos());
  //     return std::nullopt;
  //   }

  //   items->push_back(method.value());
  // }

  // return items;

  return std::nullopt;
}

static std::optional<std::vector<Expr *>> nrgen_enum(NRBuilder &b, PState &s, IReport *G,
                                                     qparse::EnumDef *n) {
  // std::string name = s.cur_named(n->get_name());
  // auto sv = b.intern(std::string_view(name));

  // if (current->getTypeMap().contains(sv)) {
  //   G->report(TypeRedefinition, IC::Error, n->get_name(),n->get_pos(),
  //         n->get_pos());
  // }

  // EResult type = nullptr;
  // if (n->get_type()) {
  //   type = nrgen_one(b, s,X, n->get_type());
  //   if (!type.has_value()) {
  //     G->report(CompilerError, IC::Error,
  //            "qparse::EnumDef::get_type() == std::nullopt",n->get_pos(),
  //           n->get_pos());
  //     return std::nullopt;
  //   }
  // } else {
  //   type = create<Tmp>(TmpType::ENUM, sv)->asType();
  // }

  // current->getTypeMap()[sv] = type.value()->asType();

  // Expr *last = nullptr;

  // for (auto it = n->get_items().begin(); it != n->get_items().end(); ++it) {
  //   EResult cur = nullptr;

  //   if (it->second) {
  //     cur = nrgen_one(b, s,X, it->second);
  //     if (!cur.has_value()) {
  //       G->report(CompilerError, IC::Error,
  //              "qparse::EnumDef::get_items() vector contains std::nullopt",n->get_pos(),
  //             n->get_pos());
  //       return std::nullopt;
  //     }

  //     last = cur.value();
  //   } else {
  //     if (!last) {
  //       cur = create<Int>(0, IntSize::U32);
  //       last = cur.value();
  //     } else {
  //       cur = create<BinExpr>(last, create<Int>(1, IntSize::U32), Op::Plus);
  //       last = cur.value();
  //     }
  //   }

  //   std::string_view field_name = b.intern(std::string_view(name + "::" +
  //   std::string(it->first)));

  //   current->getNamedConstants().insert({field_name, cur.value()});
  // }

  // return {};

  return std::nullopt;
}

static EResult nrgen_fndecl(NRBuilder &b, PState &s, IReport *G, qparse::FnDecl *n) {
  qparse::FuncTy *func_ty = n->get_type();
  const qparse::FuncParams &params = func_ty->get_params();

  std::vector<NRBuilder::FnParam> parameters;
  parameters.resize(params.size());

  for (size_t i = 0; i < params.size(); i++) {
    const auto &param = params[i];
    NRBuilder::FnParam p;

    { /* Set function parameter name */
      std::get<0>(p) = b.intern(std::get<0>(param));
    }

    { /* Set function parameter type */
      auto tmp = next_one(std::get<1>(param));
      if (!tmp.has_value()) {
        G->report(nr::CompilerError, nr::IC::Error,
                  "Failed to convert function declaration parameter type");
        return std::nullopt;
      }

      std::get<1>(p) = tmp.value()->asType();
    }

    { /* Set function parameter default value if it exists */
      if (std::get<2>(param) != nullptr) {
        auto val = next_one(std::get<2>(param));
        if (!val.has_value()) {
          G->report(nr::CompilerError, nr::IC::Error,
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
    G->report(nr::CompilerError, nr::IC::Error,
              "Failed to convert function declaration return type");
    return std::nullopt;
  }

  auto props = convert_purity(func_ty->get_purity());

  Fn *fndecl = b.createFunctionDeclaration(
      b.intern(n->get_name()), parameters, ret_type.value()->asType(), func_ty->is_variadic(),
      Vis::Pub, props.first, props.second, func_ty->is_noexcept(), func_ty->is_foreign());

  fndecl->setAbiTag(s.abi_mode);

  return fndecl;
}

static EResult nrgen_fn(NRBuilder &b, PState &s, IReport *G, qparse::FnDef *n) {
  // bool old_inside_function = s.inside_function;
  // s.inside_function = true;

  // EResult precond, postcond;
  // Seq *body = nullptr;
  // Params params;
  // qparse::FnDecl *decl = n;
  // qparse::FuncTy *fty = decl->get_type();

  // auto fnty = nrgen_one(b, s,X, fty);
  // if (!fnty.has_value()) {
  //   G->report(CompilerError, IC::Error, "qparse::FnDef::get_type() ==
  //   std::nullopt",
  //         n->get_pos(),n->get_pos());
  //   return std::nullopt;
  // }

  // /* Produce the function preconditions */
  // if ((precond = nrgen_one(b, s,X, n->get_precond()))) {
  //   precond = create<If>(create<UnExpr>(precond.value(), Op::LogicNot),
  //                        create_simple_call(b, s, "__detail::precond_fail"), createIgn());
  // }

  // /* Produce the function postconditions */
  // if ((postcond = nrgen_one(b, s,X, n->get_postcond()))) {
  //   postcond = create<If>(create<UnExpr>(postcond.value(), Op::LogicNot),
  //                         create_simple_call(b, s, "__detail::postcond_fail"), createIgn());
  // }

  // { /* Produce the function body */
  //   Type *old_return_ty = s.return_type;
  //   s.return_type = fnty.value()->as<FnTy>()->getReturn();
  //   s.local_scope.push({});

  //   auto tmp = nrgen_one(b, s,X, n->get_body());
  //   if (!tmp.has_value()) {
  //     G->report(CompilerError, IC::Error,
  //            "qparse::FnDef::get_body() == std::nullopt",n->get_pos(),n->get_pos());
  //     return std::nullopt;
  //   }

  //   if (tmp.value()->getKind() != QIR_NODE_SEQ) {
  //     tmp = create<Seq>(SeqItems({tmp.value()}));
  //   }

  //   Seq *seq = tmp.value()->as<Seq>();

  //   { /* Implicit return */
  //     if (fty->get_return_ty()->is_void()) {
  //       if (!seq->getItems().empty()) {
  //         if (seq->getItems().back()->getKind() != QIR_NODE_RET) {
  //           seq->getItems().push_back(create<Ret>(create<VoidTy>()));
  //         }
  //       }
  //     }
  //   }

  //   body = seq;

  //   if (precond) {
  //     body->getItems().insert(body->getItems().begin(), precond.value());
  //   }
  //   if (postcond) {
  //     /// TODO: add postcond at each exit point
  //   }

  //   s.local_scope.pop();
  //   s.return_type = old_return_ty;
  // }

  // auto name = s.cur_named(n->get_name());
  // auto str = b.intern(std::string_view(name));

  // current->getParameterMap()[str] = {};

  // { /* Produce the function parameters */
  //   for (auto it = fty->get_params().begin(); it != fty->get_params().end(); ++it) {
  //     /**
  //      * Parameter properties:
  //      * 1. Name - All parameters have a name.
  //      * 2. Type - All parameters have a type.
  //      * 3. Default - Optional, if the parameter has a default value.
  //      * 4. Position - All parameters have a position.
  //      */

  //     auto type = nrgen_one(b, s,X, std::get<1>(*it));
  //     if (!type.has_value()) {
  //       G->report(CompilerError, IC::Error,
  //              "qparse::FnDef::get_type() == std::nullopt",n->get_pos(),
  //             n->get_pos());
  //       return std::nullopt;
  //     }

  //     EResult def = nullptr;
  //     if (std::get<2>(*it)) {
  //       def = nrgen_one(b, s,X, std::get<2>(*it));
  //       if (!def.has_value()) {
  //         G->report(CompilerError, IC::Error,
  //                "qparse::FnDef::get_type() == std::nullopt",n->get_pos(),
  //               n->get_pos());
  //         return std::nullopt;
  //       }
  //     }

  //     std::string_view sv = b.intern(std::string_view(std::get<0>(*it)));

  //     params.push_back({type.value()->asType(), sv});
  //     current->getParameterMap()[str].push_back(
  //         {std::string(std::get<0>(*it)), type.value()->asType(), def.value()});
  //   }
  // }

  // auto obj = create<Fn>(str, std::move(params), fnty.value()->as<FnTy>()->getReturn(), body,
  //                       fty->is_variadic(), s.abi_mode);

  // current->getFunctions().insert({str, {fnty.value()->as<FnTy>(), obj}});

  // s.inside_function = old_inside_function;
  // return obj;

  return std::nullopt;
}

static std::optional<std::vector<Expr *>> nrgen_subsystem(NRBuilder &b, PState &s, IReport *G,
                                                          qparse::SubsystemDecl *n) {
  /**
   * @brief Convert a subsystem declaration to a nr sequence with
   * namespace prefixes.
   */

  std::optional<std::vector<Expr *>> items;

  if (!n->get_body()) {
    return std::nullopt;
  }

  std::string old_ns = s.ns_prefix;

  if (s.ns_prefix.empty()) {
    s.ns_prefix = std::string(n->get_name());
  } else {
    s.ns_prefix += "::" + std::string(n->get_name());
  }

  for (auto it = n->get_body()->get_items().begin(); it != n->get_body()->get_items().end(); ++it) {
    auto item = next_any(*it);
    if (!item.has_value()) {
      return std::nullopt;
    }

    items->insert(items->end(), item.value().begin(), item.value().end());
  }

  s.ns_prefix = old_ns;

  return items;
}

static std::optional<std::vector<Expr *>> nrgen_export(NRBuilder &b, PState &s, IReport *G,
                                                       qparse::ExportDecl *n) {
  AbiTag old = s.abi_mode;

  if (n->get_abi_name().empty()) {
    s.abi_mode = AbiTag::Default;
  } else if (n->get_abi_name() == "q") {
    s.abi_mode = AbiTag::Nitrate;
  } else if (n->get_abi_name() == "c") {
    s.abi_mode = AbiTag::C;
  } else {
    G->report(CompilerError, IC::Error,
              "qparse::ExportDecl abi name is not supported: '" + n->get_abi_name() + "'",
              n->get_pos());
    return std::nullopt;
  }

  if (!n->get_body()) {
    return std::nullopt;
  }

  std::string_view abi_name;

  if (n->get_abi_name().empty()) {
    abi_name = "std";
  } else {
    abi_name = b.intern(n->get_abi_name());
  }

  std::vector<nr::Expr *> items;

  for (auto it = n->get_body()->get_items().begin(); it != n->get_body()->get_items().end(); ++it) {
    auto result = next_any(*it);
    if (!result.has_value()) {
      return std::nullopt;
    }

    for (auto &item : result.value()) {
      items.push_back(create<Extern>(item, abi_name));
    }
  }

  s.abi_mode = old;

  return items;
}

static EResult nrgen_composite_field(NRBuilder &b, PState &s, IReport *G,
                                     qparse::CompositeField *n) {
  // auto type = nrgen_one(b, s,X, n->get_type());
  // if (!type) {
  //   G->report(CompilerError, IC::Error,
  //          "qparse::CompositeField::get_type() == std::nullopt",n->get_pos(),
  //         n->get_pos());
  //   return std::nullopt;
  // }

  // EResult _def = nullptr;
  // if (n->get_value()) {
  //   _def = nrgen_one(b, s,X, n->get_value());
  //   if (!_def.has_value()) {
  //     G->report(CompilerError, IC::Error,
  //            "qparse::CompositeField::get_value() == std::nullopt",n->get_pos(),
  //           n->get_pos());
  //     return std::nullopt;
  //   }
  // }

  // if (s.composite_expanse.empty()) {
  //   G->report(CompilerError, IC::Error, "state.composite_expanse.empty()",
  //         n->get_pos(),n->get_pos());
  //   return std::nullopt;
  // }

  // std::string_view dt_name = b.intern(s.composite_expanse.top());

  // current->getCompositeFields()[dt_name].push_back(
  //     {std::string(n->get_name()), type.value()->asType(), _def.value()});

  // return type;

  return std::nullopt;
}

static EResult nrgen_block(NRBuilder &b, PState &s, IReport *G, qparse::Block *n) {
  SeqItems items;
  items.reserve(n->get_items().size());

  s.local_scope.push({});

  for (auto it = n->get_items().begin(); it != n->get_items().end(); ++it) {
    auto item = next_any(*it);
    if (!item.has_value()) {
      return std::nullopt;
    }

    items.insert(items.end(), item.value().begin(), item.value().end());
  }

  s.local_scope.pop();

  return create<Seq>(std::move(items));
}

static EResult nrgen_const(NRBuilder &b, PState &s, IReport *G, qparse::ConstDecl *n) {
  auto init = next_one(n->get_value());
  auto type = next_one(n->get_type());

  if (!init.has_value()) {
    G->report(TypeInference, IC::Error, "Expected initial value in const declaration");
    return std::nullopt;
  }

  if (type.has_value()) { /* Do implicit cast */
    init = create<BinExpr>(init.value(), type.value(), Op::CastAs);
  } else if (!type.has_value()) {
    type = b.getUnknownTy();
  }

  qcore_assert(init.has_value() && type.has_value());

  StorageClass storage =
      s.inside_function ? StorageClass::LLVM_StackAlloa : StorageClass::LLVM_Static;
  Vis visibility = s.abi_mode == AbiTag::Internal ? Vis::Sec : Vis::Pub;

  Local *local =
      b.createVariable(b.intern(n->get_name()), type.value()->asType(), visibility, storage, true);

  local->setValue(init.value());
  local->setAbiTag(s.abi_mode);

  return local;
}

static EResult nrgen_var(NRBuilder &b, PState &s, IReport *G, qparse::VarDecl *n) {
  auto init = next_one(n->get_value());
  auto type = next_one(n->get_type());

  if (init.has_value() && type.has_value()) { /* Do implicit cast */
    init = create<BinExpr>(init.value(), type.value(), Op::CastAs);
  } else if (init.has_value() && !type.has_value()) {
    type = b.getUnknownTy();
  } else if (type.has_value() && !init.has_value()) {
    init = b.getDefaultValue(type.value()->asType());
  } else {
    G->report(TypeInference, IC::Error, "Expected type or initial value in var declaration");
    return std::nullopt;
  }

  qcore_assert(init.has_value() && type.has_value());

  StorageClass storage = s.inside_function ? StorageClass::Managed : StorageClass::LLVM_Static;
  Vis visibility = s.abi_mode == AbiTag::Internal ? Vis::Sec : Vis::Pub;

  Local *local =
      b.createVariable(b.intern(n->get_name()), type.value()->asType(), visibility, storage, false);

  local->setValue(init.value());
  local->setAbiTag(s.abi_mode);

  return local;
}

static EResult nrgen_let(NRBuilder &b, PState &s, IReport *G, qparse::LetDecl *n) {
  auto init = next_one(n->get_value());
  auto type = next_one(n->get_type());

  if (init.has_value() && type.has_value()) { /* Do implicit cast */
    init = create<BinExpr>(init.value(), type.value(), Op::CastAs);
  } else if (init.has_value() && !type.has_value()) {
    type = b.getUnknownTy();
  } else if (type.has_value() && !init.has_value()) {
    init = b.getDefaultValue(type.value()->asType());
  } else {
    G->report(TypeInference, IC::Error, "Expected type or initial value in let declaration");
    return std::nullopt;
  }

  qcore_assert(init.has_value() && type.has_value());

  StorageClass storage =
      s.inside_function ? StorageClass::LLVM_StackAlloa : StorageClass::LLVM_Static;
  Vis visibility = s.abi_mode == AbiTag::Internal ? Vis::Sec : Vis::Pub;

  Local *local =
      b.createVariable(b.intern(n->get_name()), type.value()->asType(), visibility, storage, false);

  local->setValue(init.value());
  local->setAbiTag(s.abi_mode);

  return local;
}

static EResult nrgen_inline_asm(NRBuilder &, PState &, IReport *G, qparse::InlineAsm *) {
  qcore_implement();
}

static EResult nrgen_return(NRBuilder &b, PState &s, IReport *G, qparse::ReturnStmt *n) {
  auto val = next_one(n->get_value());
  if (!val.has_value()) {
    val = create<VoidTy>();
  }

  val = create<BinExpr>(val.value(), s.return_type, Op::CastAs);

  return create<Ret>(val.value());
}

static EResult nrgen_retif(NRBuilder &b, PState &s, IReport *G, qparse::ReturnIfStmt *n) {
  auto cond = next_one(n->get_cond());
  if (!cond.has_value()) {
    return std::nullopt;
  }

  cond = create<BinExpr>(cond.value(), create<U1Ty>(), Op::CastAs);

  auto val = next_one(n->get_value());
  if (!val.has_value()) {
    return std::nullopt;
  }

  val = create<BinExpr>(val.value(), s.return_type, Op::CastAs);

  return create<If>(cond.value(), create<Ret>(val.value()), createIgn());
}

static EResult nrgen_retz(NRBuilder &b, PState &s, IReport *G, qparse::RetZStmt *n) {
  auto cond = next_one(n->get_cond());
  if (!cond.has_value()) {
    return std::nullopt;
  }

  cond = create<BinExpr>(cond.value(), create<U1Ty>(), Op::CastAs);

  auto inv_cond = create<UnExpr>(cond.value(), Op::LogicNot);

  auto val = next_one(n->get_value());
  if (!val.has_value()) {
    return std::nullopt;
  }

  val = create<BinExpr>(val.value(), s.return_type, Op::CastAs);

  return create<If>(inv_cond, create<Ret>(val.value()), createIgn());
}

static EResult nrgen_retv(NRBuilder &b, PState &s, IReport *G, qparse::RetVStmt *n) {
  auto cond = next_one(n->get_cond());
  if (!cond.has_value()) {
    return std::nullopt;
  }

  cond = create<BinExpr>(cond.value(), create<U1Ty>(), Op::CastAs);

  return create<If>(cond.value(), create<Ret>(createIgn()), createIgn());
}

static EResult nrgen_break(NRBuilder &, PState &, IReport *G, qparse::BreakStmt *) {
  return create<Brk>();
}

static EResult nrgen_continue(NRBuilder &, PState &, IReport *G, qparse::ContinueStmt *) {
  return create<Cont>();
}

static EResult nrgen_if(NRBuilder &b, PState &s, IReport *G, qparse::IfStmt *n) {
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

static EResult nrgen_while(NRBuilder &b, PState &s, IReport *G, qparse::WhileStmt *n) {
  auto cond = next_one(n->get_cond());
  auto body = next_one(n->get_body());

  if (!cond.has_value()) {
    cond = create<Int>(1, IntSize::U1);
  }

  cond = create<BinExpr>(cond.value(), create<U1Ty>(), Op::CastAs);

  if (!body.has_value()) {
    body = create<Seq>(SeqItems({}));
  } else if (body.value()->getKind() != QIR_NODE_SEQ) {
    body = create<Seq>(SeqItems({body.value()}));
  }

  return create<While>(cond.value(), body.value()->as<Seq>());
}

static EResult nrgen_for(NRBuilder &b, PState &s, IReport *G, qparse::ForStmt *n) {
  auto init = next_one(n->get_init());
  auto cond = next_one(n->get_cond());
  auto step = next_one(n->get_step());
  auto body = next_one(n->get_body());

  if (!init.has_value()) {
    init = create<Int>(1, IntSize::U32);
  }

  if (!cond.has_value()) {
    cond = create<Int>(1, IntSize::U32);  // infinite loop like 'for (;;) {}'
    cond = create<BinExpr>(cond.value(), create<U1Ty>(), Op::CastAs);
  }

  if (!step.has_value()) {
    step = create<Int>(1, IntSize::U32);
  }

  if (!body.has_value()) {
    body = create<Int>(1, IntSize::U32);
  }

  return create<For>(init.value(), cond.value(), step.value(), body.value());
}

static EResult nrgen_form(NRBuilder &, PState &, IReport *G, qparse::FormStmt *) {
  G->report(nr::CompilerError, IC::Error, "Concurrent for loops not implemented yet");
  return std::nullopt;
}

static EResult nrgen_foreach(NRBuilder &, PState &, IReport *G, qparse::ForeachStmt *) {
  /**
   * @brief Convert a foreach loop to a nr expression.
   * @details This is a 1-to-1 conversion of the foreach loop.
   */

  // auto idx_name = b.intern(n->get_idx_ident());
  // auto val_name = b.intern(n->get_val_ident());

  // auto iter = nrgen_one(b, s,X, n->get_expr());
  // if (!iter) {
  //   G->report(CompilerError, IC::Error, "qparse::ForeachStmt::get_expr() ==
  //   std::nullopt",n->get_start_pos(),n->get_pos()); return std::nullopt;
  // }

  // auto body = nrgen_one(b, s,X, n->get_body());
  // if (!body) {
  //   G->report(CompilerError, IC::Error, "qparse::ForeachStmt::get_body() ==
  //   std::nullopt",n->get_start_pos(),n->get_pos()); return std::nullopt;
  // }

  // return create<Foreach>(idx_name, val_name, iter, create<Seq>(SeqItems({body})));
  qcore_implement();
}

static EResult nrgen_case(NRBuilder &b, PState &s, IReport *G, qparse::CaseStmt *n) {
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

static EResult nrgen_switch(NRBuilder &b, PState &s, IReport *G, qparse::SwitchStmt *n) {
  auto cond = next_one(n->get_cond());
  if (!cond.has_value()) {
    return std::nullopt;
  }

  SwitchCases cases;
  for (auto it = n->get_cases().begin(); it != n->get_cases().end(); ++it) {
    auto item = next_one(*it);
    if (!item.has_value()) {
      G->report(CompilerError, IC::Error,
                "qparse::SwitchStmt::get_cases() vector contains std::nullopt", n->get_pos());
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

static EResult nrgen_expr_stmt(NRBuilder &b, PState &s, IReport *G, qparse::ExprStmt *n) {
  return next_one(n->get_expr());
}

static EResult nrgen_volstmt(NRBuilder &, PState &, IReport *G, qparse::VolStmt *n) {
  G->report(CompilerError, IC::Error, "Volatile statements are not supported", n->get_pos());

  return std::nullopt;
}

static std::optional<nr::Expr *> nrgen_one(NRBuilder &b, PState &s, IReport *G, qparse::Node *n) {
  using namespace nr;

  if (!n) {
    return std::nullopt;
  }

  std::optional<nr::Expr *> out;

  switch (n->this_typeid()) {
    case QAST_NODE_CEXPR:
      out = nrgen_cexpr(b, s, G, n->as<qparse::ConstExpr>());
      break;

    case QAST_NODE_BINEXPR:
      out = nrgen_binexpr(b, s, G, n->as<qparse::BinExpr>());
      break;

    case QAST_NODE_UNEXPR:
      out = nrgen_unexpr(b, s, G, n->as<qparse::UnaryExpr>());
      break;

    case QAST_NODE_TEREXPR:
      out = nrgen_terexpr(b, s, G, n->as<qparse::TernaryExpr>());
      break;

    case QAST_NODE_INT:
      out = nrgen_int(b, s, G, n->as<qparse::ConstInt>());
      break;

    case QAST_NODE_FLOAT:
      out = nrgen_float(b, s, G, n->as<qparse::ConstFloat>());
      break;

    case QAST_NODE_STRING:
      out = nrgen_string(b, s, G, n->as<qparse::ConstString>());
      break;

    case QAST_NODE_CHAR:
      out = nrgen_char(b, s, G, n->as<qparse::ConstChar>());
      break;

    case QAST_NODE_BOOL:
      out = nrgen_bool(b, s, G, n->as<qparse::ConstBool>());
      break;

    case QAST_NODE_NULL:
      out = nrgen_null(b, s, G, n->as<qparse::ConstNull>());
      break;

    case QAST_NODE_UNDEF:
      out = nrgen_undef(b, s, G, n->as<qparse::ConstUndef>());
      break;

    case QAST_NODE_CALL:
      out = nrgen_call(b, s, G, n->as<qparse::Call>());
      break;

    case QAST_NODE_LIST:
      out = nrgen_list(b, s, G, n->as<qparse::List>());
      break;

    case QAST_NODE_ASSOC:
      out = nrgen_assoc(b, s, G, n->as<qparse::Assoc>());
      break;

    case QAST_NODE_FIELD:
      out = nrgen_field(b, s, G, n->as<qparse::Field>());
      break;

    case QAST_NODE_INDEX:
      out = nrgen_index(b, s, G, n->as<qparse::Index>());
      break;

    case QAST_NODE_SLICE:
      out = nrgen_slice(b, s, G, n->as<qparse::Slice>());
      break;

    case QAST_NODE_FSTRING:
      out = nrgen_fstring(b, s, G, n->as<qparse::FString>());
      break;

    case QAST_NODE_IDENT:
      out = nrgen_ident(b, s, G, n->as<qparse::Ident>());
      break;

    case QAST_NODE_SEQ_POINT:
      out = nrgen_seq_point(b, s, G, n->as<qparse::SeqPoint>());
      break;

    case QAST_NODE_POST_UNEXPR:
      out = nrgen_post_unexpr(b, s, G, n->as<qparse::PostUnaryExpr>());
      break;

    case QAST_NODE_STMT_EXPR:
      out = nrgen_stmt_expr(b, s, G, n->as<qparse::StmtExpr>());
      break;

    case QAST_NODE_TYPE_EXPR:
      out = nrgen_type_expr(b, s, G, n->as<qparse::TypeExpr>());
      break;

    case QAST_NODE_TEMPL_CALL:
      out = nrgen_templ_call(b, s, G, n->as<qparse::TemplCall>());
      break;

    case QAST_NODE_REF_TY:
      out = nrgen_ref_ty(b, s, G, n->as<qparse::RefTy>());
      break;

    case QAST_NODE_U1_TY:
      out = nrgen_u1_ty(b, s, G, n->as<qparse::U1>());
      break;

    case QAST_NODE_U8_TY:
      out = nrgen_u8_ty(b, s, G, n->as<qparse::U8>());
      break;

    case QAST_NODE_U16_TY:
      out = nrgen_u16_ty(b, s, G, n->as<qparse::U16>());
      break;

    case QAST_NODE_U32_TY:
      out = nrgen_u32_ty(b, s, G, n->as<qparse::U32>());
      break;

    case QAST_NODE_U64_TY:
      out = nrgen_u64_ty(b, s, G, n->as<qparse::U64>());
      break;

    case QAST_NODE_U128_TY:
      out = nrgen_u128_ty(b, s, G, n->as<qparse::U128>());
      break;

    case QAST_NODE_I8_TY:
      out = nrgen_i8_ty(b, s, G, n->as<qparse::I8>());
      break;

    case QAST_NODE_I16_TY:
      out = nrgen_i16_ty(b, s, G, n->as<qparse::I16>());
      break;

    case QAST_NODE_I32_TY:
      out = nrgen_i32_ty(b, s, G, n->as<qparse::I32>());
      break;

    case QAST_NODE_I64_TY:
      out = nrgen_i64_ty(b, s, G, n->as<qparse::I64>());
      break;

    case QAST_NODE_I128_TY:
      out = nrgen_i128_ty(b, s, G, n->as<qparse::I128>());
      break;

    case QAST_NODE_F16_TY:
      out = nrgen_f16_ty(b, s, G, n->as<qparse::F16>());
      break;

    case QAST_NODE_F32_TY:
      out = nrgen_f32_ty(b, s, G, n->as<qparse::F32>());
      break;

    case QAST_NODE_F64_TY:
      out = nrgen_f64_ty(b, s, G, n->as<qparse::F64>());
      break;

    case QAST_NODE_F128_TY:
      out = nrgen_f128_ty(b, s, G, n->as<qparse::F128>());
      break;

    case QAST_NODE_VOID_TY:
      out = nrgen_void_ty(b, s, G, n->as<qparse::VoidTy>());
      break;

    case QAST_NODE_PTR_TY:
      out = nrgen_ptr_ty(b, s, G, n->as<qparse::PtrTy>());
      break;

    case QAST_NODE_OPAQUE_TY:
      out = nrgen_opaque_ty(b, s, G, n->as<qparse::OpaqueTy>());
      break;

    case QAST_NODE_STRUCT_TY:
      out = nrgen_struct_ty(b, s, G, n->as<qparse::StructTy>());
      break;

    case QAST_NODE_ARRAY_TY:
      out = nrgen_array_ty(b, s, G, n->as<qparse::ArrayTy>());
      break;

    case QAST_NODE_TUPLE_TY:
      out = nrgen_tuple_ty(b, s, G, n->as<qparse::TupleTy>());
      break;

    case QAST_NODE_FN_TY:
      out = nrgen_fn_ty(b, s, G, n->as<qparse::FuncTy>());
      break;

    case QAST_NODE_UNRES_TY:
      out = nrgen_unres_ty(b, s, G, n->as<qparse::UnresolvedType>());
      break;

    case QAST_NODE_INFER_TY:
      out = nrgen_infer_ty(b, s, G, n->as<qparse::InferType>());
      break;

    case QAST_NODE_TEMPL_TY:
      out = nrgen_templ_ty(b, s, G, n->as<qparse::TemplType>());
      break;

    case QAST_NODE_FNDECL:
      out = nrgen_fndecl(b, s, G, n->as<qparse::FnDecl>());
      break;

    case QAST_NODE_FN:
      out = nrgen_fn(b, s, G, n->as<qparse::FnDef>());
      break;

    case QAST_NODE_COMPOSITE_FIELD:
      out = nrgen_composite_field(b, s, G, n->as<qparse::CompositeField>());
      break;

    case QAST_NODE_BLOCK:
      out = nrgen_block(b, s, G, n->as<qparse::Block>());
      break;

    case QAST_NODE_CONST:
      out = nrgen_const(b, s, G, n->as<qparse::ConstDecl>());
      break;

    case QAST_NODE_VAR:
      out = nrgen_var(b, s, G, n->as<qparse::VarDecl>());
      break;

    case QAST_NODE_LET:
      out = nrgen_let(b, s, G, n->as<qparse::LetDecl>());
      break;

    case QAST_NODE_INLINE_ASM:
      out = nrgen_inline_asm(b, s, G, n->as<qparse::InlineAsm>());
      break;

    case QAST_NODE_RETURN:
      out = nrgen_return(b, s, G, n->as<qparse::ReturnStmt>());
      break;

    case QAST_NODE_RETIF:
      out = nrgen_retif(b, s, G, n->as<qparse::ReturnIfStmt>());
      break;

    case QAST_NODE_RETZ:
      out = nrgen_retz(b, s, G, n->as<qparse::RetZStmt>());
      break;

    case QAST_NODE_RETV:
      out = nrgen_retv(b, s, G, n->as<qparse::RetVStmt>());
      break;

    case QAST_NODE_BREAK:
      out = nrgen_break(b, s, G, n->as<qparse::BreakStmt>());
      break;

    case QAST_NODE_CONTINUE:
      out = nrgen_continue(b, s, G, n->as<qparse::ContinueStmt>());
      break;

    case QAST_NODE_IF:
      out = nrgen_if(b, s, G, n->as<qparse::IfStmt>());
      break;

    case QAST_NODE_WHILE:
      out = nrgen_while(b, s, G, n->as<qparse::WhileStmt>());
      break;

    case QAST_NODE_FOR:
      out = nrgen_for(b, s, G, n->as<qparse::ForStmt>());
      break;

    case QAST_NODE_FORM:
      out = nrgen_form(b, s, G, n->as<qparse::FormStmt>());
      break;

    case QAST_NODE_FOREACH:
      out = nrgen_foreach(b, s, G, n->as<qparse::ForeachStmt>());
      break;

    case QAST_NODE_CASE:
      out = nrgen_case(b, s, G, n->as<qparse::CaseStmt>());
      break;

    case QAST_NODE_SWITCH:
      out = nrgen_switch(b, s, G, n->as<qparse::SwitchStmt>());
      break;

    case QAST_NODE_EXPR_STMT:
      out = nrgen_expr_stmt(b, s, G, n->as<qparse::ExprStmt>());
      break;

    case QAST_NODE_VOLSTMT:
      out = nrgen_volstmt(b, s, G, n->as<qparse::VolStmt>());
      break;

    default: {
      break;
    }
  }

  return out;
}

static BResult nrgen_any(NRBuilder &b, PState &s, IReport *G, qparse::Node *n) {
  using namespace nr;

  if (!n) {
    return std::nullopt;
  }

  BResult out;

  switch (n->this_typeid()) {
    case QAST_NODE_TYPEDEF:
      out = nrgen_typedef(b, s, G, n->as<qparse::TypedefDecl>());
      break;

    case QAST_NODE_ENUM:
      out = nrgen_enum(b, s, G, n->as<qparse::EnumDef>());
      break;

    case QAST_NODE_STRUCT:
      out = nrgen_struct(b, s, G, n->as<qparse::StructDef>());
      break;

    case QAST_NODE_REGION:
      out = nrgen_region(b, s, G, n->as<qparse::RegionDef>());
      break;

    case QAST_NODE_GROUP:
      out = nrgen_group(b, s, G, n->as<qparse::GroupDef>());
      break;

    case QAST_NODE_UNION:
      out = nrgen_union(b, s, G, n->as<qparse::UnionDef>());
      break;

    case QAST_NODE_SUBSYSTEM:
      out = nrgen_subsystem(b, s, G, n->as<qparse::SubsystemDecl>());
      break;

    case QAST_NODE_EXPORT:
      out = nrgen_export(b, s, G, n->as<qparse::ExportDecl>());
      break;

    default: {
      auto expr = next_one(n);
      if (expr.has_value()) {
        out = {expr.value()};
      } else {
        G->report(CompilerError, IC::Error, "nr::nrgen_any() failed to convert node", n->get_pos());
        return std::nullopt;
      }
    }
  }

  return out;
}
