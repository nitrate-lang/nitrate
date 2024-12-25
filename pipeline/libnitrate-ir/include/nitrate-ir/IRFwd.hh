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
  /** @brief Nitrate abstract syntax tree node */
  typedef struct nr_node_t nr_node_t;

  /** @brief Nitrate NR module */
  typedef struct qmodule_t qmodule_t;

  /** @brief Nitrate abstract syntax tree node type */
  typedef enum nr_ty_t {
    IR_eBIN,         /* Binary expression */
    IR_eUNARY,       /* Unary expression */
    IR_ePOST_UNEXPR, /* Postfix unary expression */

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
  class Expr;
  class Type;
  class BinExpr;
  class UnExpr;
  class PostUnExpr;
  class U1Ty;
  class U8Ty;
  class U16Ty;
  class U32Ty;
  class U64Ty;
  class U128Ty;
  class I8Ty;
  class I16Ty;
  class I32Ty;
  class I64Ty;
  class I128Ty;
  class F16Ty;
  class F32Ty;
  class F64Ty;
  class F128Ty;
  class VoidTy;
  class PtrTy;
  class ConstTy;
  class OpaqueTy;
  class StructTy;
  class UnionTy;
  class ArrayTy;
  class FnTy;
  class Int;
  class Float;
  class List;
  class Call;
  class Seq;
  class Index;
  class Ident;
  class Extern;
  class Local;
  class Ret;
  class Brk;
  class Cont;
  class If;
  class While;
  class For;
  class Case;
  class Switch;
  class Fn;
  class Asm;
  class Tmp;
}  // namespace ncc::ir

#endif  // __NITRATE_IR_TYPE_DECL_H__
