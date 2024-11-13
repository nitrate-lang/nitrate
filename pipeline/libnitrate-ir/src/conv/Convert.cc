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
#include <nitrate-parser/Parser.h>

#include <boost/multiprecision/detail/standalone_config.hpp>
#include <core/Config.hh>
#include <core/PassManager.hh>
#include <cstdint>
#include <cstring>
#include <nitrate-ir/Classes.hh>
#include <nitrate-ir/Format.hh>
#include <nitrate-ir/IRBuilder.hh>
#include <nitrate-ir/IRGraph.hh>
#include <nitrate-ir/Module.hh>
#include <nitrate-ir/Report.hh>
#include <stack>
#include <string>
#include <string_view>
#include <unordered_map>

#include "nitrate-parser/Node.h"

using namespace nr::diag;
using namespace nr;

struct ConvState {
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

thread_local qmodule_t *nr::current;

static std::optional<nr::Expr *> nrgen_one(NRBuilder &b, ConvState &s, qparse::Node *node);
static std::optional<std::vector<nr::Expr *>> nrgen_any(NRBuilder &b, ConvState &s,
                                                        qparse::Node *node);

LIB_EXPORT bool nr_lower(qmodule_t *mod, qparse_node_t *base, bool diagnostics) {
  qcore_assert(mod, "nr_lower: mod == nullptr");

  if (!base) {
    return false;
  }

  std::swap(nr::nr_arena.get(), mod->getNodeArena());
  nr::current = mod;
  mod->setRoot(nullptr);
  mod->enableDiagnostics(diagnostics);

  bool success = false;

  ConvState s;
  NRBuilder builder(*mod->getLexer(), mod->getTargetInfo());

  auto result = nrgen_one(builder, s, static_cast<qparse::Node *>(base));
  if (result.has_value()) {
    mod->setRoot(result.value());
  }

  success = !mod->getFailbit();

  /* Perform the required transformations and checks
     if the first translation was successful */
  if (success) {
    /* Perform the required transformations */
    success = nr::pass::PassGroupRegistry::get("ds").run(mod, [&](std::string_view name) {
      /* Track the pass name */
      mod->applyPassLabel(std::string(name));
    });
    if (success) {
      success = nr::pass::PassGroupRegistry::get("chk").run(mod, [&](std::string_view name) {
        /* Track the analysis pass name */
        mod->applyCheckLabel(std::string(name));
      });
    } else {
      report(IssueCode::CompilerError, IssueClass::Debug, "");
    }

    success = success && !mod->getFailbit();
  }

  success || report(IssueCode::CompilerError, IssueClass::Error, "failed");

  nr::current = nullptr;
  std::swap(nr::nr_arena.get(), mod->getNodeArena());

  return success;
}

///=============================================================================

static std::string_view memorize(std::string_view sv) { return nr::current->internString(sv); }
static std::string_view memorize(qparse::String sv) {
  return memorize(std::string_view(sv.data(), sv.size()));
}

static nr::Tmp *create_simple_call(NRBuilder &, ConvState &, std::string_view name,
                                   std::vector<std::pair<std::string_view, nr::Expr *>,
                                               nr::Arena<std::pair<std::string_view, nr::Expr *>>>
                                       args = {}) {
  nr::CallArgsTmpNodeCradle datapack;

  std::get<0>(datapack) = nr::create<nr::Ident>(memorize(name), nullptr);
  std::get<1>(datapack) = std::move(args);

  return create<nr::Tmp>(nr::TmpType::CALL, std::move(datapack));
  qcore_implement(__func__);
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

std::optional<nr::Expr *> nrgen_lower_binexpr(NRBuilder &, ConvState &, nr::Expr *lhs,
                                              nr::Expr *rhs, qlex_op_t op) {
#define STD_BINOP(op) nr::create<nr::BinExpr>(lhs, rhs, nr::Op::op)
#define ASSIGN_BINOP(op)                                                                         \
  nr::create<nr::BinExpr>(                                                                       \
      lhs,                                                                                       \
      nr::create<nr::BinExpr>(static_cast<nr::Expr *>(nr_clone(nullptr, lhs)), rhs, nr::Op::op), \
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

std::optional<nr::Expr *> nrgen_lower_unexpr(NRBuilder &b, ConvState &s, nr::Expr *rhs,
                                             qlex_op_t op) {
#define STD_UNOP(op) nr::create<nr::UnExpr>(rhs, nr::Op::op)

  std::optional<Expr *> R;

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
      R = create_simple_call(b, s, "std::ceil", {{"0", arg}});
      break;
    }

    case qOpAlignof: {
      R = STD_UNOP(Alignof);
      break;
    }

    case qOpTypeof: {
      auto inferred = rhs->getType();
      if (!inferred.has_value()) {
        badtree(nullptr, "qOpTypeof: rhs->getType() == nullptr");
        break;
      }

      qcore_assert(inferred, "qOpTypeof: inferred == nullptr");
      nr::SymbolEncoding se;
      auto res = se.mangle_name(inferred.value(), nr::AbiTag::Nitrate);
      if (!res) {
        badtree(nullptr, "Failed to mangle type");
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

std::optional<nr::Expr *> nrgen_lower_post_unexpr(NRBuilder &, ConvState &, nr::Expr *lhs,
                                                  qlex_op_t op) {
#define STD_POST_OP(op) nr::create<nr::PostUnExpr>(lhs, nr::Op::op)

  std::optional<Expr *> R;

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

static std::optional<Expr *> nrgen_cexpr(NRBuilder &b, ConvState &s, qparse::ConstExpr *n) {
  auto c = nrgen_one(b, s, n->get_value());
  if (!c.has_value()) {
    badtree(n, "qparse::ConstExpr::get_value() == nullptr");
    return std::nullopt;
  }

  return c;
}

static std::optional<Expr *> nrgen_binexpr(NRBuilder &b, ConvState &s, qparse::BinExpr *n) {
  auto lhs = nrgen_one(b, s, n->get_lhs());
  if (!lhs.has_value()) {
    badtree(n, "qparse::BinExpr::get_lhs() == nullptr");
    return std::nullopt;
  }

  auto rhs = nrgen_one(b, s, n->get_rhs());
  if (!rhs.has_value()) {
    badtree(n, "qparse::BinExpr::get_rhs() == nullptr");
    return std::nullopt;
  }

  return nrgen_lower_binexpr(b, s, lhs.value(), rhs.value(), n->get_op());
}

static std::optional<Expr *> nrgen_unexpr(NRBuilder &b, ConvState &s, qparse::UnaryExpr *n) {
  auto rhs = nrgen_one(b, s, n->get_rhs());
  if (!rhs.has_value()) {
    badtree(n, "qparse::UnaryExpr::get_rhs() == nullptr");
    return std::nullopt;
  }

  return nrgen_lower_unexpr(b, s, rhs.value(), n->get_op());
}

static std::optional<Expr *> nrgen_post_unexpr(NRBuilder &b, ConvState &s,
                                               qparse::PostUnaryExpr *n) {
  auto lhs = nrgen_one(b, s, n->get_lhs());
  if (!lhs.has_value()) {
    badtree(n, "qparse::PostUnaryExpr::get_lhs() == nullptr");
    return std::nullopt;
  }

  return nrgen_lower_post_unexpr(b, s, lhs.value(), n->get_op());
}

static std::optional<Expr *> nrgen_terexpr(NRBuilder &b, ConvState &s, qparse::TernaryExpr *n) {
  auto cond = nrgen_one(b, s, n->get_cond());
  if (!cond.has_value()) {
    badtree(n, "qparse::TernaryExpr::get_cond() == nullptr");
    return std::nullopt;
  }

  auto lhs = nrgen_one(b, s, n->get_lhs());
  if (!lhs.has_value()) {
    badtree(n, "qparse::TernaryExpr::get_lhs() == nullptr");
    return std::nullopt;
  }

  auto rhs = nrgen_one(b, s, n->get_rhs());
  if (!rhs.has_value()) {
    badtree(n, "qparse::TernaryExpr::get_rhs() == nullptr");
    return std::nullopt;
  }

  return create<If>(cond.value(), lhs.value(), rhs.value());
}

static std::optional<Expr *> nrgen_int(NRBuilder &b, ConvState &, qparse::ConstInt *n) {
  boost::multiprecision::cpp_int num(n->get_value());

  if (num > UINT128_MAX) {
    return std::nullopt;
  } else if (num > UINT64_MAX) {
    return b.createFixedInteger(num.convert_to<uint128_t>(), IntSize::U128);
  } else if (num > UINT32_MAX) {
    return b.createFixedInteger(num.convert_to<uint128_t>(), IntSize::U64);
  } else {
    return b.createFixedInteger(num.convert_to<uint128_t>(), IntSize::U32);
  }
}

static std::optional<Expr *> nrgen_float(NRBuilder &, ConvState &, qparse::ConstFloat *n) {
  /// FIXME: Do checking
  return create<Float>(memorize(n->get_value()));
}

static std::optional<Expr *> nrgen_string(NRBuilder &b, ConvState &, qparse::ConstString *n) {
  return b.createStringDataArray(n->get_value());
}

static std::optional<Expr *> nrgen_char(NRBuilder &b, ConvState &, qparse::ConstChar *n) {
  if (n->get_value() > UINT8_MAX) {
    badtree(n, "Character literal value is outside the expected range of UINT8_MAX");
    return std::nullopt;
  }

  return b.createFixedInteger(n->get_value(), IntSize::U8);
}

static std::optional<Expr *> nrgen_bool(NRBuilder &b, ConvState &, qparse::ConstBool *n) {
  return b.createBool(n->get_value());
}

static std::optional<Expr *> nrgen_null(NRBuilder &, ConvState &, qparse::ConstNull *) {
  return std::nullopt;

  /// TODO: This will be a special global variable for the __builtin_optional class
}

static std::optional<Expr *> nrgen_undef(NRBuilder &, ConvState &, qparse::ConstUndef *n) {
  report(IssueCode::UnexpectedUndefLiteral, IssueClass::Error, n->get_start_pos(),
         n->get_end_pos());
  return std::nullopt;
}

static std::optional<Expr *> nrgen_call(NRBuilder &b, ConvState &s, qparse::Call *n) {
  /**
   * @brief Convert a function call to a nr call.
   * @details Recursively convert the function base and the arguments
   *         of the function call. We currently do not have enough information
   *         to handle the expansion of named / optional arguments. Therefore,
   *         we wrap the data present and we will attempt to lower the construct
   *         later.
   */

  auto target = nrgen_one(b, s, n->get_func());
  if (!target) {
    badtree(n, "qparse::Call::get_func() == nullptr");
    return std::nullopt;
  }

  CallArgsTmpNodeCradle datapack;
  for (auto it = n->get_args().begin(); it != n->get_args().end(); ++it) {
    auto arg = nrgen_one(b, s, it->second);
    if (!arg) {
      badtree(n, "qparse::Call::get_args() vector contains nullptr");
      return std::nullopt;
    }

    // Implicit conversion are done later

    std::get<1>(datapack).push_back({memorize(it->first), arg.value()});
  }
  std::get<0>(datapack) = target.value();

  return create<Tmp>(TmpType::CALL, std::move(datapack));
}

static std::optional<Expr *> nrgen_list(NRBuilder &b, ConvState &s, qparse::List *n) {
  ListItems items;

  for (auto it = n->get_items().begin(); it != n->get_items().end(); ++it) {
    auto item = nrgen_one(b, s, *it);
    if (!item.has_value()) {
      badtree(n, "qparse::List::get_items() vector contains nullptr");
      return std::nullopt;
    }

    items.push_back(item.value());
  }

  return b.createList(items, false);
}

static std::optional<Expr *> nrgen_assoc(NRBuilder &b, ConvState &s, qparse::Assoc *n) {
  auto key = nrgen_one(b, s, n->get_key());
  if (!key.has_value()) {
    badtree(n, "qparse::Assoc::get_key() == nullptr");
    return std::nullopt;
  }

  auto value = nrgen_one(b, s, n->get_value());
  if (!value.has_value()) {
    badtree(n, "qparse::Assoc::get_value() == nullptr");
    return std::nullopt;
  }

  std::array<Expr *, 2> kv = {key.value(), value.value()};
  return b.createList(kv, false);
}

static std::optional<Expr *> nrgen_field(NRBuilder &b, ConvState &s, qparse::Field *n) {
  auto base = nrgen_one(b, s, n->get_base());
  if (!base.has_value()) {
    badtree(n, "qparse::Field::get_base() == nullptr");
    return std::nullopt;
  }

  Expr *field = createStringLiteral(n->get_field());
  return create<Index>(base.value(), field);
}

static std::optional<Expr *> nrgen_index(NRBuilder &b, ConvState &s, qparse::Index *n) {
  auto base = nrgen_one(b, s, n->get_base());
  if (!base.has_value()) {
    badtree(n, "qparse::Index::get_base() == nullptr");
    return std::nullopt;
  }

  auto index = nrgen_one(b, s, n->get_index());
  if (!index.has_value()) {
    badtree(n, "qparse::Index::get_index() == nullptr");
    return std::nullopt;
  }

  return create<Index>(base.value(), index.value());
}

static std::optional<Expr *> nrgen_slice(NRBuilder &b, ConvState &s, qparse::Slice *n) {
  auto base = nrgen_one(b, s, n->get_base());
  if (!base.has_value()) {
    badtree(n, "qparse::Slice::get_base() == nullptr");
    return std::nullopt;
  }

  auto start = nrgen_one(b, s, n->get_start());
  if (!start.has_value()) {
    badtree(n, "qparse::Slice::get_start() == nullptr");
    return std::nullopt;
  }

  auto end = nrgen_one(b, s, n->get_end());
  if (!end.has_value()) {
    badtree(n, "qparse::Slice::get_end() == nullptr");
    return std::nullopt;
  }

  return create<Call>(create<Index>(base.value(), createStringLiteral("slice")),
                      CallArgs({start.value(), end.value()}));
}

static std::optional<Expr *> nrgen_fstring(NRBuilder &b, ConvState &s, qparse::FString *n) {
  if (n->get_items().empty()) {
    return b.createStringDataArray("");
  }

  if (n->get_items().size() == 1) {
    auto val = n->get_items().front();

    if (std::holds_alternative<qparse::String>(val)) {
      return createStringLiteral(std::get<qparse::String>(val));
    } else if (std::holds_alternative<qparse::Expr *>(val)) {
      auto expr = nrgen_one(b, s, std::get<qparse::Expr *>(val));

      if (!expr.has_value()) {
        badtree(n, "qparse::FString::get_items() vector contains nullptr");
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
      auto expr = nrgen_one(b, s, val);

      if (!expr.has_value()) {
        badtree(n, "qparse::FString::get_items() vector contains nullptr");
        return std::nullopt;
      }

      concated = create<BinExpr>(concated, expr.value(), Op::Plus);
    } else {
      qcore_panic("Invalid fstring item type");
    }
  }

  return concated;
}

static std::optional<Expr *> nrgen_ident(NRBuilder &, ConvState &s, qparse::Ident *n) {
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

static std::optional<Expr *> nrgen_seq_point(NRBuilder &b, ConvState &s, qparse::SeqPoint *n) {
  SeqItems items;
  items.reserve(n->get_items().size());

  for (auto it = n->get_items().begin(); it != n->get_items().end(); ++it) {
    auto item = nrgen_one(b, s, *it);
    if (!item.has_value()) {
      badtree(n, "qparse::SeqPoint::get_items() vector contains nullptr");
      return std::nullopt;
    }

    items.push_back(item.value());
  }

  return create<Seq>(std::move(items));
}

static std::optional<Expr *> nrgen_stmt_expr(NRBuilder &b, ConvState &s, qparse::StmtExpr *n) {
  auto stmt = nrgen_one(b, s, n->get_stmt());
  if (!stmt.has_value()) {
    badtree(n, "qparse::StmtExpr::get_stmt() == nullptr");
    return std::nullopt;
  }

  return stmt;
}

static std::optional<Expr *> nrgen_type_expr(NRBuilder &b, ConvState &s, qparse::TypeExpr *n) {
  auto type = nrgen_one(b, s, n->get_type());
  if (!type.has_value()) {
    badtree(n, "qparse::TypeExpr::get_type() == nullptr");
    return std::nullopt;
  }

  return type;
}

static std::optional<Expr *> nrgen_templ_call(NRBuilder &, ConvState &, qparse::TemplCall *n) {
  /// TODO: templ_call

  badtree(n, "Template call not implemented");

  return std::nullopt;
}

static std::optional<Expr *> nrgen_ref_ty(NRBuilder &b, ConvState &s, qparse::RefTy *n) {
  auto pointee = nrgen_one(b, s, n->get_item());
  if (!pointee.has_value()) {
    badtree(n, "qparse::RefTy::get_item() == nullptr");
    return std::nullopt;
  }

  return b.getPtrTy(pointee.value()->asType());
}

static std::optional<Expr *> nrgen_u1_ty(NRBuilder &b, ConvState &, qparse::U1 *) {
  return b.getU1Ty();
}
static std::optional<Expr *> nrgen_u8_ty(NRBuilder &b, ConvState &, qparse::U8 *) {
  return b.getU8Ty();
}
static std::optional<Expr *> nrgen_u16_ty(NRBuilder &b, ConvState &, qparse::U16 *) {
  return b.getU16Ty();
}
static std::optional<Expr *> nrgen_u32_ty(NRBuilder &b, ConvState &, qparse::U32 *) {
  return b.getU32Ty();
}
static std::optional<Expr *> nrgen_u64_ty(NRBuilder &b, ConvState &, qparse::U64 *) {
  return b.getU64Ty();
}
static std::optional<Expr *> nrgen_u128_ty(NRBuilder &b, ConvState &, qparse::U128 *) {
  return b.getU128Ty();
}
static std::optional<Expr *> nrgen_i8_ty(NRBuilder &b, ConvState &, qparse::I8 *) {
  return b.getI8Ty();
}
static std::optional<Expr *> nrgen_i16_ty(NRBuilder &b, ConvState &, qparse::I16 *) {
  return b.getI16Ty();
}
static std::optional<Expr *> nrgen_i32_ty(NRBuilder &b, ConvState &, qparse::I32 *) {
  return b.getI32Ty();
}
static std::optional<Expr *> nrgen_i64_ty(NRBuilder &b, ConvState &, qparse::I64 *) {
  return b.getI64Ty();
}
static std::optional<Expr *> nrgen_i128_ty(NRBuilder &b, ConvState &, qparse::I128 *) {
  return b.getI128Ty();
}
static std::optional<Expr *> nrgen_f16_ty(NRBuilder &b, ConvState &, qparse::F16 *) {
  return b.getF16Ty();
}
static std::optional<Expr *> nrgen_f32_ty(NRBuilder &b, ConvState &, qparse::F32 *) {
  return b.getF32Ty();
}
static std::optional<Expr *> nrgen_f64_ty(NRBuilder &b, ConvState &, qparse::F64 *) {
  return b.getF64Ty();
}
static std::optional<Expr *> nrgen_f128_ty(NRBuilder &b, ConvState &, qparse::F128 *) {
  return b.getF128Ty();
}
static std::optional<Expr *> nrgen_void_ty(NRBuilder &b, ConvState &, qparse::VoidTy *) {
  return b.getVoidTy();
}

static std::optional<Expr *> nrgen_ptr_ty(NRBuilder &b, ConvState &s, qparse::PtrTy *n) {
  auto pointee = nrgen_one(b, s, n->get_item());
  if (!pointee.has_value()) {
    badtree(n, "qparse::PtrTy::get_item() == nullptr");
    return std::nullopt;
  }

  return b.getPtrTy(pointee.value()->asType());
}

static std::optional<Expr *> nrgen_opaque_ty(NRBuilder &b, ConvState &, qparse::OpaqueTy *n) {
  return b.getOpaqueTy(n->get_name());
}

static std::optional<Expr *> nrgen_struct_ty(NRBuilder &b, ConvState &s, qparse::StructTy *n) {
  const qparse::StructItems &fields = n->get_items();

  std::vector<Type *> the_fields;
  the_fields.resize(fields.size());

  for (size_t i = 0; i < the_fields.size(); i++) {
    auto item = nrgen_one(b, s, fields[i].second);
    if (!item.has_value()) {
      badtree(n, "qparse::StructTy::get_items() vector contains nullptr");
      return std::nullopt;
    }

    the_fields.push_back(item.value()->asType());
  }

  return b.getStructTy(the_fields);
}

static std::optional<Expr *> nrgen_group_ty(NRBuilder &b, ConvState &s, qparse::GroupTy *n) {
  StructFields fields;

  for (auto it = n->get_items().begin(); it != n->get_items().end(); ++it) {
    auto item = nrgen_one(b, s, *it);
    if (!item.has_value()) {
      badtree(n, "qparse::GroupTy::get_items() vector contains nullptr");
      return std::nullopt;
    }

    fields.push_back(item.value()->asType());
  }

  return create<StructTy>(std::move(fields));
}

static std::optional<Expr *> nrgen_region_ty(NRBuilder &b, ConvState &s, qparse::RegionTy *n) {
  StructFields fields;

  for (auto it = n->get_items().begin(); it != n->get_items().end(); ++it) {
    auto item = nrgen_one(b, s, *it);
    if (!item.has_value()) {
      badtree(n, "qparse::RegionTy::get_items() vector contains nullptr");
      return std::nullopt;
    }

    fields.push_back(item.value()->asType());
  }

  return create<StructTy>(std::move(fields));
}

static std::optional<Expr *> nrgen_union_ty(NRBuilder &b, ConvState &s, qparse::UnionTy *n) {
  UnionFields fields;

  for (auto it = n->get_items().begin(); it != n->get_items().end(); ++it) {
    auto item = nrgen_one(b, s, *it);
    if (!item.has_value()) {
      badtree(n, "qparse::UnionTy::get_items() vector contains nullptr");
      return std::nullopt;
    }

    fields.push_back(item.value()->asType());
  }

  return create<UnionTy>(std::move(fields));
}

static std::optional<Expr *> nrgen_array_ty(NRBuilder &b, ConvState &s, qparse::ArrayTy *n) {
  auto item = nrgen_one(b, s, n->get_item());
  if (!item.has_value()) {
    badtree(n, "qparse::ArrayTy::get_item() == nullptr");
    return std::nullopt;
  }

  auto count_expr = nrgen_one(b, s, n->get_size());
  if (!count_expr.has_value()) {
    badtree(n, "qparse::ArrayTy::get_size() == nullptr");
    return std::nullopt;
  }

  auto eprintn_cb = [&](std::string_view msg) {
    report(IssueCode::CompilerError, IssueClass::Error, count_expr.value()->getLoc(), msg);
  };

  auto result = nr::comptime_impl(count_expr.value(), eprintn_cb);
  if (!result.has_value()) {
    return std::nullopt;
  }

  if (result.value()->getKind() != QIR_NODE_INT) {
    badtree(n, "Non integer literal array size is not supported");
    return std::nullopt;
  }

  uint128_t size = result.value()->as<Int>()->getValue();

  if (size > UINT64_MAX) {
    badtree(n, "Array size > UINT64_MAX");
    return std::nullopt;
  }

  return b.getArrayTy(item.value()->asType(), static_cast<uint64_t>(size));
}

static std::optional<Expr *> nrgen_tuple_ty(NRBuilder &b, ConvState &s, qparse::TupleTy *n) {
  StructFields fields;
  for (auto it = n->get_items().begin(); it != n->get_items().end(); ++it) {
    auto item = nrgen_one(b, s, *it);
    if (!item.has_value()) {
      badtree(n, "qparse::TupleTy::get_items() vector contains nullptr");
      return std::nullopt;
    }

    fields.push_back(item.value()->asType());
  }

  return create<StructTy>(std::move(fields));
}

static std::optional<Expr *> nrgen_fn_ty(NRBuilder &b, ConvState &s, qparse::FuncTy *n) {
  /**
   * @brief Convert a function type to a nr function type.
   * @details Order of function parameters is consistent with their order
   * of declaration. Support for `optional` arguments is up to the frontend,
   * for inter-language compatibility, the ABI is not concerned with optional
   * arguments as they have no significance at the binary interface level.
   */

  FnParams params;

  for (auto it = n->get_params().begin(); it != n->get_params().end(); ++it) {
    auto type = nrgen_one(b, s, std::get<1>(*it));
    if (!type.has_value()) {
      badtree(n, "qparse::FnDef::get_type() == nullptr");
      return std::nullopt;
    }

    params.push_back(type.value()->asType());
  }

  auto ret = nrgen_one(b, s, n->get_return_ty());
  if (!ret.has_value()) {
    badtree(n, "qparse::FnDef::get_ret() == nullptr");
    return std::nullopt;
  }

  FnAttrs attrs;

  if (n->is_variadic()) {
    attrs.insert(FnAttr::Variadic);
  }

  return create<FnTy>(std::move(params), ret.value()->asType(), std::move(attrs));
}

static std::optional<Expr *> nrgen_unres_ty(NRBuilder &, ConvState &s, qparse::UnresolvedType *n) {
  /**
   * @brief Convert an unresolved type to a nr type.
   * @details This is a 1-to-1 conversion of the unresolved type.
   */

  auto str = s.cur_named(n->get_name());
  auto name = memorize(std::string_view(str));

  return create<Tmp>(TmpType::NAMED_TYPE, name);
}

static std::optional<Expr *> nrgen_infer_ty(NRBuilder &, ConvState &, qparse::InferType *) {
  /// TODO: infer_ty
  return std::nullopt;
}

static std::optional<Expr *> nrgen_templ_ty(NRBuilder &, ConvState &, qparse::TemplType *n) {
  /// TODO: templ_ty
  badtree(n, "template types are not supported yet");
  return std::nullopt;
}

static std::optional<std::vector<Expr *>> nrgen_typedef(NRBuilder &b, ConvState &s,
                                                        qparse::TypedefDecl *n) {
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

  auto type = nrgen_one(b, s, n->get_type());
  if (!type) {
    badtree(n, "qparse::TypedefDecl::get_type() == nullptr");
    return std::nullopt;
  }

  current->getTypeMap()[name] = type.value()->asType();

  return std::vector<Expr *>();
}

static std::optional<Expr *> nrgen_fndecl(NRBuilder &b, ConvState &s, qparse::FnDecl *n) {
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

      auto type = nrgen_one(b, s, std::get<1>(*it));
      if (!type.has_value()) {
        badtree(n, "qparse::FnDecl::get_type() == nullptr");
        return std::nullopt;
      }

      std::optional<Expr *> def;
      if (std::get<2>(*it)) {
        def = nrgen_one(b, s, std::get<2>(*it));
        if (!def.has_value()) {
          badtree(n, "qparse::FnDecl::get_type() == nullptr");
          return std::nullopt;
        }
      }

      std::string_view sv = memorize(std::string_view(std::get<0>(*it)));

      params.push_back({type.value()->asType(), sv});
      current->getParameterMap()[str].push_back(
          {std::string(std::get<0>(*it)), type.value()->asType(), def.value()});
    }
  }

  auto fnty = nrgen_one(b, s, fty);
  if (!fnty.value()) {
    badtree(n, "qparse::FnDecl::get_type() == nullptr");
    return std::nullopt;
  }

  Fn *fn = create<Fn>(str, std::move(params), fnty.value()->as<FnTy>()->getReturn(), createIgn(),
                      fty->is_variadic(), s.abi_mode);

  current->getFunctions().insert({str, {fnty.value()->as<FnTy>(), fn}});

  return fn;
}

#define align(x, a) (((x) + (a) - 1) & ~((a) - 1))

static std::optional<std::vector<Expr *>> nrgen_struct(NRBuilder &b, ConvState &s,
                                                       qparse::StructDef *n) {
  std::string name = s.cur_named(n->get_name());
  auto sv = memorize(std::string_view(name));

  if (current->getTypeMap().contains(sv)) {
    report(IssueCode::TypeRedefinition, IssueClass::Error, n->get_name(), n->get_start_pos(),
           n->get_end_pos());
  }

  StructFields fields;
  std::optional<std::vector<Expr *>> items;
  size_t offset = 0;

  for (auto it = n->get_fields().begin(); it != n->get_fields().end(); ++it) {
    if (!*it) {
      badtree(n, "qparse::StructDef::get_fields() vector contains nullptr");
      return std::nullopt;
    }

    s.composite_expanse.push((*it)->get_name());
    auto field = nrgen_one(b, s, *it);
    s.composite_expanse.pop();
    if (!field.has_value()) {
      badtree(n, "qparse::StructDef::get_fields() vector contains issue");
      return std::nullopt;
    }

    size_t field_align = field.value()->asType()->getAlignBytes();
    size_t padding = align(offset, field_align) - offset;
    if (padding > 0) {
      fields.push_back(create<ArrayTy>(create<U8Ty>(), padding));
    }

    fields.push_back(field.value()->asType());
    offset += field.value()->asType()->getSizeBytes();
  }

  StructTy *st = create<StructTy>(std::move(fields));

  current->getTypeMap()[sv] = st;

  items = std::vector<Expr *>();
  for (auto it = n->get_methods().begin(); it != n->get_methods().end(); ++it) {
    qparse::FnDecl *cur_meth = *it;
    auto old_name = cur_meth->get_name();
    cur_meth->set_name(ns_join(n->get_name(), old_name));

    auto method = nrgen_one(b, s, *it);

    cur_meth->set_name(old_name);

    if (!method) {
      badtree(n, "qparse::StructDef::get_methods() vector contains nullptr");
      return std::nullopt;
    }

    items->push_back(method.value());
  }

  for (auto it = n->get_static_methods().begin(); it != n->get_static_methods().end(); ++it) {
    qparse::FnDecl *cur_meth = *it;
    auto old_name = cur_meth->get_name();
    cur_meth->set_name(ns_join(n->get_name(), old_name));

    auto method = nrgen_one(b, s, *it);

    cur_meth->set_name(old_name);

    if (!method) {
      badtree(n, "qparse::StructDef::get_static_methods() vector contains nullptr");
      return std::nullopt;
    }

    items->push_back(method.value());
  }

  return items;
}

static std::optional<std::vector<Expr *>> nrgen_region(NRBuilder &b, ConvState &s,
                                                       qparse::RegionDef *n) {
  /**
   * @brief Convert a region definition to a nr sequence.
   * @details This is a 1-to-1 conversion of the region definition.
   */

  std::string name = s.cur_named(n->get_name());
  auto sv = memorize(std::string_view(name));

  if (current->getTypeMap().contains(sv)) {
    report(IssueCode::TypeRedefinition, IssueClass::Error, n->get_name(), n->get_start_pos(),
           n->get_end_pos());
  }

  StructFields fields;
  std::optional<std::vector<Expr *>> items;

  for (auto it = n->get_fields().begin(); it != n->get_fields().end(); ++it) {
    if (!*it) {
      badtree(n, "qparse::RegionDef::get_fields() vector contains nullptr");
      return std::nullopt;
    }

    s.composite_expanse.push((*it)->get_name());
    auto field = nrgen_one(b, s, *it);
    s.composite_expanse.pop();

    fields.push_back(field.value()->asType());
  }

  StructTy *st = create<StructTy>(std::move(fields));

  current->getTypeMap()[sv] = st;

  items = std::vector<Expr *>();
  for (auto it = n->get_methods().begin(); it != n->get_methods().end(); ++it) {
    qparse::FnDecl *cur_meth = *it;
    auto old_name = cur_meth->get_name();
    cur_meth->set_name(ns_join(n->get_name(), old_name));

    auto method = nrgen_one(b, s, *it);

    cur_meth->set_name(old_name);

    if (!method) {
      badtree(n, "qparse::RegionDef::get_methods() vector contains nullptr");
      return std::nullopt;
    }

    items->push_back(method.value());
  }

  for (auto it = n->get_static_methods().begin(); it != n->get_static_methods().end(); ++it) {
    qparse::FnDecl *cur_meth = *it;
    auto old_name = cur_meth->get_name();
    cur_meth->set_name(ns_join(n->get_name(), old_name));

    auto method = nrgen_one(b, s, *it);

    cur_meth->set_name(old_name);

    if (!method) {
      badtree(n, "qparse::RegionDef::get_static_methods() vector contains nullptr");
      return std::nullopt;
    }

    items->push_back(method.value());
  }

  return items;
}

static std::optional<std::vector<Expr *>> nrgen_group(NRBuilder &b, ConvState &s,
                                                      qparse::GroupDef *n) {
  /**
   * @brief Convert a group definition to a nr sequence.
   * @details This is a 1-to-1 conversion of the group definition.
   */

  std::string name = s.cur_named(n->get_name());
  auto sv = memorize(std::string_view(name));

  if (current->getTypeMap().contains(sv)) {
    report(IssueCode::TypeRedefinition, IssueClass::Error, n->get_name(), n->get_start_pos(),
           n->get_end_pos());
  }

  StructFields fields;
  std::optional<std::vector<Expr *>> items;

  { /* Optimize layout with field sorting */
    std::vector<Type *> tmp_fields;
    for (auto it = n->get_fields().begin(); it != n->get_fields().end(); ++it) {
      if (!*it) {
        badtree(n, "qparse::GroupDef::get_fields() vector contains nullptr");
        return std::nullopt;
      }

      s.composite_expanse.push((*it)->get_name());
      auto field = nrgen_one(b, s, *it);
      s.composite_expanse.pop();

      tmp_fields.push_back(field.value()->asType());
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

  items = std::vector<Expr *>();
  for (auto it = n->get_methods().begin(); it != n->get_methods().end(); ++it) {
    qparse::FnDecl *cur_meth = *it;
    auto old_name = cur_meth->get_name();
    cur_meth->set_name(ns_join(n->get_name(), old_name));

    auto method = nrgen_one(b, s, *it);

    cur_meth->set_name(old_name);

    if (!method) {
      badtree(n, "qparse::GroupDef::get_methods() vector contains nullptr");
      return std::nullopt;
    }

    items->push_back(method.value());
  }

  for (auto it = n->get_static_methods().begin(); it != n->get_static_methods().end(); ++it) {
    qparse::FnDecl *cur_meth = *it;
    auto old_name = cur_meth->get_name();
    cur_meth->set_name(ns_join(n->get_name(), old_name));

    auto method = nrgen_one(b, s, *it);

    cur_meth->set_name(old_name);

    if (!method) {
      badtree(n, "qparse::GroupDef::get_static_methods() vector contains nullptr");
      return std::nullopt;
    }

    items->push_back(method.value());
  }

  return items;
}

static std::optional<std::vector<Expr *>> nrgen_union(NRBuilder &b, ConvState &s,
                                                      qparse::UnionDef *n) {
  /**
   * @brief Convert a union definition to a nr sequence.
   * @details This is a 1-to-1 conversion of the union definition.
   */

  std::string name = s.cur_named(n->get_name());
  auto sv = memorize(std::string_view(name));

  if (current->getTypeMap().contains(sv)) {
    report(IssueCode::TypeRedefinition, IssueClass::Error, n->get_name(), n->get_start_pos(),
           n->get_end_pos());
  }

  UnionFields fields;
  std::optional<std::vector<Expr *>> items;

  for (auto it = n->get_fields().begin(); it != n->get_fields().end(); ++it) {
    if (!*it) {
      badtree(n, "qparse::UnionDef::get_fields() vector contains nullptr");
      return std::nullopt;
    }

    s.composite_expanse.push((*it)->get_name());
    auto field = nrgen_one(b, s, *it);
    s.composite_expanse.pop();

    fields.push_back(field.value()->asType());
  }

  UnionTy *st = create<UnionTy>(std::move(fields));

  current->getTypeMap()[sv] = st;

  items = std::vector<Expr *>();
  for (auto it = n->get_methods().begin(); it != n->get_methods().end(); ++it) {
    qparse::FnDecl *cur_meth = *it;
    auto old_name = cur_meth->get_name();
    cur_meth->set_name(ns_join(n->get_name(), old_name));

    auto method = nrgen_one(b, s, *it);

    cur_meth->set_name(old_name);

    if (!method) {
      badtree(n, "qparse::UnionDef::get_methods() vector contains nullptr");
      return std::nullopt;
    }

    items->push_back(method.value());
  }

  for (auto it = n->get_static_methods().begin(); it != n->get_static_methods().end(); ++it) {
    qparse::FnDecl *cur_meth = *it;
    auto old_name = cur_meth->get_name();
    cur_meth->set_name(ns_join(n->get_name(), old_name));

    auto method = nrgen_one(b, s, *it);

    cur_meth->set_name(old_name);

    if (!method) {
      badtree(n, "qparse::UnionDef::get_static_methods() vector contains nullptr");
      return std::nullopt;
    }

    items->push_back(method.value());
  }

  return items;
}

static std::optional<std::vector<Expr *>> nrgen_enum(NRBuilder &b, ConvState &s,
                                                     qparse::EnumDef *n) {
  std::string name = s.cur_named(n->get_name());
  auto sv = memorize(std::string_view(name));

  if (current->getTypeMap().contains(sv)) {
    report(IssueCode::TypeRedefinition, IssueClass::Error, n->get_name(), n->get_start_pos(),
           n->get_end_pos());
  }

  std::optional<Expr *> type = nullptr;
  if (n->get_type()) {
    type = nrgen_one(b, s, n->get_type());
    if (!type.has_value()) {
      badtree(n, "qparse::EnumDef::get_type() == nullptr");
      return std::nullopt;
    }
  } else {
    type = create<Tmp>(TmpType::ENUM, sv)->asType();
  }

  current->getTypeMap()[sv] = type.value()->asType();

  Expr *last = nullptr;

  for (auto it = n->get_items().begin(); it != n->get_items().end(); ++it) {
    std::optional<Expr *> cur = nullptr;

    if (it->second) {
      cur = nrgen_one(b, s, it->second);
      if (!cur.has_value()) {
        badtree(n, "qparse::EnumDef::get_items() vector contains nullptr");
        return std::nullopt;
      }

      last = cur.value();
    } else {
      if (!last) {
        cur = create<Int>(0, IntSize::U32);
        last = cur.value();
      } else {
        cur = create<BinExpr>(last, create<Int>(1, IntSize::U32), Op::Plus);
        last = cur.value();
      }
    }

    std::string_view field_name = memorize(std::string_view(name + "::" + std::string(it->first)));

    current->getNamedConstants().insert({field_name, cur.value()});
  }

  return {};
}

static std::optional<Expr *> nrgen_fn(NRBuilder &b, ConvState &s, qparse::FnDef *n) {
  bool old_inside_function = s.inside_function;
  s.inside_function = true;

  std::optional<Expr *> precond, postcond;
  Seq *body = nullptr;
  Params params;
  qparse::FnDecl *decl = n;
  qparse::FuncTy *fty = decl->get_type();

  auto fnty = nrgen_one(b, s, fty);
  if (!fnty.has_value()) {
    badtree(n, "qparse::FnDef::get_type() == nullptr");
    return std::nullopt;
  }

  /* Produce the function preconditions */
  if ((precond = nrgen_one(b, s, n->get_precond()))) {
    precond = create<If>(create<UnExpr>(precond.value(), Op::LogicNot),
                         create_simple_call(b, s, "__detail::precond_fail"), createIgn());
  }

  /* Produce the function postconditions */
  if ((postcond = nrgen_one(b, s, n->get_postcond()))) {
    postcond = create<If>(create<UnExpr>(postcond.value(), Op::LogicNot),
                          create_simple_call(b, s, "__detail::postcond_fail"), createIgn());
  }

  { /* Produce the function body */
    Type *old_return_ty = s.return_type;
    s.return_type = fnty.value()->as<FnTy>()->getReturn();
    s.local_scope.push({});

    auto tmp = nrgen_one(b, s, n->get_body());
    if (!tmp.has_value()) {
      badtree(n, "qparse::FnDef::get_body() == nullptr");
      return std::nullopt;
    }

    if (tmp.value()->getKind() != QIR_NODE_SEQ) {
      tmp = create<Seq>(SeqItems({tmp.value()}));
    }

    Seq *seq = tmp.value()->as<Seq>();

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
      body->getItems().insert(body->getItems().begin(), precond.value());
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

      auto type = nrgen_one(b, s, std::get<1>(*it));
      if (!type.has_value()) {
        badtree(n, "qparse::FnDef::get_type() == nullptr");
        return std::nullopt;
      }

      std::optional<Expr *> def = nullptr;
      if (std::get<2>(*it)) {
        def = nrgen_one(b, s, std::get<2>(*it));
        if (!def.has_value()) {
          badtree(n, "qparse::FnDef::get_type() == nullptr");
          return std::nullopt;
        }
      }

      std::string_view sv = memorize(std::string_view(std::get<0>(*it)));

      params.push_back({type.value()->asType(), sv});
      current->getParameterMap()[str].push_back(
          {std::string(std::get<0>(*it)), type.value()->asType(), def.value()});
    }
  }

  auto obj = create<Fn>(str, std::move(params), fnty.value()->as<FnTy>()->getReturn(), body,
                        fty->is_variadic(), s.abi_mode);

  current->getFunctions().insert({str, {fnty.value()->as<FnTy>(), obj}});

  s.inside_function = old_inside_function;
  return obj;
}

static std::optional<std::vector<Expr *>> nrgen_subsystem(NRBuilder &b, ConvState &s,
                                                          qparse::SubsystemDecl *n) {
  /**
   * @brief Convert a subsystem declaration to a nr sequence with
   * namespace prefixes.
   */

  std::optional<std::vector<Expr *>> items;

  if (!n->get_body()) {
    badtree(n, "qparse::SubsystemDecl::get_body() == nullptr");
    return std::nullopt;
  }

  std::string old_ns = s.ns_prefix;

  if (s.ns_prefix.empty()) {
    s.ns_prefix = std::string(n->get_name());
  } else {
    s.ns_prefix += "::" + std::string(n->get_name());
  }

  for (auto it = n->get_body()->get_items().begin(); it != n->get_body()->get_items().end(); ++it) {
    auto item = nrgen_any(b, s, *it);

    items.insert(items.end(), item.begin(), item.end());
  }

  s.ns_prefix = old_ns;

  return items;
}

static std::optional<std::vector<Expr *>> nrgen_export(NRBuilder &b, ConvState &s,
                                                       qparse::ExportDecl *n) {
  /**
   * @brief Convert an export declaration to a nr export node.
   * @details Convert a list of statements under a common ABI into a
   * sequence under a common ABI.
   */

  AbiTag old = s.abi_mode;

  if (n->get_abi_name().empty()) {
    s.abi_mode = AbiTag::Default;
  } else if (n->get_abi_name() == "q") {
    s.abi_mode = AbiTag::Nitrate;
  } else if (n->get_abi_name() == "c") {
    s.abi_mode = AbiTag::C;
  } else {
    badtree(n, "qparse::ExportDecl abi name is not supported: '" + n->get_abi_name() + "'");
    return std::nullopt;
  }

  if (!n->get_body()) {
    badtree(n, "qparse::ExportDecl::get_body() == nullptr");
    return std::nullopt;
  }

  std::string_view abi_name;

  if (n->get_abi_name().empty()) {
    abi_name = "std";
  } else {
    abi_name = memorize(n->get_abi_name());
  }

  std::vector<nr::Expr *> items;

  for (auto it = n->get_body()->get_items().begin(); it != n->get_body()->get_items().end(); ++it) {
    auto result = nrgen_any(b, s, *it);
    for (auto &item : result) {
      items.push_back(create<Extern>(item, abi_name));
    }
  }

  s.abi_mode = old;

  return items;
}

static std::optional<Expr *> nrgen_composite_field(NRBuilder &b, ConvState &s,
                                                   qparse::CompositeField *n) {
  auto type = nrgen_one(b, s, n->get_type());
  if (!type) {
    badtree(n, "qparse::CompositeField::get_type() == nullptr");
    return std::nullopt;
  }

  Expr *_def = nullptr;
  if (n->get_value()) {
    _def = nrgen_one(b, s, n->get_value());
    if (!_def) {
      badtree(n, "qparse::CompositeField::get_value() == nullptr");
      return std::nullopt;
    }
  }

  if (s.composite_expanse.empty()) {
    badtree(n, "state.composite_expanse.empty()");
    return std::nullopt;
  }

  std::string_view dt_name = memorize(s.composite_expanse.top());

  current->getCompositeFields()[dt_name].push_back(
      {std::string(n->get_name()), type->asType(), _def});

  return type;
}

static std::optional<Expr *> nrgen_block(NRBuilder &b, ConvState &s, qparse::Block *n) {
  /**
   * @brief Convert a scope block into an expression sequence.
   * @details A QXIR sequence is a list of expressions (a sequence point).
   *          This is equivalent to a scope block.
   */

  SeqItems items;
  items.reserve(n->get_items().size());

  s.local_scope.push({});

  for (auto it = n->get_items().begin(); it != n->get_items().end(); ++it) {
    auto item = nrgen_any(b, s, *it);

    items.insert(items.end(), item.begin(), item.end());
  }

  s.local_scope.pop();

  return create<Seq>(std::move(items));
}

static std::optional<Expr *> nrgen_const(NRBuilder &b, ConvState &s, qparse::ConstDecl *n) {
  Expr *init = nrgen_one(b, s, n->get_value());
  Type *type = nullptr;
  if (n->get_type()) {
    Expr *tmp = nrgen_one(b, s, n->get_type());
    if (tmp) {
      type = tmp->asType();
    }
  }

  if (init && type) {
    init = create<BinExpr>(init, type, Op::CastAs);
  } else if (!init && type) {
    init = type;
  } else if (!init && !type) {
    badtree(n,
            "parse::ConstDecl::get_value() == nullptr && parse::ConstDecl::get_type() == nullptr");
    return std::nullopt;
  }

  if (s.inside_function) {
    std::string_view name = memorize(n->get_name());
    Local *local = create<Local>(name, init, s.abi_mode);

    qcore_assert(!s.local_scope.empty());
    if (s.local_scope.top().contains(name)) {
      report(IssueCode::VariableRedefinition, IssueClass::Error, n->get_name(), n->get_start_pos(),
             n->get_end_pos());
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

static std::optional<Expr *> nrgen_var(NRBuilder &, ConvState &, qparse::VarDecl *) {
  /// TODO: var

  return std::nullopt;
}

static std::optional<Expr *> nrgen_let(NRBuilder &b, ConvState &s, qparse::LetDecl *n) {
  Expr *init = nrgen_one(b, s, n->get_value());
  Type *type = nullptr;
  if (n->get_type()) {
    Expr *tmp = nrgen_one(b, s, n->get_type());
    if (tmp) {
      type = tmp->asType();
    }
  }

  if (init && type) {
    init = create<BinExpr>(init, type, Op::CastAs);
  } else if (!init && type) {
    init = type;
  } else if (!init && !type) {
    badtree(n,
            "parse::ConstDecl::get_value() == nullptr && parse::ConstDecl::get_type() == nullptr");
    return std::nullopt;
  }

  if (s.inside_function) {
    std::string_view name = memorize(n->get_name());
    Local *local = create<Local>(name, init, s.abi_mode);

    qcore_assert(!s.local_scope.empty());
    if (s.local_scope.top().contains(name)) {
      report(IssueCode::VariableRedefinition, IssueClass::Error, n->get_name(), n->get_start_pos(),
             n->get_end_pos());
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

static std::optional<Expr *> nrgen_inline_asm(NRBuilder &, ConvState &, qparse::InlineAsm *) {
  qcore_implement("nrgen_inline_asm");
}

static std::optional<Expr *> nrgen_return(NRBuilder &b, ConvState &s, qparse::ReturnStmt *n) {
  /**
   * @brief Convert a return statement to a nr expression.
   * @details This is a 1-to-1 conversion of the return statement.
   */

  auto val = nrgen_one(b, s, n->get_value());
  if (!val) {
    val = create<VoidTy>();
  }
  val = create<BinExpr>(val, s.return_type, Op::CastAs);

  return create<Ret>(val);
}

static std::optional<Expr *> nrgen_retif(NRBuilder &b, ConvState &s, qparse::ReturnIfStmt *n) {
  /**
   * @brief Convert a return statement to a nr expression.
   * @details Lower into an 'if (cond) {return val}' expression.
   */

  auto cond = nrgen_one(b, s, n->get_cond());
  if (!cond) {
    badtree(n, "qparse::ReturnIfStmt::get_cond() == nullptr");
    return std::nullopt;
  }

  cond = create<BinExpr>(cond, create<U1Ty>(), Op::CastAs);

  auto val = nrgen_one(b, s, n->get_value());
  if (!val) {
    badtree(n, "qparse::ReturnIfStmt::get_value() == nullptr");
    return std::nullopt;
  }
  val = create<BinExpr>(val, s.return_type, Op::CastAs);

  return create<If>(cond, create<Ret>(val), createIgn());
}

static std::optional<Expr *> nrgen_retz(NRBuilder &b, ConvState &s, qparse::RetZStmt *n) {
  /**
   * @brief Convert a return statement to a nr expression.
   * @details Lower into an 'if (!cond) {return val}' expression.
   */

  auto cond = nrgen_one(b, s, n->get_cond());
  if (!cond) {
    badtree(n, "qparse::RetZStmt::get_cond() == nullptr");
    return std::nullopt;
  }
  cond = create<BinExpr>(cond, create<U1Ty>(), Op::CastAs);

  auto inv_cond = create<UnExpr>(cond, Op::LogicNot);

  auto val = nrgen_one(b, s, n->get_value());
  if (!val) {
    badtree(n, "qparse::RetZStmt::get_value() == nullptr");
    return std::nullopt;
  }
  val = create<BinExpr>(val, s.return_type, Op::CastAs);

  return create<If>(inv_cond, create<Ret>(val), createIgn());
}

static std::optional<Expr *> nrgen_retv(NRBuilder &b, ConvState &s, qparse::RetVStmt *n) {
  /**
   * @brief Convert a return statement to a nr expression.
   * @details Lower into an 'if (cond) {return void}' expression.
   */

  auto cond = nrgen_one(b, s, n->get_cond());
  if (!cond) {
    badtree(n, "qparse::RetVStmt::get_cond() == nullptr");
    return std::nullopt;
  }
  cond = create<BinExpr>(cond, create<U1Ty>(), Op::CastAs);

  return create<If>(cond, create<Ret>(createIgn()), createIgn());
}

static std::optional<Expr *> nrgen_break(NRBuilder &, ConvState &, qparse::BreakStmt *) {
  /**
   * @brief Convert a break statement to a nr expression.
   * @details This is a 1-to-1 conversion of the break statement.
   */

  return create<Brk>();
}

static std::optional<Expr *> nrgen_continue(NRBuilder &, ConvState &, qparse::ContinueStmt *) {
  /**
   * @brief Convert a continue statement to a nr expression.
   * @details This is a 1-to-1 conversion of the continue statement.
   */

  return create<Cont>();
}

static std::optional<Expr *> nrgen_if(NRBuilder &b, ConvState &s, qparse::IfStmt *n) {
  /**
   * @brief Convert an if statement to a nr expression.
   * @details The else branch is optional, and if it is missing, it is
   *        replaced with a void expression.
   */

  auto cond = nrgen_one(b, s, n->get_cond());
  auto then = nrgen_one(b, s, n->get_then());
  auto els = nrgen_one(b, s, n->get_else());

  if (!cond) {
    badtree(n, "qparse::IfStmt::get_cond() == nullptr");
    return std::nullopt;
  }
  cond = create<BinExpr>(cond, create<U1Ty>(), Op::CastAs);

  if (!then) {
    badtree(n, "qparse::IfStmt::get_then() == nullptr");
    return std::nullopt;
  }

  if (!els) {
    els = createIgn();
  }

  return create<If>(cond, then, els);
}

static std::optional<Expr *> nrgen_while(NRBuilder &b, ConvState &s, qparse::WhileStmt *n) {
  /**
   * @brief Convert a while loop to a nr expression.
   * @details If any of the sub-expressions are missing, they are replaced
   *         with a default value of 1.
   */

  auto cond = nrgen_one(b, s, n->get_cond());
  auto body = nrgen_one(b, s, n->get_body());

  if (!cond) {
    cond = create<Int>(1, IntSize::U1);
  }

  cond = create<BinExpr>(cond, create<U1Ty>(), Op::CastAs);

  if (!body) {
    body = create<Seq>(SeqItems({}));
  } else if (body->getKind() != QIR_NODE_SEQ) {
    body = create<Seq>(SeqItems({body}));
  }

  return create<While>(cond, body->as<Seq>());
}

static std::optional<Expr *> nrgen_for(NRBuilder &b, ConvState &s, qparse::ForStmt *n) {
  /**
   * @brief Convert a for loop to a nr expression.
   * @details If any of the sub-expressions are missing, they are replaced
   *         with a default value of 1.
   */

  auto init = nrgen_one(b, s, n->get_init());
  auto cond = nrgen_one(b, s, n->get_cond());
  auto step = nrgen_one(b, s, n->get_step());
  auto body = nrgen_one(b, s, n->get_body());

  if (!init) {
    init = create<Int>(1, IntSize::U32);
  }

  if (!cond) {
    cond = create<Int>(1, IntSize::U32);  // infinite loop like 'for (;;) {}'
    cond = create<BinExpr>(cond, create<U1Ty>(), Op::CastAs);
  }

  if (!step) {
    step = create<Int>(1, IntSize::U32);
  }

  if (!body) {
    body = create<Int>(1, IntSize::U32);
  }

  return create<For>(init, cond, step, body);
}

static std::optional<Expr *> nrgen_form(NRBuilder &b, ConvState &s, qparse::FormStmt *n) {
  /**
   * @brief Convert a form loop to a nr expression.
   * @details This is a 1-to-1 conversion of the form loop.
   */

  auto maxjobs = nrgen_one(b, s, n->get_maxjobs());
  if (!maxjobs) {
    badtree(n, "qparse::FormStmt::get_maxjobs() == nullptr");
    return std::nullopt;
  }

  auto idx_name = memorize(n->get_idx_ident());
  auto val_name = memorize(n->get_val_ident());

  auto iter = nrgen_one(b, s, n->get_expr());
  if (!iter) {
    badtree(n, "qparse::FormStmt::get_expr() == nullptr");
    return std::nullopt;
  }

  auto body = nrgen_one(b, s, n->get_body());
  if (!body) {
    badtree(n, "qparse::FormStmt::get_body() == nullptr");
    return std::nullopt;
  }

  return create<Form>(idx_name, val_name, maxjobs, iter, create<Seq>(SeqItems({body})));
}

static std::optional<Expr *> nrgen_foreach(NRBuilder &, ConvState &, qparse::ForeachStmt *) {
  /**
   * @brief Convert a foreach loop to a nr expression.
   * @details This is a 1-to-1 conversion of the foreach loop.
   */

  // auto idx_name = memorize(n->get_idx_ident());
  // auto val_name = memorize(n->get_val_ident());

  // auto iter = nrgen_one(b, s, n->get_expr());
  // if (!iter) {
  //   badtree(n, "qparse::ForeachStmt::get_expr() == nullptr");
  //   return std::nullopt;
  // }

  // auto body = nrgen_one(b, s, n->get_body());
  // if (!body) {
  //   badtree(n, "qparse::ForeachStmt::get_body() == nullptr");
  //   return std::nullopt;
  // }

  // return create<Foreach>(idx_name, val_name, iter, create<Seq>(SeqItems({body})));
  qcore_implement(__func__);
}

static std::optional<Expr *> nrgen_case(NRBuilder &b, ConvState &s, qparse::CaseStmt *n) {
  /**
   * @brief Convert a case statement to a nr expression.
   * @details This is a 1-to-1 conversion of the case statement.
   */

  auto cond = nrgen_one(b, s, n->get_cond());
  if (!cond) {
    badtree(n, "qparse::CaseStmt::get_cond() == nullptr");
    return std::nullopt;
  }

  auto body = nrgen_one(b, s, n->get_body());
  if (!body) {
    badtree(n, "qparse::CaseStmt::get_body() == nullptr");
    return std::nullopt;
  }

  return create<Case>(cond, body);
}

static std::optional<Expr *> nrgen_switch(NRBuilder &b, ConvState &s, qparse::SwitchStmt *n) {
  /**
   * @brief Convert a switch statement to a nr expression.
   * @details If the default case is missing, it is replaced with a void
   *        expression.
   */

  auto cond = nrgen_one(b, s, n->get_cond());
  if (!cond) {
    badtree(n, "qparse::SwitchStmt::get_cond() == nullptr");
    return std::nullopt;
  }

  SwitchCases cases;
  for (auto it = n->get_cases().begin(); it != n->get_cases().end(); ++it) {
    auto item = nrgen_one(b, s, *it);
    if (!item) {
      badtree(n, "qparse::SwitchStmt::get_cases() vector contains nullptr");
      return std::nullopt;
    }

    cases.push_back(item->as<Case>());
  }

  Expr *def = nullptr;
  if (n->get_default()) {
    def = nrgen_one(b, s, n->get_default());
    if (!def) {
      badtree(n, "qparse::SwitchStmt::get_default() == nullptr");
      return std::nullopt;
    }
  } else {
    def = createIgn();
  }

  return create<Switch>(cond, std::move(cases), def);
}

static std::optional<Expr *> nrgen_expr_stmt(NRBuilder &b, ConvState &s, qparse::ExprStmt *n) {
  /**
   * @brief Convert an expression inside a statement to a nr expression.
   * @details This is a 1-to-1 conversion of the expression statement.
   */

  return nrgen_one(b, s, n->get_expr());
}

static std::optional<Expr *> nrgen_volstmt(NRBuilder &, ConvState &, qparse::VolStmt *) {
  /**
   * @brief Convert a volatile statement to a nr volatile expression.
   * @details This is a 1-to-1 conversion of the volatile statement.
   */

  // auto expr = nrgen_one(b, s, n->get_stmt());
  // expr->setVolatile(true);

  // return expr;

  qcore_implement(__func__);
}

static std::optional<nr::Expr *> nrgen_one(NRBuilder &b, ConvState &s, qparse::Node *n) {
  using namespace nr;

  if (!n) {
    return nullptr;
  }

  nr::Expr *out = nullptr;

  switch (n->this_typeid()) {
    case QAST_NODE_CEXPR:
      out = nrgen_cexpr(b, s, n->as<qparse::ConstExpr>());
      break;

    case QAST_NODE_BINEXPR:
      out = nrgen_binexpr(b, s, n->as<qparse::BinExpr>());
      break;

    case QAST_NODE_UNEXPR:
      out = nrgen_unexpr(b, s, n->as<qparse::UnaryExpr>());
      break;

    case QAST_NODE_TEREXPR:
      out = nrgen_terexpr(b, s, n->as<qparse::TernaryExpr>());
      break;

    case QAST_NODE_INT:
      out = nrgen_int(b, s, n->as<qparse::ConstInt>());
      break;

    case QAST_NODE_FLOAT:
      out = nrgen_float(b, s, n->as<qparse::ConstFloat>());
      break;

    case QAST_NODE_STRING:
      out = nrgen_string(b, s, n->as<qparse::ConstString>());
      break;

    case QAST_NODE_CHAR:
      out = nrgen_char(b, s, n->as<qparse::ConstChar>());
      break;

    case QAST_NODE_BOOL:
      out = nrgen_bool(b, s, n->as<qparse::ConstBool>());
      break;

    case QAST_NODE_NULL:
      out = nrgen_null(b, s, n->as<qparse::ConstNull>());
      break;

    case QAST_NODE_UNDEF:
      out = nrgen_undef(b, s, n->as<qparse::ConstUndef>());
      break;

    case QAST_NODE_CALL:
      out = nrgen_call(b, s, n->as<qparse::Call>());
      break;

    case QAST_NODE_LIST:
      out = nrgen_list(b, s, n->as<qparse::List>());
      break;

    case QAST_NODE_ASSOC:
      out = nrgen_assoc(b, s, n->as<qparse::Assoc>());
      break;

    case QAST_NODE_FIELD:
      out = nrgen_field(b, s, n->as<qparse::Field>());
      break;

    case QAST_NODE_INDEX:
      out = nrgen_index(b, s, n->as<qparse::Index>());
      break;

    case QAST_NODE_SLICE:
      out = nrgen_slice(b, s, n->as<qparse::Slice>());
      break;

    case QAST_NODE_FSTRING:
      out = nrgen_fstring(b, s, n->as<qparse::FString>());
      break;

    case QAST_NODE_IDENT:
      out = nrgen_ident(b, s, n->as<qparse::Ident>());
      break;

    case QAST_NODE_SEQ_POINT:
      out = nrgen_seq_point(b, s, n->as<qparse::SeqPoint>());
      break;

    case QAST_NODE_POST_UNEXPR:
      out = nrgen_post_unexpr(b, s, n->as<qparse::PostUnaryExpr>());
      break;

    case QAST_NODE_STMT_EXPR:
      out = nrgen_stmt_expr(b, s, n->as<qparse::StmtExpr>());
      break;

    case QAST_NODE_TYPE_EXPR:
      out = nrgen_type_expr(b, s, n->as<qparse::TypeExpr>());
      break;

    case QAST_NODE_TEMPL_CALL:
      out = nrgen_templ_call(b, s, n->as<qparse::TemplCall>());
      break;

    case QAST_NODE_REF_TY:
      out = nrgen_ref_ty(b, s, n->as<qparse::RefTy>());
      break;

    case QAST_NODE_U1_TY:
      out = nrgen_u1_ty(b, s, n->as<qparse::U1>());
      break;

    case QAST_NODE_U8_TY:
      out = nrgen_u8_ty(b, s, n->as<qparse::U8>());
      break;

    case QAST_NODE_U16_TY:
      out = nrgen_u16_ty(b, s, n->as<qparse::U16>());
      break;

    case QAST_NODE_U32_TY:
      out = nrgen_u32_ty(b, s, n->as<qparse::U32>());
      break;

    case QAST_NODE_U64_TY:
      out = nrgen_u64_ty(b, s, n->as<qparse::U64>());
      break;

    case QAST_NODE_U128_TY:
      out = nrgen_u128_ty(b, s, n->as<qparse::U128>());
      break;

    case QAST_NODE_I8_TY:
      out = nrgen_i8_ty(b, s, n->as<qparse::I8>());
      break;

    case QAST_NODE_I16_TY:
      out = nrgen_i16_ty(b, s, n->as<qparse::I16>());
      break;

    case QAST_NODE_I32_TY:
      out = nrgen_i32_ty(b, s, n->as<qparse::I32>());
      break;

    case QAST_NODE_I64_TY:
      out = nrgen_i64_ty(b, s, n->as<qparse::I64>());
      break;

    case QAST_NODE_I128_TY:
      out = nrgen_i128_ty(b, s, n->as<qparse::I128>());
      break;

    case QAST_NODE_F16_TY:
      out = nrgen_f16_ty(b, s, n->as<qparse::F16>());
      break;

    case QAST_NODE_F32_TY:
      out = nrgen_f32_ty(b, s, n->as<qparse::F32>());
      break;

    case QAST_NODE_F64_TY:
      out = nrgen_f64_ty(b, s, n->as<qparse::F64>());
      break;

    case QAST_NODE_F128_TY:
      out = nrgen_f128_ty(b, s, n->as<qparse::F128>());
      break;

    case QAST_NODE_VOID_TY:
      out = nrgen_void_ty(b, s, n->as<qparse::VoidTy>());
      break;

    case QAST_NODE_PTR_TY:
      out = nrgen_ptr_ty(b, s, n->as<qparse::PtrTy>());
      break;

    case QAST_NODE_OPAQUE_TY:
      out = nrgen_opaque_ty(b, s, n->as<qparse::OpaqueTy>());
      break;

    case QAST_NODE_STRUCT_TY:
      out = nrgen_struct_ty(b, s, n->as<qparse::StructTy>());
      break;

    case QAST_NODE_GROUP_TY:
      out = nrgen_group_ty(b, s, n->as<qparse::GroupTy>());
      break;

    case QAST_NODE_REGION_TY:
      out = nrgen_region_ty(b, s, n->as<qparse::RegionTy>());
      break;

    case QAST_NODE_UNION_TY:
      out = nrgen_union_ty(b, s, n->as<qparse::UnionTy>());
      break;

    case QAST_NODE_ARRAY_TY:
      out = nrgen_array_ty(b, s, n->as<qparse::ArrayTy>());
      break;

    case QAST_NODE_TUPLE_TY:
      out = nrgen_tuple_ty(b, s, n->as<qparse::TupleTy>());
      break;

    case QAST_NODE_FN_TY:
      out = nrgen_fn_ty(b, s, n->as<qparse::FuncTy>());
      break;

    case QAST_NODE_UNRES_TY:
      out = nrgen_unres_ty(b, s, n->as<qparse::UnresolvedType>());
      break;

    case QAST_NODE_INFER_TY:
      out = nrgen_infer_ty(b, s, n->as<qparse::InferType>());
      break;

    case QAST_NODE_TEMPL_TY:
      out = nrgen_templ_ty(b, s, n->as<qparse::TemplType>());
      break;

    case QAST_NODE_FNDECL:
      out = nrgen_fndecl(b, s, n->as<qparse::FnDecl>());
      break;

    case QAST_NODE_FN:
      out = nrgen_fn(b, s, n->as<qparse::FnDef>());
      break;

    case QAST_NODE_COMPOSITE_FIELD:
      out = nrgen_composite_field(b, s, n->as<qparse::CompositeField>());
      break;

    case QAST_NODE_BLOCK:
      out = nrgen_block(b, s, n->as<qparse::Block>());
      break;

    case QAST_NODE_CONST:
      out = nrgen_const(b, s, n->as<qparse::ConstDecl>());
      break;

    case QAST_NODE_VAR:
      out = nrgen_var(b, s, n->as<qparse::VarDecl>());
      break;

    case QAST_NODE_LET:
      out = nrgen_let(b, s, n->as<qparse::LetDecl>());
      break;

    case QAST_NODE_INLINE_ASM:
      out = nrgen_inline_asm(b, s, n->as<qparse::InlineAsm>());
      break;

    case QAST_NODE_RETURN:
      out = nrgen_return(b, s, n->as<qparse::ReturnStmt>());
      break;

    case QAST_NODE_RETIF:
      out = nrgen_retif(b, s, n->as<qparse::ReturnIfStmt>());
      break;

    case QAST_NODE_RETZ:
      out = nrgen_retz(b, s, n->as<qparse::RetZStmt>());
      break;

    case QAST_NODE_RETV:
      out = nrgen_retv(b, s, n->as<qparse::RetVStmt>());
      break;

    case QAST_NODE_BREAK:
      out = nrgen_break(b, s, n->as<qparse::BreakStmt>());
      break;

    case QAST_NODE_CONTINUE:
      out = nrgen_continue(b, s, n->as<qparse::ContinueStmt>());
      break;

    case QAST_NODE_IF:
      out = nrgen_if(b, s, n->as<qparse::IfStmt>());
      break;

    case QAST_NODE_WHILE:
      out = nrgen_while(b, s, n->as<qparse::WhileStmt>());
      break;

    case QAST_NODE_FOR:
      out = nrgen_for(b, s, n->as<qparse::ForStmt>());
      break;

    case QAST_NODE_FORM:
      out = nrgen_form(b, s, n->as<qparse::FormStmt>());
      break;

    case QAST_NODE_FOREACH:
      out = nrgen_foreach(b, s, n->as<qparse::ForeachStmt>());
      break;

    case QAST_NODE_CASE:
      out = nrgen_case(b, s, n->as<qparse::CaseStmt>());
      break;

    case QAST_NODE_SWITCH:
      out = nrgen_switch(b, s, n->as<qparse::SwitchStmt>());
      break;

    case QAST_NODE_EXPR_STMT:
      out = nrgen_expr_stmt(b, s, n->as<qparse::ExprStmt>());
      break;

    case QAST_NODE_VOLSTMT:
      out = nrgen_volstmt(b, s, n->as<qparse::VolStmt>());
      break;

    default: {
      qcore_panicf("nr: unknown node type: %d", static_cast<int>(n->this_typeid()));
    }
  }

  if (!out) {
    qcore_panicf("nr: conversion failed for node type: %d", static_cast<int>(n->this_typeid()));
  }

  out->setLocDangerous({n->get_start_pos(), n->get_end_pos()});

  return out;
}

static std::optional<std::vector<nr::Expr *>> nrgen_any(NRBuilder &b, ConvState &s,
                                                        qparse::Node *n) {
  using namespace nr;

  if (!n) {
    return {};
  }

  std::vector<nr::Expr *> out;

  switch (n->this_typeid()) {
    case QAST_NODE_TYPEDEF:
      out = nrgen_typedef(b, s, n->as<qparse::TypedefDecl>());
      break;

    case QAST_NODE_ENUM:
      out = nrgen_enum(b, s, n->as<qparse::EnumDef>());
      break;

    case QAST_NODE_STRUCT:
      out = nrgen_struct(b, s, n->as<qparse::StructDef>());
      break;

    case QAST_NODE_REGION:
      out = nrgen_region(b, s, n->as<qparse::RegionDef>());
      break;

    case QAST_NODE_GROUP:
      out = nrgen_group(b, s, n->as<qparse::GroupDef>());
      break;

    case QAST_NODE_UNION:
      out = nrgen_union(b, s, n->as<qparse::UnionDef>());
      break;

    case QAST_NODE_SUBSYSTEM:
      out = nrgen_subsystem(b, s, n->as<qparse::SubsystemDecl>());
      break;

    case QAST_NODE_EXPORT:
      out = nrgen_export(b, s, n->as<qparse::ExportDecl>());
      break;

    default: {
      auto expr = nrgen_one(b, s, n);
      if (expr) {
        out.push_back(expr);
      } else {
        badtree(n, "nr::nrgen_any() failed to convert node");
        return std::nullopt;
      }
    }
  }

  return out;
}
