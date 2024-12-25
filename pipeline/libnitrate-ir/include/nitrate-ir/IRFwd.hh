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
    IR_BINEXPR,     /* Binary expression */
    IR_UNEXPR,      /* Unary expression */
    IR_POST_UNEXPR, /* Postfix unary expression */

    IR_INT,   /* Integer literal */
    IR_FLOAT, /* Floating-point literal */
    IR_LIST,  /* List literal */

    IR_CALL,   /* Function call */
    IR_SEQ,    /* Sequence */
    IR_INDEX,  /* Index / member access */
    IR_IDENT,  /* Identifier */
    IR_EXTERN, /* Linker visibility modifier */
    IR_LOCAL,  /* Variable declaration */
    IR_RET,    /* Return statement */
    IR_BRK,    /* Break statement */
    IR_CONT,   /* Continue statement */
    IR_IF,     /* If statement */
    IR_WHILE,  /* While loop */
    IR_FOR,    /* For loop */
    IR_CASE,   /* Case statement */
    IR_SWITCH, /* Switch statement */
    IR_FN,     /* Function definition */
    IR_ASM,    /* Inline assembly */
    IR_IGN,    /* No-op */

    IR_U1,        /* 1-bit unsigned integer (boolean) */
    IR_U8,        /* 8-bit unsigned integer */
    IR_U16,       /* 16-bit unsigned integer */
    IR_U32,       /* 32-bit unsigned integer */
    IR_U64,       /* 64-bit unsigned integer */
    IR_U128,      /* 128-bit unsigned integer */
    IR_I8,        /* 8-bit signed integer */
    IR_I16,       /* 16-bit signed integer */
    IR_I32,       /* 32-bit signed integer */
    IR_I64,       /* 64-bit signed integer */
    IR_I128,      /* 128-bit signed integer */
    IR_F16_TY,    /* 16-bit floating-point */
    IR_F32_TY,    /* 32-bit floating-point */
    IR_F64_TY,    /* 64-bit floating-point */
    IR_F128_TY,   /* 128-bit floating-point */
    IR_VOID_TY,   /* Void type */
    IR_PTR_TY,    /* Pointer type */
    IR_OPAQUE_TY, /* Opaque type */
    IR_STRUCT_TY, /* Struct type */
    IR_UNION,     /* Union type */
    IR_ARRAY_TY,  /* Array type */
    IR_FN_TY,     /* Function type */
    IR_CONST_TY,  /* Constant wrapper type */

    IR_TMP, /* Temp node; must be resolved with more information */

    IR_FIRST = IR_BINEXPR,
    IR_LAST = IR_TMP,
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
