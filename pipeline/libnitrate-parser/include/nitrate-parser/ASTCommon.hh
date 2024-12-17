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

#ifndef __NITRATE_AST_ASTCOMMON_H__
#define __NITRATE_AST_ASTCOMMON_H__

#include <boost/flyweight.hpp>
#include <boost/flyweight/no_tracking.hpp>
#include <string>

typedef enum npar_ty_t {
  /*****************************************************************************
   * Base
   ****************************************************************************/

  QAST_BASE = 0, /* Polymorphic base node */

  /*****************************************************************************
   * Expressions
   ****************************************************************************/

  QAST_BINEXPR = 1,     /* Binary expression */
  QAST_UNEXPR = 2,      /* Unary expression */
  QAST_POST_UNEXPR = 3, /* Post-unary expression */
  QAST_TEREXPR = 4,     /* Ternary expression */
  QAST_INT = 5,         /* Integer literal */
  QAST_FLOAT = 6,       /* Floating-point literal */
  QAST_STRING = 7,      /* String literal */
  QAST_CHAR = 8,        /* Character literal */
  QAST_BOOL = 9,        /* Boolean literal */
  QAST_NULL = 10,       /* Null literal */
  QAST_UNDEF = 11,      /* Undefined expression */
  QAST_CALL = 12,       /* Function call */
  QAST_LIST = 13,       /* List expression */
  QAST_ASSOC = 14,      /* Associative pair */
  QAST_FIELD = 15,      /* Field access */
  QAST_INDEX = 16,      /* Index access */
  QAST_SLICE = 17,      /* Slice access */
  QAST_FSTRING = 18,    /* Formatted string */
  QAST_IDENT = 19,      /* Identifier */
  QAST_SEQ = 20,        /* Sequence point */
  QAST_SEXPR = 21,      /* Statement expression */
  QAST_TEXPR = 22,      /* Type expression */
  QAST_TEMPL_CALL = 23, /* Template call */

  QAST__EXPR_FIRST = QAST_BINEXPR,
  QAST__EXPR_LAST = QAST_TEMPL_CALL,

  /*****************************************************************************
   * Types
   ****************************************************************************/

  QAST_U1 = 30,       /* 1-bit unsigned integer (boolean) */
  QAST_U8 = 31,       /* 8-bit unsigned integer */
  QAST_U16 = 32,      /* 16-bit unsigned integer */
  QAST_U32 = 33,      /* 32-bit unsigned integer */
  QAST_U64 = 34,      /* 64-bit unsigned integer */
  QAST_U128 = 35,     /* 128-bit unsigned integer */
  QAST_I8 = 36,       /* 8-bit signed integer */
  QAST_I16 = 37,      /* 16-bit signed integer */
  QAST_I32 = 38,      /* 32-bit signed integer */
  QAST_I64 = 39,      /* 64-bit signed integer */
  QAST_I128 = 40,     /* 128-bit signed integer */
  QAST_F16 = 41,      /* 16-bit floating-point number */
  QAST_F32 = 42,      /* 32-bit floating-point number */
  QAST_F64 = 43,      /* 64-bit floating-point number */
  QAST_F128 = 44,     /* 128-bit floating-point number */
  QAST_VOID = 45,     /* Void type */
  QAST_INFER = 46,    /* Inferred type */
  QAST_OPAQUE = 47,   /* Opaque named type */
  QAST_NAMED = 48,    /* Unresolved type name */
  QAST_REF = 49,      /* Reference type */
  QAST_PTR = 50,      /* Raw pointer type */
  QAST_ARRAY = 51,    /* Basic array type */
  QAST_TUPLE = 52,    /* Tuple type */
  QAST_TEMPLATE = 53, /* Template type */
  QAST_FUNCTOR = 54,  /* Function type */

  QAST__TYPE_FIRST = QAST_U1,
  QAST__TYPE_LAST = QAST_FUNCTOR,

  /*****************************************************************************
   * Statements
   ****************************************************************************/

  QAST_IF = 60,         /* If statement */
  QAST_RETIF = 61,      /* Return-if statement */
  QAST_SWITCH = 62,     /* Switch statement */
  QAST_CASE = 63,       /* Case statement */
  QAST_RETURN = 64,     /* Return statement */
  QAST_BREAK = 65,      /* Break statement */
  QAST_CONTINUE = 66,   /* Continue statement */
  QAST_WHILE = 67,      /* While statement */
  QAST_FOR = 68,        /* For statement */
  QAST_FOREACH = 69,    /* Foreach statement */
  QAST_INLINE_ASM = 70, /* Inline assembly statement */
  QAST_ESTMT = 71,      /* Expression-statement adapter */
  QAST_TYPEDEF = 80,    /* Type alias declaration */
  QAST_STRUCT = 81,     /* Struct definition */
  QAST_ENUM = 83,       /* Enum definition */
  QAST_SCOPE = 84,      /* Namespace scope */
  QAST_BLOCK = 85,      /* Block statement */
  QAST_EXPORT = 86,     /* Export statement */
  QAST_VAR = 87,        /* Variable declaration */
  QAST_FUNCTION = 88,   /* Function definition */

  QAST__STMT_FIRST = QAST_IF,
  QAST__STMT_LAST = QAST_FUNCTION,

  QAST__FIRST = QAST_BASE,
  QAST__LAST = QAST_FUNCTION,
} npar_ty_t;

