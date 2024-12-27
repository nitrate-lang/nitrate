// ////////////////////////////////////////////////////////////////////////////////
// /// ///
// ///     .-----------------.    .----------------.     .----------------. ///
// ///    | .--------------. |   | .--------------. |   | .--------------. | ///
// ///    | | ____  _____  | |   | |     ____     | |   | |    ______    | | ///
// ///    | ||_   _|_   _| | |   | |   .'    `.   | |   | |   / ____ `.  | | ///
// ///    | |  |   \ | |   | |   | |  /  .--.  \  | |   | |   `'  __) |  | | ///
// ///    | |  | |\ \| |   | |   | |  | |    | |  | |   | |   _  |__ '.  | | ///
// ///    | | _| |_\   |_  | |   | |  \  `--'  /  | |   | |  | \____) |  | | ///
// ///    | ||_____|\____| | |   | |   `.____.'   | |   | |   \______.'  | | ///
// ///    | |              | |   | |              | |   | |              | | ///
// ///    | '--------------' |   | '--------------' |   | '--------------' | ///
// ///     '----------------'     '----------------'     '----------------' ///
// /// ///
// ///   * NITRATE TOOLCHAIN - The official toolchain for the Nitrate language.
// ///
// ///   * Copyright (C) 2024 Wesley C. Jones ///
// /// ///
// ///   The Nitrate Toolchain is free software; you can redistribute it or ///
// ///   modify it under the terms of the GNU Lesser General Public ///
// ///   License as published by the Free Software Foundation; either ///
// ///   version 2.1 of the License, or (at your option) any later version. ///
// /// ///
// ///   The Nitrate Toolcain is distributed in the hope that it will be ///
// ///   useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
// ///
// ///   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU ///
// ///   Lesser General Public License for more details. ///
// /// ///
// ///   You should have received a copy of the GNU Lesser General Public ///
// ///   License along with the Nitrate Toolchain; if not, see ///
// ///   <https://www.gnu.org/licenses/>. ///
// /// ///
// ////////////////////////////////////////////////////////////////////////////////

// #include <boost/multiprecision/cpp_int.hpp>
// #include <cstdint>
// #include <nitrate-core/Macro.hh>
// #include <nitrate-ir/IR.hh>
// #include <nitrate-ir/IR/Nodes.hh>

// #include "nitrate-ir/IRBase.hh"

// /// TODO: Test this code

// using namespace ncc;
// using namespace ncc::ir;

// static FlowPtr<Type>signed_complement(nr_ty_t ty) {
//   switch (ty) {
//     case IR_tI8:
//       return create<U8Ty>();
//     case IR_tI16:
//       return create<U16Ty>();
//     case IR_tI32:
//       return create<U32Ty>();
//     case IR_tI64:
//       return create<U64Ty>();
//     case IR_tI128:
//       return create<U128Ty>();
//     default:
//       return nullptr;
//   }
// }

// static std::optional<FlowPtr<Type>> promote(std::optional<FlowPtr<Type>> lhs,
//                                      std::optional<FlowPtr<Type>> rhs) {
//   if (!lhs.has_value() || !rhs.has_value()) {
//     return std::nullopt;
//   }

//   auto L = lhs.value(), R = rhs.value();

//   if (L->is_readonly() && !R->is_readonly()) {
//     R = create<ConstTy>(R);
//   } else if (!L->is_readonly() && R->is_readonly()) {
//     L = create<ConstTy>(L);
//   }

//   ///===========================================================================
//   /// NOTE: If L && R are the same type, the type is their identity.
//   if (L->isSame(R)) {
//     return L;
//   }
//   ///===========================================================================

//   nr_ty_t LT = L->getKind(), RT = R->getKind();

//   ///===========================================================================
//   /// NOTE: Primitive numeric types are promoted according to the following
//   /// rules:
//   if (L->is_numeric() && R->is_numeric()) {
//     ///===========================================================================
//     /// NOTE: Floating point always takes precedence over integers.
//     if (L->is(IR_tVOID) || R->is(IR_tVOID)) {
//       return std::nullopt;
//     }

