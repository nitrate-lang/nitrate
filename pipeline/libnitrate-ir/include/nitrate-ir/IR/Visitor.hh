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
  template <typename A = void>
  class IRVisitor {
  public:
    virtual ~IRVisitor() = default;

    virtual void Visit(FlowPtr<GenericExpr<A>> n) = 0;
    virtual void Visit(FlowPtr<GenericBinExpr<A>> n) = 0;
    virtual void Visit(FlowPtr<GenericUnary<A>> n) = 0;
    virtual void Visit(FlowPtr<GenericU1Ty<A>> n) = 0;
    virtual void Visit(FlowPtr<GenericU8Ty<A>> n) = 0;
    virtual void Visit(FlowPtr<GenericU16Ty<A>> n) = 0;
    virtual void Visit(FlowPtr<GenericU32Ty<A>> n) = 0;
    virtual void Visit(FlowPtr<GenericU64Ty<A>> n) = 0;
    virtual void Visit(FlowPtr<GenericU128Ty<A>> n) = 0;
    virtual void Visit(FlowPtr<GenericI8Ty<A>> n) = 0;
    virtual void Visit(FlowPtr<GenericI16Ty<A>> n) = 0;
    virtual void Visit(FlowPtr<GenericI32Ty<A>> n) = 0;
    virtual void Visit(FlowPtr<GenericI64Ty<A>> n) = 0;
    virtual void Visit(FlowPtr<GenericI128Ty<A>> n) = 0;
    virtual void Visit(FlowPtr<GenericF16Ty<A>> n) = 0;
    virtual void Visit(FlowPtr<GenericF32Ty<A>> n) = 0;
    virtual void Visit(FlowPtr<GenericF64Ty<A>> n) = 0;
    virtual void Visit(FlowPtr<GenericF128Ty<A>> n) = 0;
    virtual void Visit(FlowPtr<GenericVoidTy<A>> n) = 0;
    virtual void Visit(FlowPtr<GenericPtrTy<A>> n) = 0;
    virtual void Visit(FlowPtr<GenericConstTy<A>> n) = 0;
    virtual void Visit(FlowPtr<GenericOpaqueTy<A>> n) = 0;
    virtual void Visit(FlowPtr<GenericStructTy<A>> n) = 0;
    virtual void Visit(FlowPtr<GenericUnionTy<A>> n) = 0;
    virtual void Visit(FlowPtr<GenericArrayTy<A>> n) = 0;
    virtual void Visit(FlowPtr<GenericFnTy<A>> n) = 0;
    virtual void Visit(FlowPtr<GenericInt<A>> n) = 0;
    virtual void Visit(FlowPtr<GenericFloat<A>> n) = 0;
    virtual void Visit(FlowPtr<GenericList<A>> n) = 0;
    virtual void Visit(FlowPtr<GenericCall<A>> n) = 0;
    virtual void Visit(FlowPtr<GenericSeq<A>> n) = 0;
    virtual void Visit(FlowPtr<GenericIndex<A>> n) = 0;
    virtual void Visit(FlowPtr<GenericIdent<A>> n) = 0;
    virtual void Visit(FlowPtr<GenericExtern<A>> n) = 0;
    virtual void Visit(FlowPtr<GenericLocal<A>> n) = 0;
    virtual void Visit(FlowPtr<GenericRet<A>> n) = 0;
    virtual void Visit(FlowPtr<GenericBrk<A>> n) = 0;
    virtual void Visit(FlowPtr<GenericCont<A>> n) = 0;
    virtual void Visit(FlowPtr<GenericIf<A>> n) = 0;
    virtual void Visit(FlowPtr<GenericWhile<A>> n) = 0;
    virtual void Visit(FlowPtr<GenericFor<A>> n) = 0;
    virtual void Visit(FlowPtr<GenericCase<A>> n) = 0;
    virtual void Visit(FlowPtr<GenericSwitch<A>> n) = 0;
    virtual void Visit(FlowPtr<GenericFunction<A>> n) = 0;
    virtual void Visit(FlowPtr<GenericAsm<A>> n) = 0;
    virtual void Visit(FlowPtr<GenericTmp<A>> n) = 0;

    template <typename T>
    void Dispatch(FlowPtr<T> n) {
      switch (n->GetKind()) {
        case IR_eBIN: {
          visit(n.template as<GenericBinExpr<A>>());
          break;
        }

        case IR_eUNARY: {
          visit(n.template as<GenericUnary<A>>());
          break;
        }

        case IR_eINT: {
          visit(n.template as<GenericInt<A>>());
          break;
        }

        case IR_eFLOAT: {
          visit(n.template as<GenericFloat<A>>());
          break;
        }

        case IR_eLIST: {
          visit(n.template as<GenericList<A>>());
          break;
        }

        case IR_eCALL: {
          visit(n.template as<GenericCall<A>>());
          break;
        }

        case IR_eSEQ: {
          visit(n.template as<GenericSeq<A>>());
          break;
        }

        case IR_eINDEX: {
          visit(n.template as<GenericIndex<A>>());
          break;
        }

        case IR_eIDENT: {
          visit(n.template as<GenericIdent<A>>());
          break;
        }

        case IR_eEXTERN: {
          visit(n.template as<GenericExtern<A>>());
          break;
        }

        case IR_eLOCAL: {
          visit(n.template as<GenericLocal<A>>());
          break;
        }

        case IR_eRET: {
          visit(n.template as<GenericRet<A>>());
          break;
        }

        case IR_eBRK: {
          visit(n.template as<GenericBrk<A>>());
          break;
        }

        case IR_eSKIP: {
          visit(n.template as<GenericCont<A>>());
          break;
        }

        case IR_eIF: {
          visit(n.template as<GenericIf<A>>());
          break;
        }

        case IR_eWHILE: {
          visit(n.template as<GenericWhile<A>>());
          break;
        }

        case IR_eFOR: {
          visit(n.template as<GenericFor<A>>());
          break;
        }

        case IR_eCASE: {
          visit(n.template as<GenericCase<A>>());
          break;
        }

        case IR_eSWITCH: {
          visit(n.template as<GenericSwitch<A>>());
          break;
        }

        case IR_eFUNCTION: {
          visit(n.template as<GenericFunction<A>>());
          break;
        }

        case IR_eASM: {
          visit(n.template as<GenericAsm<A>>());
          break;
        }

        case IR_eIGN: {
          visit(n.template as<GenericExpr<A>>());
          break;
        }

        case IR_tU1: {
          visit(n.template as<GenericU1Ty<A>>());
          break;
        }

        case IR_tU8: {
          visit(n.template as<GenericU8Ty<A>>());
          break;
        }

        case IR_tU16: {
          visit(n.template as<GenericU16Ty<A>>());
          break;
        }

        case IR_tU32: {
          visit(n.template as<GenericU32Ty<A>>());
          break;
        }

        case IR_tU64: {
          visit(n.template as<GenericU64Ty<A>>());
          break;
        }

        case IR_tU128: {
          visit(n.template as<GenericU128Ty<A>>());
          break;
        }

        case IR_tI8: {
          visit(n.template as<GenericI8Ty<A>>());
          break;
        }

        case IR_tI16: {
          visit(n.template as<GenericI16Ty<A>>());
          break;
        }

        case IR_tI32: {
          visit(n.template as<GenericI32Ty<A>>());
          break;
        }

        case IR_tI64: {
          visit(n.template as<GenericI64Ty<A>>());
          break;
        }

        case IR_tI128: {
          visit(n.template as<GenericI128Ty<A>>());
          break;
        }

        case IR_tF16_TY: {
          visit(n.template as<GenericF16Ty<A>>());
          break;
        }

        case IR_tF32_TY: {
          visit(n.template as<GenericF32Ty<A>>());
          break;
        }

        case IR_tF64_TY: {
          visit(n.template as<GenericF64Ty<A>>());
          break;
        }

        case IR_tF128_TY: {
          visit(n.template as<GenericF128Ty<A>>());
          break;
        }

        case IR_tVOID: {
          visit(n.template as<GenericVoidTy<A>>());
          break;
        }

        case IR_tPTR: {
          visit(n.template as<GenericPtrTy<A>>());
          break;
        }

        case IR_tCONST: {
          visit(n.template as<GenericConstTy<A>>());
          break;
        }

        case IR_tOPAQUE: {
          visit(n.template as<GenericOpaqueTy<A>>());
          break;
        }

        case IR_tSTRUCT: {
          visit(n.template as<GenericStructTy<A>>());
          break;
        }

        case IR_tUNION: {
          visit(n.template as<GenericUnionTy<A>>());
          break;
        }

        case IR_tARRAY: {
          visit(n.template as<GenericArrayTy<A>>());
          break;
        }

        case IR_tFUNC: {
          visit(n.template as<GenericFnTy<A>>());
          break;
        }

        case IR_tTMP: {
          visit(n.template as<GenericTmp<A>>());
          break;
        }
      }
    }
  };
}  // namespace ncc::ir

#endif  // __NITRATE_IR_VISITOR_H__
