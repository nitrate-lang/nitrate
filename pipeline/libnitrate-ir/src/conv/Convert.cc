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

class QError : public std::exception {
public:
  QError() = default;
};

static nr::Expr *nrgen_one(NRBuilder &b, ConvState &s, qparse::Node *node);
static std::vector<nr::Expr *> nrgen_any(NRBuilder &b, ConvState &s, qparse::Node *node);

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

  try {
    ConvState s;
    NRBuilder builder(*mod->getLexer(), mod->getTargetInfo());
    mod->setRoot(nrgen_one(builder, s, static_cast<qparse::Node *>(base)));

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
  } catch (QError &) {
    success = false;
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

nr::Expr *nrgen_lower_binexpr(NRBuilder &, ConvState &, nr::Expr *lhs, nr::Expr *rhs,
                              qlex_op_t op) {
#define STD_BINOP(op) nr::create<nr::BinExpr>(lhs, rhs, nr::Op::op)
#define ASSIGN_BINOP(op)                                                                         \
  nr::create<nr::BinExpr>(                                                                       \
      lhs,                                                                                       \
      nr::create<nr::BinExpr>(static_cast<nr::Expr *>(nr_clone(nullptr, lhs)), rhs, nr::Op::op), \
      nr::Op::Set)

  nr::Expr *R = nullptr;

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

nr::Expr *nrgen_lower_unexpr(NRBuilder &b, ConvState &s, nr::Expr *rhs, qlex_op_t op) {
#define STD_UNOP(op) nr::create<nr::UnExpr>(rhs, nr::Op::op)

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
      auto bits = nr::create<nr::UnExpr>(rhs, nr::Op::Bitsizeof);
      auto arg = nr::create<nr::BinExpr>(bits, nr::create<nr::Float>(8, nr::FloatSize::F64),
                                         nr::Op::Slash);
      return create_simple_call(b, s, "std::ceil", {{"0", arg}});
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
      nr::SymbolEncoding se;
      auto res = se.mangle_name(inferred.value(), nr::AbiTag::Nitrate);
      if (!res) {
        badtree(nullptr, "Failed to mangle type");
        throw QError();
      }

      return createStringLiteral(res.value());
    }
    case qOpBitsizeof: {
      return STD_UNOP(Bitsizeof);
    }
    default: {
      throw QError();
    }
  }
}

