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
///   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSe.  See the GNU      ///
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
      return MakeFlowPtr(GetF128Ty());
    }

    if (l->is(IR_tF64_TY) || r->is(IR_tF64_TY)) {
      return MakeFlowPtr(GetF64Ty());
    }

    if (l->is(IR_tF32_TY) || r->is(IR_tF32_TY)) {
      return MakeFlowPtr(GetF32Ty());
    }

    if (l->is(IR_tF16_TY) || r->is(IR_tF16_TY)) {
      return MakeFlowPtr(GetF16Ty());
    }
    ///===========================================================================

    ///===========================================================================
    /// NOTE: If L && R are both unsigned integers, the larger type is used.
    if (l->IsUnsigned() && r->IsUnsigned()) {
      auto ls = l->GetSizeBits(), rs = r->GetSizeBits();
      if (!ls.has_value() || !rs.has_value()) {
        return std::nullopt;
      }
      return ls > rs ? l : r;
    }
    ///===========================================================================

    ///===========================================================================
    /// NOTE: If L && R are both signed integers, the larger type is used.
    if ((l->IsSigned() && l->IsIntegral()) &&
        (r->IsSigned() && r->IsIntegral())) {
      auto ls = l->GetSizeBits(), rs = r->GetSizeBits();
      if (!ls.has_value() || !rs.has_value()) {
        return std::nullopt;
      }
      return ls > rs ? l : r;
    }
    ///===========================================================================

    ///===========================================================================
    /// NOTE: If either L or R is a signed integer, the signed integer is
    /// promoted.
    if (l->IsIntegral() && l->IsSigned()) {
      auto ls = l->GetSizeBits(), rs = r->GetSizeBits();
      if (!ls.has_value() || !rs.has_value()) {
        return std::nullopt;
      }
      if (ls > rs) {
        return SignedComplement(lt);
      } else {
        return r;
      }
    } else if (r->IsIntegral() && r->IsSigned()) {
      auto ls = l->GetSizeBits(), rs = r->GetSizeBits();
      if (!ls.has_value() || !rs.has_value()) {
        return std::nullopt;
      }
      if (rs > ls) {
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
      r = e;
      break;
    }

    case OpMinus: {
      r = e;
      break;
    }

    case OpTimes: {
      if (e.has_value() && e.value()->IsPointer()) {
        r = e.value()->as<PtrTy>()->GetPointee();
      }
      break;
    }

    case OpBitAnd: {
      e && (r = GetPtrTy(e.value()));
      break;
    }

    case OpBitNot: {
      r = e;
      break;
    }

    case OpLogicNot: {
      r = GetU1Ty();
      break;
    }

    case OpInc: {
      r = e;
      break;
    }

    case OpDec: {
      r = e;
      break;
    }

    case OpSizeof: {
      r = GetU64Ty();
      break;
    }

    case OpBitsizeof: {
      r = GetU64Ty();
      break;
    }

    case OpAlignof: {
      r = GetU64Ty();
      break;
    }

    case OpTypeof: {
      r = e;
      break;
    }

    case OpComptime: {
      r = e;
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

  return e;
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
      r = Promote(lhs, rhs);
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
      r = GetU1Ty();
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
      r = rhs;
      break;
    }
  }

  return r;
}

class InferenceVisitor : public IRVisitor<void> {
  std::optional<FlowPtr<Type>> m_r;

public:
  InferenceVisitor() {}
  virtual ~InferenceVisitor() = default;

  std::optional<FlowPtr<Type>> Get() { return m_r; }

  void Visit(FlowPtr<Expr>) override {}

  void Visit(FlowPtr<BinExpr> n) override {
    auto lhs = n->GetLHS()->GetType(), rhs = n->GetRHS()->GetType();

    if (auto type = InferBinaryExpression(lhs, n->GetOp(), rhs)) {
      m_r = type.value();
    }
  }

  void Visit(FlowPtr<Unary> n) override {
    if (auto e = n->GetExpr()->GetType()) {
      if (auto type = InferUnaryExpression(e.value(), n->GetOp())) {
        m_r = type.value();
      }
    }
  }

  void Visit(FlowPtr<U1Ty> n) override { m_r = n; }
  void Visit(FlowPtr<U8Ty> n) override { m_r = n; }
  void Visit(FlowPtr<U16Ty> n) override { m_r = n; }
  void Visit(FlowPtr<U32Ty> n) override { m_r = n; }
  void Visit(FlowPtr<U64Ty> n) override { m_r = n; }
  void Visit(FlowPtr<U128Ty> n) override { m_r = n; }
  void Visit(FlowPtr<I8Ty> n) override { m_r = n; }
  void Visit(FlowPtr<I16Ty> n) override { m_r = n; }
  void Visit(FlowPtr<I32Ty> n) override { m_r = n; }
  void Visit(FlowPtr<I64Ty> n) override { m_r = n; }
  void Visit(FlowPtr<I128Ty> n) override { m_r = n; }
  void Visit(FlowPtr<F16Ty> n) override { m_r = n; }
  void Visit(FlowPtr<F32Ty> n) override { m_r = n; }
  void Visit(FlowPtr<F64Ty> n) override { m_r = n; }
  void Visit(FlowPtr<F128Ty> n) override { m_r = n; }
  void Visit(FlowPtr<VoidTy> n) override { m_r = n; }
  void Visit(FlowPtr<PtrTy> n) override { m_r = n; }
  void Visit(FlowPtr<ConstTy> n) override { m_r = n; }
  void Visit(FlowPtr<OpaqueTy> n) override { m_r = n; }
  void Visit(FlowPtr<StructTy> n) override { m_r = n; }
  void Visit(FlowPtr<UnionTy> n) override { m_r = n; }
  void Visit(FlowPtr<ArrayTy> n) override { m_r = n; }
  void Visit(FlowPtr<FnTy> n) override { m_r = n; }

