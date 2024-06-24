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

#include <IR/Q/Expr.h>
#include <IR/Q/Math.h>
#include <IR/Q/Variable.h>
#include <core/Logger.h>
#include <optimizer/Passes.h>

#include <boost/multiprecision/cpp_dec_float.hpp>
#include <boost/multiprecision/cpp_int.hpp>

using namespace libquixcc;
using namespace libquixcc::ir::q;
using cpp_int = boost::multiprecision::cpp_int;
using cpp_dec_float_100 = boost::multiprecision::cpp_dec_float_100;

static void try_to_merge(Expr *lhs, Expr *rhs, Expr **merged) {
  if (!((lhs->is<Number>() && rhs->is<Number>()) ||
        (lhs->is<String>() && rhs->is<String>()) ||
        (lhs->is<Char>() && rhs->is<Char>()))) {
    /* Cannot merge non-literal values */
    return;
  }

  if (lhs->is<String>() && rhs->is<String>()) {
    /* Merge string literals */
    *merged = String::create(dynamic_cast<String *>(lhs)->value +
                             dynamic_cast<String *>(rhs)->value);
    return;
  }

  if (lhs->is<Char>() && rhs->is<Char>()) {
    /* Merge character literals */
    *merged = String::create(dynamic_cast<Char *>(lhs)->value +
                             dynamic_cast<Char *>(rhs)->value);
    return;
  }

  if (!(lhs->is<Number>() && rhs->is<Number>())) {
    /* Cannot merge non-numeric values (for now) */
    return;
  }

  /* Calculate constant expression */
  Number *nlhs = dynamic_cast<Number *>(lhs);
  Number *nrhs = dynamic_cast<Number *>(rhs);

  if (nlhs->value.find(".") != std::string::npos ||
      nrhs->value.find(".") != std::string::npos) {
    cpp_dec_float_100 lhs_val, rhs_val;
    lhs_val.assign(nlhs->value);
    rhs_val.assign(nrhs->value);

    switch ((QType)(*merged)->ntype) {
      case QType::Add: {
        *merged = Number::create(cpp_dec_float_100(lhs_val + rhs_val).str());
        break;
      }
      case QType::Sub: {
        *merged = Number::create(cpp_dec_float_100(lhs_val - rhs_val).str());
        break;
      }
      case QType::Mul: {
        *merged = Number::create(cpp_dec_float_100(lhs_val * rhs_val).str());
        break;
      }
      case QType::Div: {
        if (rhs_val == 0) {
          LOG(ERROR) << "Division by zero is undefined" << std::endl;
          return;
        }
        *merged = Number::create(cpp_dec_float_100(lhs_val / rhs_val).str());
        break;
      }
      case QType::Mod: {
        double lhs_valf = lhs_val.convert_to<double>();
        double rhs_valf = rhs_val.convert_to<double>();
        if (rhs_valf == 0) {
          LOG(ERROR) << "Modulo by zero is undefined" << std::endl;
          return;
        }
        double result = fmod(lhs_valf, rhs_valf);
        *merged = Number::create(std::to_string((double)result));
        break;
      }
      case QType::BitAnd:
      case QType::BitOr:
      case QType::BitXor:
      case QType::Shl:
      case QType::Shr:
      case QType::Rotl:
      case QType::Rotr:
        LOG(ERROR)
            << "Bitwise operations are not supported on floating-point numbers"
            << std::endl;
        break;
      case QType::Eq: {
        if (lhs_val == rhs_val) {
          *merged = Number::create("1");
        } else {
          *merged = Number::create("0");
        }
        break;
      }
      case QType::Ne: {
        if (lhs_val != rhs_val) {
          *merged = Number::create("1");
        } else {
          *merged = Number::create("0");
        }
        break;
      }
      case QType::Lt: {
        if (lhs_val < rhs_val) {
          *merged = Number::create("1");
        } else {
          *merged = Number::create("0");
        }
        break;
      }
      case QType::Gt: {
        if (lhs_val > rhs_val) {
          *merged = Number::create("1");
        } else {
          *merged = Number::create("0");
        }
        break;
      }
      case QType::Le: {
        if (lhs_val <= rhs_val) {
          *merged = Number::create("1");
        } else {
          *merged = Number::create("0");
        }
        break;
      }
      case QType::Ge: {
        if (lhs_val >= rhs_val) {
          *merged = Number::create("1");
        } else {
          *merged = Number::create("0");
        }
        break;
      }
      case QType::And: {
        *merged = Number::create(cpp_dec_float_100(lhs_val && rhs_val).str());
        break;
      }
      case QType::Or: {
        *merged = Number::create(cpp_dec_float_100(lhs_val || rhs_val).str());
        break;
      }
      case QType::Xor: {
        if ((lhs_val && rhs_val) || (!lhs_val && !rhs_val)) {
          *merged = Number::create("0");
        } else {
          *merged = Number::create("1");
        }
        break;
      }
      default:
        break;
    }
  } else {
    cpp_int lhs_val, rhs_val;
    lhs_val.assign(nlhs->value);
    rhs_val.assign(nrhs->value);

    switch ((QType)(*merged)->ntype) {
      case QType::Add: {
        *merged = Number::create(cpp_int(lhs_val + rhs_val).str());
        break;
      }
      case QType::Sub: {
        *merged = Number::create(cpp_int(lhs_val - rhs_val).str());
        break;
      }
      case QType::Mul: {
        *merged = Number::create(cpp_int(lhs_val * rhs_val).str());
        break;
      }
      case QType::Div: {
        if (rhs_val == 0) {
          LOG(ERROR) << "Division by zero is undefined" << std::endl;
          return;
        }
        *merged = Number::create(cpp_int(lhs_val / rhs_val).str());
        break;
      }
      case QType::Mod: {
        if (rhs_val == 0) {
          LOG(ERROR) << "Modulo by zero is undefined" << std::endl;
          return;
        }
        *merged = Number::create(cpp_int(lhs_val % rhs_val).str());
        break;
      }
      case QType::BitAnd: {
        *merged = Number::create(cpp_int(lhs_val & rhs_val).str());
        break;
      }
      case QType::BitOr: {
        *merged = Number::create(cpp_int(lhs_val | rhs_val).str());
        break;
      }
      case QType::BitXor: {
        *merged = Number::create(cpp_int(lhs_val ^ rhs_val).str());
        break;
      }
      case QType::Shl: {
        /// TOOD: implement Shl
        break;
      }
      case QType::Shr: {
        /// TOOD: implement Shr
        break;
      }
      case QType::Rotl: {
        /// TOOD: implement Rotl
        break;
      }
      case QType::Rotr: {
        /// TOOD: implement Rotr
        break;
      }
      case QType::Eq: {
        if (lhs_val == rhs_val) {
          *merged = Number::create("1");
        } else {
          *merged = Number::create("0");
        }
        break;
      }
      case QType::Ne: {
        if (lhs_val != rhs_val) {
          *merged = Number::create("1");
        } else {
          *merged = Number::create("0");
        }
        break;
      }
      case QType::Lt: {
        if (lhs_val < rhs_val) {
          *merged = Number::create("1");
        } else {
          *merged = Number::create("0");
        }
        break;
      }
      case QType::Gt: {
        if (lhs_val > rhs_val) {
          *merged = Number::create("1");
        } else {
          *merged = Number::create("0");
        }
        break;
      }
      case QType::Le: {
        if (lhs_val <= rhs_val) {
          *merged = Number::create("1");
        } else {
          *merged = Number::create("0");
        }
        break;
      }
      case QType::Ge: {
        if (lhs_val >= rhs_val) {
          *merged = Number::create("1");
        } else {
          *merged = Number::create("0");
        }
        break;
      }
      case QType::And: {
        *merged = Number::create(cpp_int(lhs_val && rhs_val).str());
        break;
      }
      case QType::Or: {
        *merged = Number::create(cpp_int(lhs_val || rhs_val).str());
        break;
      }
      case QType::Xor: {
        if ((lhs_val && rhs_val) || (!lhs_val && !rhs_val)) {
          *merged = Number::create("0");
        } else {
          *merged = Number::create("1");
        }
        break;
      }
      default:
        break;
    }
  }
}

