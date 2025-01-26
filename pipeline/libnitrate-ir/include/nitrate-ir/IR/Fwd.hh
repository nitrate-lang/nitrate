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
#include <cstdint>

namespace ncc::ir {
  class IRModule;

  enum nr_ty_t : uint8_t {
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
  };

  constexpr size_t kIRNodeKindCount = (IR_LAST - IR_FIRST + 1);
}  // namespace ncc::ir

namespace ncc::ir {
  template <class Attorney>
  class GenericExpr;
  template <class Attorney>
  class GenericType;
  template <class Attorney>
  class GenericBinary;
  template <class Attorney>
  class GenericUnary;
  template <class Attorney>
  class GenericU1Ty;
  template <class Attorney>
  class GenericU8Ty;
  template <class Attorney>
  class GenericU16Ty;
  template <class Attorney>
  class GenericU32Ty;
  template <class Attorney>
  class GenericU64Ty;
  template <class Attorney>
  class GenericU128Ty;
  template <class Attorney>
  class GenericI8Ty;
  template <class Attorney>
  class GenericI16Ty;
  template <class Attorney>
  class GenericI32Ty;
  template <class Attorney>
  class GenericI64Ty;
  template <class Attorney>
  class GenericI128Ty;
  template <class Attorney>
  class GenericF16Ty;
  template <class Attorney>
  class GenericF32Ty;
  template <class Attorney>
  class GenericF64Ty;
  template <class Attorney>
  class GenericF128Ty;
  template <class Attorney>
  class GenericVoidTy;
  template <class Attorney>
  class GenericPtrTy;
  template <class Attorney>
  class GenericConstTy;
  template <class Attorney>
  class GenericOpaqueTy;
  template <class Attorney>
  class GenericStructTy;
  template <class Attorney>
  class GenericUnionTy;
  template <class Attorney>
  class GenericArrayTy;
  template <class Attorney>
  class GenericFnTy;
  template <class Attorney>
  class GenericInt;
  template <class Attorney>
  class GenericFloat;
  template <class Attorney>
  class GenericList;
  template <class Attorney>
  class GenericCall;
  template <class Attorney>
  class GenericSeq;
  template <class Attorney>
  class GenericIndex;
  template <class Attorney>
  class GenericIdentifier;
  template <class Attorney>
  class GenericExtern;
  template <class Attorney>
  class GenericLocal;
  template <class Attorney>
  class GenericRet;
  template <class Attorney>
  class GenericBrk;
  template <class Attorney>
  class GenericCont;
  template <class Attorney>
  class GenericIf;
  template <class Attorney>
  class GenericWhile;
  template <class Attorney>
  class GenericFor;
  template <class Attorney>
  class GenericCase;
  template <class Attorney>
  class GenericSwitch;
  template <class Attorney>
  class GenericFunction;
  template <class Attorney>
  class GenericAsm;
  template <class Attorney>
  class GenericTmp;

  using Expr = GenericExpr<void>;
  using Type = GenericType<void>;
  using Binary = GenericBinary<void>;
  using Unary = GenericUnary<void>;
  using U1Ty = GenericU1Ty<void>;
  using U8Ty = GenericU8Ty<void>;
  using U16Ty = GenericU16Ty<void>;
  using U32Ty = GenericU32Ty<void>;
  using U64Ty = GenericU64Ty<void>;
  using U128Ty = GenericU128Ty<void>;
  using I8Ty = GenericI8Ty<void>;
  using I16Ty = GenericI16Ty<void>;
  using I32Ty = GenericI32Ty<void>;
  using I64Ty = GenericI64Ty<void>;
  using I128Ty = GenericI128Ty<void>;
  using F16Ty = GenericF16Ty<void>;
  using F32Ty = GenericF32Ty<void>;
  using F64Ty = GenericF64Ty<void>;
  using F128Ty = GenericF128Ty<void>;
  using VoidTy = GenericVoidTy<void>;
  using PtrTy = GenericPtrTy<void>;
  using ConstTy = GenericConstTy<void>;
  using OpaqueTy = GenericOpaqueTy<void>;
  using StructTy = GenericStructTy<void>;
  using UnionTy = GenericUnionTy<void>;
  using ArrayTy = GenericArrayTy<void>;
  using FnTy = GenericFnTy<void>;
  using Int = GenericInt<void>;
  using Float = GenericFloat<void>;
  using List = GenericList<void>;
  using Call = GenericCall<void>;
  using Seq = GenericSeq<void>;
  using Index = GenericIndex<void>;
  using Identifier = GenericIdentifier<void>;
  using Extern = GenericExtern<void>;
  using Local = GenericLocal<void>;
  using Ret = GenericRet<void>;
  using Brk = GenericBrk<void>;
  using Cont = GenericCont<void>;
  using If = GenericIf<void>;
  using While = GenericWhile<void>;
  using For = GenericFor<void>;
  using Case = GenericCase<void>;
  using Switch = GenericSwitch<void>;
  using Function = GenericFunction<void>;
  using Asm = GenericAsm<void>;
  using Tmp = GenericTmp<void>;
}  // namespace ncc::ir

#endif  // __NITRATE_IR_TYPE_DECL_H__
