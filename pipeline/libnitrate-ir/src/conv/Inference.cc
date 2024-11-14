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

#include <boost/multiprecision/cpp_int.hpp>
#include <cstdint>
#include <nitrate-ir/IRGraph.hh>

using namespace nr;

static bool is_unsigned_integer(nr_ty_t ty) {
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

static bool is_signed_integer(nr_ty_t ty) {
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

static bool is_primitive_numeric(nr_ty_t ty) {
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

static Type *signed_complement(nr_ty_t ty) {
  switch (ty) {
    case QIR_NODE_I8_TY:
      return create<U8Ty>();
    case QIR_NODE_I16_TY:
      return create<U16Ty>();
    case QIR_NODE_I32_TY:
      return create<U32Ty>();
    case QIR_NODE_I64_TY:
      return create<U64Ty>();
    case QIR_NODE_I128_TY:
      return create<U128Ty>();
    default:
      return nullptr;
  }
}

static Type *binexpr_promote(Type *L, Type *R, uint32_t PtrSizeBytes) {
  if (L == nullptr || R == nullptr) {
    return nullptr;
  }

  ///===========================================================================
  /// NOTE: If L && R are the same type, the type is their identity.
  if (L->isSame(R)) {
    return L;
  }
  ///===========================================================================

  nr_ty_t LT = L->getKind(), RT = R->getKind();

  ///===========================================================================
  /// NOTE: Primitive numeric types are promoted according to the following rules:
  if (is_primitive_numeric(LT) && is_primitive_numeric(RT)) {
    ///===========================================================================
    /// NOTE: Floating point always takes precedence over integers.
    if (L->is(QIR_NODE_VOID_TY) || R->is(QIR_NODE_VOID_TY)) {
      return nullptr;
    }

    if (L->is(QIR_NODE_F128_TY) || R->is(QIR_NODE_F128_TY)) {
      return create<F128Ty>();
    }

    if (L->is(QIR_NODE_F64_TY) || R->is(QIR_NODE_F64_TY)) {
      return create<F64Ty>();
    }

    if (L->is(QIR_NODE_F32_TY) || R->is(QIR_NODE_F32_TY)) {
      return create<F32Ty>();
    }

    if (L->is(QIR_NODE_F16_TY) || R->is(QIR_NODE_F16_TY)) {
      return create<F16Ty>();
    }
    ///===========================================================================

    ///===========================================================================
    /// NOTE: If L && R are both unsigned integers, the larger type is used.
    if (is_unsigned_integer(LT) && is_unsigned_integer(RT)) {
      size_t LS = L->getSizeBits(PtrSizeBytes), RS = R->getSizeBits(PtrSizeBytes);
      return LS > RS ? L : R;
    }
    ///===========================================================================

    ///===========================================================================
    /// NOTE: If L && R are both signed integers, the larger type is used.
    if (is_signed_integer(LT) && is_signed_integer(RT)) {
      size_t LS = L->getSizeBits(PtrSizeBytes), RS = R->getSizeBits(PtrSizeBytes);
      return LS > RS ? L : R;
    }
    ///===========================================================================

    ///===========================================================================
    /// NOTE: If either L or R is a signed integer, the signed integer is promoted.
    if (is_signed_integer(LT)) {
      size_t LS = L->getSizeBits(PtrSizeBytes), RS = R->getSizeBits(PtrSizeBytes);
      if (LS > RS) {
        return signed_complement(LT);
      } else {
        return R;
      }
    } else if (is_signed_integer(RT)) {
      size_t LS = L->getSizeBits(PtrSizeBytes), RS = R->getSizeBits(PtrSizeBytes);
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

  else {
    return nullptr;
  }
}

LIB_EXPORT nr_node_t *nr_infer(nr_node_t *_node, uint32_t PtrSizeBytes) {
  qcore_assert(_node != nullptr);

  Expr *E = static_cast<Expr *>(_node);
  Type *T = nullptr;

  if (E->isType()) {
    T = E->asType();
  } else {
    switch (E->getKind()) {
      case QIR_NODE_BINEXPR: {
        BinExpr *B = E->as<BinExpr>();
        switch (B->getOp()) {
          case Op::Plus: {
            T = binexpr_promote(B->getLHS()->getType().value_or(nullptr),
                                B->getRHS()->getType().value_or(nullptr), PtrSizeBytes);
            break;
          }
          case Op::Minus: {
            T = binexpr_promote(B->getLHS()->getType().value_or(nullptr),
                                B->getRHS()->getType().value_or(nullptr), PtrSizeBytes);
            break;
          }
          case Op::Times: {
            T = binexpr_promote(B->getLHS()->getType().value_or(nullptr),
                                B->getRHS()->getType().value_or(nullptr), PtrSizeBytes);
            break;
          }
          case Op::Slash: {
            T = binexpr_promote(B->getLHS()->getType().value_or(nullptr),
                                B->getRHS()->getType().value_or(nullptr), PtrSizeBytes);
            break;
          }
          case Op::Percent: {
            T = binexpr_promote(B->getLHS()->getType().value_or(nullptr),
                                B->getRHS()->getType().value_or(nullptr), PtrSizeBytes);
            break;
          }
          case Op::BitAnd: {
            T = binexpr_promote(B->getLHS()->getType().value_or(nullptr),
                                B->getRHS()->getType().value_or(nullptr), PtrSizeBytes);
            break;
          }
          case Op::BitOr: {
            T = binexpr_promote(B->getLHS()->getType().value_or(nullptr),
                                B->getRHS()->getType().value_or(nullptr), PtrSizeBytes);
            break;
          }
          case Op::BitXor: {
            T = binexpr_promote(B->getLHS()->getType().value_or(nullptr),
                                B->getRHS()->getType().value_or(nullptr), PtrSizeBytes);
            break;
          }
          case Op::BitNot: {
            T = binexpr_promote(B->getLHS()->getType().value_or(nullptr),
                                B->getRHS()->getType().value_or(nullptr), PtrSizeBytes);
            break;
          }
          case Op::LogicAnd: {
            T = create<U1Ty>();
            break;
          }
          case Op::LogicOr: {
            T = create<U1Ty>();
            break;
          }
          case Op::LogicNot: {
            T = create<U1Ty>();
            break;
          }
          case Op::LShift: {
            T = B->getLHS()->getType().value_or(nullptr);
            break;
          }
          case Op::RShift: {
            T = B->getLHS()->getType().value_or(nullptr);
            break;
          }
          case Op::ROTR: {
            T = B->getLHS()->getType().value_or(nullptr);
            break;
          }
          case Op::ROTL: {
            T = B->getLHS()->getType().value_or(nullptr);
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
            T = B->getRHS()->getType().value_or(nullptr);
            break;
          }
          case Op::LT: {
            T = create<U1Ty>();
            break;
          }
          case Op::GT: {
            T = create<U1Ty>();
            break;
          }
          case Op::LE: {
            T = create<U1Ty>();
            break;
          }
          case Op::GE: {
            T = create<U1Ty>();
            break;
          }
          case Op::Eq: {
            T = create<U1Ty>();
            break;
          }
          case Op::NE: {
            T = create<U1Ty>();
            break;
          }
          case Op::Alignof: {
            T = nullptr;  // Illegal
            break;
          }
          case Op::BitcastAs: {
            T = B->getRHS()->getType().value_or(nullptr);
            break;
          }
          case Op::CastAs: {
            T = B->getRHS()->getType().value_or(nullptr);
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
            T = U->getExpr()->getType().value_or(nullptr);
            break;
          }
          case Op::Minus: {
            T = U->getExpr()->getType().value_or(nullptr);
            break;
          }
          case Op::Times: {
            Type *x = U->getExpr()->getType().value_or(nullptr);
            if (!x) {
              T = nullptr;
            } else if (x->is(QIR_NODE_PTR_TY)) {
              T = x->as<PtrTy>()->getPointee();
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
            Type *x = U->getExpr()->getType().value_or(nullptr);
            if (x) {
              T = create<PtrTy>(x);
            } else {
              T = nullptr;
            }
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
            T = U->getExpr()->getType().value_or(nullptr);
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
            T = create<U1Ty>();
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
            T = U->getExpr()->getType().value_or(nullptr);
            break;
          }
          case Op::Dec: {
            T = U->getExpr()->getType().value_or(nullptr);
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
            T = create<U64Ty>();
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
            T = create<U64Ty>();
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
            T = P->getExpr()->getType().value_or(nullptr);
            break;
          }
          case Op::Dec: {
            T = P->getExpr()->getType().value_or(nullptr);
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

        nr::uint128_t num = I->getValue();

        if (num > UINT64_MAX) {
          T = create<I128Ty>();
        } else if (num > UINT32_MAX) {
          T = create<I64Ty>();
        } else {
          T = create<I32Ty>();
        }
        break;
      }
      case QIR_NODE_FLOAT: {
        Float *F = E->as<Float>();

        switch (F->getSize()) {
          case nr::FloatSize::F16:
            T = create<F16Ty>();
            break;
          case nr::FloatSize::F32:
            T = create<F32Ty>();
            break;
          case nr::FloatSize::F64:
            T = create<F64Ty>();
            break;
          case nr::FloatSize::F128:
            T = create<F128Ty>();
            break;
        }

        break;
      }
      case QIR_NODE_LIST: {
        if (E->as<List>()->getItems().empty()) {
          T = create<StructTy>(StructFields());
        } else {
          std::vector<Type *> types;
          for (auto &item : E->as<List>()->getItems()) {
            Type *x = item->getType().value_or(nullptr);
            if (!x) {
              T = nullptr;
              break;
            }
            types.push_back(x);
          }

          bool homogeneous = std::all_of(types.begin(), types.end(),
                                         [&](Type *X) { return X->isSame(types.front()); });

          if (homogeneous) {
            T = create<ArrayTy>(types.front(), types.size());
          } else {
            T = create<StructTy>(StructFields(types.begin(), types.end()));
          }
        }
        break;
      }
      case QIR_NODE_CALL: {
        Type *x = E->as<Call>()->getTarget()->getType().value_or(nullptr);
        if (!x) {
          T = nullptr;
        } else {
          qcore_assert(x->getKind() == QIR_NODE_FN_TY, "Call target must be a function");
          T = x->as<FnTy>()->getReturn();
        }
        break;
      }
      case QIR_NODE_SEQ: {
        if (E->as<Seq>()->getItems().empty()) {
          T = create<VoidTy>();
        } else {
          T = E->as<Seq>()->getItems().back()->getType().value_or(nullptr);
        }
        break;
      }
      case QIR_NODE_INDEX: {
        Type *B = E->as<Index>()->getExpr()->getType().value_or(nullptr);
        if (!B) {
          T = nullptr;
          break;
        }
        Expr *V = E->as<Index>()->getIndex();

        if (B->is(QIR_NODE_PTR_TY)) {  // *X -> X
          T = B->as<PtrTy>()->getPointee();
        } else if (B->is(QIR_NODE_ARRAY_TY)) {  // [X; N] -> X
          T = B->as<ArrayTy>()->getElement();
        } else if (B->is(QIR_NODE_STRUCT_TY)) {  // struct { a, b, c } -> a | b | c
          if (!V->is(QIR_NODE_INT)) {
            T = nullptr;  // Invalid must be of type int to index into a struct
          } else {
            nr::uint128_t num = V->as<Int>()->getValue();
            if (num < B->as<StructTy>()->getFields().size()) {
              T = B->as<StructTy>()->getFields()[num.convert_to<std::size_t>()];
            } else {
              T = nullptr;  // Invalid out of bounds
            }
          }
        } else if (B->is(QIR_NODE_UNION_TY)) {
          if (!V->is(QIR_NODE_INT)) {
            T = nullptr;  // Invalid must be of type int to index into a union
          } else {
            nr::uint128_t num = V->as<Int>()->getValue();

            if (num < B->as<UnionTy>()->getFields().size()) {
              T = B->as<UnionTy>()->getFields()[num.convert_to<std::size_t>()];
            } else {
              T = nullptr;  // Invalid out of bounds
            }
          }
        } else {
          T = nullptr;  // Invalid type to index into
        }
        break;
      }
      case QIR_NODE_IDENT: {
        if (E->as<Ident>()->getWhat() != nullptr) {
          T = E->as<Ident>()->getWhat()->getType().value_or(nullptr);
        }
        break;
      }
      case QIR_NODE_EXTERN: {
        T = create<VoidTy>();
        break;
      }
      case QIR_NODE_LOCAL: {
        T = E->as<Local>()->getValue()->getType().value_or(nullptr);
        break;
      }
      case QIR_NODE_RET: {
        T = E->as<Ret>()->getExpr()->getType().value_or(nullptr);
        break;
      }
      case QIR_NODE_BRK: {
        T = create<VoidTy>();
        break;
      }
      case QIR_NODE_CONT: {
        T = create<VoidTy>();
        break;
      }
      case QIR_NODE_IF: {
        T = create<VoidTy>();
        break;
      }
      case QIR_NODE_WHILE: {
        T = create<VoidTy>();
        break;
      }
      case QIR_NODE_FOR: {
        T = create<VoidTy>();
        break;
      }
      case QIR_NODE_CASE: {
        T = create<VoidTy>();
        break;
      }
      case QIR_NODE_SWITCH: {
        T = create<VoidTy>();
        break;
      }
      case QIR_NODE_FN: {
        FnParams params;
        for (auto &param : E->as<Fn>()->getParams()) {
          Type *paramType = param.first->getType().value_or(nullptr);
          if (paramType) {
            params.push_back(paramType);
          } else {
            T = nullptr;
            break;
          }
        }

        FnAttrs attrs;
        T = create<FnTy>(std::move(params), E->as<Fn>()->getReturn(), std::move(attrs));
        break;
      }
      case QIR_NODE_ASM: {
        T = create<VoidTy>();
        break;
      }
      case QIR_NODE_IGN: {
        T = create<VoidTy>();
        break;
      }
      case QIR_NODE_TMP: {
        T = nullptr;
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

bool nr::Type::hasKnownSize() noexcept {
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
      return this->as<ArrayTy>()->getElement()->hasKnownSize();
    }
    case QIR_NODE_OPAQUE_TY: {
      return false;
    }
    default: {
      return false;
    }
  }
}

bool nr::Type::hasKnownAlign() noexcept {
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
                         [](Type *T) { return T->hasKnownAlign(); });
    }
    case QIR_NODE_UNION_TY: {
      return std::all_of(this->as<UnionTy>()->getFields().begin(),
                         this->as<UnionTy>()->getFields().end(),
                         [](Type *T) { return T->hasKnownAlign(); });
    }
    case QIR_NODE_ARRAY_TY: {
      return this->as<ArrayTy>()->getElement()->hasKnownAlign();
    }
    case QIR_NODE_OPAQUE_TY: {
      return false;
    }
    default: {
      return false;
    }
  }
}

CPP_EXPORT uint64_t nr::Type::getSizeBits(uint32_t PtrSizeBytes) {
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
      size = PtrSizeBytes * 8;
      break;
    }
    case QIR_NODE_STRUCT_TY: {
      size = 0;
      for (auto &field : this->as<StructTy>()->getFields()) {
        size += field->getSizeBits(PtrSizeBytes);
      }
      break;
    }
    case QIR_NODE_UNION_TY: {
      size = 0;
      for (auto &field : this->as<UnionTy>()->getFields()) {
        size = std::max(size, field->getSizeBits(PtrSizeBytes));
      }
      break;
    }
    case QIR_NODE_ARRAY_TY: {
      size = this->as<ArrayTy>()->getElement()->getSizeBits(PtrSizeBytes) *
             this->as<ArrayTy>()->getCount();
      break;
    }
    case QIR_NODE_FN_TY: {
      size = PtrSizeBytes * 8;
      break;
    }
    default: {
      qcore_panicf("Invalid type kind: %d", this->getKind());
    }
  }

  return size;
}

CPP_EXPORT uint64_t nr::Type::getAlignBits(uint32_t PtrSizeBytes) {
  qcore_assert(this->hasKnownSize(), "Attempted to get the size of a type with an unknown size");

  uint64_t align;

  switch (this->getKind()) {
    case QIR_NODE_U1_TY: {
      align = 8;
      break;
    }
    case QIR_NODE_U8_TY: {
      align = 8;
      break;
    }
    case QIR_NODE_U16_TY: {
      align = 16;
      break;
    }
    case QIR_NODE_U32_TY: {
      align = 32;
      break;
    }
    case QIR_NODE_U64_TY: {
      align = 64;
      break;
    }
    case QIR_NODE_U128_TY: {
      align = 128;
      break;
    }
    case QIR_NODE_I8_TY: {
      align = 8;
      break;
    }
    case QIR_NODE_I16_TY: {
      align = 16;
      break;
    }
    case QIR_NODE_I32_TY: {
      align = 32;
      break;
    }
    case QIR_NODE_I64_TY: {
      align = 64;
      break;
    }
    case QIR_NODE_I128_TY: {
      align = 128;
      break;
    }
    case QIR_NODE_F16_TY: {
      align = 16;
      break;
    }
    case QIR_NODE_F32_TY: {
      align = 32;
      break;
    }
    case QIR_NODE_F64_TY: {
      align = 64;
      break;
    }
    case QIR_NODE_F128_TY: {
      align = 128;
      break;
    }
    case QIR_NODE_VOID_TY: {
      align = 0;
      break;
    }
    case QIR_NODE_PTR_TY: {
      align = PtrSizeBytes * 8;
      break;
    }
    case QIR_NODE_STRUCT_TY: {
      align = 0;
      for (const auto &field : this->as<StructTy>()->getFields()) {
        align = std::max(align, field->getSizeBits(PtrSizeBytes));
      }
      break;
    }
    case QIR_NODE_UNION_TY: {
      align = 0;
      for (const auto &field : this->as<UnionTy>()->getFields()) {
        align = std::max(align, field->getSizeBits(PtrSizeBytes));
      }
      break;
    }
    case QIR_NODE_ARRAY_TY: {
      align = this->as<ArrayTy>()->getElement()->getAlignBits(PtrSizeBytes);
      break;
    }
    case QIR_NODE_FN_TY: {
      align = PtrSizeBytes * 8;
      break;
    }
    default: {
      qcore_panicf("Invalid type kind: %d", this->getKind());
    }
  }

  return align;
}