nr::Expr *nrgen_lower_post_unexpr(NRBuilder &, ConvState &, nr::Expr *lhs, qlex_op_t op) {
#define STD_POST_OP(op) nr::create<nr::PostUnExpr>(lhs, nr::Op::op)

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

static Expr *nrgen_cexpr(NRBuilder &b, ConvState &s, qparse::ConstExpr *n) {
  auto c = nrgen_one(b, s, n->get_value());
  if (!c) {
    badtree(n, "qparse::ConstExpr::get_value() == nullptr");
    throw QError();
  }

  return c;
}

static Expr *nrgen_binexpr(NRBuilder &b, ConvState &s, qparse::BinExpr *n) {
  /**
   * @brief Convert a binary expression to a nr expression.
   * @details Recursively convert the left and right hand sides of the
   *         binary expression, then convert the operator to a nr
   *         compatible operator.
   */

  auto lhs = nrgen_one(b, s, n->get_lhs());
  if (!lhs) {
    badtree(n, "qparse::BinExpr::get_lhs() == nullptr");
    throw QError();
  }

  auto rhs = nrgen_one(b, s, n->get_rhs());

  if (!rhs) {
    badtree(n, "qparse::BinExpr::get_rhs() == nullptr");
    throw QError();
  }

  return nrgen_lower_binexpr(b, s, lhs, rhs, n->get_op());
}

static Expr *nrgen_unexpr(NRBuilder &b, ConvState &s, qparse::UnaryExpr *n) {
  /**
   * @brief Convert a unary expression to a nr expression.
   * @details Recursively convert the left hand side of the unary
   *         expression, then convert the operator to a nr compatible
   *         operator.
   */

  auto rhs = nrgen_one(b, s, n->get_rhs());
  if (!rhs) {
    badtree(n, "qparse::UnaryExpr::get_rhs() == nullptr");
    throw QError();
  }

  return nrgen_lower_unexpr(b, s, rhs, n->get_op());
}

static Expr *nrgen_post_unexpr(NRBuilder &b, ConvState &s, qparse::PostUnaryExpr *n) {
  /**
   * @brief Convert a post-unary expression to a nr expression.
   * @details Recursively convert the left hand side of the post-unary
   *         expression, then convert the operator to a nr compatible
   *         operator.
   */

  auto lhs = nrgen_one(b, s, n->get_lhs());
  if (!lhs) {
    badtree(n, "qparse::PostUnaryExpr::get_lhs() == nullptr");
    throw QError();
  }

  return nrgen_lower_post_unexpr(b, s, lhs, n->get_op());
}

static Expr *nrgen_terexpr(NRBuilder &b, ConvState &s, qparse::TernaryExpr *n) {
  /**
   * @brief Convert a ternary expression to a if-else expression.
   * @details Recursively convert the condition, then the true and false
   *        branches of the ternary expression.
   */

  auto cond = nrgen_one(b, s, n->get_cond());
  if (!cond) {
    badtree(n, "qparse::TernaryExpr::get_cond() == nullptr");
    throw QError();
  }

  auto t = nrgen_one(b, s, n->get_lhs());
  if (!t) {
    badtree(n, "qparse::TernaryExpr::get_lhs() == nullptr");
    throw QError();
  }

  auto f = nrgen_one(b, s, n->get_rhs());
  if (!f) {
    badtree(n, "qparse::TernaryExpr::get_rhs() == nullptr");
    throw QError();
  }

  return create<If>(cond, t, f);
}

static Expr *nrgen_int(NRBuilder &, ConvState &, qparse::ConstInt *n) {
  /**
   * @brief Convert an integer constant to a nr number.
   * @details This is a 1-to-1 conversion of the integer constant.
   */

  boost::multiprecision::cpp_int num(n->get_value());

  if (num > UINT64_MAX) {
    return create<Int>(n->get_value(), IntSize::U128);
  } else if (num > UINT32_MAX) {
    return create<Int>(n->get_value(), IntSize::U64);
  } else {
    return create<Int>(n->get_value(), IntSize::U32);
  }
}

static Expr *nrgen_float(NRBuilder &, ConvState &, qparse::ConstFloat *n) {
  /**
   * @brief Convert a floating point constant to a nr number.
   * @details This is a 1-to-1 conversion of the floating point constant.
   */

  return create<Float>(memorize(n->get_value()));
}

static Expr *nrgen_string(NRBuilder &, ConvState &, qparse::ConstString *n) {
  /**
   * @brief Convert a string constant to a nr string.
   * @details This is a 1-to-1 conversion of the string constant.
   */

  return createStringLiteral(n->get_value());
}

static Expr *nrgen_char(NRBuilder &, ConvState &, qparse::ConstChar *n) {
  /**
   * @brief Convert a character constant to a nr number.
   * @details Convert the char32 codepoint to a nr number literal.
   */

  return create<Int>(n->get_value(), IntSize::U8);
}

static Expr *nrgen_bool(NRBuilder &, ConvState &, qparse::ConstBool *n) {
  /**
   * @brief Convert a boolean constant to a nr number.
   * @details QXIIR does not have boolean types, so we convert
   *          them to integers.
   */

  if (n->get_value()) {
    return create<Int>(1, IntSize::U1);
  } else {
    return create<Int>(0, IntSize::U1);
  }
}

static Expr *nrgen_null(NRBuilder &, ConvState &, qparse::ConstNull *) {
  return create<BinExpr>(
      create<BinExpr>(create<Int>(0, IntSize::U32), createUPtrIntTy(), Op::CastAs),
      create<PtrTy>(create<VoidTy>()), Op::BitcastAs);
}

static Expr *nrgen_undef(NRBuilder &, ConvState &, qparse::ConstUndef *n) {
  report(IssueCode::UnexpectedUndefLiteral, IssueClass::Error, n->get_start_pos(),
         n->get_end_pos());
  throw QError();
}

static Expr *nrgen_call(NRBuilder &b, ConvState &s, qparse::Call *n) {
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
    throw QError();
  }

  CallArgsTmpNodeCradle datapack;
  for (auto it = n->get_args().begin(); it != n->get_args().end(); ++it) {
    auto arg = nrgen_one(b, s, it->second);
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

static Expr *nrgen_list(NRBuilder &b, ConvState &s, qparse::List *n) {
  /**
   * @brief Convert a list of expressions to a nr list.
   * @details This is a 1-to-1 conversion of the list of expressions.
   */

  ListItems items;

  for (auto it = n->get_items().begin(); it != n->get_items().end(); ++it) {
    auto item = nrgen_one(b, s, *it);
    if (!item) {
      badtree(n, "qparse::List::get_items() vector contains nullptr");
      throw QError();
    }

    items.push_back(item);
  }

  return create<List>(std::move(items), false);
}

static Expr *nrgen_assoc(NRBuilder &b, ConvState &s, qparse::Assoc *n) {
  /**
   * @brief Convert an associative list to a nr list.
   * @details This is a 1-to-1 conversion of the associative list.
   */

  auto key = nrgen_one(b, s, n->get_key());
  if (!key) {
    badtree(n, "qparse::Assoc::get_key() == nullptr");
    throw QError();
  }

  auto value = nrgen_one(b, s, n->get_value());
  if (!value) {
    badtree(n, "qparse::Assoc::get_value() == nullptr");
    throw QError();
  }

  return create<List>(ListItems({key, value}), false);
}

static Expr *nrgen_field(NRBuilder &b, ConvState &s, qparse::Field *n) {
  /**
   * @brief Convert a field access to a nr expression.
   * @details Store the base and field name in a temporary node cradle
   *          for later lowering.
   */

  auto base = nrgen_one(b, s, n->get_base());
  if (!base) {
    badtree(n, "qparse::Field::get_base() == nullptr");
    throw QError();
  }

  return create<Index>(base, createStringLiteral(n->get_field()));
}

static Expr *nrgen_index(NRBuilder &b, ConvState &s, qparse::Index *n) {
  /**
   * @brief Convert an index expression to a nr expression.
   * @details Recursively convert the base and index of the index
   *         expression.
   */

  auto base = nrgen_one(b, s, n->get_base());
  if (!base) {
    badtree(n, "qparse::Index::get_base() == nullptr");
    throw QError();
  }

  auto index = nrgen_one(b, s, n->get_index());
  if (!index) {
    badtree(n, "qparse::Index::get_index() == nullptr");
    throw QError();
  }

  return create<Index>(base, index);
}

static Expr *nrgen_slice(NRBuilder &b, ConvState &s, qparse::Slice *n) {
  /**
   * @brief Convert a slice expression to a nr expression.
   * @details Recursively convert the base, start, and end of the slice
   *         expression and pass them so the .slice() method which is
   *         assumed to be present in the base object.
   */

  auto base = nrgen_one(b, s, n->get_base());
  if (!base) {
    badtree(n, "qparse::Slice::get_base() == nullptr");
    throw QError();
  }

  auto start = nrgen_one(b, s, n->get_start());
  if (!start) {
    badtree(n, "qparse::Slice::get_start() == nullptr");
    throw QError();
  }

  auto end = nrgen_one(b, s, n->get_end());
  if (!end) {
    badtree(n, "qparse::Slice::get_end() == nullptr");
    throw QError();
  }

  return create<Call>(create<Index>(base, createStringLiteral("slice")), CallArgs({start, end}));
}

static Expr *nrgen_fstring(NRBuilder &b, ConvState &s, qparse::FString *n) {
  /**
   * @brief Convert a formatted string to a nr string concatenation.
   */

  if (n->get_items().empty()) {
    return createStringLiteral("");
  }

  if (n->get_items().size() == 1) {
    auto val = n->get_items().front();
    if (std::holds_alternative<qparse::String>(val)) {
      return createStringLiteral(std::get<qparse::String>(val));
    } else if (std::holds_alternative<qparse::Expr *>(val)) {
      auto expr = nrgen_one(b, s, std::get<qparse::Expr *>(val));
      if (!expr) {
        badtree(n, "qparse::FString::get_items() vector contains nullptr");
        throw QError();
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

static Expr *nrgen_ident(NRBuilder &, ConvState &s, qparse::Ident *n) {
  /**
   * @brief Convert an identifier to a nr expression.
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

static Expr *nrgen_seq_point(NRBuilder &b, ConvState &s, qparse::SeqPoint *n) {
  /**
   * @brief Convert a sequence point to a nr expression.
   * @details This is a 1-to-1 conversion of the sequence point.
   */

  SeqItems items;
  items.reserve(n->get_items().size());

  for (auto it = n->get_items().begin(); it != n->get_items().end(); ++it) {
    auto item = nrgen_one(b, s, *it);
    if (!item) {
      badtree(n, "qparse::SeqPoint::get_items() vector contains nullptr");
      throw QError();
    }

    items.push_back(item);
  }

  return create<Seq>(std::move(items));
}

static Expr *nrgen_stmt_expr(NRBuilder &b, ConvState &s, qparse::StmtExpr *n) {
  /**
   * @brief Unwrap a statement inside an expression into a nr expression.
   * @details This is a 1-to-1 conversion of the statement expression.
   */

  auto stmt = nrgen_one(b, s, n->get_stmt());
  if (!stmt) {
    badtree(n, "qparse::StmtExpr::get_stmt() == nullptr");
    throw QError();
  }

  return stmt;
}

static Expr *nrgen_type_expr(NRBuilder &b, ConvState &s, qparse::TypeExpr *n) {
  /*
   * @brief Convert a type expression to a nr expression.
   * @details This is a 1-to-1 conversion of the type expression.
   */

  auto type = nrgen_one(b, s, n->get_type());
  if (!type) {
    badtree(n, "qparse::TypeExpr::get_type() == nullptr");
    throw QError();
  }

  return type;
}

static Expr *nrgen_templ_call(NRBuilder &, ConvState &, qparse::TemplCall *n) {
  /// TODO: templ_call

  badtree(n, "Template call not implemented");

  throw QError();
}

static Expr *nrgen_ref_ty(NRBuilder &b, ConvState &s, qparse::RefTy *n) {
  auto pointee = nrgen_one(b, s, n->get_item());
  if (!pointee) {
    badtree(n, "qparse::RefTy::get_item() == nullptr");
    throw QError();
  }

  return create<PtrTy>(pointee->asType());
}

static Expr *nrgen_u1_ty(NRBuilder &b, ConvState &, qparse::U1 *) { return b.getU1Ty(); }
static Expr *nrgen_u8_ty(NRBuilder &b, ConvState &, qparse::U8 *) { return b.getU8Ty(); }
static Expr *nrgen_u16_ty(NRBuilder &b, ConvState &, qparse::U16 *) { return b.getU16Ty(); }
static Expr *nrgen_u32_ty(NRBuilder &b, ConvState &, qparse::U32 *) { return b.getU32Ty(); }
static Expr *nrgen_u64_ty(NRBuilder &b, ConvState &, qparse::U64 *) { return b.getU64Ty(); }
static Expr *nrgen_u128_ty(NRBuilder &b, ConvState &, qparse::U128 *) { return b.getU128Ty(); }
static Expr *nrgen_i8_ty(NRBuilder &b, ConvState &, qparse::I8 *) { return b.getI8Ty(); }
static Expr *nrgen_i16_ty(NRBuilder &b, ConvState &, qparse::I16 *) { return b.getI16Ty(); }
static Expr *nrgen_i32_ty(NRBuilder &b, ConvState &, qparse::I32 *) { return b.getI32Ty(); }
static Expr *nrgen_i64_ty(NRBuilder &b, ConvState &, qparse::I64 *) { return b.getI64Ty(); }
static Expr *nrgen_i128_ty(NRBuilder &b, ConvState &, qparse::I128 *) { return b.getI128Ty(); }
static Expr *nrgen_f16_ty(NRBuilder &b, ConvState &, qparse::F16 *) { return b.getF16Ty(); }
static Expr *nrgen_f32_ty(NRBuilder &b, ConvState &, qparse::F32 *) { return b.getF32Ty(); }
static Expr *nrgen_f64_ty(NRBuilder &b, ConvState &, qparse::F64 *) { return b.getF64Ty(); }
static Expr *nrgen_f128_ty(NRBuilder &b, ConvState &, qparse::F128 *) { return b.getF128Ty(); }
static Expr *nrgen_void_ty(NRBuilder &b, ConvState &, qparse::VoidTy *) { return b.getVoidTy(); }

static Expr *nrgen_ptr_ty(NRBuilder &b, ConvState &s, qparse::PtrTy *n) {
  /**
   * @brief Convert a pointer type to a nr pointer type.
   * @details This is a 1-to-1 conversion of the pointer type.
   */

  auto pointee = nrgen_one(b, s, n->get_item());
  if (!pointee) {
    badtree(n, "qparse::PtrTy::get_item() == nullptr");
    throw QError();
  }

  return create<PtrTy>(pointee->asType());
}

static Expr *nrgen_opaque_ty(NRBuilder &, ConvState &, qparse::OpaqueTy *n) {
  /**
   * @brief Convert an opaque type to a nr opaque type.
   * @details This is a 1-to-1 conversion of the opaque type.
   */

  return create<OpaqueTy>(memorize(n->get_name()));
}

static Expr *nrgen_struct_ty(NRBuilder &b, ConvState &s, qparse::StructTy *n) {
  /**
   * @brief Convert a struct type to a nr struct type.
   * @details This is a 1-to-1 conversion of the struct type.
   */

  StructFields fields;

  for (auto it = n->get_items().begin(); it != n->get_items().end(); ++it) {
    auto item = nrgen_one(b, s, it->second)->asType();
    if (!item) {
      badtree(n, "qparse::StructTy::get_items() vector contains nullptr");
      throw QError();
    }

    fields.push_back(item);
  }

  return create<StructTy>(std::move(fields));
}

static Expr *nrgen_group_ty(NRBuilder &b, ConvState &s, qparse::GroupTy *n) {
  /**
   * @brief Convert a group type to a nr struct type.
   * @details This is a 1-to-1 conversion of the group type.
   */

  StructFields fields;

  for (auto it = n->get_items().begin(); it != n->get_items().end(); ++it) {
    auto item = nrgen_one(b, s, *it)->asType();
    if (!item) {
      badtree(n, "qparse::GroupTy::get_items() vector contains nullptr");
      throw QError();
    }

    fields.push_back(item);
  }

  return create<StructTy>(std::move(fields));
}

static Expr *nrgen_region_ty(NRBuilder &b, ConvState &s, qparse::RegionTy *n) {
  /**
   * @brief Convert a region type to a nr struct type.
   * @details This is a 1-to-1 conversion of the region type.
   */

  StructFields fields;

  for (auto it = n->get_items().begin(); it != n->get_items().end(); ++it) {
    auto item = nrgen_one(b, s, *it)->asType();
    if (!item) {
      badtree(n, "qparse::RegionTy::get_items() vector contains nullptr");
      throw QError();
    }

    fields.push_back(item);
  }

  return create<StructTy>(std::move(fields));
}

static Expr *nrgen_union_ty(NRBuilder &b, ConvState &s, qparse::UnionTy *n) {
  /**
   * @brief Convert a union type to a nr struct type.
   * @details This is a 1-to-1 conversion of the union type.
   */

  UnionFields fields;

  for (auto it = n->get_items().begin(); it != n->get_items().end(); ++it) {
    auto item = nrgen_one(b, s, *it)->asType();
    if (!item) {
      badtree(n, "qparse::UnionTy::get_items() vector contains nullptr");
      throw QError();
    }

    fields.push_back(item);
  }

  return create<UnionTy>(std::move(fields));
}

static Expr *nrgen_array_ty(NRBuilder &b, ConvState &s, qparse::ArrayTy *n) {
  /**
   * @brief Convert an array type to a nr array type.
   * @details This is a 1-to-1 conversion of the array type.
   */

  auto item = nrgen_one(b, s, n->get_item());
  if (!item) {
    badtree(n, "qparse::ArrayTy::get_item() == nullptr");
    throw QError();
  }

  auto count = nrgen_one(b, s, n->get_size());
  if (!count) {
    badtree(n, "qparse::ArrayTy::get_size() == nullptr");
    throw QError();
  }

  /// TODO: Invoke an interpreter to calculate size expression
  if (count->getKind() != QIR_NODE_INT) {
    badtree(n, "Non integer literal array size is not supported");
    throw QError();
  }

  uint128_t size = count->as<Int>()->getValue();

  if (size > UINT64_MAX) {
    badtree(n, "Array size > UINT64_MAX");
    throw QError();
  }

  return create<ArrayTy>(item->asType(), static_cast<uint64_t>(size));
}

static Expr *nrgen_tuple_ty(NRBuilder &b, ConvState &s, qparse::TupleTy *n) {
  /**
   * @brief Convert a tuple type to a nr struct type.
   * @details This is a 1-to-1 conversion of the tuple type.
   */

  StructFields fields;
  for (auto it = n->get_items().begin(); it != n->get_items().end(); ++it) {
    auto item = nrgen_one(b, s, *it)->asType();
    if (!item) {
      badtree(n, "qparse::TupleTy::get_items() vector contains nullptr");
      throw QError();
    }

    fields.push_back(item);
  }

  return create<StructTy>(std::move(fields));
}

static Expr *nrgen_fn_ty(NRBuilder &b, ConvState &s, qparse::FuncTy *n) {
  /**
   * @brief Convert a function type to a nr function type.
   * @details Order of function parameters is consistent with their order
   * of declaration. Support for `optional` arguments is up to the frontend,
   * for inter-language compatibility, the ABI is not concerned with optional
   * arguments as they have no significance at the binary interface level.
   */

  FnParams params;

  for (auto it = n->get_params().begin(); it != n->get_params().end(); ++it) {
    Type *type = nrgen_one(b, s, std::get<1>(*it))->asType();
    if (!type) {
      badtree(n, "qparse::FnDef::get_type() == nullptr");
      throw QError();
    }

    params.push_back(type);
  }

  Type *ret = nrgen_one(b, s, n->get_return_ty())->asType();
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

static Expr *nrgen_unres_ty(NRBuilder &, ConvState &s, qparse::UnresolvedType *n) {
  /**
   * @brief Convert an unresolved type to a nr type.
   * @details This is a 1-to-1 conversion of the unresolved type.
   */

  auto str = s.cur_named(n->get_name());
  auto name = memorize(std::string_view(str));

  return create<Tmp>(TmpType::NAMED_TYPE, name);
}

static Expr *nrgen_infer_ty(NRBuilder &, ConvState &, qparse::InferType *) {
  /// TODO: infer_ty
  throw QError();
}

static Expr *nrgen_templ_ty(NRBuilder &, ConvState &, qparse::TemplType *n) {
  /// TODO: templ_ty
  badtree(n, "template types are not supported yet");
  throw QError();
}

static std::vector<Expr *> nrgen_typedef(NRBuilder &b, ConvState &s, qparse::TypedefDecl *n) {
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
    throw QError();
  }

  current->getTypeMap()[name] = type->asType();

  return {};
}

static Expr *nrgen_fndecl(NRBuilder &b, ConvState &s, qparse::FnDecl *n) {
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

      Type *type = nrgen_one(b, s, std::get<1>(*it))->asType();
      if (!type) {
        badtree(n, "qparse::FnDecl::get_type() == nullptr");
        throw QError();
      }

      Expr *def = nullptr;
      if (std::get<2>(*it)) {
        def = nrgen_one(b, s, std::get<2>(*it));
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

  Expr *fnty = nrgen_one(b, s, fty);
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

static std::vector<Expr *> nrgen_struct(NRBuilder &b, ConvState &s, qparse::StructDef *n) {
  /**
   * @brief Convert a struct definition to a nr sequence.
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
    auto field = nrgen_one(b, s, *it);
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

    auto method = nrgen_one(b, s, *it);

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

    auto method = nrgen_one(b, s, *it);

    cur_meth->set_name(old_name);

    if (!method) {
      badtree(n, "qparse::StructDef::get_static_methods() vector contains nullptr");
      throw QError();
    }

    items.push_back(method);
  }

  return items;
}

static std::vector<Expr *> nrgen_region(NRBuilder &b, ConvState &s, qparse::RegionDef *n) {
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
  std::vector<Expr *> items;

  for (auto it = n->get_fields().begin(); it != n->get_fields().end(); ++it) {
    if (!*it) {
      badtree(n, "qparse::RegionDef::get_fields() vector contains nullptr");
      throw QError();
    }

    s.composite_expanse.push((*it)->get_name());
    auto field = nrgen_one(b, s, *it);
    s.composite_expanse.pop();

    fields.push_back(field->asType());
  }

  StructTy *st = create<StructTy>(std::move(fields));

  current->getTypeMap()[sv] = st;

  for (auto it = n->get_methods().begin(); it != n->get_methods().end(); ++it) {
    qparse::FnDecl *cur_meth = *it;
    auto old_name = cur_meth->get_name();
    cur_meth->set_name(ns_join(n->get_name(), old_name));

    auto method = nrgen_one(b, s, *it);

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

    auto method = nrgen_one(b, s, *it);

    cur_meth->set_name(old_name);

    if (!method) {
      badtree(n, "qparse::RegionDef::get_static_methods() vector contains nullptr");
      throw QError();
    }

    items.push_back(method);
  }

  return items;
}

static std::vector<Expr *> nrgen_group(NRBuilder &b, ConvState &s, qparse::GroupDef *n) {
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
  std::vector<Expr *> items;

  { /* Optimize layout with field sorting */
    std::vector<Type *> tmp_fields;
    for (auto it = n->get_fields().begin(); it != n->get_fields().end(); ++it) {
      if (!*it) {
        badtree(n, "qparse::GroupDef::get_fields() vector contains nullptr");
        throw QError();
      }

      s.composite_expanse.push((*it)->get_name());
      auto field = nrgen_one(b, s, *it);
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

    auto method = nrgen_one(b, s, *it);

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

    auto method = nrgen_one(b, s, *it);

    cur_meth->set_name(old_name);

    if (!method) {
      badtree(n, "qparse::GroupDef::get_static_methods() vector contains nullptr");
      throw QError();
    }

    items.push_back(method);
  }

  return items;
}

static std::vector<Expr *> nrgen_union(NRBuilder &b, ConvState &s, qparse::UnionDef *n) {
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
  std::vector<Expr *> items;

  for (auto it = n->get_fields().begin(); it != n->get_fields().end(); ++it) {
    if (!*it) {
      badtree(n, "qparse::UnionDef::get_fields() vector contains nullptr");
      throw QError();
    }

    s.composite_expanse.push((*it)->get_name());
    auto field = nrgen_one(b, s, *it);
    s.composite_expanse.pop();

    fields.push_back(field->asType());
  }

  UnionTy *st = create<UnionTy>(std::move(fields));

  current->getTypeMap()[sv] = st;

  for (auto it = n->get_methods().begin(); it != n->get_methods().end(); ++it) {
    qparse::FnDecl *cur_meth = *it;
    auto old_name = cur_meth->get_name();
    cur_meth->set_name(ns_join(n->get_name(), old_name));

    auto method = nrgen_one(b, s, *it);

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

    auto method = nrgen_one(b, s, *it);

    cur_meth->set_name(old_name);

    if (!method) {
      badtree(n, "qparse::UnionDef::get_static_methods() vector contains nullptr");
      throw QError();
    }

    items.push_back(method);
  }

  return items;
}

static std::vector<Expr *> nrgen_enum(NRBuilder &b, ConvState &s, qparse::EnumDef *n) {
  /**
   * @brief Convert an enum definition to a nr sequence.
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
    type = nrgen_one(b, s, n->get_type())->asType();
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
      cur = nrgen_one(b, s, it->second);
      if (!cur) {
        badtree(n, "qparse::EnumDef::get_items() vector contains nullptr");
        throw QError();
      }

      last = cur;
    } else {
      if (!last) {
        last = cur = create<Int>(0, IntSize::U32);
      } else {
        last = cur = create<BinExpr>(last, create<Int>(1, IntSize::U32), Op::Plus);
      }
    }

    std::string_view field_name = memorize(std::string_view(name + "::" + std::string(it->first)));

    current->getNamedConstants().insert({field_name, cur});
  }

  return {};
}

static Expr *nrgen_fn(NRBuilder &b, ConvState &s, qparse::FnDef *n) {
  bool old_inside_function = s.inside_function;
  s.inside_function = true;

  Expr *precond = nullptr, *postcond = nullptr;
  Seq *body = nullptr;
  Params params;
  qparse::FnDecl *decl = n;
  qparse::FuncTy *fty = decl->get_type();

  FnTy *fnty = static_cast<FnTy *>(nrgen_one(b, s, fty));
  if (!fnty) {
    badtree(n, "qparse::FnDef::get_type() == nullptr");
    throw QError();
  }

  /* Produce the function preconditions */
  if ((precond = nrgen_one(b, s, n->get_precond()))) {
    precond = create<If>(create<UnExpr>(precond, Op::LogicNot),
                         create_simple_call(b, s, "__detail::precond_fail"), createIgn());
  }

  /* Produce the function postconditions */
  if ((postcond = nrgen_one(b, s, n->get_postcond()))) {
    postcond = create<If>(create<UnExpr>(postcond, Op::LogicNot),
                          create_simple_call(b, s, "__detail::postcond_fail"), createIgn());
  }

  { /* Produce the function body */
    Type *old_return_ty = s.return_type;
    s.return_type = fnty->getReturn();
    s.local_scope.push({});

    Expr *tmp = nrgen_one(b, s, n->get_body());
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

      Type *type = nrgen_one(b, s, std::get<1>(*it))->asType();
      if (!type) {
        badtree(n, "qparse::FnDef::get_type() == nullptr");
        throw QError();
      }

      Expr *def = nullptr;
      if (std::get<2>(*it)) {
        def = nrgen_one(b, s, std::get<2>(*it));
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

static std::vector<Expr *> nrgen_subsystem(NRBuilder &b, ConvState &s, qparse::SubsystemDecl *n) {
  /**
   * @brief Convert a subsystem declaration to a nr sequence with
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

  for (auto it = n->get_body()->get_items().begin(); it != n->get_body()->get_items().end(); ++it) {
    auto item = nrgen_any(b, s, *it);

    items.insert(items.end(), item.begin(), item.end());
  }

  s.ns_prefix = old_ns;

  return items;
}

static std::vector<Expr *> nrgen_export(NRBuilder &b, ConvState &s, qparse::ExportDecl *n) {
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

static Expr *nrgen_composite_field(NRBuilder &b, ConvState &s, qparse::CompositeField *n) {
  auto type = nrgen_one(b, s, n->get_type());
  if (!type) {
    badtree(n, "qparse::CompositeField::get_type() == nullptr");
    throw QError();
  }

  Expr *_def = nullptr;
  if (n->get_value()) {
    _def = nrgen_one(b, s, n->get_value());
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

static Expr *nrgen_block(NRBuilder &b, ConvState &s, qparse::Block *n) {
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

static Expr *nrgen_const(NRBuilder &b, ConvState &s, qparse::ConstDecl *n) {
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
    throw QError();
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

static Expr *nrgen_var(NRBuilder &, ConvState &, qparse::VarDecl *) {
  /// TODO: var

  throw QError();
}

static Expr *nrgen_let(NRBuilder &b, ConvState &s, qparse::LetDecl *n) {
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
    throw QError();
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

static Expr *nrgen_inline_asm(NRBuilder &, ConvState &, qparse::InlineAsm *) {
  qcore_implement("nrgen_inline_asm");
}

static Expr *nrgen_return(NRBuilder &b, ConvState &s, qparse::ReturnStmt *n) {
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

static Expr *nrgen_retif(NRBuilder &b, ConvState &s, qparse::ReturnIfStmt *n) {
  /**
   * @brief Convert a return statement to a nr expression.
   * @details Lower into an 'if (cond) {return val}' expression.
   */

  auto cond = nrgen_one(b, s, n->get_cond());
  if (!cond) {
    badtree(n, "qparse::ReturnIfStmt::get_cond() == nullptr");
    throw QError();
  }

  cond = create<BinExpr>(cond, create<U1Ty>(), Op::CastAs);

  auto val = nrgen_one(b, s, n->get_value());
  if (!val) {
    badtree(n, "qparse::ReturnIfStmt::get_value() == nullptr");
    throw QError();
  }
  val = create<BinExpr>(val, s.return_type, Op::CastAs);

  return create<If>(cond, create<Ret>(val), createIgn());
}

static Expr *nrgen_retz(NRBuilder &b, ConvState &s, qparse::RetZStmt *n) {
  /**
   * @brief Convert a return statement to a nr expression.
   * @details Lower into an 'if (!cond) {return val}' expression.
   */

  auto cond = nrgen_one(b, s, n->get_cond());
  if (!cond) {
    badtree(n, "qparse::RetZStmt::get_cond() == nullptr");
    throw QError();
  }
  cond = create<BinExpr>(cond, create<U1Ty>(), Op::CastAs);

  auto inv_cond = create<UnExpr>(cond, Op::LogicNot);

  auto val = nrgen_one(b, s, n->get_value());
  if (!val) {
    badtree(n, "qparse::RetZStmt::get_value() == nullptr");
    throw QError();
  }
  val = create<BinExpr>(val, s.return_type, Op::CastAs);

  return create<If>(inv_cond, create<Ret>(val), createIgn());
}

static Expr *nrgen_retv(NRBuilder &b, ConvState &s, qparse::RetVStmt *n) {
  /**
   * @brief Convert a return statement to a nr expression.
   * @details Lower into an 'if (cond) {return void}' expression.
   */

  auto cond = nrgen_one(b, s, n->get_cond());
  if (!cond) {
    badtree(n, "qparse::RetVStmt::get_cond() == nullptr");
    throw QError();
  }
  cond = create<BinExpr>(cond, create<U1Ty>(), Op::CastAs);

  return create<If>(cond, create<Ret>(createIgn()), createIgn());
}

static Expr *nrgen_break(NRBuilder &, ConvState &, qparse::BreakStmt *) {
  /**
   * @brief Convert a break statement to a nr expression.
   * @details This is a 1-to-1 conversion of the break statement.
   */

  return create<Brk>();
}

static Expr *nrgen_continue(NRBuilder &, ConvState &, qparse::ContinueStmt *) {
  /**
   * @brief Convert a continue statement to a nr expression.
   * @details This is a 1-to-1 conversion of the continue statement.
   */

  return create<Cont>();
}

static Expr *nrgen_if(NRBuilder &b, ConvState &s, qparse::IfStmt *n) {
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

static Expr *nrgen_while(NRBuilder &b, ConvState &s, qparse::WhileStmt *n) {
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

static Expr *nrgen_for(NRBuilder &b, ConvState &s, qparse::ForStmt *n) {
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

static Expr *nrgen_form(NRBuilder &b, ConvState &s, qparse::FormStmt *n) {
  /**
   * @brief Convert a form loop to a nr expression.
   * @details This is a 1-to-1 conversion of the form loop.
   */

  auto maxjobs = nrgen_one(b, s, n->get_maxjobs());
  if (!maxjobs) {
    badtree(n, "qparse::FormStmt::get_maxjobs() == nullptr");
    throw QError();
  }

  auto idx_name = memorize(n->get_idx_ident());
  auto val_name = memorize(n->get_val_ident());

  auto iter = nrgen_one(b, s, n->get_expr());
  if (!iter) {
    badtree(n, "qparse::FormStmt::get_expr() == nullptr");
    throw QError();
  }

  auto body = nrgen_one(b, s, n->get_body());
  if (!body) {
    badtree(n, "qparse::FormStmt::get_body() == nullptr");
    throw QError();
  }

  return create<Form>(idx_name, val_name, maxjobs, iter, create<Seq>(SeqItems({body})));
}

static Expr *nrgen_foreach(NRBuilder &, ConvState &, qparse::ForeachStmt *) {
  /**
   * @brief Convert a foreach loop to a nr expression.
   * @details This is a 1-to-1 conversion of the foreach loop.
   */

  // auto idx_name = memorize(n->get_idx_ident());
  // auto val_name = memorize(n->get_val_ident());

  // auto iter = nrgen_one(b, s, n->get_expr());
  // if (!iter) {
  //   badtree(n, "qparse::ForeachStmt::get_expr() == nullptr");
  //   throw QError();
  // }

  // auto body = nrgen_one(b, s, n->get_body());
  // if (!body) {
  //   badtree(n, "qparse::ForeachStmt::get_body() == nullptr");
  //   throw QError();
  // }

  // return create<Foreach>(idx_name, val_name, iter, create<Seq>(SeqItems({body})));
  qcore_implement(__func__);
}

static Expr *nrgen_case(NRBuilder &b, ConvState &s, qparse::CaseStmt *n) {
  /**
   * @brief Convert a case statement to a nr expression.
   * @details This is a 1-to-1 conversion of the case statement.
   */

  auto cond = nrgen_one(b, s, n->get_cond());
  if (!cond) {
    badtree(n, "qparse::CaseStmt::get_cond() == nullptr");
    throw QError();
  }

  auto body = nrgen_one(b, s, n->get_body());
  if (!body) {
    badtree(n, "qparse::CaseStmt::get_body() == nullptr");
    throw QError();
  }

  return create<Case>(cond, body);
}

static Expr *nrgen_switch(NRBuilder &b, ConvState &s, qparse::SwitchStmt *n) {
  /**
   * @brief Convert a switch statement to a nr expression.
   * @details If the default case is missing, it is replaced with a void
   *        expression.
   */

  auto cond = nrgen_one(b, s, n->get_cond());
  if (!cond) {
    badtree(n, "qparse::SwitchStmt::get_cond() == nullptr");
    throw QError();
  }

  SwitchCases cases;
  for (auto it = n->get_cases().begin(); it != n->get_cases().end(); ++it) {
    auto item = nrgen_one(b, s, *it);
    if (!item) {
      badtree(n, "qparse::SwitchStmt::get_cases() vector contains nullptr");
      throw QError();
    }

    cases.push_back(item->as<Case>());
  }

  Expr *def = nullptr;
  if (n->get_default()) {
    def = nrgen_one(b, s, n->get_default());
    if (!def) {
      badtree(n, "qparse::SwitchStmt::get_default() == nullptr");
      throw QError();
    }
  } else {
    def = createIgn();
  }

  return create<Switch>(cond, std::move(cases), def);
}

static Expr *nrgen_expr_stmt(NRBuilder &b, ConvState &s, qparse::ExprStmt *n) {
  /**
   * @brief Convert an expression inside a statement to a nr expression.
   * @details This is a 1-to-1 conversion of the expression statement.
   */

  return nrgen_one(b, s, n->get_expr());
}

static Expr *nrgen_volstmt(NRBuilder &, ConvState &, qparse::VolStmt *) {
  /**
   * @brief Convert a volatile statement to a nr volatile expression.
   * @details This is a 1-to-1 conversion of the volatile statement.
   */

  // auto expr = nrgen_one(b, s, n->get_stmt());
  // expr->setVolatile(true);

  // return expr;

  qcore_implement(__func__);
}

static nr::Expr *nrgen_one(NRBuilder &b, ConvState &s, qparse::Node *n) {
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

static std::vector<nr::Expr *> nrgen_any(NRBuilder &b, ConvState &s, qparse::Node *n) {
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
        throw QError();
      }
    }
  }

  return out;
}