#define QAST_COUNT (QAST__LAST - QAST__FIRST + 1)

struct npar_node_t;

namespace npar {
  class Stmt;
  class Type;
  class Expr;
  class ExprStmt;
  class StmtExpr;
  class TypeExpr;
  class NamedTy;
  class InferTy;
  class TemplType;
  class U1;
  class U8;
  class U16;
  class U32;
  class U64;
  class U128;
  class I8;
  class I16;
  class I32;
  class I64;
  class I128;
  class F16;
  class F32;
  class F64;
  class F128;
  class VoidTy;
  class PtrTy;
  class OpaqueTy;
  class TupleTy;
  class ArrayTy;
  class RefTy;
  class FuncTy;
  class UnaryExpr;
  class BinExpr;
  class PostUnaryExpr;
  class TernaryExpr;
  class ConstInt;
  class ConstFloat;
  class ConstBool;
  class ConstString;
  class ConstChar;
  class ConstNull;
  class ConstUndef;
  class Call;
  class TemplCall;
  class List;
  class Assoc;
  class Field;
  class Index;
  class Slice;
  class FString;
  class Ident;
  class SeqPoint;
  class Block;
  class VarDecl;
  class InlineAsm;
  class IfStmt;
  class WhileStmt;
  class ForStmt;
  class ForeachStmt;
  class BreakStmt;
  class ContinueStmt;
  class ReturnStmt;
  class ReturnIfStmt;
  class CaseStmt;
  class SwitchStmt;
  class TypedefStmt;
  class Function;
  class StructDef;
  class EnumDef;
  class ScopeStmt;
  class ExportStmt;
}  // namespace npar

namespace npar {
  enum class Vis {
    Pub = 0,
    Sec = 1,
    Pro = 2,
  };

  enum class VarDeclType { Const, Var, Let };

  enum class CompositeType { Region, Struct, Group, Class, Union };

  enum class FuncPurity {
    IMPURE_THREAD_UNSAFE,
    IMPURE_THREAD_SAFE,
    PURE,
    QUASI,
    RETRO,
  };

  enum class SafetyMode {
    Unknown = 0,
    Safe = 1,
    Unsafe = 2,
  };

  using ASTString =
      boost::flyweight<std::string, boost::flyweights::no_tracking>;

#define npar_pack __attribute__((packed))

  struct npar_pack str_ref {
    const char* data;
    uint32_t size;

    static str_ref get(std::string_view sv) {
      assert(sv.size() <= UINT32_MAX);

      return {sv.data(), static_cast<uint32_t>(sv.size())};
    }

    operator std::string_view() const { return {data, size}; }
  };

}  // namespace npar

#endif
