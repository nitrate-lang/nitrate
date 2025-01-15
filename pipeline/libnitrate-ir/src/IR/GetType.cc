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
#include <nitrate-core/Macro.hh>
#include <nitrate-ir/IR.hh>
#include <nitrate-ir/IR/Nodes.hh>
#include <unordered_set>

using namespace ncc;
using namespace ncc::ir;

static NullableFlowPtr<Type> SignedComplement(nr_ty_t ty) {
  switch (ty) {
    case IR_tI8: {
      return GetU8Ty();
    }

    case IR_tI16: {
      return GetU16Ty();
    }

    case IR_tI32: {
      return GetU32Ty();
    }

    case IR_tI64: {
      return GetU64Ty();
    }

    case IR_tI128: {
      return GetU128Ty();
    }

    default: {
      return nullptr;
    }
  }
}

static NullableFlowPtr<Type> Promote(NullableFlowPtr<Type> lhs,
                                     NullableFlowPtr<Type> rhs) {
  if (!lhs.has_value() || !rhs.has_value()) {
    return std::nullopt;
  }

  auto l = lhs.value(), r = rhs.value();

  if (l->IsReadonly() && !r->IsReadonly()) {
    r = GetConstTy(r);
  } else if (!l->IsReadonly() && r->IsReadonly()) {
    l = GetConstTy(l);
  }

  ///===========================================================================
  /// NOTE: If L && R are the same type, the type is their identity.
  if (l->IsEq(r.get())) {
    return l;
  }
  ///===========================================================================

  nr_ty_t lt = l->GetKind(), rt = r->GetKind();

  ///===========================================================================
  /// NOTE: Primitive numeric types are promoted according to the following
  /// rules:
  if (l->IsNumeric() && r->IsNumeric()) {
    ///===========================================================================
    /// NOTE: Floating point always takes precedence over integers.
    if (l->is(IR_tVOID) || r->is(IR_tVOID)) {
      return std::nullopt;
    }

    if (l->is(IR_tF128_TY) || r->is(IR_tF128_TY)) {
      return MakeFlowPtr(getF128Ty());
    }

    if (l->is(IR_tF64_TY) || r->is(IR_tF64_TY)) {
      return MakeFlowPtr(getF64Ty());
    }

    if (l->is(IR_tF32_TY) || r->is(IR_tF32_TY)) {
      return MakeFlowPtr(getF32Ty());
    }

    if (l->is(IR_tF16_TY) || r->is(IR_tF16_TY)) {
      return MakeFlowPtr(getF16Ty());
    }
    ///===========================================================================

    ///===========================================================================
    /// NOTE: If L && R are both unsigned integers, the larger type is used.
    if (l->is_unsigned() && r->is_unsigned()) {
      auto ls = l->getSizeBits(), rs = r->getSizeBits();
      if (!LS.has_value() || !RS.has_value()) {
        return std::nullopt;
      }
      return LS > RS ? l : r;
    }
    ///===========================================================================

    ///===========================================================================
    /// NOTE: If L && R are both signed integers, the larger type is used.
    if ((l->is_signed() && l->is_integral()) &&
        (r->is_signed() && r->is_integral())) {
      auto ls = l->getSizeBits(), rs = r->getSizeBits();
      if (!LS.has_value() || !RS.has_value()) {
        return std::nullopt;
      }
      return LS > RS ? l : r;
    }
    ///===========================================================================

    ///===========================================================================
    /// NOTE: If either L or R is a signed integer, the signed integer is
    /// promoted.
    if (l->is_integral() && l->is_signed()) {
      auto ls = l->getSizeBits(), rs = r->getSizeBits();
      if (!LS.has_value() || !RS.has_value()) {
        return std::nullopt;
      }
      if (ls > RS) {
        return SignedComplement(lt);
      } else {
        return r;
      }
    } else if (r->is_integral() && r->is_signed()) {
      auto ls = l->getSizeBits(), rs = r->getSizeBits();
      if (!LS.has_value() || !RS.has_value()) {
        return std::nullopt;
      }
      if (rs > LS) {
        return SignedComplement(rt);
      } else {
        return l;
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

////////////////////////////////////////////////////////////////////////////////

static NullableFlowPtr<Type> InferUnaryExpression(NullableFlowPtr<Type> e,
                                                  lex::Operator o) {
  using namespace lex;

  NullableFlowPtr<ir::Type> r;

  switch (o) {
    case OpPlus: {
      r = E;
      break;
    }

    case OpMinus: {
      r = E;
      break;
    }

    case OpTimes: {
      if (e.has_value() && E.value()->is_pointer()) {
        r = E.value()->as<PtrTy>()->getPointee();
      }
      break;
    }

    case OpBitAnd: {
      e && (r = getPtrTy(E.value()));
      break;
    }

    case OpBitNot: {
      r = E;
      break;
    }

    case OpLogicNot: {
      r = getU1Ty();
      break;
    }

    case OpInc: {
      r = E;
      break;
    }

    case OpDec: {
      r = E;
      break;
    }

    case OpSizeof: {
      r = getU64Ty();
      break;
    }

    case OpBitsizeof: {
      r = getU64Ty();
      break;
    }

    case OpAlignof: {
      r = getU64Ty();
      break;
    }

    case OpTypeof: {
      r = E;
      break;
    }

    case OpComptime: {
      r = E;
      break;
    }

    case OpSlash:
    case OpPercent:
    case OpBitOr:
    case OpBitXor:
    case OpLShift:
    case OpRShift:
    case OpROTL:
    case OpROTR:
    case OpLogicAnd:
    case OpLogicOr:
    case OpLogicXor:
    case OpLT:
    case OpGT:
    case OpLE:
    case OpGE:
    case OpEq:
    case OpNE:
    case OpSet:
    case OpPlusSet:
    case OpMinusSet:
    case OpTimesSet:
    case OpSlashSet:
    case OpPercentSet:
    case OpBitAndSet:
    case OpBitOrSet:
    case OpBitXorSet:
    case OpLogicAndSet:
    case OpLogicOrSet:
    case OpLogicXorSet:
    case OpLShiftSet:
    case OpRShiftSet:
    case OpROTLSet:
    case OpROTRSet:
    case OpAs:
    case OpBitcastAs:
    case OpIn:
    case OpOut:
    case OpDot:
    case OpRange:
    case OpEllipsis:
    case OpArrow:
    case OpTernary: {
      break;
    }
  }

  return R;
}

static NullableFlowPtr<Type> InferBinaryExpression(NullableFlowPtr<Type> lhs,
                                                   lex::Operator o,
                                                   NullableFlowPtr<Type> rhs) {
  using namespace lex;

  NullableFlowPtr<ir::Type> r;

  switch (o) {
    case OpPlus:
    case OpMinus:
    case OpTimes:
    case OpSlash:
    case OpPercent:
    case OpBitAnd:
    case OpBitOr:
    case OpBitXor:
    case OpBitNot:
    case OpIn:
    case OpOut:
    case OpSizeof:
    case OpBitsizeof:
    case OpAlignof:
    case OpTypeof:
    case OpComptime:
    case OpDot:
    case OpRange:
    case OpEllipsis:
    case OpArrow:
    case OpTernary:
    case OpInc:
    case OpDec: {
      r = promote(LHS, RHS);
    }

    case OpLogicAnd:
    case OpLogicOr:
    case OpLogicXor:
    case OpLogicNot:
    case OpLT:
    case OpGT:
    case OpLE:
    case OpGE:
    case OpEq:
    case OpNE: {
      r = getU1Ty();
      break;
    }

    case OpSet:
    case OpPlusSet:
    case OpMinusSet:
    case OpTimesSet:
    case OpSlashSet:
    case OpPercentSet:
    case OpBitAndSet:
    case OpBitOrSet:
    case OpBitXorSet:
    case OpLogicAndSet:
    case OpLogicOrSet:
    case OpLogicXorSet:
    case OpLShiftSet:
    case OpRShiftSet:
    case OpROTLSet:
    case OpROTRSet:
    case OpAs:
    case OpBitcastAs:
    case OpLShift:
    case OpRShift:
    case OpROTL:
    case OpROTR: {
      r = RHS;
      break;
    }
  }

  return R;
}

class InferenceVisitor : public IRVisitor<void> {
  std::optional<FlowPtr<Type>> m_R;

public:
  InferenceVisitor() {}
  virtual ~InferenceVisitor() = default;

  std::optional<FlowPtr<Type>> Get() { return R; }

  void visit(FlowPtr<Expr>) override {}

  void visit(FlowPtr<BinExpr> n) override {
    auto lhs = n->getLHS()->getType(), rhs = n->getRHS()->getType();

    if (auto Type = InferBinaryExpression(LHS, n->getOp(), RHS)) {
      R = Type.value();
    }
  }

  void visit(FlowPtr<Unary> n) override {
    if (auto E = n->getExpr()->getType()) {
      if (auto Type = InferUnaryExpression(E.value(), n->getOp())) {
        R = Type.value();
      }
    }
  }

  void visit(FlowPtr<U1Ty> n) override { R = n; }
  void visit(FlowPtr<U8Ty> n) override { R = n; }
  void visit(FlowPtr<U16Ty> n) override { R = n; }
  void visit(FlowPtr<U32Ty> n) override { R = n; }
  void visit(FlowPtr<U64Ty> n) override { R = n; }
  void visit(FlowPtr<U128Ty> n) override { R = n; }
  void visit(FlowPtr<I8Ty> n) override { R = n; }
  void visit(FlowPtr<I16Ty> n) override { R = n; }
  void visit(FlowPtr<I32Ty> n) override { R = n; }
  void visit(FlowPtr<I64Ty> n) override { R = n; }
  void visit(FlowPtr<I128Ty> n) override { R = n; }
  void visit(FlowPtr<F16Ty> n) override { R = n; }
  void visit(FlowPtr<F32Ty> n) override { R = n; }
  void visit(FlowPtr<F64Ty> n) override { R = n; }
  void visit(FlowPtr<F128Ty> n) override { R = n; }
  void visit(FlowPtr<VoidTy> n) override { R = n; }
  void visit(FlowPtr<PtrTy> n) override { R = n; }
  void visit(FlowPtr<ConstTy> n) override { R = n; }
  void visit(FlowPtr<OpaqueTy> n) override { R = n; }
  void visit(FlowPtr<StructTy> n) override { R = n; }
  void visit(FlowPtr<UnionTy> n) override { R = n; }
  void visit(FlowPtr<ArrayTy> n) override { R = n; }
  void visit(FlowPtr<FnTy> n) override { R = n; }

  void visit(FlowPtr<Int> n) override {
    switch (n->getSize()) {
      case 1: {
        R = getU1Ty();
        break;
      }

      case 8: {
        R = getU8Ty();
        break;
      }

      case 16: {
        R = getU16Ty();
        break;
      }

      case 32: {
        R = getU32Ty();
        break;
      }

      case 64: {
        R = getU64Ty();
        break;
      }

      case 128: {
        R = getU128Ty();
        break;
      }

      default: {
        // Invalid: integer size must be 1, 8, 16, 32, 64, or 128
      }
    }
  }

  void visit(FlowPtr<Float> n) override {
    switch (n->getSize()) {
      case 16: {
        R = getF16Ty();
        break;
      }

      case 32: {
        R = getF32Ty();
        break;
      }

      case 64: {
        R = getF64Ty();
        break;
      }

      case 128: {
        R = getF128Ty();
        break;
      }
    }
  }

  void visit(FlowPtr<List> n) override {
    if (n->empty()) {
      R = getStructTy({});
    } else if (n->isHomogenous()) {
      if (auto item_type = (*n->begin())->getType()) {
        R = getArrayTy(item_type.value(), n->size());
      }
    } else {
      std::vector<FlowPtr<Type>> types;
      types.reserve(n->size());
      bool okay = true;

      for (size_t i = 0; i < n->size(); ++i) {
        if (auto item_type = n->at(i)->getType()) {
          types.push_back(item_type.value());
        } else {
          okay = false;
          break;
        }
      }

      okay && (R = getStructTy(types));
    }
  }

  void visit(FlowPtr<Call> n) override {
    if (auto callee = n->getTarget()) {
      if (auto callee_ty = callee.value()->getType()) {
        if (callee_ty.value()->is_function()) {
          R = callee_ty.value()->as<FnTy>()->getReturn();
        }
      }
    }
  }

  void visit(FlowPtr<Seq> n) override {
    if (n->empty()) {
      R = getVoidTy();
    } else {
      R = (*n->end())->getType();
    }
  }

  void visit(FlowPtr<Index> n) override {
    auto base_type_opt = n->getExpr()->getType();

    if (base_type_opt.has_value(); auto base = base_type_opt.value()) {
      if (base->is(IR_tPTR)) {  // *X -> X
        R = base->as<PtrTy>()->getPointee();
      } else if (base->is(IR_tARRAY)) {  // [X; N] -> X
        R = base->as<ArrayTy>()->getElement();
      } else if (base->is(IR_tSTRUCT)) {  // struct { a, b, c }[1] -> b

        if (auto index = n->getIndex(); index->is(IR_eINT)) {
          auto num = index->as<Int>()->getValue();
          auto st = base->as<StructTy>();

          if (num < st->getFields().size()) {
            R = st->getFields()[num.convert_to<std::size_t>()];
          } else {
            // Out of bounds
          }
        } else {
          // Invalid: index must be of type int to index into a struct
        }
      } else if (base->is(IR_tUNION)) {
        if (auto index = n->getIndex(); index->is(IR_eINT)) {
          auto num = index->as<Int>()->getValue();
          auto st = base->as<UnionTy>();

          if (num < st->getFields().size()) {
            R = st->getFields()[num.convert_to<std::size_t>()];
          } else {
            // Out of bounds
          }
        } else {
          // Invalid: index must be of type int to index into a union
        }
      }
    }
  }

  void visit(FlowPtr<Ident> n) override {
    if (n->getWhat().has_value()) {
      R = n->getWhat().value()->getType();
    }
  }

  void visit(FlowPtr<Extern>) override { R = getVoidTy(); }
  void visit(FlowPtr<Local> n) override { R = n->getValue()->getType(); }
  void visit(FlowPtr<Ret>) override { R = getVoidTy(); }
  void visit(FlowPtr<Brk>) override { R = getVoidTy(); }
  void visit(FlowPtr<Cont>) override { R = getVoidTy(); }
  void visit(FlowPtr<If>) override { R = getVoidTy(); }
  void visit(FlowPtr<While>) override { R = getVoidTy(); }
  void visit(FlowPtr<For>) override { R = getVoidTy(); }
  void visit(FlowPtr<Case>) override { R = getVoidTy(); }
  void visit(FlowPtr<Switch>) override { R = getVoidTy(); }

  void visit(FlowPtr<Function> n) override {
    std::vector<FlowPtr<Type>> params;
    params.reserve(n->getParams().size());

    for (auto item : n->getParams()) {
      params.push_back(item.first);
    }

    R = getFnTy(params, n->getReturn(), n->isVariadic());
  }

  void visit(FlowPtr<Asm>) override { R = getVoidTy(); }
  void visit(FlowPtr<Tmp> n) override { R = n; }
};

NCC_EXPORT std::optional<FlowPtr<Type>> detail::ExprGetType(Expr* e) {
  static thread_local struct State {
    std::unordered_set<FlowPtr<Expr>> m_visited;
    size_t m_depth = 0;
  } state;

  state.m_depth++;

  InferenceVisitor visitor;
  e->Accept(visitor);

  state.m_depth--;

  if (state.m_depth == 0) {
    state.visited.clear();
  }

  return visitor.Get();
}
