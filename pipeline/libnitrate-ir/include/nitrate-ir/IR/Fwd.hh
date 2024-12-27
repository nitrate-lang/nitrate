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

#ifndef __NITRATE_IR_TYPE_DECL_H__
#define __NITRATE_IR_TYPE_DECL_H__

#include <cstddef>

namespace ncc::ir {
  class IRModule;

  typedef enum nr_ty_t {
    IR_eBIN,      /* Binary expression */
    IR_eUNARY,    /* Unary expression */
    IR_eINT,      /* Integer literal */
    IR_eFLOAT,    /* Floating-point literal */
    IR_eLIST,     /* List literal */
    IR_eCALL,     /* Function call */
    IR_eSEQ,      /* Sequence */
    IR_eINDEX,    /* Index / member access */
    IR_eIDENT,    /* Identifier */
    IR_eEXTERN,   /* Linker visibility modifier */
    IR_eLOCAL,    /* Variable declaration */
    IR_eRET,      /* Return statement */
    IR_eBRK,      /* Break statement */
    IR_eSKIP,     /* Continue statement */
    IR_eIF,       /* If statement */
    IR_eWHILE,    /* While loop */
    IR_eFOR,      /* For loop */
    IR_eCASE,     /* Case statement */
    IR_eSWITCH,   /* Switch statement */
    IR_eFUNCTION, /* Function definition */
    IR_eASM,      /* Inline assembly */
    IR_eIGN,      /* No-op */

    IR_tU1,      /* 1-bit unsigned integer (boolean) */
    IR_tU8,      /* 8-bit unsigned integer */
    IR_tU16,     /* 16-bit unsigned integer */
    IR_tU32,     /* 32-bit unsigned integer */
    IR_tU64,     /* 64-bit unsigned integer */
    IR_tU128,    /* 128-bit unsigned integer */
    IR_tI8,      /* 8-bit signed integer */
    IR_tI16,     /* 16-bit signed integer */
    IR_tI32,     /* 32-bit signed integer */
    IR_tI64,     /* 64-bit signed integer */
    IR_tI128,    /* 128-bit signed integer */
    IR_tF16_TY,  /* 16-bit floating-point */
    IR_tF32_TY,  /* 32-bit floating-point */
    IR_tF64_TY,  /* 64-bit floating-point */
    IR_tF128_TY, /* 128-bit floating-point */
    IR_tVOID,    /* Void type */
    IR_tPTR,     /* Pointer type */
    IR_tOPAQUE,  /* Opaque type */
    IR_tSTRUCT,  /* Struct type */
    IR_tUNION,   /* Union type */
    IR_tARRAY,   /* Array type */
    IR_tFUNC,    /* Function type */
    IR_tCONST,   /* Constant wrapper type */

    IR_tTMP, /* Temp node; must be resolved with more information */

    IR_FIRST = IR_eBIN,
    IR_LAST = IR_tTMP,
  } nr_ty_t;

  constexpr size_t IR_COUNT = (IR_LAST - IR_FIRST + 1);
}  // namespace ncc::ir

