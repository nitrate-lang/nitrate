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

static bool is_unsigned_integer(qxir_ty_t ty) {
  switch (ty) {
    case QIR_NODE_U1_TY:
    case QIR_NODE_U8_TY:
    case QIR_NODE_U16_TY:
    case QIR_NODE_U32_TY:
    case QIR_NODE_U64_TY:
    case QIR_NODE_U128_TY:
      return true;
    default:
      return false;
  }
}

static bool is_signed_integer(qxir_ty_t ty) {
  switch (ty) {
    case QIR_NODE_I8_TY:
    case QIR_NODE_I16_TY:
    case QIR_NODE_I32_TY:
    case QIR_NODE_I64_TY:
    case QIR_NODE_I128_TY:
      return true;
    default:
      return false;
  }
}

static bool is_primitive_numeric(qxir_ty_t ty) {
  switch (ty) {
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
      return true;
    default:
      return false;
  }
}

static Type *signed_complement(qxir_ty_t ty) {
  switch (ty) {
    case QIR_NODE_I8_TY:
      return getType<U8Ty>();
    case QIR_NODE_I16_TY:
      return getType<U16Ty>();
    case QIR_NODE_I32_TY:
      return getType<U32Ty>();
    case QIR_NODE_I64_TY:
      return getType<U64Ty>();
    case QIR_NODE_I128_TY:
      return getType<U128Ty>();
    default:
      return nullptr;
  }
}

static Type *binexpr_promote(Type *L, Type *R) {
  ///===========================================================================
  /// NOTE: If L && R are the same type, the type is their identity.
  if (L->cmp_eq(R)) {
    return L;
  }
  ///===========================================================================

  qxir_ty_t LT = L->getKind(), RT = R->getKind();

  ///===========================================================================
  /// NOTE: Primitive numeric types are promoted according to the following rules:
  if (is_primitive_numeric(LT) && is_primitive_numeric(RT)) {
    ///===========================================================================
    /// NOTE: Floating point always takes precedence over integers.
    if (L->is(QIR_NODE_VOID_TY) || R->is(QIR_NODE_VOID_TY)) {
      return nullptr;
    }

    if (L->is(QIR_NODE_F128_TY) || R->is(QIR_NODE_F128_TY)) {
      return getType<F128Ty>();
    }

    if (L->is(QIR_NODE_F64_TY) || R->is(QIR_NODE_F64_TY)) {
      return getType<F64Ty>();
    }

    if (L->is(QIR_NODE_F32_TY) || R->is(QIR_NODE_F32_TY)) {
      return getType<F32Ty>();
    }

    if (L->is(QIR_NODE_F16_TY) || R->is(QIR_NODE_F16_TY)) {
      return getType<F16Ty>();
    }
    ///===========================================================================

    ///===========================================================================
    /// NOTE: If L && R are both unsigned integers, the larger type is used.
    if (is_unsigned_integer(LT) && is_unsigned_integer(RT)) {
      size_t LS = L->getSizeBits(), RS = R->getSizeBits();
      return LS > RS ? L : R;
    }
    ///===========================================================================

    ///===========================================================================
    /// NOTE: If L && R are both signed integers, the larger type is used.
    if (is_signed_integer(LT) && is_signed_integer(RT)) {
      size_t LS = L->getSizeBits(), RS = R->getSizeBits();
      return LS > RS ? L : R;
    }
    ///===========================================================================

    ///===========================================================================
    /// NOTE: If either L or R is a signed integer, the signed integer is promoted.
    if (is_signed_integer(LT)) {
      size_t LS = L->getSizeBits(), RS = R->getSizeBits();
      if (LS > RS) {
        return signed_complement(LT);
      } else {
        return R;
      }
    } else if (is_signed_integer(RT)) {
      size_t LS = L->getSizeBits(), RS = R->getSizeBits();
      if (RS > LS) {
        return signed_complement(RT);
      } else {
        return L;
      }
    } else {
      qcore_assert(false, "Unreachable");
    }
    ///===========================================================================
  }
  ///===========================================================================

  else

  ///===========================================================================
  /// NOTE: Non-primitive numeric types are promoted according to the following rules:
  {
    /// TODO: Non-primitive numeric type promotion
    qcore_implement("Non-primitive numeric type promotion");

    /**
     * @note
     *
     * 1. User defined type conversion
     * 2. Complex string expressions like `"HELLO" * 2` == `"HELLOHELLO"`
     * 3. Complex list expressions like `[1, 2, 3] * 2` == `[2, 4, 6]` or `[1, 2, 3] + [4, 5, 6] ==
     * [1, 2, 3, 4, 5, 6]`
     * 4. Everything else gets either an error or void?
     */
  }
  ///===========================================================================
}

