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

#ifndef __NITRATE_IR_VISITOR_H__
#define __NITRATE_IR_VISITOR_H__

#include <nitrate-core/FlowPtr.hh>
#include <nitrate-ir/IR/Fwd.hh>

namespace ncc::ir {
  class IRVisitor {
  public:
    virtual ~IRVisitor() = default;

    virtual void visit(FlowPtr<Expr> n) = 0;
    virtual void visit(FlowPtr<Type> n) = 0;
    virtual void visit(FlowPtr<BinExpr> n) = 0;
    virtual void visit(FlowPtr<Unary> n) = 0;
    virtual void visit(FlowPtr<U1Ty> n) = 0;
    virtual void visit(FlowPtr<U8Ty> n) = 0;
    virtual void visit(FlowPtr<U16Ty> n) = 0;
    virtual void visit(FlowPtr<U32Ty> n) = 0;
    virtual void visit(FlowPtr<U64Ty> n) = 0;
    virtual void visit(FlowPtr<U128Ty> n) = 0;
    virtual void visit(FlowPtr<I8Ty> n) = 0;
    virtual void visit(FlowPtr<I16Ty> n) = 0;
    virtual void visit(FlowPtr<I32Ty> n) = 0;
    virtual void visit(FlowPtr<I64Ty> n) = 0;
    virtual void visit(FlowPtr<I128Ty> n) = 0;
    virtual void visit(FlowPtr<F16Ty> n) = 0;
    virtual void visit(FlowPtr<F32Ty> n) = 0;
    virtual void visit(FlowPtr<F64Ty> n) = 0;
    virtual void visit(FlowPtr<F128Ty> n) = 0;
    virtual void visit(FlowPtr<VoidTy> n) = 0;
    virtual void visit(FlowPtr<PtrTy> n) = 0;
    virtual void visit(FlowPtr<ConstTy> n) = 0;
    virtual void visit(FlowPtr<OpaqueTy> n) = 0;
    virtual void visit(FlowPtr<StructTy> n) = 0;
    virtual void visit(FlowPtr<UnionTy> n) = 0;
    virtual void visit(FlowPtr<ArrayTy> n) = 0;
    virtual void visit(FlowPtr<FnTy> n) = 0;
    virtual void visit(FlowPtr<Int> n) = 0;
    virtual void visit(FlowPtr<Float> n) = 0;
    virtual void visit(FlowPtr<List> n) = 0;
    virtual void visit(FlowPtr<Call> n) = 0;
    virtual void visit(FlowPtr<Seq> n) = 0;
    virtual void visit(FlowPtr<Index> n) = 0;
    virtual void visit(FlowPtr<Ident> n) = 0;
    virtual void visit(FlowPtr<Extern> n) = 0;
    virtual void visit(FlowPtr<Local> n) = 0;
    virtual void visit(FlowPtr<Ret> n) = 0;
    virtual void visit(FlowPtr<Brk> n) = 0;
    virtual void visit(FlowPtr<Cont> n) = 0;
    virtual void visit(FlowPtr<If> n) = 0;
    virtual void visit(FlowPtr<While> n) = 0;
    virtual void visit(FlowPtr<For> n) = 0;
    virtual void visit(FlowPtr<Case> n) = 0;
    virtual void visit(FlowPtr<Switch> n) = 0;
    virtual void visit(FlowPtr<Function> n) = 0;
    virtual void visit(FlowPtr<Asm> n) = 0;
    virtual void visit(FlowPtr<Tmp> n) = 0;

