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

#include <boost/multiprecision/cpp_int.hpp>
#include <cstdint>
#include <nitrate-core/Macro.hh>
#include <nitrate-ir/IR.hh>
#include <nitrate-ir/IRGraph.hh>

/// TODO: Test this code

using namespace ncc;
using namespace ncc::ir;

static Type *signed_complement(nr_ty_t ty) {
  switch (ty) {
    case IR_I8_TY:
      return create<U8Ty>();
    case IR_I16_TY:
      return create<U16Ty>();
    case IR_I32_TY:
      return create<U32Ty>();
    case IR_I64_TY:
      return create<U64Ty>();
    case IR_I128_TY:
      return create<U128Ty>();
    default:
      return nullptr;
  }
}

static std::optional<Type *> promote(std::optional<Type *> lhs,
                                     std::optional<Type *> rhs) {
  if (!lhs.has_value() || !rhs.has_value()) {
    return std::nullopt;
  }

  auto L = lhs.value(), R = rhs.value();

  if (L->is_readonly() && !R->is_readonly()) {
    R = create<ConstTy>(R);
  } else if (!L->is_readonly() && R->is_readonly()) {
    L = create<ConstTy>(L);
  }

  ///===========================================================================
  /// NOTE: If L && R are the same type, the type is their identity.
  if (L->isSame(R)) {
    return L;
  }
  ///===========================================================================

  nr_ty_t LT = L->getKind(), RT = R->getKind();

  ///===========================================================================
  /// NOTE: Primitive numeric types are promoted according to the following
  /// rules:
  if (L->is_numeric() && R->is_numeric()) {
    ///===========================================================================
    /// NOTE: Floating point always takes precedence over integers.
    if (L->is(IR_VOID_TY) || R->is(IR_VOID_TY)) {
      return std::nullopt;
    }

    if (L->is(IR_F128_TY) || R->is(IR_F128_TY)) {
      return create<F128Ty>();
    }

    if (L->is(IR_F64_TY) || R->is(IR_F64_TY)) {
      return create<F64Ty>();
    }

    if (L->is(IR_F32_TY) || R->is(IR_F32_TY)) {
      return create<F32Ty>();
    }

    if (L->is(IR_F16_TY) || R->is(IR_F16_TY)) {
      return create<F16Ty>();
    }
    ///===========================================================================

    ///===========================================================================
    /// NOTE: If L && R are both unsigned integers, the larger type is used.
    if (L->is_unsigned() && R->is_unsigned()) {
      auto LS = L->getSizeBits(), RS = R->getSizeBits();
      if (!LS.has_value() || !RS.has_value()) {
        return std::nullopt;
      }
      return LS > RS ? L : R;
    }
    ///===========================================================================

    ///===========================================================================
    /// NOTE: If L && R are both signed integers, the larger type is used.
    if ((L->is_signed() && L->is_integral()) &&
        (R->is_signed() && R->is_integral())) {
      auto LS = L->getSizeBits(), RS = R->getSizeBits();
      if (!LS.has_value() || !RS.has_value()) {
        return std::nullopt;
      }
      return LS > RS ? L : R;
    }
    ///===========================================================================

    ///===========================================================================
    /// NOTE: If either L or R is a signed integer, the signed integer is
    /// promoted.
    if (L->is_integral() && L->is_signed()) {
      auto LS = L->getSizeBits(), RS = R->getSizeBits();
      if (!LS.has_value() || !RS.has_value()) {
        return std::nullopt;
      }
      if (LS > RS) {
        return signed_complement(LT);
      } else {
        return R;
      }
    } else if (R->is_integral() && R->is_signed()) {
      auto LS = L->getSizeBits(), RS = R->getSizeBits();
      if (!LS.has_value() || !RS.has_value()) {
        return std::nullopt;
      }
      if (RS > LS) {
        return signed_complement(RT);
      } else {
        return L;
      }
    } else {
      return std::nullopt;
    }
    ///===========================================================================
  }
  ///===========================================================================

  else {
    return std::nullopt;
  }
}