//     if (L->is(IR_tF128_TY) || R->is(IR_tF128_TY)) {
//       return create<F128Ty>();
//     }

//     if (L->is(IR_tF64_TY) || R->is(IR_tF64_TY)) {
//       return create<F64Ty>();
//     }

//     if (L->is(IR_tF32_TY) || R->is(IR_tF32_TY)) {
//       return create<F32Ty>();
//     }

//     if (L->is(IR_tF16_TY) || R->is(IR_tF16_TY)) {
//       return create<F16Ty>();
//     }
//     ///===========================================================================

//     ///===========================================================================
//     /// NOTE: If L && R are both unsigned integers, the larger type is used.
//     if (L->is_unsigned() && R->is_unsigned()) {
//       auto LS = L->getSizeBits(), RS = R->getSizeBits();
//       if (!LS.has_value() || !RS.has_value()) {
//         return std::nullopt;
//       }
//       return LS > RS ? L : R;
//     }
//     ///===========================================================================

//     ///===========================================================================
//     /// NOTE: If L && R are both signed integers, the larger type is used.
//     if ((L->is_signed() && L->is_integral()) &&
//         (R->is_signed() && R->is_integral())) {
//       auto LS = L->getSizeBits(), RS = R->getSizeBits();
//       if (!LS.has_value() || !RS.has_value()) {
//         return std::nullopt;
//       }
//       return LS > RS ? L : R;
//     }
//     ///===========================================================================

//     ///===========================================================================
//     /// NOTE: If either L or R is a signed integer, the signed integer is
//     /// promoted.
//     if (L->is_integral() && L->is_signed()) {
//       auto LS = L->getSizeBits(), RS = R->getSizeBits();
//       if (!LS.has_value() || !RS.has_value()) {
//         return std::nullopt;
//       }
//       if (LS > RS) {
//         return signed_complement(LT);
//       } else {
//         return R;
//       }
//     } else if (R->is_integral() && R->is_signed()) {
//       auto LS = L->getSizeBits(), RS = R->getSizeBits();
//       if (!LS.has_value() || !RS.has_value()) {
//         return std::nullopt;
//       }
//       if (RS > LS) {
//         return signed_complement(RT);
//       } else {
//         return L;
//       }
//     } else {
//       return std::nullopt;
//     }
//     ///===========================================================================
//   }
//   ///===========================================================================

//   else {
//     return std::nullopt;
//   }
// }

// static std::optional<FlowPtr<Type>> GetTypeImpl(
//     FlowPtr<Expr> E, std::unordered_set<FlowPtr<Expr>> &visited) {
//   visited.insert(E);

//   std::optional<FlowPtr<Type>> R;

