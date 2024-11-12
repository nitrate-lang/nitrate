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
#include <nitrate-ir/IR.h>

#include <boost/multiprecision/cpp_dec_float.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#include <nitrate-ir/IRGraph.hh>
#include <stack>

using namespace nr;

using VirtAddress = uint64_t;

struct ScopeBlock {
  std::unordered_map<std::string_view, Local *> locals;
  std::unordered_map<std::string_view, Fn *> functions;
};

struct CallFrame {
  std::string_view name;
  VirtAddress address;
};

using VirtObject = std::variant<Local *, Fn *>;

class Program {
  std::vector<ScopeBlock> scope_stack;
  std::stack<CallFrame> call_stack;
  std::stack<std::string> errors;

public:
  std::optional<Local *> find_variable(std::string_view name) const {
    auto local_it = scope_stack.back().locals.find(name);
    if (local_it != scope_stack.back().locals.end()) {
      return local_it->second;
    } else [[unlikely]] {
      return std::nullopt;
    }
  }

  std::optional<Fn *> find_function(std::string_view name) const {
    auto fn_it = scope_stack.back().functions.find(name);
    if (fn_it != scope_stack.back().functions.end()) {
      return fn_it->second;
    } else [[unlikely]] {
      return std::nullopt;
    }
  }

  void eprintn(std::string_view message) { errors.push(std::string(message)); }

  Program() {
    scope_stack.push_back({});
    call_stack.push({});
  }
};

static std::optional<Expr *> compute_binexpr(Program &P, Expr *L, Op O, Expr *R) noexcept {
  std::optional<Expr *> ANS;

  Type *LT = L->getType().value_or(nullptr);
  if (!LT) [[unlikely]] {
    P.eprintn("Failed to get type of left-hand side of binary expression");
    return std::nullopt;
  }

  Type *RT = R->getType().value_or(nullptr);
  if (!RT) [[unlikely]] {
    P.eprintn("Failed to get type of right-hand side of binary expression");
    return std::nullopt;
  }

  switch (O) {
    case Op::Plus: {
      /// TODO: Implement operator
      break;
    }

    case Op::Minus: {
      /// TODO: Implement operator
      break;
    }

    case Op::Times: {
      /// TODO: Implement operator
      break;
    }

    case Op::Slash: {
      /// TODO: Implement operator
      break;
    }

    case Op::Percent: {
      /// TODO: Implement operator
      break;
    }

    case Op::BitAnd: {
      /// TODO: Implement operator
      break;
    }

    case Op::BitOr: {
      /// TODO: Implement operator
      break;
    }

    case Op::BitXor: {
      /// TODO: Implement operator
      break;
    }

    case Op::LogicAnd: {
      /// TODO: Implement operator
      break;
    }

    case Op::LogicOr: {
      /// TODO: Implement operator
      break;
    }

    case Op::LShift: {
      /// TODO: Implement operator
      break;
    }

    case Op::RShift: {
      /// TODO: Implement operator
      break;
    }

    case Op::ROTR: {
      /// TODO: Implement operator
      break;
    }

    case Op::ROTL: {
      /// TODO: Implement operator
      break;
    }

    case Op::Set: {
      /// TODO: Implement operator
      break;
    }

    case Op::LT: {
      /// TODO: Implement operator
      break;
    }

    case Op::GT: {
      /// TODO: Implement operator
      break;
    }

    case Op::LE: {
      /// TODO: Implement operator
      break;
    }

    case Op::GE: {
      /// TODO: Implement operator
      break;
    }

    case Op::Eq: {
      /// TODO: Implement operator
      break;
    }

    case Op::NE: {
      /// TODO: Implement operator
      break;
    }

    case Op::BitcastAs: {
      /// TODO: Implement operator
      break;
    }

    case Op::CastAs: {
      /// TODO: Implement operator
      break;
    }

    default: {
      break;
    }
  }

  return ANS;
}

static std::optional<Expr *> compute_unexpr(Program &P, Expr *E, Op O) {
  std::optional<Expr *> ANS;

  Type *ET = E->getType().value_or(nullptr);
  if (!ET) [[unlikely]] {
    P.eprintn("Failed to get type of unary expression");
    return std::nullopt;
  }

  switch (O) {
    case Op::Plus: {
      /// TODO: Implement operator
      break;
    }

    case Op::Minus: {
      /// TODO: Implement operator
      break;
    }

    case Op::Times: {
      /// TODO: Implement operator
      break;
    }

    case Op::BitAnd: {
      /// TODO: Implement operator
      break;
    }

    case Op::BitNot: {
      /// TODO: Implement operator
      break;
    }

    case Op::LogicNot: {
      /// TODO: Implement operator
      break;
    }

    case Op::Inc: {
      /// TODO: Implement operator
      break;
    }

    case Op::Dec: {
      /// TODO: Implement operator
      break;
    }

    case Op::Alignof: {
      /// TODO: Implement operator
      break;
    }

    case Op::Bitsizeof: {
      /// TODO: Implement operator
      break;
    }

    default: {
      break;
    }
  }

  return ANS;
}