LIB_EXPORT qxir_node_t *qxir_infer(qxir_node_t *_node) {
  qcore_assert(_node != nullptr);

  Expr *E = static_cast<Expr *>(_node);
  Type *T = nullptr;

  if (E->isType()) {
    T = E->as<Type>();
  } else {
    switch (E->getKind()) {
      case QIR_NODE_BINEXPR: {
        BinExpr *B = E->as<BinExpr>();
        switch (B->getOp()) {
          case Op::Plus: {
            T = binexpr_promote(B->getLHS()->getType(), B->getRHS()->getType());
            break;
          }
          case Op::Minus: {
            T = binexpr_promote(B->getLHS()->getType(), B->getRHS()->getType());
            break;
          }
          case Op::Times: {
            T = binexpr_promote(B->getLHS()->getType(), B->getRHS()->getType());
            break;
          }
          case Op::Slash: {
            T = binexpr_promote(B->getLHS()->getType(), B->getRHS()->getType());
            break;
          }
          case Op::Percent: {
            T = binexpr_promote(B->getLHS()->getType(), B->getRHS()->getType());
            break;
          }
          case Op::BitAnd: {
            T = binexpr_promote(B->getLHS()->getType(), B->getRHS()->getType());
            break;
          }
          case Op::BitOr: {
            T = binexpr_promote(B->getLHS()->getType(), B->getRHS()->getType());
            break;
          }
          case Op::BitXor: {
            T = binexpr_promote(B->getLHS()->getType(), B->getRHS()->getType());
            break;
          }
          case Op::BitNot: {
            T = binexpr_promote(B->getLHS()->getType(), B->getRHS()->getType());
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
            T = nullptr;  // Illegal
            break;
          }
          case Op::Dec: {
            T = nullptr;  // Illegal
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
            T = nullptr;  // Illegal
            break;
          }
          case Op::Typeof: {
            T = nullptr;  // Illegal
            break;
          }
          case Op::Offsetof: {
            T = nullptr;  // Illegal
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
            T = nullptr;  // Illegal
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
              T = nullptr;  // Invalid operation: * is only valid on pointers
            }
            break;
          }
          case Op::Slash: {
            T = nullptr;  // Illegal
            break;
          }
          case Op::Percent: {
            T = nullptr;  // Illegal
            break;
          }
          case Op::BitAnd: {
            T = create<PtrTy>(U->getExpr()->getType());
            break;
          }
          case Op::BitOr: {
            T = nullptr;  // Illegal
            break;
          }
          case Op::BitXor: {
            T = nullptr;  // Illegal
            break;
          }
          case Op::BitNot: {
            T = U->getExpr()->getType();
            break;
          }
          case Op::LogicAnd: {
            T = nullptr;  // Illegal
            break;
          }
          case Op::LogicOr: {
            T = nullptr;  // Illegal
            break;
          }
          case Op::LogicNot: {
            T = getType<U1Ty>();
            break;
          }
          case Op::LShift: {
            T = nullptr;  // Illegal
            break;
          }
          case Op::RShift: {
            T = nullptr;  // Illegal
            break;
          }
          case Op::ROTR: {
            T = nullptr;  // Illegal
            break;
          }
          case Op::ROTL: {
            T = nullptr;  // Illegal
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
            T = nullptr;  // Illegal
            break;
          }
          case Op::LT: {
            T = nullptr;  // Illegal
            break;
          }
          case Op::GT: {
            T = nullptr;  // Illegal
            break;
          }
          case Op::LE: {
            T = nullptr;  // Illegal
            break;
          }
          case Op::GE: {
            T = nullptr;  // Illegal
            break;
          }
          case Op::Eq: {
            T = nullptr;  // Illegal
            break;
          }
          case Op::NE: {
            T = nullptr;  // Illegal
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
            T = nullptr;  // Illegal
            break;
          }
          case Op::CastAs: {
            T = nullptr;  // Illegal
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
            T = nullptr;  // Illegal
            break;
          }
          case Op::Minus: {
            T = nullptr;  // Illegal
            break;
          }
          case Op::Times: {
            T = nullptr;  // Illegal
            break;
          }
          case Op::Slash: {
            T = nullptr;  // Illegal
            break;
          }
          case Op::Percent: {
            T = nullptr;  // Illegal
            break;
          }
          case Op::BitAnd: {
            T = nullptr;  // Illegal
            break;
          }
          case Op::BitOr: {
            T = nullptr;  // Illegal
            break;
          }
          case Op::BitXor: {
            T = nullptr;  // Illegal
            break;
          }
          case Op::BitNot: {
            T = nullptr;  // Illegal
            break;
          }
          case Op::LogicAnd: {
            T = nullptr;  // Illegal
            break;
          }
          case Op::LogicOr: {
            T = nullptr;  // Illegal
            break;
          }
          case Op::LogicNot: {
            T = nullptr;  // Illegal
            break;
          }
          case Op::LShift: {
            T = nullptr;  // Illegal
            break;
          }
          case Op::RShift: {
            T = nullptr;  // Illegal
            break;
          }
          case Op::ROTR: {
            T = nullptr;  // Illegal
            break;
          }
          case Op::ROTL: {
            T = nullptr;  // Illegal
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
            T = nullptr;  // Illegal
            break;
          }
          case Op::LT: {
            T = nullptr;  // Illegal
            break;
          }
          case Op::GT: {
            T = nullptr;  // Illegal
            break;
          }
          case Op::LE: {
            T = nullptr;  // Illegal
            break;
          }
          case Op::GE: {
            T = nullptr;  // Illegal
            break;
          }
          case Op::Eq: {
            T = nullptr;  // Illegal
            break;
          }
          case Op::NE: {
            T = nullptr;  // Illegal
            break;
          }
          case Op::Alignof: {
            T = nullptr;  // Illegal
            break;
          }
          case Op::Typeof: {
            T = nullptr;  // Illegal
            break;
          }
          case Op::Offsetof: {
            T = nullptr;  // Illegal
            break;
          }
          case Op::BitcastAs: {
            T = nullptr;  // Illegal
            break;
          }
          case Op::CastAs: {
            T = nullptr;  // Illegal
            break;
          }
          case Op::Bitsizeof: {
            T = nullptr;  // Illegal
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
          T = getType<F64Ty>();
        } else {
          std::string_view val = F->getStringRepresentation();
          if (val.ends_with("f32")) {
            T = getType<F32Ty>();
          } else if (val.ends_with("f64")) {
            T = getType<F64Ty>();
          } else if (val.ends_with("f128")) {
            T = getType<F128Ty>();
          } else {
            T = nullptr;  // Unknown floating-point type suffix
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
            T = create<StructTy>(StructFields(types.begin(), types.end()));
          }
        }
        break;
      }
      case QIR_NODE_ALLOC: {
        T = create<PtrTy>(E->as<Alloc>()->getAllocType());
        break;
      }
      case QIR_NODE_CALL: {
        T = E->as<Call>()->getTarget()->getType()->as<FnTy>()->getReturn();
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
            T = nullptr;  // Invalid must be of type int to index into a struct
          } else {
            if (V->as<Int>()->isNativeRepresentation()) {
              auto idx = V->as<Int>()->getNativeRepresentation();
              if (idx < B->as<StructTy>()->getFields().size()) {
                T = B->as<StructTy>()->getFields()[idx];
              } else {
                T = nullptr;  // Invalid out of bounds
              }

            } else {
              std::string_view val = V->as<Int>()->getStringRepresentation();
              boost::multiprecision::cpp_int num(val.data());
              if (num < B->as<StructTy>()->getFields().size()) {
                T = B->as<StructTy>()->getFields()[num.convert_to<std::size_t>()];
              } else {
                T = nullptr;  // Invalid out of bounds
              }
            }
          }
        } else if (B->is(QIR_NODE_UNION_TY)) {
          if (!V->is(QIR_NODE_INT)) {
            T = nullptr;  // Invalid must be of type int to index into a union
          } else {
            if (V->as<Int>()->isNativeRepresentation()) {
              auto idx = V->as<Int>()->getNativeRepresentation();
              if (idx < B->as<UnionTy>()->getFields().size()) {
                T = B->as<UnionTy>()->getFields()[idx];
              } else {
                T = nullptr;  // Invalid out of bounds
              }

            } else {
              std::string_view val = V->as<Int>()->getStringRepresentation();
              boost::multiprecision::cpp_int num(val.data());
              if (num < B->as<UnionTy>()->getFields().size()) {
                T = B->as<UnionTy>()->getFields()[num.convert_to<std::size_t>()];
              } else {
                T = nullptr;  // Invalid out of bounds
              }
            }
          }
        } else {
          T = nullptr;  // Invalid type to index into
        }
        break;
      }
      case QIR_NODE_IDENT: {
        T = E->as<Ident>()->getWhat()->getType();
        break;
      }
      case QIR_NODE_EXTERN: {
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
      case QIR_NODE_TMP: {
        qcore_panic("Temporary nodes must be resolved with more information prior to inference");
        break;
      }
      default: {
        T = nullptr;  // Unknown node kind
        break;
      }
    }
  }

  return T;
}