namespace ncc::ir {
  template <class Attorney>
  class IR_Vertex_Expr;
  template <class Attorney>
  class IR_Vertex_Type;
  template <class Attorney>
  class IR_Vertex_BinExpr;
  template <class Attorney>
  class IR_Vertex_Unary;
  template <class Attorney>
  class IR_Vertex_U1Ty;
  template <class Attorney>
  class IR_Vertex_U8Ty;
  template <class Attorney>
  class IR_Vertex_U16Ty;
  template <class Attorney>
  class IR_Vertex_U32Ty;
  template <class Attorney>
  class IR_Vertex_U64Ty;
  template <class Attorney>
  class IR_Vertex_U128Ty;
  template <class Attorney>
  class IR_Vertex_I8Ty;
  template <class Attorney>
  class IR_Vertex_I16Ty;
  template <class Attorney>
  class IR_Vertex_I32Ty;
  template <class Attorney>
  class IR_Vertex_I64Ty;
  template <class Attorney>
  class IR_Vertex_I128Ty;
  template <class Attorney>
  class IR_Vertex_F16Ty;
  template <class Attorney>
  class IR_Vertex_F32Ty;
  template <class Attorney>
  class IR_Vertex_F64Ty;
  template <class Attorney>
  class IR_Vertex_F128Ty;
  template <class Attorney>
  class IR_Vertex_VoidTy;
  template <class Attorney>
  class IR_Vertex_PtrTy;
  template <class Attorney>
  class IR_Vertex_ConstTy;
  template <class Attorney>
  class IR_Vertex_OpaqueTy;
  template <class Attorney>
  class IR_Vertex_StructTy;
  template <class Attorney>
  class IR_Vertex_UnionTy;
  template <class Attorney>
  class IR_Vertex_ArrayTy;
  template <class Attorney>
  class IR_Vertex_FunctionTy;
  template <class Attorney>
  class IR_Vertex_Int;
  template <class Attorney>
  class IR_Vertex_Float;
  template <class Attorney>
  class IR_Vertex_List;
  template <class Attorney>
  class IR_Vertex_Call;
  template <class Attorney>
  class IR_Vertex_Seq;
  template <class Attorney>
  class IR_Vertex_Index;
  template <class Attorney>
  class IR_Vertex_Ident;
  template <class Attorney>
  class IR_Vertex_Extern;
  template <class Attorney>
  class IR_Vertex_Local;
  template <class Attorney>
  class IR_Vertex_Ret;
  template <class Attorney>
  class IR_Vertex_Brk;
  template <class Attorney>
  class IR_Vertex_Cont;
  template <class Attorney>
  class IR_Vertex_If;
  template <class Attorney>
  class IR_Vertex_While;
  template <class Attorney>
  class IR_Vertex_For;
  template <class Attorney>
  class IR_Vertex_Case;
  template <class Attorney>
  class IR_Vertex_Switch;
  template <class Attorney>
  class IR_Vertex_Function;
  template <class Attorney>
  class IR_Vertex_Asm;
  template <class Attorney>
  class IR_Vertex_Tmp;

  using Expr = IR_Vertex_Expr<void>;
  using Type = IR_Vertex_Type<void>;
  using BinExpr = IR_Vertex_BinExpr<void>;
  using Unary = IR_Vertex_Unary<void>;
  using U1Ty = IR_Vertex_U1Ty<void>;
  using U8Ty = IR_Vertex_U8Ty<void>;
  using U16Ty = IR_Vertex_U16Ty<void>;
  using U32Ty = IR_Vertex_U32Ty<void>;
  using U64Ty = IR_Vertex_U64Ty<void>;
  using U128Ty = IR_Vertex_U128Ty<void>;
  using I8Ty = IR_Vertex_I8Ty<void>;
  using I16Ty = IR_Vertex_I16Ty<void>;
  using I32Ty = IR_Vertex_I32Ty<void>;
  using I64Ty = IR_Vertex_I64Ty<void>;
  using I128Ty = IR_Vertex_I128Ty<void>;
  using F16Ty = IR_Vertex_F16Ty<void>;
  using F32Ty = IR_Vertex_F32Ty<void>;
  using F64Ty = IR_Vertex_F64Ty<void>;
  using F128Ty = IR_Vertex_F128Ty<void>;
  using VoidTy = IR_Vertex_VoidTy<void>;
  using PtrTy = IR_Vertex_PtrTy<void>;
  using ConstTy = IR_Vertex_ConstTy<void>;
  using OpaqueTy = IR_Vertex_OpaqueTy<void>;
  using StructTy = IR_Vertex_StructTy<void>;
  using UnionTy = IR_Vertex_UnionTy<void>;
  using ArrayTy = IR_Vertex_ArrayTy<void>;
  using FnTy = IR_Vertex_FunctionTy<void>;
  using Int = IR_Vertex_Int<void>;
  using Float = IR_Vertex_Float<void>;
  using List = IR_Vertex_List<void>;
  using Call = IR_Vertex_Call<void>;
  using Seq = IR_Vertex_Seq<void>;
  using Index = IR_Vertex_Index<void>;
  using Ident = IR_Vertex_Ident<void>;
  using Extern = IR_Vertex_Extern<void>;
  using Local = IR_Vertex_Local<void>;
  using Ret = IR_Vertex_Ret<void>;
  using Brk = IR_Vertex_Brk<void>;
  using Cont = IR_Vertex_Cont<void>;
  using If = IR_Vertex_If<void>;
  using While = IR_Vertex_While<void>;
  using For = IR_Vertex_For<void>;
  using Case = IR_Vertex_Case<void>;
  using Switch = IR_Vertex_Switch<void>;
  using Function = IR_Vertex_Function<void>;
  using Asm = IR_Vertex_Asm<void>;
  using Tmp = IR_Vertex_Tmp<void>;
}  // namespace ncc::ir

#endif  // __NITRATE_IR_TYPE_DECL_H__
