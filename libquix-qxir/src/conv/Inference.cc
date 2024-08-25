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

#define __QXIR_IMPL__
#include <core/LibMacro.h>
#include <quix-qxir/Inference.h>
#include <quix-qxir/Node.h>

#include <boost/multiprecision/cpp_int.hpp>

using namespace qxir;

static Type *binexpr_pinf(Expr *lhs, Expr *rhs) {
  /// TODO:
}

LIB_EXPORT qxir_node_t *qxir_infer(qxir_node_t *_node) {
  qcore_assert(_node != nullptr);
  
  Expr *E = static_cast<Expr *>(_node);

  if (E->isType()) { /* Types are expressions too; The type is their identity. */
    return E;
  }

  Type *T = nullptr;

  switch (E->getKind()) {
    case QIR_NODE_BINEXPR: {
      BinExpr *B = E->as<BinExpr>();
      switch (B->getOp()) {
        case Op::Plus: {
          T = binexpr_pinf(B->getLHS(), B->getRHS());
          break;
        }
        case Op::Minus: {
          T = binexpr_pinf(B->getLHS(), B->getRHS());
          break;
        }
        case Op::Times: {
          T = binexpr_pinf(B->getLHS(), B->getRHS());
          break;
        }
        case Op::Slash: {
          T = binexpr_pinf(B->getLHS(), B->getRHS());
          break;
        }
        case Op::Percent: {
          T = binexpr_pinf(B->getLHS(), B->getRHS());
          break;
        }
        case Op::BitAnd: {
          T = binexpr_pinf(B->getLHS(), B->getRHS());
          break;
        }
        case Op::BitOr: {
          T = binexpr_pinf(B->getLHS(), B->getRHS());
          break;
        }
        case Op::BitXor: {
          T = binexpr_pinf(B->getLHS(), B->getRHS());
          break;
        }
        case Op::BitNot: {
          T = binexpr_pinf(B->getLHS(), B->getRHS());
          break;
        }
        case Op::LogicAnd: {
          T = getType<U1Ty>();
          break;
        }
        case Op::LogicOr: {
          T = getType<U1Ty>();
          break;
        }
        case Op::LogicNot: {
          T = getType<U1Ty>();
          break;
        }
        case Op::LShift: {
          T = B->getLHS()->getType();
          break;
        }
        case Op::RShift: {
          T = B->getLHS()->getType();
          break;
        }
        case Op::ROTR: {
          T = B->getLHS()->getType();
          break;
        }
        case Op::ROTL: {
          T = B->getLHS()->getType();
          break;
        }
        case Op::Inc: {
          qcore_panic("Invalid operation: ++ is not a valid binary operation");
          break;
        }
        case Op::Dec: {
          qcore_panic("Invalid operation: -- is not a valid binary operation");
          break;
        }
        case Op::Set: {
          T = B->getRHS()->getType();
          break;
        }
        case Op::LT: {
          T = getType<U1Ty>();
          break;
        }
        case Op::GT: {
          T = getType<U1Ty>();
          break;
        }
        case Op::LE: {
          T = getType<U1Ty>();
          break;
        }
        case Op::GE: {
          T = getType<U1Ty>();
          break;
        }
        case Op::Eq: {
          T = getType<U1Ty>();
          break;
        }
        case Op::NE: {
          T = getType<U1Ty>();
          break;
        }
        case Op::Alignof: {
          qcore_panic("Invalid operation: alignof is not a valid binary operation");
          break;
        }
        case Op::Typeof: {
          qcore_panic("Invalid operation: typeof is not a valid binary operation");
          break;
        }
        case Op::Offsetof: {
          qcore_panic("Invalid operation: offsetof is not a valid binary operation");
          break;
        }
        case Op::BitcastAs: {
          T = B->getRHS()->getType();
          break;
        }
        case Op::CastAs: {
          T = B->getRHS()->getType();
          break;
        }
        case Op::Bitsizeof: {
          qcore_panic("Invalid operation: bitsizeof is not a valid binary operation");
          break;
        }
      }
      break;
    }
    case QIR_NODE_UNEXPR: {
      UnExpr *U = E->as<UnExpr>();
      switch (E->as<UnExpr>()->getOp()) {
        case Op::Plus: {
          T = U->getExpr()->getType();
          break;
        }
        case Op::Minus: {
          T = U->getExpr()->getType();
          break;
        }
        case Op::Times: {
          if (U->getExpr()->getType()->is(QIR_NODE_PTR_TY)) {
            T = U->getExpr()->getType()->as<PtrTy>()->getPointee();
          } else {
            qcore_panic("Invalid operation: * is only valid on pointers");
          }
          break;
        }
        case Op::Slash: {
          qcore_panic("Invalid operation: / is not a valid unary operation");
          break;
        }
        case Op::Percent: {
          qcore_panic("Invalid operation: % is not a valid unary operation");
          break;
        }
        case Op::BitAnd: {
          T = create<PtrTy>(U->getExpr()->getType());
          break;
        }
        case Op::BitOr: {
          qcore_panic("Invalid operation: | is not a valid unary operation");
          break;
        }
        case Op::BitXor: {
          qcore_panic("Invalid operation: ^ is not a valid unary operation");
          break;
        }
        case Op::BitNot: {
          T = U->getExpr()->getType();
          break;
        }
        case Op::LogicAnd: {
          qcore_panic("Invalid operation: && is not a valid unary operation");
          break;
        }
        case Op::LogicOr: {
          qcore_panic("Invalid operation: || is not a valid unary operation");
          break;
        }
        case Op::LogicNot: {
          T = getType<U1Ty>();
          break;
        }
        case Op::LShift: {
          qcore_panic("Invalid operation: << is not a valid unary operation");
          break;
        }
        case Op::RShift: {
          qcore_panic("Invalid operation: >> is not a valid unary operation");
          break;
        }
        case Op::ROTR: {
          qcore_panic("Invalid operation: >>> is not a valid unary operation");
          break;
        }
        case Op::ROTL: {
          qcore_panic("Invalid operation: <<< is not a valid unary operation");
          break;
        }
        case Op::Inc: {
          T = U->getExpr()->getType();
          break;
        }
        case Op::Dec: {
          T = U->getExpr()->getType();
          break;
        }
        case Op::Set: {
          qcore_panic("Invalid operation: = is not a valid unary operation");
          break;
        }
        case Op::LT: {
          qcore_panic("Invalid operation: < is not a valid unary operation");
          break;
        }
        case Op::GT: {
          qcore_panic("Invalid operation: > is not a valid unary operation");
          break;
        }
        case Op::LE: {
          qcore_panic("Invalid operation: <= is not a valid unary operation");
          break;
        }
        case Op::GE: {
          qcore_panic("Invalid operation: >= is not a valid unary operation");
          break;
        }
        case Op::Eq: {
          qcore_panic("Invalid operation: == is not a valid unary operation");
          break;
        }
        case Op::NE: {
          qcore_panic("Invalid operation: != is not a valid unary operation");
          break;
        }
        case Op::Alignof: {
          T = getType<U64Ty>();
          break;
        }
        case Op::Typeof: {
          /**
           * JUSTIFICATION: How about the typeinfo is just serialized
           * as a string with some standard format?
           *
           * That way I don't have to add a way for the type inference system
           * to create/push new types to context like C++20's std::type_info.
           *
           * The actual detail of what is encoded could be configurable.
           */

          T = getType<StringTy>();
          break;
        }
        case Op::Offsetof: {
          T = getType<U64Ty>();
          break;
        }
        case Op::BitcastAs: {
          qcore_panic("Invalid operation: bitcast_as is not a valid unary operation");
          break;
        }
        case Op::CastAs: {
          qcore_panic("Invalid operation: cast_as is not a valid unary operation");
          break;
        }
        case Op::Bitsizeof: {
          T = getType<U64Ty>();
          break;
        }
      }
      break;
    }
    case QIR_NODE_POST_UNEXPR: {
      PostUnExpr *P = E->as<PostUnExpr>();
      switch (E->as<PostUnExpr>()->getOp()) {
        case Op::Plus: {
          qcore_panic("Invalid operation: + is not a valid post-unary operation");
          break;
        }
        case Op::Minus: {
          qcore_panic("Invalid operation: - is not a valid post-unary operation");
          break;
        }
        case Op::Times: {
          qcore_panic("Invalid operation: * is not a valid post-unary operation");
          break;
        }
        case Op::Slash: {
          qcore_panic("Invalid operation: / is not a valid post-unary operation");
          break;
        }
        case Op::Percent: {
          qcore_panic("Invalid operation: % is not a valid post-unary operation");
          break;
        }
        case Op::BitAnd: {
          qcore_panic("Invalid operation: & is not a valid post-unary operation");
          break;
        }
        case Op::BitOr: {
          qcore_panic("Invalid operation: | is not a valid post-unary operation");
          break;
        }
        case Op::BitXor: {
          qcore_panic("Invalid operation: ^ is not a valid post-unary operation");
          break;
        }
        case Op::BitNot: {
          qcore_panic("Invalid operation: ~ is not a valid post-unary operation");
          break;
        }
        case Op::LogicAnd: {
          qcore_panic("Invalid operation: && is not a valid post-unary operation");
          break;
        }
        case Op::LogicOr: {
          qcore_panic("Invalid operation: || is not a valid post-unary operation");
          break;
        }
        case Op::LogicNot: {
          qcore_panic("Invalid operation: ! is not a valid post-unary operation");
          break;
        }
        case Op::LShift: {
          qcore_panic("Invalid operation: << is not a valid post-unary operation");
          break;
        }
        case Op::RShift: {
          qcore_panic("Invalid operation: >> is not a valid post-unary operation");
          break;
        }
        case Op::ROTR: {
          qcore_panic("Invalid operation: >>> is not a valid post-unary operation");
          break;
        }
        case Op::ROTL: {
          qcore_panic("Invalid operation: <<< is not a valid post-unary operation");
          break;
        }
        case Op::Inc: {
          T = P->getExpr()->getType();
          break;
        }
        case Op::Dec: {
          T = P->getExpr()->getType();
          break;
        }
        case Op::Set: {
          qcore_panic("Invalid operation: = is not a valid post-unary operation");
          break;
        }
        case Op::LT: {
          qcore_panic("Invalid operation: < is not a valid post-unary operation");
          break;
        }
        case Op::GT: {
          qcore_panic("Invalid operation: > is not a valid post-unary operation");
          break;
        }
        case Op::LE: {
          qcore_panic("Invalid operation: <= is not a valid post-unary operation");
          break;
        }
        case Op::GE: {
          qcore_panic("Invalid operation: >= is not a valid post-unary operation");
          break;
        }
        case Op::Eq: {
          qcore_panic("Invalid operation: == is not a valid post-unary operation");
          break;
        }
        case Op::NE: {
          qcore_panic("Invalid operation: != is not a valid post-unary operation");
          break;
        }
        case Op::Alignof: {
          qcore_panic("Invalid operation: alignof is not a valid post-unary operation");
          break;
        }
        case Op::Typeof: {
          qcore_panic("Invalid operation: typeof is not a valid post-unary operation");
          break;
        }
        case Op::Offsetof: {
          qcore_panic("Invalid operation: offsetof is not a valid post-unary operation");
          break;
        }
        case Op::BitcastAs: {
          qcore_panic("Invalid operation: bitcast_as is not a valid post-unary operation");
          break;
        }
        case Op::CastAs: {
          qcore_panic("Invalid operation: cast_as is not a valid post-unary operation");
          break;
        }
        case Op::Bitsizeof: {
          qcore_panic("Invalid operation: bitsizeof is not a valid post-unary operation");
          break;
        }
      }
      break;
    }
    case QIR_NODE_INT: {
      Int *I = E->as<Int>();

      if (I->isNativeRepresentation()) {
        if (I->getNativeRepresentation() > UINT32_MAX) {
          T = getType<U64Ty>();
        } else {
          T = getType<U32Ty>();
        }
      } else {
        std::string_view val = I->getStringRepresentation();
        boost::multiprecision::cpp_int num(val.data());
        if (num > UINT64_MAX) {
          T = getType<U128Ty>();
        } else if (num > UINT32_MAX) {
          T = getType<U64Ty>();
        } else {
          T = getType<U32Ty>();
        }
      }
      break;
    }
    case QIR_NODE_FLOAT: {
      Float *F = E->as<Float>();

      if (F->isNativeRepresentation()) {
        return getType<F64Ty>();
      } else {
        std::string_view val = F->getStringRepresentation();
        if (val.ends_with("f32")) {
          T = getType<F32Ty>();
        } else if (val.ends_with("f64")) {
          T = getType<F64Ty>();
        } else if (val.ends_with("f128")) {
          T = getType<F128Ty>();
        } else {
          qcore_panicf("Unknown floating-point type: %s", val.data());
        }
      }
      break;
    }
    case QIR_NODE_STRING: {
      T = getType<StringTy>();
      break;
    }
    case QIR_NODE_LIST: {
      if (E->as<List>()->getItems().empty()) {
        T = create<ListTy>(getType<VoidTy>());
      } else {
        std::vector<Type *> types;
        for (auto &item : E->as<List>()->getItems()) {
          types.push_back(item->getType());
        }

        bool homogeneous = std::all_of(types.begin(), types.end(),
                                       [&](Type *X) { return X->cmp_eq(types.front()); });

        if (homogeneous) {
          T = create<ListTy>(types.front());
        } else {
          T = create<StructTy>(std::move(types));
        }
      }
      break;
    }
    case QIR_NODE_ALLOC: {
      T = create<PtrTy>(E->as<Alloc>()->getAllocType());
      break;
    }
    case QIR_NODE_CALL: {
      T = E->as<Call>()->getFn()->getReturn();
      break;
    }
    case QIR_NODE_SEQ: {
      if (E->as<Seq>()->getItems().empty()) {
        T = getType<VoidTy>();
      } else {
        T = E->as<Seq>()->getItems().back()->getType();
      }
      break;
    }
    case QIR_NODE_ASYNC: {
      T = getType<VoidTy>();
      break;
    }
    case QIR_NODE_INDEX: {
      Type *B = E->as<Index>()->getExpr()->getType();
      Expr *V = E->as<Index>()->getIndex();

      if (B->is(QIR_NODE_PTR_TY)) {  // *X -> X
        T = B->as<PtrTy>()->getPointee();
      } else if (B->is(QIR_NODE_LIST_TY)) {  // [X] -> X
        T = B->as<ListTy>()->getElement();
      } else if (B->is(QIR_NODE_ARRAY_TY)) {  // [X; N] -> X
        T = B->as<ArrayTy>()->getElement();
      } else if (B->is(QIR_NODE_STRING_TY)) {  // string -> u8
        T = getType<U8Ty>();
      } else if (B->is(QIR_NODE_STRUCT_TY)) {  // struct { a, b, c } -> a | b | c
        if (!V->is(QIR_NODE_INT)) {
          qcore_panic("Invalid must be of type int to index into a struct");
        }

        if (V->as<Int>()->isNativeRepresentation()) {
          auto idx = V->as<Int>()->getNativeRepresentation();
          if (idx >= B->as<StructTy>()->getFields().size()) {
            qcore_panicf("Index out of bounds: %d", idx);
          }

          T = B->as<StructTy>()->getFields()[idx];
        } else {
          std::string_view val = V->as<Int>()->getStringRepresentation();
          boost::multiprecision::cpp_int num(val.data());
          if (num >= B->as<StructTy>()->getFields().size()) {
            qcore_panicf("Index out of bounds: %s", val.data());
          }

          T = B->as<StructTy>()->getFields()[num.convert_to<std::size_t>()];
        }
      } else if (B->is(QIR_NODE_UNION_TY)) {
        if (!V->is(QIR_NODE_INT)) {
          qcore_panic("Invalid must be of type int to index into a union");
        }

        if (V->as<Int>()->isNativeRepresentation()) {
          auto idx = V->as<Int>()->getNativeRepresentation();
          if (idx >= B->as<UnionTy>()->getFields().size()) {
            qcore_panicf("Index out of bounds: %d", idx);
          }

          T = B->as<UnionTy>()->getFields()[idx];
        } else {
          std::string_view val = V->as<Int>()->getStringRepresentation();
          boost::multiprecision::cpp_int num(val.data());
          if (num >= B->as<UnionTy>()->getFields().size()) {
            qcore_panicf("Index out of bounds: %s", val.data());
          }

          T = B->as<UnionTy>()->getFields()[num.convert_to<std::size_t>()];
        }
      } else {
        qcore_panic("Invalid index operation");
      }
      break;
    }
    case QIR_NODE_IDENT: {
      T = E->as<Ident>()->getWhat()->getType();
      break;
    }
    case QIR_NODE_EXPORT: {
      T = getType<VoidTy>();
      break;
    }
    case QIR_NODE_LOCAL: {
      T = E->as<Local>()->getValue()->getType();
      break;
    }
    case QIR_NODE_RET: {
      T = getType<VoidTy>();
      break;
    }
    case QIR_NODE_BRK: {
      T = getType<VoidTy>();
      break;
    }
    case QIR_NODE_CONT: {
      T = getType<VoidTy>();
      break;
    }
    case QIR_NODE_IF: {
      T = getType<VoidTy>();
      break;
    }
    case QIR_NODE_WHILE: {
      T = getType<VoidTy>();
      break;
    }
    case QIR_NODE_FOR: {
      T = getType<VoidTy>();
      break;
    }
    case QIR_NODE_FORM: {
      T = getType<VoidTy>();
      break;
    }
    case QIR_NODE_FOREACH: {
      T = getType<VoidTy>();
      break;
    }
    case QIR_NODE_CASE: {
      T = getType<VoidTy>();
      break;
    }
    case QIR_NODE_SWITCH: {
      T = getType<VoidTy>();
      break;
    }
    case QIR_NODE_FN: {
      FnParams params;
      for (auto &param : E->as<Fn>()->getParams()) {
        params.push_back(param->getType());
      }

      FnAttrs attrs;
      T = create<FnTy>(std::move(params), E->as<Fn>()->getBody()->getType(), std::move(attrs));
      break;
    }
    case QIR_NODE_ASM: {
      T = getType<VoidTy>();
      break;
    }
    default: {
      qcore_panicf("Unknown node kind: %d", E->getKind());
      break;
    }
  }

  return T;
}