std::optional<nr::Expr *> evaluate(Program &P, nr::Expr *x) noexcept {
  nr::current = x->getModule();

  auto x_kind = x->getKind();

  switch (x_kind) {
      ///**********************************************************************///
      ///                          SIMPLE EXPRESSION NODES                     ///
      ///**********************************************************************///

    case QIR_NODE_BINEXPR: {
      BinExpr *B = x->as<BinExpr>();

      auto L = evaluate(P, B->getLHS());
      if (!L.has_value()) {
        P.eprintn("Failed to evaluate left-hand side of binary expression");
        return std::nullopt;
      }

      auto R = evaluate(P, B->getRHS());
      if (!R.has_value()) {
        P.eprintn("Failed to evaluate right-hand side of binary expression");
        return std::nullopt;
      }

      /// TODO: Type promotion

      return compute_binexpr(P, L.value(), B->getOp(), R.value());
    }

    case QIR_NODE_UNEXPR: {
      UnExpr *U = x->as<UnExpr>();

      auto E = evaluate(P, U->getExpr());
      if (!E.has_value()) {
        P.eprintn("Failed to evaluate unary expression");
        return std::nullopt;
      }

      E = compute_unexpr(P, E.value(), U->getOp());
      if (!E.has_value()) {
        P.eprintn("Failed to compute unary expression");
        return std::nullopt;
      }

      return E;
    }

    case QIR_NODE_POST_UNEXPR: {
      PostUnExpr *U = x->as<PostUnExpr>();

      auto E = evaluate(P, U->getExpr());
      if (!E.has_value()) {
        P.eprintn("Failed to evaluate post-unary expression");
        return std::nullopt;
      }

      if (!compute_unexpr(P, E.value(), U->getOp()).has_value()) {
        P.eprintn("Failed to compute post-unary expression");
        return std::nullopt;
      }

      return E;
    }

      ///**********************************************************************///
      ///                     COMPLEX EXPRESSION NODES                         ///
      ///**********************************************************************///

    case QIR_NODE_CALL: {
      /// TODO: Implement expression
      P.eprintn("Call expressions are not yet implemented");
      return std::nullopt;
    }

    case QIR_NODE_SEQ: {
      /// TODO: Implement expression
      P.eprintn("Sequence expressions are not yet implemented");
      return std::nullopt;
    }

    case QIR_NODE_INDEX: {
      /// TODO: Implement expression
      P.eprintn("Index expressions are not yet implemented");
      return std::nullopt;
    }

    case QIR_NODE_IDENT: {
      /// TODO: Implement expression
      P.eprintn("Identifier expressions are not yet implemented");
      return std::nullopt;
    }

    case QIR_NODE_EXTERN: {
      /// TODO: Implement expression
      P.eprintn("Extern expressions are not yet implemented");
      return std::nullopt;
    }

    case QIR_NODE_LOCAL: {
      /// TODO: Implement expression
      P.eprintn("Local expressions are not yet implemented");
      return std::nullopt;
    }

    case QIR_NODE_RET: {
      /// TODO: Implement expression
      P.eprintn("Return expressions are not yet implemented");
      return std::nullopt;
    }

    case QIR_NODE_BRK: {
      /// TODO: Implement expression
      P.eprintn("Break expressions are not yet implemented");
      return std::nullopt;
    }

    case QIR_NODE_CONT: {
      /// TODO: Implement expression
      P.eprintn("Continue expressions are not yet implemented");
      return std::nullopt;
    }

    case QIR_NODE_IF: {
      /// TODO: Implement expression
      P.eprintn("If expressions are not yet implemented");
      return std::nullopt;
    }

    case QIR_NODE_WHILE: {
      /// TODO: Implement expression
      P.eprintn("While expressions are not yet implemented");
      return std::nullopt;
    }

    case QIR_NODE_FOR: {
      /// TODO: Implement expression
      P.eprintn("For expressions are not yet implemented");
      return std::nullopt;
    }

    case QIR_NODE_FORM: {
      /// TODO: Implement expression
      P.eprintn("Form expressions are not yet implemented");
      return std::nullopt;
    }

    case QIR_NODE_CASE: {
      /// TODO: Implement expression
      P.eprintn("Case expressions are not yet implemented");
      return std::nullopt;
    }

    case QIR_NODE_SWITCH: {
      /// TODO: Implement expression
      P.eprintn("Switch expressions are not yet implemented");
      return std::nullopt;
    }

    case QIR_NODE_FN: {
      /// TODO: Implement expression
      P.eprintn("Function expressions are not yet implemented");
      return std::nullopt;
    }

    case QIR_NODE_ASM: {
      /// TODO: Implement expression
      P.eprintn("Asm expressions are not yet implemented");
      return std::nullopt;
    }

    case QIR_NODE_IGN: {
      P.eprintn("Unexpected 'QIR_NODE_IGN' node in program DAG");
      return std::nullopt;
    }

    case QIR_NODE_TMP: {
      P.eprintn("Unexpected 'QIR_NODE_TMP' node in program DAG");
      return std::nullopt;
    }

      ///**********************************************************************///
      ///                          PASS THROUGH NODES                          ///
      ///**********************************************************************///

    case QIR_NODE_INT:
    case QIR_NODE_FLOAT:
    case QIR_NODE_LIST:
    case QIR_NODE_U1_TY:
    case QIR_NODE_U8_TY:
    case QIR_NODE_U16_TY:
    case QIR_NODE_U32_TY:
    case QIR_NODE_U64_TY:
    case QIR_NODE_U128_TY:
    case QIR_NODE_I8_TY:
    case QIR_NODE_I16_TY:
    case QIR_NODE_I32_TY:
    case QIR_NODE_I64_TY:
    case QIR_NODE_I128_TY:
    case QIR_NODE_F16_TY:
    case QIR_NODE_F32_TY:
    case QIR_NODE_F64_TY:
    case QIR_NODE_F128_TY:
    case QIR_NODE_VOID_TY:
    case QIR_NODE_PTR_TY:
    case QIR_NODE_OPAQUE_TY:
    case QIR_NODE_STRUCT_TY:
    case QIR_NODE_UNION_TY:
    case QIR_NODE_ARRAY_TY:
    case QIR_NODE_FN_TY: {
      return x;
    }
  }
}

std::optional<nr::Expr *> nr::evaluate_impl(nr::Expr *x) noexcept {
  Program P;

  return evaluate(P, x);
}