static void reduce_expr(Expr **expr) {
  if ((*expr)->is<Number>() || (*expr)->is<String>() || (*expr)->is<Char>()) {
    /* Base case: Skip if the expression is a literal */
    return;
  }

#define BINARY_EXPR_REDUCE(_Name)              \
  case QType::_Name: {                         \
    auto e = reinterpret_cast<_Name **>(expr); \
    reduce_expr(&(*e)->lhs);                   \
    reduce_expr(&(*e)->rhs);                   \
    try_to_merge((*e)->lhs, (*e)->rhs, expr);  \
    break;                                     \
  }

  switch ((QType)(*expr)->ntype) {
    BINARY_EXPR_REDUCE(Add);
    BINARY_EXPR_REDUCE(Sub);
    BINARY_EXPR_REDUCE(Mul);
    BINARY_EXPR_REDUCE(Div);
    BINARY_EXPR_REDUCE(Mod);
    BINARY_EXPR_REDUCE(BitAnd);
    BINARY_EXPR_REDUCE(BitOr);
    BINARY_EXPR_REDUCE(BitXor);
    BINARY_EXPR_REDUCE(Shl);
    BINARY_EXPR_REDUCE(Shr);
    BINARY_EXPR_REDUCE(Rotl);
    BINARY_EXPR_REDUCE(Rotr);
    BINARY_EXPR_REDUCE(Eq);
    BINARY_EXPR_REDUCE(Ne);
    BINARY_EXPR_REDUCE(Lt);
    BINARY_EXPR_REDUCE(Gt);
    BINARY_EXPR_REDUCE(Le);
    BINARY_EXPR_REDUCE(Ge);
    BINARY_EXPR_REDUCE(And);
    BINARY_EXPR_REDUCE(Or);
    BINARY_EXPR_REDUCE(Xor);
    default:
      break;
  }
}

bool libquixcc::optimizer::passes::ExprReduce(
    quixcc_job_t &job, std::unique_ptr<libquixcc::ir::q::QModule> &ir) {
  auto filter_expr = [](const ir::q::Value *val) -> IterOp {
    /* Test if the value is an expression */
    if (dynamic_cast<const ir::q::Expr *>(val) == nullptr) {
      /* Skip non-expression values */
      return IterOp::Skip;
    }

    /* Continue iterating */
    return IterOp::Do;
  };

  auto transform = [](ir::q::Value **val) {
    auto expr = dynamic_cast<ir::q::Expr *>(*val);
    /* Skip if the value is not an expression */
    if (expr == nullptr) {
      return;
    }

    /* Reduce the expression */
    reduce_expr(reinterpret_cast<ir::q::Expr **>(val));
  };

  ir->bft_iter(transform, filter_expr);

  return true;
}