//   switch (E->getKind()) {
//     case IR_eBIN: {
//       switch (const BinExpr *B = E->as<BinExpr>(); B->getOp()) {
//         case Op::Plus: {
//           R = promote(B->getLHS()->getType(), B->getRHS()->getType());
//           break;
//         }
//         case Op::Minus: {
//           R = promote(B->getLHS()->getType(), B->getRHS()->getType());
//           break;
//         }
//         case Op::Times: {
//           R = promote(B->getLHS()->getType(), B->getRHS()->getType());
//           break;
//         }
//         case Op::Slash: {
//           R = promote(B->getLHS()->getType(), B->getRHS()->getType());
//           break;
//         }
//         case Op::Percent: {
//           R = promote(B->getLHS()->getType(), B->getRHS()->getType());
//           break;
//         }
//         case Op::BitAnd: {
//           R = promote(B->getLHS()->getType(), B->getRHS()->getType());
//           break;
//         }
//         case Op::BitOr: {
//           R = promote(B->getLHS()->getType(), B->getRHS()->getType());
//           break;
//         }
//         case Op::BitXor: {
//           R = promote(B->getLHS()->getType(), B->getRHS()->getType());
//           break;
//         }
//         case Op::BitNot: {
//           R = promote(B->getLHS()->getType(), B->getRHS()->getType());
//           break;
//         }
//         case Op::LogicAnd: {
//           R = create<U1Ty>();
//           break;
//         }
//         case Op::LogicOr: {
//           R = create<U1Ty>();
//           break;
//         }
//         case Op::LogicNot: {
//           R = create<U1Ty>();
//           break;
//         }
//         case Op::LShift: {
//           R = B->getLHS()->getType();
//           break;
//         }
//         case Op::RShift: {
//           R = B->getLHS()->getType();
//           break;
//         }
//         case Op::Inc: {
//           R = std::nullopt;
//           break;
//         }
//         case Op::Dec: {
//           R = std::nullopt;
//           break;
//         }
//         case Op::Set: {
//           R = B->getRHS()->getType();
//           break;
//         }
//         case Op::LT: {
//           R = create<U1Ty>();
//           break;
//         }
//         case Op::GT: {
//           R = create<U1Ty>();
//           break;
//         }
//         case Op::LE: {
//           R = create<U1Ty>();
//           break;
//         }
//         case Op::GE: {
//           R = create<U1Ty>();
//           break;
//         }
//         case Op::Eq: {
//           R = create<U1Ty>();
//           break;
//         }
//         case Op::NE: {
//           R = create<U1Ty>();
//           break;
//         }
//         case Op::Alignof: {
//           R = std::nullopt;
//           break;
//         }
//         case Op::BitcastAs: {
//           R = B->getRHS()->getType();
//           break;
//         }
//         case Op::CastAs: {
//           R = B->getRHS()->getType();
//           break;
//         }
//         case Op::Bitsizeof: {
//           R = std::nullopt;
//           break;
//         }
//       }
//       break;
//     }
//     case IR_eUNARY: {
//       switch (const Unary *U = E->as<Unary>(); E->as<Unary>()->getOp()) {
//         case Op::Plus: {
//           R = U->getExpr()->getType();
//           break;
//         }
//         case Op::Minus: {
//           R = U->getExpr()->getType();
//           break;
//         }
//         case Op::Times: {
//           if (auto x = U->getExpr()->getType()) {
//             /* Operator '*' is only valid on pointers */
//             if (x.value()->is(IR_tPTR)) {
//               R = x.value()->as<PtrTy>()->getPointee();
//             }
//           }
//           break;
//         }
//         case Op::BitAnd: {
//           if (auto x = U->getExpr()->getType()) {
//             R = create<PtrTy>(x.value());
//           }
//           break;
//         }
//         case Op::BitNot: {
//           R = U->getExpr()->getType();
//           break;
//         }
//         case Op::LogicNot: {
//           R = create<U1Ty>();
//           break;
//         }

//         case Op::Inc: {
//           R = U->getExpr()->getType();
//           break;
//         }
//         case Op::Dec: {
//           R = U->getExpr()->getType();
//           break;
//         }
//         case Op::Alignof: {
//           R = create<U64Ty>();
//           break;
//         }
//         case Op::Bitsizeof: {
//           R = create<U64Ty>();
//           break;
//         }
//         case Op::Slash:
//         case Op::Percent:
//         case Op::BitOr:
//         case Op::BitXor:
//         case Op::LogicAnd:
//         case Op::LogicOr:
//         case Op::LShift:
//         case Op::RShift:
//         case Op::Set:
//         case Op::LT:
//         case Op::GT:
//         case Op::LE:
//         case Op::GE:
//         case Op::Eq:
//         case Op::NE:
//         case Op::BitcastAs:
//         case Op::CastAs: {
//           R = std::nullopt;
//           break;
//         }
//       }
//       break;
//     }
//     case IR_eINT: {
//       auto size = E->as<Int>()->getSize();

//       if (size == 1) {
//         R = create<U1Ty>();
//       } else if (size <= 8) {
//         R = create<I8Ty>();
//       } else if (size <= 16) {
//         R = create<I16Ty>();
//       } else if (size <= 32) {
//         R = create<I32Ty>();
//       } else if (size <= 64) {
//         R = create<I64Ty>();
//       } else if (size <= 128) {
//         R = create<I128Ty>();
//       }