  void Visit(FlowPtr<Int> n) override {
    switch (n->GetSize()) {
      case 1: {
        m_r = GetU1Ty();
        break;
      }

      case 8: {
        m_r = GetU8Ty();
        break;
      }

      case 16: {
        m_r = GetU16Ty();
        break;
      }

      case 32: {
        m_r = GetU32Ty();
        break;
      }

      case 64: {
        m_r = GetU64Ty();
        break;
      }

      case 128: {
        m_r = GetU128Ty();
        break;
      }

      default: {
        // Invalid: integer size must be 1, 8, 16, 32, 64, or 128
      }
    }
  }

  void Visit(FlowPtr<Float> n) override {
    switch (n->GetSize()) {
      case 16: {
        m_r = GetF16Ty();
        break;
      }

      case 32: {
        m_r = GetF32Ty();
        break;
      }

      case 64: {
        m_r = GetF64Ty();
        break;
      }

      case 128: {
        m_r = GetF128Ty();
        break;
      }
    }
  }

  void Visit(FlowPtr<List> n) override {
    if (n->Empty()) {
      m_r = GetStructTy({});
    } else if (n->IsHomogenous()) {
      if (auto item_type = (*n->Begin())->GetType()) {
        m_r = GetArrayTy(item_type.value(), n->Size());
      }
    } else {
      std::vector<FlowPtr<Type>> types;
      types.reserve(n->Size());
      bool okay = true;

      for (size_t i = 0; i < n->Size(); ++i) {
        if (auto item_type = n->At(i)->GetType()) {
          types.push_back(item_type.value());
        } else {
          okay = false;
          break;
        }
      }

      okay && (m_r = GetStructTy(types));
    }
  }

  void Visit(FlowPtr<Call> n) override {
    if (auto callee = n->GetTarget()) {
      if (auto callee_ty = callee.value()->GetType()) {
        if (callee_ty.value()->IsFunction()) {
          m_r = callee_ty.value()->as<FnTy>()->GetReturn();
        }
      }
    }
  }

  void Visit(FlowPtr<Seq> n) override {
    if (n->Empty()) {
      m_r = GetVoidTy();
    } else {
      m_r = (*n->End())->GetType();
    }
  }

  void Visit(FlowPtr<Index> n) override {
    auto base_type_opt = n->GetExpr()->GetType();

    if (base_type_opt.has_value(); auto base = base_type_opt.value()) {
      if (base->is(IR_tPTR)) {  // *X -> X
        m_r = base->as<PtrTy>()->GetPointee();
      } else if (base->is(IR_tARRAY)) {  // [X; N] -> X
        m_r = base->as<ArrayTy>()->GetElement();
      } else if (base->is(IR_tSTRUCT)) {  // struct { a, b, c }[1] -> b

        if (auto index = n->GetIndex(); index->is(IR_eINT)) {
          auto num = index->as<Int>()->GetValue();
          auto st = base->as<StructTy>();

          if (num < st->GetFields().size()) {
            m_r = st->GetFields()[num.convert_to<std::size_t>()];
          } else {
            // Out of bounds
          }
        } else {
          // Invalid: index must be of type int to index into a struct
        }
      } else if (base->is(IR_tUNION)) {
        if (auto index = n->GetIndex(); index->is(IR_eINT)) {
          auto num = index->as<Int>()->GetValue();
          auto st = base->as<UnionTy>();

          if (num < st->GetFields().size()) {
            m_r = st->GetFields()[num.convert_to<std::size_t>()];
          } else {
            // Out of bounds
          }
        } else {
          // Invalid: index must be of type int to index into a union
        }
      }
    }
  }

  void Visit(FlowPtr<Ident> n) override {
    if (n->GetWhat().has_value()) {
      m_r = n->GetWhat().value()->GetType();
    }
  }

  void Visit(FlowPtr<Extern>) override { m_r = GetVoidTy(); }
  void Visit(FlowPtr<Local> n) override { m_r = n->GetValue()->GetType(); }
  void Visit(FlowPtr<Ret>) override { m_r = GetVoidTy(); }
  void Visit(FlowPtr<Brk>) override { m_r = GetVoidTy(); }
  void Visit(FlowPtr<Cont>) override { m_r = GetVoidTy(); }
  void Visit(FlowPtr<If>) override { m_r = GetVoidTy(); }
  void Visit(FlowPtr<While>) override { m_r = GetVoidTy(); }
  void Visit(FlowPtr<For>) override { m_r = GetVoidTy(); }
  void Visit(FlowPtr<Case>) override { m_r = GetVoidTy(); }
  void Visit(FlowPtr<Switch>) override { m_r = GetVoidTy(); }

  void Visit(FlowPtr<Function> n) override {
    std::vector<FlowPtr<Type>> params;
    params.reserve(n->GetParams().size());

    for (auto item : n->GetParams()) {
      params.push_back(item.first);
    }

    m_r = GetFnTy(params, n->GetReturn(), n->IsVariadic());
  }

  void Visit(FlowPtr<Asm>) override { m_r = GetVoidTy(); }
  void Visit(FlowPtr<Tmp> n) override { m_r = n; }
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
    state.m_visited.clear();
  }

  return visitor.Get();
}
