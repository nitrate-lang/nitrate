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

    virtual void visit(FlowPtr<IR_Vertex_Expr<A>> n) = 0;
    virtual void visit(FlowPtr<IR_Vertex_BinExpr<A>> n) = 0;
    virtual void visit(FlowPtr<IR_Vertex_Unary<A>> n) = 0;
    virtual void visit(FlowPtr<IR_Vertex_U1Ty<A>> n) = 0;
    virtual void visit(FlowPtr<IR_Vertex_U8Ty<A>> n) = 0;
    virtual void visit(FlowPtr<IR_Vertex_U16Ty<A>> n) = 0;
    virtual void visit(FlowPtr<IR_Vertex_U32Ty<A>> n) = 0;
    virtual void visit(FlowPtr<IR_Vertex_U64Ty<A>> n) = 0;
    virtual void visit(FlowPtr<IR_Vertex_U128Ty<A>> n) = 0;
    virtual void visit(FlowPtr<IR_Vertex_I8Ty<A>> n) = 0;
    virtual void visit(FlowPtr<IR_Vertex_I16Ty<A>> n) = 0;
    virtual void visit(FlowPtr<IR_Vertex_I32Ty<A>> n) = 0;
    virtual void visit(FlowPtr<IR_Vertex_I64Ty<A>> n) = 0;
    virtual void visit(FlowPtr<IR_Vertex_I128Ty<A>> n) = 0;
    virtual void visit(FlowPtr<IR_Vertex_F16Ty<A>> n) = 0;
    virtual void visit(FlowPtr<IR_Vertex_F32Ty<A>> n) = 0;
    virtual void visit(FlowPtr<IR_Vertex_F64Ty<A>> n) = 0;
    virtual void visit(FlowPtr<IR_Vertex_F128Ty<A>> n) = 0;
    virtual void visit(FlowPtr<IR_Vertex_VoidTy<A>> n) = 0;
    virtual void visit(FlowPtr<IR_Vertex_PtrTy<A>> n) = 0;
    virtual void visit(FlowPtr<IR_Vertex_ConstTy<A>> n) = 0;
    virtual void visit(FlowPtr<IR_Vertex_OpaqueTy<A>> n) = 0;
    virtual void visit(FlowPtr<IR_Vertex_StructTy<A>> n) = 0;
    virtual void visit(FlowPtr<IR_Vertex_UnionTy<A>> n) = 0;
    virtual void visit(FlowPtr<IR_Vertex_ArrayTy<A>> n) = 0;
    virtual void visit(FlowPtr<IR_Vertex_FnTy<A>> n) = 0;
    virtual void visit(FlowPtr<IR_Vertex_Int<A>> n) = 0;
    virtual void visit(FlowPtr<IR_Vertex_Float<A>> n) = 0;
    virtual void visit(FlowPtr<IR_Vertex_List<A>> n) = 0;
    virtual void visit(FlowPtr<IR_Vertex_Call<A>> n) = 0;
    virtual void visit(FlowPtr<IR_Vertex_Seq<A>> n) = 0;
    virtual void visit(FlowPtr<IR_Vertex_Index<A>> n) = 0;
    virtual void visit(FlowPtr<IR_Vertex_Ident<A>> n) = 0;
    virtual void visit(FlowPtr<IR_Vertex_Extern<A>> n) = 0;
    virtual void visit(FlowPtr<IR_Vertex_Local<A>> n) = 0;
    virtual void visit(FlowPtr<IR_Vertex_Ret<A>> n) = 0;
    virtual void visit(FlowPtr<IR_Vertex_Brk<A>> n) = 0;
    virtual void visit(FlowPtr<IR_Vertex_Cont<A>> n) = 0;
    virtual void visit(FlowPtr<IR_Vertex_If<A>> n) = 0;
    virtual void visit(FlowPtr<IR_Vertex_While<A>> n) = 0;
    virtual void visit(FlowPtr<IR_Vertex_For<A>> n) = 0;
    virtual void visit(FlowPtr<IR_Vertex_Case<A>> n) = 0;
    virtual void visit(FlowPtr<IR_Vertex_Switch<A>> n) = 0;
    virtual void visit(FlowPtr<IR_Vertex_Function<A>> n) = 0;
    virtual void visit(FlowPtr<IR_Vertex_Asm<A>> n) = 0;
    virtual void visit(FlowPtr<IR_Vertex_Tmp<A>> n) = 0;

    template <typename T>
    void dispatch(FlowPtr<T> n) {
      switch (n->GetKind()) {
        case IR_eBIN: {
          visit(n.template as<IR_Vertex_BinExpr<A>>());
          break;
        }

        case IR_eUNARY: {
          visit(n.template as<IR_Vertex_Unary<A>>());
          break;
        }

        case IR_eINT: {
          visit(n.template as<IR_Vertex_Int<A>>());
          break;
        }

        case IR_eFLOAT: {
          visit(n.template as<IR_Vertex_Float<A>>());
          break;
        }

        case IR_eLIST: {
          visit(n.template as<IR_Vertex_List<A>>());
          break;
        }

        case IR_eCALL: {
          visit(n.template as<IR_Vertex_Call<A>>());
          break;
        }

        case IR_eSEQ: {
          visit(n.template as<IR_Vertex_Seq<A>>());
          break;
        }

        case IR_eINDEX: {
          visit(n.template as<IR_Vertex_Index<A>>());
          break;
        }

        case IR_eIDENT: {
          visit(n.template as<IR_Vertex_Ident<A>>());
          break;
        }

        case IR_eEXTERN: {
          visit(n.template as<IR_Vertex_Extern<A>>());
          break;
        }

        case IR_eLOCAL: {
          visit(n.template as<IR_Vertex_Local<A>>());
          break;
        }

        case IR_eRET: {
          visit(n.template as<IR_Vertex_Ret<A>>());
          break;
        }

        case IR_eBRK: {
          visit(n.template as<IR_Vertex_Brk<A>>());
          break;
        }

        case IR_eSKIP: {
          visit(n.template as<IR_Vertex_Cont<A>>());
          break;
        }

        case IR_eIF: {
          visit(n.template as<IR_Vertex_If<A>>());
          break;
        }

        case IR_eWHILE: {
          visit(n.template as<IR_Vertex_While<A>>());
          break;
        }

        case IR_eFOR: {
          visit(n.template as<IR_Vertex_For<A>>());
          break;
        }

        case IR_eCASE: {
          visit(n.template as<IR_Vertex_Case<A>>());
          break;
        }

        case IR_eSWITCH: {
          visit(n.template as<IR_Vertex_Switch<A>>());
          break;
        }

        case IR_eFUNCTION: {
          visit(n.template as<IR_Vertex_Function<A>>());
          break;
        }

        case IR_eASM: {
          visit(n.template as<IR_Vertex_Asm<A>>());
          break;
        }

        case IR_eIGN: {
          visit(n.template as<IR_Vertex_Expr<A>>());
          break;
        }

        case IR_tU1: {
          visit(n.template as<IR_Vertex_U1Ty<A>>());
          break;
        }

        case IR_tU8: {
          visit(n.template as<IR_Vertex_U8Ty<A>>());
          break;
        }

        case IR_tU16: {
          visit(n.template as<IR_Vertex_U16Ty<A>>());
          break;
        }

        case IR_tU32: {
          visit(n.template as<IR_Vertex_U32Ty<A>>());
          break;
        }

        case IR_tU64: {
          visit(n.template as<IR_Vertex_U64Ty<A>>());
          break;
        }

        case IR_tU128: {
          visit(n.template as<IR_Vertex_U128Ty<A>>());
          break;
        }

        case IR_tI8: {
          visit(n.template as<IR_Vertex_I8Ty<A>>());
          break;
        }

        case IR_tI16: {
          visit(n.template as<IR_Vertex_I16Ty<A>>());
          break;
        }

        case IR_tI32: {
          visit(n.template as<IR_Vertex_I32Ty<A>>());
          break;
        }

        case IR_tI64: {
          visit(n.template as<IR_Vertex_I64Ty<A>>());
          break;
        }

        case IR_tI128: {
          visit(n.template as<IR_Vertex_I128Ty<A>>());
          break;
        }

        case IR_tF16_TY: {
          visit(n.template as<IR_Vertex_F16Ty<A>>());
          break;
        }

        case IR_tF32_TY: {
          visit(n.template as<IR_Vertex_F32Ty<A>>());
          break;
        }

        case IR_tF64_TY: {
          visit(n.template as<IR_Vertex_F64Ty<A>>());
          break;
        }

        case IR_tF128_TY: {
          visit(n.template as<IR_Vertex_F128Ty<A>>());
          break;
        }

        case IR_tVOID: {
          visit(n.template as<IR_Vertex_VoidTy<A>>());
          break;
        }

        case IR_tPTR: {
          visit(n.template as<IR_Vertex_PtrTy<A>>());
          break;
        }

        case IR_tCONST: {
          visit(n.template as<IR_Vertex_ConstTy<A>>());
          break;
        }

        case IR_tOPAQUE: {
          visit(n.template as<IR_Vertex_OpaqueTy<A>>());
          break;
        }

        case IR_tSTRUCT: {
          visit(n.template as<IR_Vertex_StructTy<A>>());
          break;
        }

        case IR_tUNION: {
          visit(n.template as<IR_Vertex_UnionTy<A>>());
          break;
        }

        case IR_tARRAY: {
          visit(n.template as<IR_Vertex_ArrayTy<A>>());
          break;
        }

        case IR_tFUNC: {
          visit(n.template as<IR_Vertex_FnTy<A>>());
          break;
        }

        case IR_tTMP: {
          visit(n.template as<IR_Vertex_Tmp<A>>());
          break;
        }
      }
    }
  };
}  // namespace ncc::ir

#endif  // __NITRATE_IR_VISITOR_H__