//       break;
//     }
//     case IR_eFLOAT: {
//       switch (E->as<Float>()->getSize()) {
//         case FloatSize::F16:
//           R = create<F16Ty>();
//           break;
//         case FloatSize::F32:
//           R = create<F32Ty>();
//           break;
//         case FloatSize::F64:
//           R = create<F64Ty>();
//           break;
//         case FloatSize::F128:
//           R = create<F128Ty>();
//           break;
//       }

//       break;
//     }
//     case IR_eLIST: {
//       if (E->as<List>()->size() == 0) {
//         R = create<StructTy>(StructFields());
//       } else {
//         std::vector<FlowPtr<Type>> types;
//         bool failed = false;
//         for (const auto &item : *E->as<List>()) {
//           auto x = item->getType();
//           if (!x) {
//             R = std::nullopt;
//             failed = true;
//             break;
//           }
//           types.push_back(x.value());
//         }
//         if (!failed) {
//           if (E->as<List>()->isHomogenous()) {
//             R = create<ArrayTy>(types.front(), types.size());
//           } else {
//             R = create<StructTy>(StructFields(types.begin(), types.end()));
//           }
//         }
//       }
//       break;
//     }
//     case IR_eCALL: {
//       if (auto x = E->as<Call>()->getTarget()->getType()) {
//         if (x.value()->is_function()) {
//           R = x.value()->as<FnTy>()->getReturn();
//         }
//       }
//       break;
//     }
//     case IR_eSEQ: {
//       if (E->as<Seq>()->getItems().empty()) {
//         R = create<VoidTy>();
//       } else {
//         R = E->as<Seq>()->getItems().back()->getType();
//       }
//       break;
//     }
//     case IR_eINDEX: {
//       if (auto B_ = E->as<Index>()->getExpr()->getType();
//           auto B = B_.value_or(nullptr)) {
//         auto V = E->as<Index>()->getIndex();

//         if (B->is(IR_tPTR)) {  // *X -> X
//           R = B->as<PtrTy>()->getPointee();
//         } else if (B->is(IR_tARRAY)) {  // [X; N] -> X
//           R = B->as<ArrayTy>()->getElement();
//         } else if (B->is(IR_tSTRUCT)) {  // struct { a, b, c } -> a | b
//                                          // | c
//           if (!V->is(IR_eINT)) {
//             R = std::nullopt;  // Invalid must be of type int to index into a
//                                // struct
//           } else {
//             uint128_t num = V->as<Int>()->getValue();
//             if (num < B->as<StructTy>()->getFields().size()) {
//               R =
//               B->as<StructTy>()->getFields()[num.convert_to<std::size_t>()];
//             } else {
//               R = std::nullopt;  // Invalid out of bounds
//             }
//           }
//         } else if (B->is(IR_tUNION)) {
//           if (!V->is(IR_eINT)) {
//             R = std::nullopt;  // Invalid must be of type int to index into a
//                                // union
//           } else {
//             uint128_t num = V->as<Int>()->getValue();