static std::optional<Type *> nr_infer_impl(
    const Expr *_node, std::unordered_set<const Expr *> &visited) {
  visited.insert(_node);

  const Expr *E = static_cast<const Expr *>(_node);
  std::optional<Type *> R;

  switch (E->getKind()) {
    case IR_BINEXPR: {
      switch (const BinExpr *B = E->as<BinExpr>(); B->getOp()) {
        case Op::Plus: {
          R = promote(B->getLHS()->getType(), B->getRHS()->getType());
          break;
        }
        case Op::Minus: {
          R = promote(B->getLHS()->getType(), B->getRHS()->getType());
          break;
        }
        case Op::Times: {
          R = promote(B->getLHS()->getType(), B->getRHS()->getType());
          break;
        }
        case Op::Slash: {
          R = promote(B->getLHS()->getType(), B->getRHS()->getType());
          break;
        }
        case Op::Percent: {
          R = promote(B->getLHS()->getType(), B->getRHS()->getType());
          break;
        }
        case Op::BitAnd: {
          R = promote(B->getLHS()->getType(), B->getRHS()->getType());
          break;
        }
        case Op::BitOr: {
          R = promote(B->getLHS()->getType(), B->getRHS()->getType());
          break;
        }
        case Op::BitXor: {
          R = promote(B->getLHS()->getType(), B->getRHS()->getType());
          break;
        }
        case Op::BitNot: {
          R = promote(B->getLHS()->getType(), B->getRHS()->getType());
          break;
        }
        case Op::LogicAnd: {
          R = create<U1Ty>();
          break;
        }
        case Op::LogicOr: {
          R = create<U1Ty>();
          break;
        }
        case Op::LogicNot: {
          R = create<U1Ty>();
          break;
        }
        case Op::LShift: {
          R = B->getLHS()->getType();
          break;
        }
        case Op::RShift: {
          R = B->getLHS()->getType();
          break;
        }
        case Op::Inc: {
          R = std::nullopt;
          break;
        }
        case Op::Dec: {
          R = std::nullopt;
          break;
        }
        case Op::Set: {
          R = B->getRHS()->getType();
          break;
        }
        case Op::LT: {
          R = create<U1Ty>();
          break;
        }
        case Op::GT: {
          R = create<U1Ty>();
          break;
        }
        case Op::LE: {
          R = create<U1Ty>();
          break;
        }
        case Op::GE: {
          R = create<U1Ty>();
          break;
        }
        case Op::Eq: {
          R = create<U1Ty>();
          break;
        }
        case Op::NE: {
          R = create<U1Ty>();
          break;
        }
        case Op::Alignof: {
          R = std::nullopt;
          break;
        }
        case Op::BitcastAs: {
          R = B->getRHS()->getType();
          break;
        }
        case Op::CastAs: {
          R = B->getRHS()->getType();
          break;
        }
        case Op::Bitsizeof: {
          R = std::nullopt;
          break;
        }
      }
      break;
    }
    case IR_UNEXPR: {
      switch (const UnExpr *U = E->as<UnExpr>(); E->as<UnExpr>()->getOp()) {
        case Op::Plus: {
          R = U->getExpr()->getType();
          break;
        }
        case Op::Minus: {
          R = U->getExpr()->getType();
          break;
        }
        case Op::Times: {
          if (auto x = U->getExpr()->getType()) {
            /* Operator '*' is only valid on pointers */
            if (x.value()->is(IR_PTR_TY)) {
              R = x.value()->as<PtrTy>()->getPointee();
            }
          }
          break;
        }
        case Op::BitAnd: {
          if (auto x = U->getExpr()->getType()) {
            R = create<PtrTy>(x.value());
          }
          break;
        }
        case Op::BitNot: {
          R = U->getExpr()->getType();
          break;
        }
        case Op::LogicNot: {
          R = create<U1Ty>();
          break;
        }

        case Op::Inc: {
          R = U->getExpr()->getType();
          break;
        }
        case Op::Dec: {
          R = U->getExpr()->getType();
          break;
        }
        case Op::Alignof: {
          R = create<U64Ty>();
          break;
        }
        case Op::Bitsizeof: {
          R = create<U64Ty>();
          break;
        }
        case Op::Slash:
        case Op::Percent:
        case Op::BitOr:
        case Op::BitXor:
        case Op::LogicAnd:
        case Op::LogicOr:
        case Op::LShift:
        case Op::RShift:
        case Op::Set:
        case Op::LT:
        case Op::GT:
        case Op::LE:
        case Op::GE:
        case Op::Eq:
        case Op::NE:
        case Op::BitcastAs:
        case Op::CastAs: {
          R = std::nullopt;
          break;
        }
      }
      break;
    }
    case IR_POST_UNEXPR: {
      switch (const PostUnExpr *P = E->as<PostUnExpr>();
              E->as<PostUnExpr>()->getOp()) {
        case Op::Inc: {
          R = P->getExpr()->getType();
          break;
        }
        case Op::Dec: {
          R = P->getExpr()->getType();
          break;
        }
        case Op::Plus:
        case Op::Minus:
        case Op::Times:
        case Op::Slash:
        case Op::Percent:
        case Op::BitAnd:
        case Op::BitOr:
        case Op::BitXor:
        case Op::BitNot:
        case Op::LogicAnd:
        case Op::LogicOr:
        case Op::LogicNot:
        case Op::LShift:
        case Op::RShift:
        case Op::Set:
        case Op::LT:
        case Op::GT:
        case Op::LE:
        case Op::GE:
        case Op::Eq:
        case Op::NE:
        case Op::Alignof:
        case Op::BitcastAs:
        case Op::CastAs:
        case Op::Bitsizeof: {
          R = std::nullopt;
          break;
        }
      }
      break;
    }
    case IR_INT: {
      auto size = E->as<Int>()->getSize();

      if (size == 1) {
        R = create<U1Ty>();
      } else if (size <= 8) {
        R = create<I8Ty>();
      } else if (size <= 16) {
        R = create<I16Ty>();
      } else if (size <= 32) {
        R = create<I32Ty>();
      } else if (size <= 64) {
        R = create<I64Ty>();
      } else if (size <= 128) {
        R = create<I128Ty>();
      }

      break;
    }
    case IR_FLOAT: {
      switch (E->as<Float>()->getSize()) {
        case FloatSize::F16:
          R = create<F16Ty>();
          break;
        case FloatSize::F32:
          R = create<F32Ty>();
          break;
        case FloatSize::F64:
          R = create<F64Ty>();
          break;
        case FloatSize::F128:
          R = create<F128Ty>();
          break;
      }

      break;
    }
    case IR_LIST: {
      if (E->as<List>()->size() == 0) {
        R = create<StructTy>(StructFields());
      } else {
        std::vector<Type *> types;
        bool failed = false;
        for (const auto &item : *E->as<List>()) {
          auto x = item->getType();
          if (!x) {
            R = std::nullopt;
            failed = true;
            break;
          }
          types.push_back(x.value());
        }
        if (!failed) {
          if (E->as<List>()->isHomogenous()) {
            R = create<ArrayTy>(types.front(), types.size());
          } else {
            R = create<StructTy>(StructFields(types.begin(), types.end()));
          }
        }
      }
      break;
    }
    case IR_CALL: {
      if (auto x = E->as<Call>()->getTarget()->getType()) {
        if (x.value()->is_function()) {
          R = x.value()->as<FnTy>()->getReturn();
        }
      }
      break;
    }
    case IR_SEQ: {
      if (E->as<Seq>()->getItems().empty()) {
        R = create<VoidTy>();
      } else {
        R = E->as<Seq>()->getItems().back()->getType();
      }
      break;
    }
    case IR_INDEX: {
      if (auto B_ = E->as<Index>()->getExpr()->getType();
          auto B = B_.value_or(nullptr)) {
        Expr *V = E->as<Index>()->getIndex();

        if (B->is(IR_PTR_TY)) {  // *X -> X
          R = B->as<PtrTy>()->getPointee();
        } else if (B->is(IR_ARRAY_TY)) {  // [X; N] -> X
          R = B->as<ArrayTy>()->getElement();
        } else if (B->is(IR_STRUCT_TY)) {  // struct { a, b, c } -> a | b
                                           // | c
          if (!V->is(IR_INT)) {
            R = std::nullopt;  // Invalid must be of type int to index into a
                               // struct
          } else {
            uint128_t num = V->as<Int>()->getValue();
            if (num < B->as<StructTy>()->getFields().size()) {
              R = B->as<StructTy>()->getFields()[num.convert_to<std::size_t>()];
            } else {
              R = std::nullopt;  // Invalid out of bounds
            }
          }
        } else if (B->is(IR_UNION_TY)) {
          if (!V->is(IR_INT)) {
            R = std::nullopt;  // Invalid must be of type int to index into a
                               // union
          } else {
            uint128_t num = V->as<Int>()->getValue();

            if (num < B->as<UnionTy>()->getFields().size()) {
              R = B->as<UnionTy>()->getFields()[num.convert_to<std::size_t>()];
            } else {
              R = std::nullopt;  // Invalid out of bounds
            }
          }
        }
      }
      break;
    }
    case IR_IDENT: {
      Expr *what = E->as<Ident>()->getWhat();
      if (!visited.contains(what)) [[likely]] {
        if (what != nullptr) {
          R = what->getType();
        }
      }
      break;
    }
    case IR_EXTERN: {
      R = create<VoidTy>();
      break;
    }
    case IR_LOCAL: {
      const Local *local = E->as<Local>();
      R = local->getValue()->getType();
      if (R.has_value() && local->isReadonly()) {
        R = create<ConstTy>(R.value());
      }
      break;
    }
    case IR_RET: {
      R = E->as<Ret>()->getExpr()->getType();
      break;
    }
    case IR_BRK: {
      R = create<VoidTy>();
      break;
    }
    case IR_CONT: {
      R = create<VoidTy>();
      break;
    }
    case IR_IF: {
      R = create<VoidTy>();
      break;
    }
    case IR_WHILE: {
      R = create<VoidTy>();
      break;
    }
    case IR_FOR: {
      R = create<VoidTy>();
      break;
    }
    case IR_CASE: {
      R = create<VoidTy>();
      break;
    }
    case IR_SWITCH: {
      R = create<VoidTy>();
      break;
    }
    case IR_FN: {
      bool failed = false;
      const auto &params = E->as<Fn>()->getParams();
      FnParams param_types(params.size());

      for (size_t i = 0; i < params.size(); ++i) {
        if (auto paramType = params[i].first->getType()) {
          param_types[i] = paramType.value();
        } else {
          failed = true;
          break;
        }
      }

      if (!failed) {
        FnAttrs attrs;
        if (E->as<Fn>()->isVariadic()) {
          attrs.insert(FnAttr::Variadic);
        }

        R = create<FnTy>(std::move(param_types), E->as<Fn>()->getReturn(),
                         std::move(attrs));
      }

      break;
    }
    case IR_ASM: {
      R = create<VoidTy>();
      break;
    }
    case IR_IGN: {
      R = create<VoidTy>();
      break;
    }
    case IR_TMP: {
      R = std::nullopt;
      break;
    }

    case IR_U1_TY:
    case IR_U8_TY:
    case IR_U16_TY:
    case IR_U32_TY:
    case IR_U64_TY:
    case IR_U128_TY:
    case IR_I8_TY:
    case IR_I16_TY:
    case IR_I32_TY:
    case IR_I64_TY:
    case IR_I128_TY:
    case IR_F16_TY:
    case IR_F32_TY:
    case IR_F64_TY:
    case IR_F128_TY:
    case IR_VOID_TY:
    case IR_PTR_TY:
    case IR_CONST_TY:
    case IR_OPAQUE_TY:
    case IR_STRUCT_TY:
    case IR_UNION_TY:
    case IR_ARRAY_TY:
    case IR_FN_TY: {
      R = const_cast<Type *>(E->asType());
      break;
    }
  }

  return R;
}