    template <typename T>
    void dispatch(FlowPtr<T> n) {
      switch (n->getKind()) {
        case IR_eBIN: {
          visit(n.template as<BinExpr>());
          break;
        }

        case IR_eUNARY: {
          visit(n.template as<Unary>());
          break;
        }

        case IR_eINT: {
          visit(n.template as<Int>());
          break;
        }

        case IR_eFLOAT: {
          visit(n.template as<Float>());
          break;
        }

        case IR_eLIST: {
          visit(n.template as<List>());
          break;
        }

        case IR_eCALL: {
          visit(n.template as<Call>());
          break;
        }

        case IR_eSEQ: {
          visit(n.template as<Seq>());
          break;
        }

        case IR_eINDEX: {
          visit(n.template as<Index>());
          break;
        }

        case IR_eIDENT: {
          visit(n.template as<Ident>());
          break;
        }

        case IR_eEXTERN: {
          visit(n.template as<Extern>());
          break;
        }

        case IR_eLOCAL: {
          visit(n.template as<Local>());
          break;
        }

        case IR_eRET: {
          visit(n.template as<Ret>());
          break;
        }

        case IR_eBRK: {
          visit(n.template as<Brk>());
          break;
        }

        case IR_eSKIP: {
          visit(n.template as<Cont>());
          break;
        }

        case IR_eIF: {
          visit(n.template as<If>());
          break;
        }

        case IR_eWHILE: {
          visit(n.template as<While>());
          break;
        }

        case IR_eFOR: {
          visit(n.template as<For>());
          break;
        }

        case IR_eCASE: {
          visit(n.template as<Case>());
          break;
        }

        case IR_eSWITCH: {
          visit(n.template as<Switch>());
          break;
        }

        case IR_eFUNCTION: {
          visit(n.template as<Function>());
          break;
        }

        case IR_eASM: {
          visit(n.template as<Asm>());
          break;
        }

        case IR_eIGN: {
          visit(n.template as<Expr>());
          break;
        }

        case IR_tU1: {
          visit(n.template as<U1Ty>());
          break;
        }

        case IR_tU8: {
          visit(n.template as<U8Ty>());
          break;
        }

        case IR_tU16: {
          visit(n.template as<U16Ty>());
          break;
        }

        case IR_tU32: {
          visit(n.template as<U32Ty>());
          break;
        }

        case IR_tU64: {
          visit(n.template as<U64Ty>());
          break;
        }

        case IR_tU128: {
          visit(n.template as<U128Ty>());
          break;
        }

        case IR_tI8: {
          visit(n.template as<I8Ty>());
          break;
        }

        case IR_tI16: {
          visit(n.template as<I16Ty>());
          break;
        }

        case IR_tI32: {
          visit(n.template as<I32Ty>());
          break;
        }

        case IR_tI64: {
          visit(n.template as<I64Ty>());
          break;
        }

        case IR_tI128: {
          visit(n.template as<I128Ty>());
          break;
        }

        case IR_tF16_TY: {
          visit(n.template as<F16Ty>());
          break;
        }

        case IR_tF32_TY: {
          visit(n.template as<F32Ty>());
          break;
        }

        case IR_tF64_TY: {
          visit(n.template as<F64Ty>());
          break;
        }

        case IR_tF128_TY: {
          visit(n.template as<F128Ty>());
          break;
        }

        case IR_tVOID: {
          visit(n.template as<VoidTy>());
          break;
        }

        case IR_tPTR: {
          visit(n.template as<PtrTy>());
          break;
        }

        case IR_tCONST: {
          visit(n.template as<ConstTy>());
          break;
        }

        case IR_tOPAQUE: {
          visit(n.template as<OpaqueTy>());
          break;
        }

        case IR_tSTRUCT: {
          visit(n.template as<StructTy>());
          break;
        }

        case IR_tUNION: {
          visit(n.template as<UnionTy>());
          break;
        }

        case IR_tARRAY: {
          visit(n.template as<ArrayTy>());
          break;
        }

        case IR_tFUNC: {
          visit(n.template as<FnTy>());
          break;
        }

        case IR_tTMP: {
          visit(n.template as<Tmp>());
          break;
        }
      }
    }
  };
}  // namespace ncc::ir

#endif  // __NITRATE_IR_VISITOR_H__