bool qxir::Type::hasKnownSize() noexcept {
  switch (this->getKind()) {
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
    case QIR_NODE_FN_TY:
      return true;
    case QIR_NODE_STRUCT_TY: {
      return std::all_of(this->as<StructTy>()->getFields().begin(),
                         this->as<StructTy>()->getFields().end(),
                         [](Type *T) { return T->hasKnownSize(); });
    }
    case QIR_NODE_UNION_TY: {
      return std::all_of(this->as<UnionTy>()->getFields().begin(),
                         this->as<UnionTy>()->getFields().end(),
                         [](Type *T) { return T->hasKnownSize(); });
    }
    case QIR_NODE_ARRAY_TY: {
      return this->as<ArrayTy>()->getElement()->hasKnownSize() &&
             this->as<ArrayTy>()->getCount()->isConst();
    }
    case QIR_NODE_OPAQUE_TY:
    case QIR_NODE_STRING_TY:
    case QIR_NODE_LIST_TY:
    case QIR_NODE_INTRIN_TY:
      return false;
    default: {
      qcore_panicf("Invalid type kind: %d", this->getKind());
    }
  }
}

CPP_EXPORT uint64_t qxir::Type::getSizeBits() {
  qcore_assert(this->hasKnownSize(), "Attempted to get the size of a type with an unknown size");

  uint64_t size;

  switch (this->getKind()) {
    case QIR_NODE_U1_TY: {
      size = 8;
      break;
    }
    case QIR_NODE_U8_TY: {
      size = 8;
      break;
    }
    case QIR_NODE_U16_TY: {
      size = 16;
      break;
    }
    case QIR_NODE_U32_TY: {
      size = 32;
      break;
    }
    case QIR_NODE_U64_TY: {
      size = 64;
      break;
    }
    case QIR_NODE_U128_TY: {
      size = 128;
      break;
    }
    case QIR_NODE_I8_TY: {
      size = 8;
      break;
    }
    case QIR_NODE_I16_TY: {
      size = 16;
      break;
    }
    case QIR_NODE_I32_TY: {
      size = 32;
      break;
    }
    case QIR_NODE_I64_TY: {
      size = 64;
      break;
    }
    case QIR_NODE_I128_TY: {
      size = 128;
      break;
    }
    case QIR_NODE_F16_TY: {
      size = 16;
      break;
    }
    case QIR_NODE_F32_TY: {
      size = 32;
      break;
    }
    case QIR_NODE_F64_TY: {
      size = 64;
      break;
    }
    case QIR_NODE_F128_TY: {
      size = 128;
      break;
    }
    case QIR_NODE_VOID_TY: {
      size = 0;
      break;
    }
    case QIR_NODE_PTR_TY: {
      size = getModule()->getTargetInfo().getPointerSize();
      break;
    }
    case QIR_NODE_STRUCT_TY: {
      size = 0;
      for (auto &field : this->as<StructTy>()->getFields()) {
        size += field->getSizeBits();
      }
      break;
    }
    case QIR_NODE_UNION_TY: {
      size = 0;
      for (auto &field : this->as<UnionTy>()->getFields()) {
        size = std::max(size, field->getSizeBits());
      }
      break;
    }
    case QIR_NODE_ARRAY_TY: {
      std::optional<uint64_t> element_size =
          qxir::uint_as<uint64_t>(this->as<ArrayTy>()->getCount());

      if (!element_size.has_value()) {
        qcore_panic("Array type size calculation failed");
      }

      size = this->as<ArrayTy>()->getElement()->getSizeBits() * element_size.value();
      break;
    }
    case QIR_NODE_FN_TY: {
      size = getModule()->getTargetInfo().getPointerSize();
      break;
    }
    default: {
      qcore_panicf("Invalid type kind: %d", this->getKind());
    }
  }

  return size;
}