//             if (num < B->as<UnionTy>()->getFields().size()) {
//               R =
//               B->as<UnionTy>()->getFields()[num.convert_to<std::size_t>()];
//             } else {
//               R = std::nullopt;  // Invalid out of bounds
//             }
//           }
//         }
//       }
//       break;
//     }
//     case IR_eIDENT: {
//       auto what = E->as<Ident>()->getWhat();
//       if (!visited.contains(what)) [[likely]] {
//         if (what != nullptr) {
//           R = what->getType();
//         }
//       }
//       break;
//     }
//     case IR_eEXTERN: {
//       R = create<VoidTy>();
//       break;
//     }
//     case IR_eLOCAL: {
//       const Local *local = E->as<Local>();
//       R = local->getValue()->getType();
//       if (R.has_value() && local->isReadonly()) {
//         R = create<ConstTy>(R.value());
//       }
//       break;
//     }
//     case IR_eRET: {
//       R = E->as<Ret>()->getExpr()->getType();
//       break;
//     }
//     case IR_eBRK: {
//       R = create<VoidTy>();
//       break;
//     }
//     case IR_eSKIP: {
//       R = create<VoidTy>();
//       break;
//     }
//     case IR_eIF: {
//       R = create<VoidTy>();
//       break;
//     }
//     case IR_eWHILE: {
//       R = create<VoidTy>();
//       break;
//     }
//     case IR_eFOR: {
//       R = create<VoidTy>();
//       break;
//     }
//     case IR_eCASE: {
//       R = create<VoidTy>();
//       break;
//     }
//     case IR_eSWITCH: {
//       R = create<VoidTy>();
//       break;
//     }
//     case IR_eFUNCTION: {
//       bool failed = false;
//       const auto &params = E->as<Function>()->getParams();
//       FnParams<void> param_types(params.size());

//       for (size_t i = 0; i < params.size(); ++i) {
//         if (auto paramType = params[i].first->getType()) {
//           param_types[i] = paramType.value();
//         } else {
//           failed = true;
//           break;
//         }
//       }

//       if (!failed) {
//         R = create<FnTy>(std::move(param_types),
//         E->as<Function>()->getReturn(),
//                          E->as<Function>()->isVariadic());
//       }

//       break;
//     }
//     case IR_eASM: {
//       R = create<VoidTy>();
//       break;
//     }
//     case IR_eIGN: {
//       R = create<VoidTy>();
//       break;
//     }
//     case IR_tTMP: {
//       R = std::nullopt;
//       break;
//     }

//     case IR_tU1:
//     case IR_tU8:
//     case IR_tU16:
//     case IR_tU32:
//     case IR_tU64:
//     case IR_tU128:
//     case IR_tI8:
//     case IR_tI16:
//     case IR_tI32:
//     case IR_tI64:
//     case IR_tI128:
//     case IR_tF16_TY:
//     case IR_tF32_TY:
//     case IR_tF64_TY:
//     case IR_tF128_TY:
//     case IR_tVOID:
//     case IR_tPTR:
//     case IR_tCONST:
//     case IR_tOPAQUE:
//     case IR_tSTRUCT:
//     case IR_tUNION:
//     case IR_tARRAY:
//     case IR_tFUNC: {
//       R = const_cast<FlowPtr<Type>>(E->asType());
//       break;
//     }
//   }

//   return R;
// }

// CPP_EXPORT std::optional<FlowPtr<Type>> detail::InferTypeImpl(Expr *E) {
//   static thread_local struct State {
//     std::unordered_set<FlowPtr<Expr>> visited;
//     size_t depth = 0;
//   } state;

//   state.depth++;

//   auto R = GetTypeImpl(E, state.visited);

//   state.depth--;

//   if (state.depth == 0) {
//     state.visited.clear();
//   }

//   return R;
// }