CPP_EXPORT nr_node_t *ir::nr_infer(const nr_node_t *_node, void *) {
  static thread_local struct State {
    std::unordered_set<const Expr *> visited;
    size_t depth = 0;
  } state;

  state.depth++;

  auto R = nr_infer_impl(static_cast<const Expr *>(_node), state.visited);

  state.depth--;

  if (state.depth == 0) {
    state.visited.clear();
  }

  return const_cast<Type *>(R.value_or(nullptr));
}

CPP_EXPORT std::optional<uint64_t> Type::getSizeBits() const {
  switch (this->getKind()) {
    case IR_U1_TY: {
      return 8;
    }
    case IR_U8_TY: {
      return 8;
    }
    case IR_U16_TY: {
      return 16;
    }
    case IR_U32_TY: {
      return 32;
    }
    case IR_U64_TY: {
      return 64;
    }
    case IR_U128_TY: {
      return 128;
    }
    case IR_I8_TY: {
      return 8;
    }
    case IR_I16_TY: {
      return 16;
    }
    case IR_I32_TY: {
      return 32;
    }
    case IR_I64_TY: {
      return 64;
    }
    case IR_I128_TY: {
      return 128;
    }
    case IR_F16_TY: {
      return 16;
    }
    case IR_F32_TY: {
      return 32;
    }
    case IR_F64_TY: {
      return 64;
    }
    case IR_F128_TY: {
      return 128;
    }
    case IR_VOID_TY: {
      return 0;
    }
    case IR_PTR_TY: {
      return this->as<PtrTy>()->getPlatformPointerSizeBytes() * 8;
    }
    case IR_CONST_TY: {
      return this->as<ConstTy>()->getItem()->getSizeBits();
    }
    case IR_STRUCT_TY: {
      size_t sum = 0;
      for (auto field : this->as<StructTy>()->getFields()) {
        if (auto field_size = field->getSizeBits()) {
          sum += field_size.value();
        } else {
          return std::nullopt;
        }
      }
      return sum;
    }
    case IR_UNION_TY: {
      size_t max = 0;
      for (auto field : this->as<UnionTy>()->getFields()) {
        if (auto field_size = field->getSizeBits()) {
          max = std::max(max, field_size.value());
        } else {
          return std::nullopt;
        }
      }
      return max;
    }
    case IR_ARRAY_TY: {
      if (auto element_size =
              this->as<ArrayTy>()->getElement()->getSizeBits()) {
        return element_size.value() * this->as<ArrayTy>()->getCount();
      } else {
        return std::nullopt;
      }
      break;
    }
    case IR_FN_TY: {
      return this->as<FnTy>()->getPlatformPointerSizeBytes() * 8;
    }
    case IR_OPAQUE_TY: {
      return std::nullopt;
    }
    default: {
      return std::nullopt;
    }
  }
}