// CPP_EXPORT std::optional<uint64_t> detail::Type_getSizeBitsImpl(
//     const FlowPtr<Type>self) {
//   switch (self->getKind()) {
//     case IR_tU1: {
//       return 8;
//     }
//     case IR_tU8: {
//       return 8;
//     }
//     case IR_tU16: {
//       return 16;
//     }
//     case IR_tU32: {
//       return 32;
//     }
//     case IR_tU64: {
//       return 64;
//     }
//     case IR_tU128: {
//       return 128;
//     }
//     case IR_tI8: {
//       return 8;
//     }
//     case IR_tI16: {
//       return 16;
//     }
//     case IR_tI32: {
//       return 32;
//     }
//     case IR_tI64: {
//       return 64;
//     }
//     case IR_tI128: {
//       return 128;
//     }
//     case IR_tF16_TY: {
//       return 16;
//     }
//     case IR_tF32_TY: {
//       return 32;
//     }
//     case IR_tF64_TY: {
//       return 64;
//     }
//     case IR_tF128_TY: {
//       return 128;
//     }
//     case IR_tVOID: {
//       return 0;
//     }
//     case IR_tPTR: {
//       return self->as<PtrTy>()->getNativeSize() * 8;
//     }
//     case IR_tCONST: {
//       return self->as<ConstTy>()->getItem()->getSizeBits();
//     }
//     case IR_tSTRUCT: {
//       size_t sum = 0;
//       for (auto field : self->as<StructTy>()->getFields()) {
//         if (auto field_size = field->getSizeBits()) {
//           sum += field_size.value();
//         } else {
//           return std::nullopt;
//         }
//       }
//       return sum;
//     }
//     case IR_tUNION: {
//       size_t max = 0;
//       for (auto field : self->as<UnionTy>()->getFields()) {
//         if (auto field_size = field->getSizeBits()) {
//           max = std::max(max, field_size.value());
//         } else {
//           return std::nullopt;
//         }
//       }
//       return max;
//     }
//     case IR_tARRAY: {
//       if (auto element_size =
//               self->as<ArrayTy>()->getElement()->getSizeBits()) {
//         return element_size.value() * self->as<ArrayTy>()->getCount();
//       } else {
//         return std::nullopt;
//       }
//       break;
//     }
//     case IR_tFUNC: {
//       return self->as<FnTy>()->getNativeSize() * 8;
//     }
//     case IR_tOPAQUE: {
//       return std::nullopt;
//     }
//     default: {
//       return std::nullopt;
//     }
//   }
// }

// CPP_EXPORT std::optional<uint64_t> detail::Type_getAlignBitsImpl(
//     const FlowPtr<Type>self) {
//   switch (self->getKind()) {
//     case IR_tU1: {
//       return 8;
//     }
//     case IR_tU8: {
//       return 8;
//     }
//     case IR_tU16: {
//       return 16;
//     }
//     case IR_tU32: {
//       return 32;
//     }
//     case IR_tU64: {
//       return 64;
//     }
//     case IR_tU128: {
//       return 128;
//     }
//     case IR_tI8: {
//       return 8;
//     }
//     case IR_tI16: {
//       return 16;
//     }
//     case IR_tI32: {
//       return 32;
//     }
//     case IR_tI64: {
//       return 64;
//     }
//     case IR_tI128: {
//       return 128;
//     }
//     case IR_tF16_TY: {
//       return 16;
//     }
//     case IR_tF32_TY: {
//       return 32;
//     }
//     case IR_tF64_TY: {
//       return 64;
//     }
//     case IR_tF128_TY: {
//       return 128;
//     }
//     case IR_tVOID: {
//       return 0;
//     }
//     case IR_tPTR: {
//       return self->as<PtrTy>()->getNativeSize() * 8;
//     }
//     case IR_tCONST: {
//       return self->as<ConstTy>()->getItem()->getAlignBits();
//     }
//     case IR_tSTRUCT: {
//       size_t max_align = 0;
//       for (auto field : self->as<StructTy>()->getFields()) {
//         if (auto field_align = field->getAlignBits()) {
//           max_align = std::max(max_align, field_align.value());
//         } else {
//           return std::nullopt;
//         }
//       }
//       return max_align;
//     }
//     case IR_tUNION: {
//       size_t max_align = 0;
//       for (auto field : self->as<UnionTy>()->getFields()) {
//         if (auto field_align = field->getAlignBits()) {
//           max_align = std::max(max_align, field_align.value());
//         } else {
//           return std::nullopt;
//         }
//       }
//       return max_align;
//     }
//     case IR_tARRAY: {
//       return self->as<ArrayTy>()->getElement()->getAlignBits();
//     }
//     case IR_tFUNC: {
//       return self->as<FnTy>()->getNativeSize() * 8;
//     }
//     case IR_tOPAQUE: {
//       return std::nullopt;
//     }
//     default: {
//       return std::nullopt;
//     }
//   }
// }