CPP_EXPORT std::optional<uint64_t> Type::getAlignBits() const {
  switch (this->getKind()) {
    case IR_U1_TY: {
      return 8;
    }
    case IR_U8_TY: {
      return 8;
    }
    case IR_U16_TY: {
      return 16;
    }
    case IR_U32_TY: {
      return 32;
    }
    case IR_U64_TY: {
      return 64;
    }
    case IR_U128_TY: {
      return 128;
    }
    case IR_I8_TY: {
      return 8;
    }
    case IR_I16_TY: {
      return 16;
    }
    case IR_I32_TY: {
      return 32;
    }
    case IR_I64_TY: {
      return 64;
    }
    case IR_I128_TY: {
      return 128;
    }
    case IR_F16_TY: {
      return 16;
    }
    case IR_F32_TY: {
      return 32;
    }
    case IR_F64_TY: {
      return 64;
    }
    case IR_F128_TY: {
      return 128;
    }
    case IR_VOID_TY: {
      return 0;
    }
    case IR_PTR_TY: {
      return this->as<PtrTy>()->getPlatformPointerSizeBytes() * 8;
    }
    case IR_CONST_TY: {
      return this->as<ConstTy>()->getItem()->getAlignBits();
    }
    case IR_STRUCT_TY: {
      size_t max_align = 0;
      for (auto field : this->as<StructTy>()->getFields()) {
        if (auto field_align = field->getAlignBits()) {
          max_align = std::max(max_align, field_align.value());
        } else {
          return std::nullopt;
        }
      }
      return max_align;
    }
    case IR_UNION_TY: {
      size_t max_align = 0;
      for (auto field : this->as<UnionTy>()->getFields()) {
        if (auto field_align = field->getAlignBits()) {
          max_align = std::max(max_align, field_align.value());
        } else {
          return std::nullopt;
        }
      }
      return max_align;
    }
    case IR_ARRAY_TY: {
      return this->as<ArrayTy>()->getElement()->getAlignBits();
    }
    case IR_FN_TY: {
      return this->as<FnTy>()->getPlatformPointerSizeBytes() * 8;
    }
    case IR_OPAQUE_TY: {
      return std::nullopt;
    }
    default: {
      return std::nullopt;
    }
  }
}
