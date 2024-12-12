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

#ifndef __NITRATE_PARSER_NODE_H__
#define __NITRATE_PARSER_NODE_H__

#ifndef __cplusplus
#error "This code requires c++"
#endif
#include <nitrate-core/Error.h>
#include <nitrate-core/Macro.h>
#include <nitrate-core/Memory.h>
#include <nitrate-lexer/Token.h>

#include <boost/flyweight.hpp>
#include <cassert>
#include <iostream>
#include <map>
#include <nitrate-core/Classes.hh>
#include <nitrate-parser/Visitor.hh>
#include <optional>
#include <ostream>
#include <set>
#include <string>
#include <tuple>
#include <type_traits>
#include <variant>
#include <vector>

typedef enum npar_ty_t {
  QAST_NODE_NODE,

  QAST_NODE_BINEXPR,
  QAST_NODE_UNEXPR,
  QAST_NODE_TEREXPR,
  QAST_NODE_INT,
  QAST_NODE_FLOAT,
  QAST_NODE_STRING,
  QAST_NODE_CHAR,
  QAST_NODE_BOOL,
  QAST_NODE_NULL,
  QAST_NODE_UNDEF,
  QAST_NODE_CALL,
  QAST_NODE_LIST,
  QAST_NODE_ASSOC,
  QAST_NODE_FIELD,
  QAST_NODE_INDEX,
  QAST_NODE_SLICE,
  QAST_NODE_FSTRING,
  QAST_NODE_IDENT,
  QAST_NODE_SEQ,
  QAST_NODE_POST_UNEXPR,
  QAST_NODE_STMT_EXPR,
  QAST_NODE_TYPE_EXPR,
  QAST_NODE_TEMPL_CALL,

  QAST_NODE__EXPR_FIRST = QAST_NODE_BINEXPR,
  QAST_NODE__EXPR_LAST = QAST_NODE_TEMPL_CALL,

  QAST_NODE_REF_TY,
  QAST_NODE_U1_TY,
  QAST_NODE_U8_TY,
  QAST_NODE_U16_TY,
  QAST_NODE_U32_TY,
  QAST_NODE_U64_TY,
  QAST_NODE_U128_TY,
  QAST_NODE_I8_TY,
  QAST_NODE_I16_TY,
  QAST_NODE_I32_TY,
  QAST_NODE_I64_TY,
  QAST_NODE_I128_TY,
  QAST_NODE_F16_TY,
  QAST_NODE_F32_TY,
  QAST_NODE_F64_TY,
  QAST_NODE_F128_TY,
  QAST_NODE_VOID_TY,
  QAST_NODE_PTR_TY,
  QAST_NODE_OPAQUE_TY,
  QAST_NODE_ARRAY_TY,
  QAST_NODE_TUPLE_TY,
  QAST_NODE_FN_TY,
  QAST_NODE_UNRES_TY,
  QAST_NODE_INFER_TY,
  QAST_NODE_TEMPL_TY,

  QAST_NODE__TYPE_FIRST = QAST_NODE_REF_TY,
  QAST_NODE__TYPE_LAST = QAST_NODE_TEMPL_TY,

  QAST_NODE_TYPEDEF,
  QAST_NODE_FNDECL,
  QAST_NODE_STRUCT,
  QAST_NODE_ENUM,
  QAST_NODE_FN,
  QAST_NODE_SCOPE,
  QAST_NODE_EXPORT,
  QAST_NODE_STRUCT_FIELD,

  QAST_NODE_BLOCK,
  QAST_NODE_VAR,
  QAST_NODE_INLINE_ASM,
  QAST_NODE_RETURN,
  QAST_NODE_RETIF,
  QAST_NODE_BREAK,
  QAST_NODE_CONTINUE,
  QAST_NODE_IF,
  QAST_NODE_WHILE,
  QAST_NODE_FOR,
  QAST_NODE_FOREACH,
  QAST_NODE_CASE,
  QAST_NODE_SWITCH,
  QAST_NODE_EXPR_STMT,

  QAST_NODE__STMT_FIRST = QAST_NODE_TYPEDEF,
  QAST_NODE__STMT_LAST = QAST_NODE_EXPR_STMT,

  QAST_NODE__FIRST = QAST_NODE_NODE,
  QAST_NODE__LAST = QAST_NODE_EXPR_STMT,
} npar_ty_t;

#define QAST_NODE_COUNT (QAST_NODE__LAST - QAST_NODE__FIRST + 1)

namespace npar {
  class ArenaAllocatorImpl {
    qcore_arena m_arena;

  public:
    ArenaAllocatorImpl() = default;

    void *allocate(std::size_t bytes);
    void deallocate(void *ptr);

    void swap(qcore_arena_t &arena);

    qcore_arena_t &get() { return *m_arena.get(); }
  };

  extern thread_local ArenaAllocatorImpl npar_arena;

  template <class T>
  struct Arena {
    typedef T value_type;

    Arena() = default;

    template <class U>
    constexpr Arena(const Arena<U> &) {}

    [[nodiscard]] T *allocate(std::size_t n) {
      return static_cast<T *>(npar_arena.allocate(sizeof(T) * n));
    }

    void deallocate(T *p, std::size_t n) {
      (void)n;
      (void)p;
    }
  };

  template <class T, class U>
  bool operator==(const Arena<T> &, const Arena<U> &) {
    return true;
  }
  template <class T, class U>
  bool operator!=(const Arena<T> &, const Arena<U> &) {
    return false;
  }
};  // namespace npar

struct npar_node_t {
private:
  npar_ty_t m_node_type : 7;
  uint32_t m_fileid : 24;
  uint32_t m_offset : 32;
  bool m_mock : 1;

public:
  constexpr npar_node_t(npar_ty_t ty, bool mock = false,
                        uint32_t fileid = QLEX_NOFILE,
                        uint32_t offset = QLEX_EOFF)
      : m_node_type(ty), m_fileid(fileid), m_offset(offset), m_mock(mock){};

  constexpr void accept(npar::ASTVisitor &v) const {
    using namespace npar;

    switch (getKind()) {
      case QAST_NODE_NODE: {
        v.visit(*as<npar_node_t>());
        break;
      }
      case QAST_NODE_BINEXPR: {
        v.visit(*as<BinExpr>());
        break;
      }
      case QAST_NODE_UNEXPR: {
        v.visit(*as<UnaryExpr>());
        break;
      }
      case QAST_NODE_TEREXPR: {
        v.visit(*as<TernaryExpr>());
        break;
      }
      case QAST_NODE_INT: {
        v.visit(*as<ConstInt>());
        break;
      }
      case QAST_NODE_FLOAT: {
        v.visit(*as<ConstFloat>());
        break;
      }
      case QAST_NODE_STRING: {
        v.visit(*as<ConstString>());
        break;
      }
      case QAST_NODE_CHAR: {
        v.visit(*as<ConstChar>());
        break;
      }
      case QAST_NODE_BOOL: {
        v.visit(*as<ConstBool>());
        break;
      }
      case QAST_NODE_NULL: {
        v.visit(*as<ConstNull>());
        break;
      }
      case QAST_NODE_UNDEF: {
        v.visit(*as<ConstUndef>());
        break;
      }
      case QAST_NODE_CALL: {
        v.visit(*as<Call>());
        break;
      }
      case QAST_NODE_LIST: {
        v.visit(*as<List>());
        break;
      }
      case QAST_NODE_ASSOC: {
        v.visit(*as<Assoc>());
        break;
      }
      case QAST_NODE_FIELD: {
        v.visit(*as<Field>());
        break;
      }
      case QAST_NODE_INDEX: {
        v.visit(*as<Index>());
        break;
      }
      case QAST_NODE_SLICE: {
        v.visit(*as<Slice>());
        break;
      }
      case QAST_NODE_FSTRING: {
        v.visit(*as<FString>());
        break;
      }
      case QAST_NODE_IDENT: {
        v.visit(*as<Ident>());
        break;
      }
      case QAST_NODE_SEQ: {
        v.visit(*as<SeqPoint>());
        break;
      }
      case QAST_NODE_POST_UNEXPR: {
        v.visit(*as<PostUnaryExpr>());
        break;
      }
      case QAST_NODE_STMT_EXPR: {
        v.visit(*as<StmtExpr>());
        break;
      }
      case QAST_NODE_TYPE_EXPR: {
        v.visit(*as<TypeExpr>());
        break;
      }
      case QAST_NODE_TEMPL_CALL: {
        v.visit(*as<TemplCall>());
        break;
      }
      case QAST_NODE_REF_TY: {
        v.visit(*as<RefTy>());
        break;
      }
      case QAST_NODE_U1_TY: {
        v.visit(*as<U1>());
        break;
      }
      case QAST_NODE_U8_TY: {
        v.visit(*as<U8>());
        break;
      }
      case QAST_NODE_U16_TY: {
        v.visit(*as<U16>());
        break;
      }
      case QAST_NODE_U32_TY: {
        v.visit(*as<U32>());
        break;
      }
      case QAST_NODE_U64_TY: {
        v.visit(*as<U64>());
        break;
      }
      case QAST_NODE_U128_TY: {
        v.visit(*as<U128>());
        break;
      }
      case QAST_NODE_I8_TY: {
        v.visit(*as<I8>());
        break;
      }
      case QAST_NODE_I16_TY: {
        v.visit(*as<I16>());
        break;
      }
      case QAST_NODE_I32_TY: {
        v.visit(*as<I32>());
        break;
      }
      case QAST_NODE_I64_TY: {
        v.visit(*as<I64>());
        break;
      }
      case QAST_NODE_I128_TY: {
        v.visit(*as<I128>());
        break;
      }
      case QAST_NODE_F16_TY: {
        v.visit(*as<F16>());
        break;
      }
      case QAST_NODE_F32_TY: {
        v.visit(*as<F32>());
        break;
      }
      case QAST_NODE_F64_TY: {
        v.visit(*as<F64>());
        break;
      }
      case QAST_NODE_F128_TY: {
        v.visit(*as<F128>());
        break;
      }
      case QAST_NODE_VOID_TY: {
        v.visit(*as<VoidTy>());
        break;
      }
      case QAST_NODE_PTR_TY: {
        v.visit(*as<PtrTy>());
        break;
      }
      case QAST_NODE_OPAQUE_TY: {
        v.visit(*as<OpaqueTy>());
        break;
      }
      case QAST_NODE_ARRAY_TY: {
        v.visit(*as<ArrayTy>());
        break;
      }
      case QAST_NODE_TUPLE_TY: {
        v.visit(*as<TupleTy>());
        break;
      }
      case QAST_NODE_FN_TY: {
        v.visit(*as<FuncTy>());
        break;
      }
      case QAST_NODE_UNRES_TY: {
        v.visit(*as<NamedTy>());
        break;
      }
      case QAST_NODE_INFER_TY: {
        v.visit(*as<InferTy>());
        break;
      }
      case QAST_NODE_TEMPL_TY: {
        v.visit(*as<TemplType>());
        break;
      }
      case QAST_NODE_TYPEDEF: {
        v.visit(*as<TypedefStmt>());
        break;
      }
      case QAST_NODE_FNDECL: {
        v.visit(*as<FnDecl>());
        break;
      }
      case QAST_NODE_STRUCT: {
        v.visit(*as<StructDef>());
        break;
      }
      case QAST_NODE_ENUM: {
        v.visit(*as<EnumDef>());
        break;
      }
      case QAST_NODE_FN: {
        v.visit(*as<FnDef>());
        break;
      }
      case QAST_NODE_SCOPE: {
        v.visit(*as<ScopeStmt>());
        break;
      }
      case QAST_NODE_EXPORT: {
        v.visit(*as<ExportStmt>());
        break;
      }
      case QAST_NODE_STRUCT_FIELD: {
        v.visit(*as<StructField>());
        break;
      }
      case QAST_NODE_BLOCK: {
        v.visit(*as<Block>());
        break;
      }
      case QAST_NODE_VAR: {
        v.visit(*as<VarDecl>());
        break;
      }
      case QAST_NODE_INLINE_ASM: {
        v.visit(*as<InlineAsm>());
        break;
      }
      case QAST_NODE_RETURN: {
        v.visit(*as<ReturnStmt>());
        break;
      }
      case QAST_NODE_RETIF: {
        v.visit(*as<ReturnIfStmt>());
        break;
      }
      case QAST_NODE_BREAK: {
        v.visit(*as<BreakStmt>());
        break;
      }
      case QAST_NODE_CONTINUE: {
        v.visit(*as<ContinueStmt>());
        break;
      }
      case QAST_NODE_IF: {
        v.visit(*as<IfStmt>());
        break;
      }
      case QAST_NODE_WHILE: {
        v.visit(*as<WhileStmt>());
        break;
      }
      case QAST_NODE_FOR: {
        v.visit(*as<ForStmt>());
        break;
      }
      case QAST_NODE_FOREACH: {
        v.visit(*as<ForeachStmt>());
        break;
      }
      case QAST_NODE_CASE: {
        v.visit(*as<CaseStmt>());
        break;
      }
      case QAST_NODE_SWITCH: {
        v.visit(*as<SwitchStmt>());
        break;
      }
      case QAST_NODE_EXPR_STMT: {
        v.visit(*as<ExprStmt>());
        break;
      }
    }
  };

  ///======================================================================
  /* Efficient LLVM reflection */

  static constexpr uint32_t getKindSize(npar_ty_t kind);
  static constexpr std::string_view getKindName(npar_ty_t kind);

  template <typename T>
  static constexpr npar_ty_t getTypeCode() {
    using namespace npar;

    if constexpr (std::is_same_v<T, npar_node_t>) {
      return QAST_NODE_NODE;
    } else if constexpr (std::is_same_v<T, Stmt>) {
      return QAST_NODE_NODE;
    } else if constexpr (std::is_same_v<T, Type>) {
      return QAST_NODE_NODE;
    } else if constexpr (std::is_same_v<T, BinExpr>) {
      return QAST_NODE_BINEXPR;
    } else if constexpr (std::is_same_v<T, UnaryExpr>) {
      return QAST_NODE_UNEXPR;
    } else if constexpr (std::is_same_v<T, TernaryExpr>) {
      return QAST_NODE_TEREXPR;
    } else if constexpr (std::is_same_v<T, ConstInt>) {
      return QAST_NODE_INT;
    } else if constexpr (std::is_same_v<T, ConstFloat>) {
      return QAST_NODE_FLOAT;
    } else if constexpr (std::is_same_v<T, ConstString>) {
      return QAST_NODE_STRING;
    } else if constexpr (std::is_same_v<T, ConstChar>) {
      return QAST_NODE_CHAR;
    } else if constexpr (std::is_same_v<T, ConstBool>) {
      return QAST_NODE_BOOL;
    } else if constexpr (std::is_same_v<T, ConstNull>) {
      return QAST_NODE_NULL;
    } else if constexpr (std::is_same_v<T, ConstUndef>) {
      return QAST_NODE_UNDEF;
    } else if constexpr (std::is_same_v<T, Call>) {
      return QAST_NODE_CALL;
    } else if constexpr (std::is_same_v<T, List>) {
      return QAST_NODE_LIST;
    } else if constexpr (std::is_same_v<T, Assoc>) {
      return QAST_NODE_ASSOC;
    } else if constexpr (std::is_same_v<T, Field>) {
      return QAST_NODE_FIELD;
    } else if constexpr (std::is_same_v<T, Index>) {
      return QAST_NODE_INDEX;
    } else if constexpr (std::is_same_v<T, Slice>) {
      return QAST_NODE_SLICE;
    } else if constexpr (std::is_same_v<T, FString>) {
      return QAST_NODE_FSTRING;
    } else if constexpr (std::is_same_v<T, Ident>) {
      return QAST_NODE_IDENT;
    } else if constexpr (std::is_same_v<T, SeqPoint>) {
      return QAST_NODE_SEQ;
    } else if constexpr (std::is_same_v<T, PostUnaryExpr>) {
      return QAST_NODE_POST_UNEXPR;
    } else if constexpr (std::is_same_v<T, StmtExpr>) {
      return QAST_NODE_STMT_EXPR;
    } else if constexpr (std::is_same_v<T, TypeExpr>) {
      return QAST_NODE_TYPE_EXPR;
    } else if constexpr (std::is_same_v<T, TemplCall>) {
      return QAST_NODE_TEMPL_CALL;
    } else if constexpr (std::is_same_v<T, RefTy>) {
      return QAST_NODE_REF_TY;
    } else if constexpr (std::is_same_v<T, U1>) {
      return QAST_NODE_U1_TY;
    } else if constexpr (std::is_same_v<T, U8>) {
      return QAST_NODE_U8_TY;
    } else if constexpr (std::is_same_v<T, U16>) {
      return QAST_NODE_U16_TY;
    } else if constexpr (std::is_same_v<T, U32>) {
      return QAST_NODE_U32_TY;
    } else if constexpr (std::is_same_v<T, U64>) {
      return QAST_NODE_U64_TY;
    } else if constexpr (std::is_same_v<T, U128>) {
      return QAST_NODE_U128_TY;
    } else if constexpr (std::is_same_v<T, I8>) {
      return QAST_NODE_I8_TY;
    } else if constexpr (std::is_same_v<T, I16>) {
      return QAST_NODE_I16_TY;
    } else if constexpr (std::is_same_v<T, I32>) {
      return QAST_NODE_I32_TY;
    } else if constexpr (std::is_same_v<T, I64>) {
      return QAST_NODE_I64_TY;
    } else if constexpr (std::is_same_v<T, I128>) {
      return QAST_NODE_I128_TY;
    } else if constexpr (std::is_same_v<T, F16>) {
      return QAST_NODE_F16_TY;
    } else if constexpr (std::is_same_v<T, F32>) {
      return QAST_NODE_F32_TY;
    } else if constexpr (std::is_same_v<T, F64>) {
      return QAST_NODE_F64_TY;
    } else if constexpr (std::is_same_v<T, F128>) {
      return QAST_NODE_F128_TY;
    } else if constexpr (std::is_same_v<T, VoidTy>) {
      return QAST_NODE_VOID_TY;
    } else if constexpr (std::is_same_v<T, PtrTy>) {
      return QAST_NODE_PTR_TY;
    } else if constexpr (std::is_same_v<T, OpaqueTy>) {
      return QAST_NODE_OPAQUE_TY;
    } else if constexpr (std::is_same_v<T, ArrayTy>) {
      return QAST_NODE_ARRAY_TY;
    } else if constexpr (std::is_same_v<T, TupleTy>) {
      return QAST_NODE_TUPLE_TY;
    } else if constexpr (std::is_same_v<T, FuncTy>) {
      return QAST_NODE_FN_TY;
    } else if constexpr (std::is_same_v<T, NamedTy>) {
      return QAST_NODE_UNRES_TY;
    } else if constexpr (std::is_same_v<T, InferTy>) {
      return QAST_NODE_INFER_TY;
    } else if constexpr (std::is_same_v<T, TemplType>) {
      return QAST_NODE_TEMPL_TY;
    } else if constexpr (std::is_same_v<T, TypedefStmt>) {
      return QAST_NODE_TYPEDEF;
    } else if constexpr (std::is_same_v<T, FnDecl>) {
      return QAST_NODE_FNDECL;
    } else if constexpr (std::is_same_v<T, StructDef>) {
      return QAST_NODE_STRUCT;
    } else if constexpr (std::is_same_v<T, EnumDef>) {
      return QAST_NODE_ENUM;
    } else if constexpr (std::is_same_v<T, FnDef>) {
      return QAST_NODE_FN;
    } else if constexpr (std::is_same_v<T, ScopeStmt>) {
      return QAST_NODE_SCOPE;
    } else if constexpr (std::is_same_v<T, ExportStmt>) {
      return QAST_NODE_EXPORT;
    } else if constexpr (std::is_same_v<T, StructField>) {
      return QAST_NODE_STRUCT_FIELD;
    } else if constexpr (std::is_same_v<T, Block>) {
      return QAST_NODE_BLOCK;
    } else if constexpr (std::is_same_v<T, VarDecl>) {
      return QAST_NODE_VAR;
    } else if constexpr (std::is_same_v<T, InlineAsm>) {
      return QAST_NODE_INLINE_ASM;
    } else if constexpr (std::is_same_v<T, ReturnStmt>) {
      return QAST_NODE_RETURN;
    } else if constexpr (std::is_same_v<T, ReturnIfStmt>) {
      return QAST_NODE_RETIF;
    } else if constexpr (std::is_same_v<T, BreakStmt>) {
      return QAST_NODE_BREAK;
    } else if constexpr (std::is_same_v<T, ContinueStmt>) {
      return QAST_NODE_CONTINUE;
    } else if constexpr (std::is_same_v<T, IfStmt>) {
      return QAST_NODE_IF;
    } else if constexpr (std::is_same_v<T, WhileStmt>) {
      return QAST_NODE_WHILE;
    } else if constexpr (std::is_same_v<T, ForStmt>) {
      return QAST_NODE_FOR;
    } else if constexpr (std::is_same_v<T, ForeachStmt>) {
      return QAST_NODE_FOREACH;
    } else if constexpr (std::is_same_v<T, CaseStmt>) {
      return QAST_NODE_CASE;
    } else if constexpr (std::is_same_v<T, SwitchStmt>) {
      return QAST_NODE_SWITCH;
    } else if constexpr (std::is_same_v<T, ExprStmt>) {
      return QAST_NODE_EXPR_STMT;
    }
  }

  constexpr npar_ty_t getKind() const { return m_node_type; }
  constexpr auto getKindName() const { return getKindName(m_node_type); }

  ///======================================================================

  constexpr bool is_type() const {
    auto kind = getKind();
    return kind >= QAST_NODE__TYPE_FIRST && kind <= QAST_NODE__TYPE_LAST;
  }

  constexpr bool is_stmt() const {
    auto kind = getKind();
    return kind >= QAST_NODE__STMT_FIRST && kind <= QAST_NODE__STMT_LAST;
  }

  constexpr bool is_expr() const {
    auto kind = getKind();
    return kind >= QAST_NODE__EXPR_FIRST && kind <= QAST_NODE__EXPR_LAST;
  }

  template <typename T>
  static constexpr T *safeCastAs(npar_node_t *ptr) {
    if (!ptr) {
      return nullptr;
    }

#ifndef NDEBUG
    if (getTypeCode<T>() != ptr->getKind()) [[unlikely]] {
      qcore_panicf("Invalid cast from %s to %s", ptr->getKindName(),
                   getKindName(getTypeCode<T>()));
    }
#endif

    return static_cast<T *>(ptr);
  }

  /**
   * @brief Type-safe cast (type check only in debug mode).
   *
   * @tparam T The type to cast to.
   * @return T* The casted pointer. It may be nullptr if the source pointer is
   * nullptr.
   * @warning This function will panic if the cast is invalid.
   */
  template <typename T>
  constexpr T *as() {
    return safeCastAs<T>(this);
  }

  /**
   * @brief Type-safe cast (type check only in debug mode).
   *
   * @tparam T The type to cast to.
   * @return const T* The casted pointer. It may be nullptr if the source
   * pointer is nullptr.
   * @warning This function will panic if the cast is invalid.
   */
  template <typename T>
  constexpr const T *as() const {
    return safeCastAs<T>(const_cast<npar_node_t *>(this));
  }

  template <typename T>
  constexpr bool is() const {
    return npar_node_t::getTypeCode<T>() == getKind();
  }

  constexpr bool is(npar_ty_t type) const { return type == getKind(); }

  constexpr bool is_expr_stmt(npar_ty_t type) const;
  constexpr bool is_stmt_expr(npar_ty_t type) const;

  constexpr bool isSame(const npar_node_t *other) const;

  uint64_t hash64() const;

  std::ostream &dump(std::ostream &os = std::cerr,
                     bool isForDebug = false) const;

  constexpr void set_offset(uint32_t pos) { m_offset = pos; }

  constexpr uint32_t get_offset() const { return m_offset; }
  constexpr uint32_t get_fileid() const { return m_fileid; }

  constexpr std::tuple<uint32_t, uint32_t> get_pos() const {
    return {m_offset, m_fileid};
  }
} __attribute__((packed));

static_assert(sizeof(npar_node_t) == 8);

namespace npar {
  enum class Vis {
    Pub = 0,
    Sec = 1,
    Pro = 2,
  };

  static_assert((int)Vis::Pub == 0);
  static_assert((int)Vis::Sec == 1);
  static_assert((int)Vis::Pro == 2);

  using SmallString = boost::flyweight<std::string>;

  SmallString SaveString(std::string_view str);

  class Stmt : public npar_node_t {
  public:
    constexpr Stmt(npar_ty_t ty) : npar_node_t(ty){};
  };

  class Expr;

  class Type : public npar_node_t {
    std::pair<Expr *, Expr *> m_range;
    Expr *m_width;

  public:
    constexpr Type(npar_ty_t ty)
        : npar_node_t(ty), m_range({nullptr, nullptr}), m_width(nullptr) {}

    constexpr bool is_primitive() const {
      switch (getKind()) {
        case QAST_NODE_U1_TY:
        case QAST_NODE_U8_TY:
        case QAST_NODE_U16_TY:
        case QAST_NODE_U32_TY:
        case QAST_NODE_U64_TY:
        case QAST_NODE_U128_TY:
        case QAST_NODE_I8_TY:
        case QAST_NODE_I16_TY:
        case QAST_NODE_I32_TY:
        case QAST_NODE_I64_TY:
        case QAST_NODE_I128_TY:
        case QAST_NODE_F16_TY:
        case QAST_NODE_F32_TY:
        case QAST_NODE_F64_TY:
        case QAST_NODE_F128_TY:
        case QAST_NODE_VOID_TY:
          return true;
        default:
          return false;
      }
    }
    constexpr bool is_array() const { return getKind() == QAST_NODE_ARRAY_TY; };
    constexpr bool is_tuple() const { return getKind() == QAST_NODE_TUPLE_TY; }
    constexpr bool is_pointer() const { return getKind() == QAST_NODE_PTR_TY; }
    constexpr bool is_function() const { return getKind() == QAST_NODE_FN_TY; }
    constexpr bool is_composite() const { return is_array() || is_tuple(); }
    constexpr bool is_numeric() const {
      return getKind() >= QAST_NODE_U1_TY && getKind() <= QAST_NODE_F128_TY;
    }
    constexpr bool is_integral() const {
      return getKind() >= QAST_NODE_U1_TY && getKind() <= QAST_NODE_I128_TY;
    }
    constexpr bool is_floating_point() const {
      return getKind() >= QAST_NODE_F16_TY && getKind() <= QAST_NODE_F128_TY;
    }
    constexpr bool is_signed() const {
      return getKind() >= QAST_NODE_I8_TY && getKind() <= QAST_NODE_I128_TY;
    }
    constexpr bool is_unsigned() const {
      return getKind() >= QAST_NODE_U1_TY && getKind() <= QAST_NODE_U128_TY;
    }
    constexpr bool is_void() const { return getKind() == QAST_NODE_VOID_TY; }
    constexpr bool is_bool() const { return getKind() == QAST_NODE_U1_TY; }
    constexpr bool is_ref() const { return getKind() == QAST_NODE_REF_TY; }
    bool is_ptr_to(Type *type) const;

    constexpr let get_width() const { return m_width; }
    constexpr let get_range() const { return m_range; }

    constexpr void set_range(Expr *start, Expr *end) { m_range = {start, end}; }
    constexpr void set_width(Expr *width) { m_width = width; }
  };

  typedef std::tuple<SmallString, Type *, Expr *> TemplateParameter;
  typedef std::vector<TemplateParameter, Arena<TemplateParameter>>
      TemplateParameters;

  class Expr : public npar_node_t {
  public:
    constexpr Expr(npar_ty_t ty) : npar_node_t(ty) {}
  };

  class ExprStmt : public Stmt {
    Expr *m_expr;

  public:
    constexpr ExprStmt(Expr *expr) : Stmt(QAST_NODE_EXPR_STMT), m_expr(expr) {}

    let get_expr() const { return m_expr; }
  };

  class StmtExpr : public Expr {
    Stmt *m_stmt;

  public:
    constexpr StmtExpr(Stmt *stmt) : Expr(QAST_NODE_STMT_EXPR), m_stmt(stmt) {}

    let get_stmt() const { return m_stmt; }
  };

  class TypeExpr : public Expr {
    Type *m_type;

  public:
    constexpr TypeExpr(Type *type) : Expr(QAST_NODE_TYPE_EXPR), m_type(type) {}

    let get_type() const { return m_type; }
  };

  class NamedTy : public Type {
    SmallString m_name;

  public:
    NamedTy(SmallString name) : Type(QAST_NODE_UNRES_TY), m_name(name) {}

    let get_name() const { return m_name; }
  };

  class InferTy : public Type {
  public:
    constexpr InferTy() : Type(QAST_NODE_INFER_TY) {}
  };

  typedef std::vector<Expr *, Arena<Expr *>> TemplTypeArgs;

  class TemplType : public Type {
    Type *m_template;
    TemplTypeArgs m_args;

  public:
    TemplType(Type *templ, const TemplTypeArgs &args)
        : Type(QAST_NODE_TEMPL_TY), m_template(templ), m_args(args) {}

    let get_template() const { return m_template; }
    let get_args() const { return m_args; }
  };

  class U1 : public Type {
  public:
    constexpr U1() : Type(QAST_NODE_U1_TY){};
  };

  class U8 : public Type {
  public:
    constexpr U8() : Type(QAST_NODE_U8_TY){};
  };

  class U16 : public Type {
  public:
    constexpr U16() : Type(QAST_NODE_U16_TY){};
  };

  class U32 : public Type {
  public:
    constexpr U32() : Type(QAST_NODE_U32_TY){};
  };

  class U64 : public Type {
  public:
    constexpr U64() : Type(QAST_NODE_U64_TY){};
  };

  class U128 : public Type {
  public:
    constexpr U128() : Type(QAST_NODE_U128_TY){};
  };

  class I8 : public Type {
  public:
    constexpr I8() : Type(QAST_NODE_I8_TY){};
  };

  class I16 : public Type {
  public:
    constexpr I16() : Type(QAST_NODE_I16_TY){};
  };

  class I32 : public Type {
  public:
    constexpr I32() : Type(QAST_NODE_I32_TY){};
  };

  class I64 : public Type {
  public:
    constexpr I64() : Type(QAST_NODE_I64_TY){};
  };

  class I128 : public Type {
  public:
    constexpr I128() : Type(QAST_NODE_I128_TY){};
  };

  class F16 : public Type {
  public:
    constexpr F16() : Type(QAST_NODE_F16_TY){};
  };

  class F32 : public Type {
  public:
    constexpr F32() : Type(QAST_NODE_F32_TY){};
  };

  class F64 : public Type {
  public:
    constexpr F64() : Type(QAST_NODE_F64_TY){};
  };

  class F128 : public Type {
  public:
    constexpr F128() : Type(QAST_NODE_F128_TY){};
  };

  class VoidTy : public Type {
  public:
    constexpr VoidTy() : Type(QAST_NODE_VOID_TY) {}
  };

  class PtrTy : public Type {
    Type *m_item;
    bool m_is_volatile;

  public:
    constexpr PtrTy(Type *item, bool is_volatile = false)
        : Type(QAST_NODE_PTR_TY), m_item(item), m_is_volatile(is_volatile) {}

    let get_item() const { return m_item; }
    bool is_volatile() const { return m_is_volatile; }
  };

  class OpaqueTy : public Type {
    SmallString m_name;

  public:
    OpaqueTy(SmallString name) : Type(QAST_NODE_OPAQUE_TY), m_name(name) {}

    let get_name() const { return m_name; }
  };

  typedef std::vector<Type *, Arena<Type *>> TupleTyItems;
  class TupleTy : public Type {
    TupleTyItems m_items;

  public:
    TupleTy(const TupleTyItems &items)
        : Type(QAST_NODE_TUPLE_TY), m_items(items) {}

    let get_items() const { return m_items; }
  };

  class ArrayTy : public Type {
    Type *m_item;
    Expr *m_size;

  public:
    constexpr ArrayTy(Type *item, Expr *size)
        : Type(QAST_NODE_ARRAY_TY), m_item(item), m_size(size) {}

    let get_item() const { return m_item; }
    let get_size() const { return m_size; }
  };

  class RefTy : public Type {
    Type *m_item;

  public:
    constexpr RefTy(Type *item) : Type(QAST_NODE_REF_TY), m_item(item) {}

    let get_item() const { return m_item; }
  };

  typedef std::pair<SmallString, Type *> StructItem;
  typedef std::vector<StructItem, Arena<StructItem>> StructItems;

  enum class FuncPurity {
    IMPURE_THREAD_UNSAFE,
    IMPURE_THREAD_SAFE,
    PURE,
    QUASIPURE,
    RETROPURE,
  };

  typedef std::tuple<SmallString, Type *, Expr *> FuncParam;
  typedef std::vector<FuncParam, Arena<FuncParam>> FuncParams;

  class FuncTy : public Type {
    FuncParams m_params;
    Type *m_return;
    FuncPurity m_purity;
    bool m_variadic;
    bool m_is_foreign;
    bool m_noreturn;

  public:
    FuncTy()
        : Type(QAST_NODE_FN_TY),
          m_return(nullptr),
          m_purity(FuncPurity::IMPURE_THREAD_UNSAFE),
          m_variadic(false),
          m_is_foreign(false),
          m_noreturn(false) {}

    FuncTy(Type *return_type, FuncParams parameters, bool variadic = false,
           FuncPurity purity = FuncPurity::IMPURE_THREAD_UNSAFE,
           bool is_foreign = false, bool noreturn = false)
        : Type(QAST_NODE_FN_TY),
          m_params(parameters),
          m_return(return_type),
          m_purity(purity),
          m_variadic(variadic),
          m_is_foreign(is_foreign),
          m_noreturn(noreturn) {
      assert(!noreturn || (purity == FuncPurity::IMPURE_THREAD_UNSAFE ||
                           purity == FuncPurity::IMPURE_THREAD_SAFE));
    }
    FuncTy(Type *return_type, std::vector<Type *, Arena<Type *>> parameters,
           bool variadic = false,
           FuncPurity purity = FuncPurity::IMPURE_THREAD_UNSAFE,
           bool is_foreign = false, bool noreturn = false)
        : Type(QAST_NODE_FN_TY),
          m_return(return_type),
          m_purity(purity),
          m_variadic(variadic),
          m_is_foreign(is_foreign),
          m_noreturn(noreturn) {
      assert(!noreturn || (purity == FuncPurity::IMPURE_THREAD_UNSAFE ||
                           purity == FuncPurity::IMPURE_THREAD_SAFE));

      for (size_t i = 0; i < parameters.size(); i++) {
        m_params.push_back(
            FuncParam("_" + std::to_string(i), parameters[i], nullptr));
      }
    }

    let is_noreturn() const { return m_noreturn; }
    let get_return_ty() const { return m_return; }
    let get_params() const { return m_params; }
    let get_purity() const { return m_purity; }
    let is_variadic() const { return m_variadic; }
    let is_foreign() const { return m_is_foreign; }

    void set_return_ty(Type *return_ty) { m_return = return_ty; }
    void set_purity(FuncPurity purity) { m_purity = purity; }
    void set_variadic(bool variadic) { m_variadic = variadic; }
    void set_foreign(bool is_foreign) { m_is_foreign = is_foreign; }
    auto &get_params() { return m_params; }
  };

  ///=============================================================================

  class UnaryExpr : public Expr {
    Expr *m_rhs;
    qlex_op_t m_op;

  public:
    constexpr UnaryExpr(qlex_op_t op, Expr *rhs)
        : Expr(QAST_NODE_UNEXPR), m_rhs(rhs), m_op(op) {}

    let get_rhs() const { return m_rhs; }
    qlex_op_t get_op() const { return m_op; }
  };

  class BinExpr : public Expr {
    Expr *m_lhs;
    Expr *m_rhs;
    qlex_op_t m_op;

  public:
    constexpr BinExpr(Expr *lhs, qlex_op_t op, Expr *rhs)
        : Expr(QAST_NODE_BINEXPR), m_lhs(lhs), m_rhs(rhs), m_op(op) {}

    let get_lhs() const { return m_lhs; }
    let get_rhs() const { return m_rhs; }
    qlex_op_t get_op() const { return m_op; }
  };

  class PostUnaryExpr : public Expr {
    Expr *m_lhs;
    qlex_op_t m_op;

  public:
    constexpr PostUnaryExpr(Expr *lhs, qlex_op_t op = qOpTernary)
        : Expr(QAST_NODE_POST_UNEXPR), m_lhs(lhs), m_op(op) {}

    let get_lhs() const { return m_lhs; }
    let get_op() const { return m_op; }
  };

  class TernaryExpr : public Expr {
    Expr *m_cond;
    Expr *m_lhs;
    Expr *m_rhs;

  public:
    constexpr TernaryExpr(Expr *cond, Expr *lhs, Expr *rhs)
        : Expr(QAST_NODE_TEREXPR), m_cond(cond), m_lhs(lhs), m_rhs(rhs) {}

    let get_cond() const { return m_cond; }
    let get_lhs() const { return m_lhs; }
    let get_rhs() const { return m_rhs; }
  };

  ///=============================================================================

  class ConstInt : public Expr {
    SmallString m_value;

  public:
    ConstInt(SmallString value) : Expr(QAST_NODE_INT), m_value(value) {}

    let get_value() const { return m_value; }
  };

  class ConstFloat : public Expr {
    SmallString m_value;

  public:
    ConstFloat(SmallString value) : Expr(QAST_NODE_FLOAT), m_value(value) {}

    let get_value() const { return m_value; }
  };

  class ConstBool : public Expr {
    bool m_value;

  public:
    constexpr ConstBool(bool value = false)
        : Expr(QAST_NODE_BOOL), m_value(value) {}

    bool get_value() const { return m_value; }
  };

  class ConstString : public Expr {
    SmallString m_value;

  public:
    ConstString(SmallString value) : Expr(QAST_NODE_STRING), m_value(value) {}

    let get_value() const { return m_value; }
  };

  class ConstChar : public Expr {
    uint8_t m_value;

  public:
    constexpr ConstChar(uint8_t value = 0)
        : Expr(QAST_NODE_CHAR), m_value(value) {}

    uint8_t get_value() const { return m_value; }
  };

  class ConstNull : public Expr {
  public:
    constexpr ConstNull() : Expr(QAST_NODE_NULL) {}
  };

  class ConstUndef : public Expr {
  public:
    constexpr ConstUndef() : Expr(QAST_NODE_UNDEF) {}
  };

  ///=============================================================================

  typedef std::pair<SmallString, Expr *> CallArg;
  typedef std::vector<CallArg, Arena<CallArg>> CallArgs;

  class Call : public Expr {
    Expr *m_func;
    CallArgs m_args;

  public:
    Call(Expr *func, CallArgs args = {})
        : Expr(QAST_NODE_CALL), m_func(func), m_args(args) {}

    let get_func() const { return m_func; }
    let get_args() const { return m_args; }
  };

  typedef std::map<SmallString, Expr *, std::less<SmallString>,
                   Arena<std::pair<const SmallString, Expr *>>>
      TemplateArgs;

  class TemplCall : public Expr {
    TemplateArgs m_template_args;
    Expr *m_func;
    CallArgs m_args;

  public:
    TemplCall(Expr *func, CallArgs args = {}, TemplateArgs template_args = {})
        : Expr(QAST_NODE_TEMPL_CALL),
          m_template_args(template_args),
          m_func(func),
          m_args(args) {}

    let get_func() const { return m_func; }
    let get_template_args() const { return m_template_args; }
    let get_args() const { return m_args; }
  };

  typedef std::vector<Expr *, Arena<Expr *>> ListData;

  class List : public Expr {
    ListData m_items;

  public:
    List(const ListData &items) : Expr(QAST_NODE_LIST), m_items(items) {}

    let get_items() const { return m_items; }
  };

  class Assoc : public Expr {
    Expr *m_key;
    Expr *m_value;

  public:
    constexpr Assoc(Expr *key, Expr *value)
        : Expr(QAST_NODE_ASSOC), m_key(key), m_value(value) {}

    let get_key() const { return m_key; }
    let get_value() const { return m_value; }
  };

  class Field : public Expr {
    Expr *m_base;
    SmallString m_field;

  public:
    Field(Expr *base, SmallString field)
        : Expr(QAST_NODE_FIELD), m_base(base), m_field(field) {}

    let get_base() const { return m_base; }
    let get_field() const { return m_field; }
  };

  class Index : public Expr {
    Expr *m_base;
    Expr *m_index;

  public:
    constexpr Index(Expr *base, Expr *index)
        : Expr(QAST_NODE_INDEX), m_base(base), m_index(index) {}

    let get_base() const { return m_base; }
    let get_index() const { return m_index; }
  };

  class Slice : public Expr {
    Expr *m_base;
    Expr *m_start;
    Expr *m_end;

  public:
    constexpr Slice(Expr *base, Expr *start, Expr *end)
        : Expr(QAST_NODE_SLICE), m_base(base), m_start(start), m_end(end) {}

    let get_base() const { return m_base; }
    let get_start() const { return m_start; }
    let get_end() const { return m_end; }
  };

  typedef std::vector<std::variant<SmallString, Expr *>,
                      Arena<std::variant<SmallString, Expr *>>>
      FStringItems;

  class FString : public Expr {
    FStringItems m_items;

  public:
    FString(FStringItems items = {})
        : Expr(QAST_NODE_FSTRING), m_items(items) {}

    let get_items() const { return m_items; }
  };

  class Ident : public Expr {
    SmallString m_name;

  public:
    Ident(SmallString name) : Expr(QAST_NODE_IDENT), m_name(name) {}

    let get_name() const { return m_name; }
  };

  typedef std::vector<Expr *, Arena<Expr *>> SeqPointItems;
  class SeqPoint : public Expr {
    SeqPointItems m_items;

  public:
    SeqPoint(const SeqPointItems &items)
        : Expr(QAST_NODE_SEQ), m_items(items) {}

    let get_items() const { return m_items; }
  };

  ///=============================================================================

  typedef std::vector<Stmt *, Arena<Stmt *>> BlockItems;

  enum class SafetyMode {
    Unknown = 0,
    Safe = 1,
    Unsafe = 2,
  };

  class Block : public Stmt {
    BlockItems m_items;
    SafetyMode m_safety;

  public:
    Block(const BlockItems &items = {}, SafetyMode safety = SafetyMode::Unknown)
        : Stmt(QAST_NODE_BLOCK), m_items(items), m_safety(safety) {}

    let get_items() const { return m_items; }
    let get_safety() const { return m_safety; }

    void set_safety(SafetyMode safety) { m_safety = safety; }
    auto &get_items() { return m_items; }
  };

  enum class VarDeclType { Const, Var, Let };

  using VarDeclAttributes = std::set<Expr *, std::less<Expr *>, Arena<Expr *>>;

  class VarDecl : public Stmt {
    VarDeclAttributes m_attributes;
    SmallString m_name;
    Type *m_type;
    Expr *m_value;
    VarDeclType m_decl_type;

  public:
    VarDecl(SmallString name, Type *type, Expr *value, VarDeclType decl_type,
            VarDeclAttributes attributes)
        : Stmt(QAST_NODE_VAR),
          m_attributes(attributes),
          m_name(name),
          m_type(type),
          m_value(value),
          m_decl_type(decl_type) {}

    let get_name() const { return m_name; }
    let get_type() const { return m_type; }
    let get_value() const { return m_value; }
    let get_decl_type() const { return m_decl_type; }
    let get_attributes() const { return m_attributes; }
  };

  typedef std::vector<Expr *, Arena<Expr *>> InlineAsmArgs;

  class InlineAsm : public Stmt {
    SmallString m_code;
    InlineAsmArgs m_args;

  public:
    InlineAsm(SmallString code, const InlineAsmArgs &args)
        : Stmt(QAST_NODE_INLINE_ASM), m_code(code), m_args(args) {}

    let get_code() const { return m_code; }
    let get_args() const { return m_args; }
  };

  class IfStmt : public Stmt {
    Expr *m_cond;
    Stmt *m_then;
    Stmt *m_else;

  public:
    constexpr IfStmt(Expr *cond, Stmt *then, Stmt *else_)
        : Stmt(QAST_NODE_IF), m_cond(cond), m_then(then), m_else(else_) {}

    let get_cond() const { return m_cond; }
    let get_then() const { return m_then; }
    let get_else() const { return m_else; }
  };

  class WhileStmt : public Stmt {
    Expr *m_cond;
    Stmt *m_body;

  public:
    constexpr WhileStmt(Expr *cond, Stmt *body)
        : Stmt(QAST_NODE_WHILE), m_cond(cond), m_body(body) {}

    let get_cond() const { return m_cond; }
    let get_body() const { return m_body; }
  };

  class ForStmt : public Stmt {
    std::optional<Stmt *> m_init;
    std::optional<Expr *> m_cond, m_step;
    Stmt *m_body;

  public:
    constexpr ForStmt(std::optional<Stmt *> init, std::optional<Expr *> cond,
                      std::optional<Expr *> step, Stmt *body)
        : Stmt(QAST_NODE_FOR),
          m_init(init),
          m_cond(cond),
          m_step(step),
          m_body(body) {}

    let get_init() const { return m_init; }
    let get_cond() const { return m_cond; }
    let get_step() const { return m_step; }
    let get_body() const { return m_body; }
  };

  class ForeachStmt : public Stmt {
    SmallString m_idx_ident;
    SmallString m_val_ident;
    Expr *m_expr;
    Stmt *m_body;

  public:
    ForeachStmt(SmallString idx_ident, SmallString val_ident, Expr *expr,
                Stmt *body)
        : Stmt(QAST_NODE_FOREACH),
          m_idx_ident(idx_ident),
          m_val_ident(val_ident),
          m_expr(expr),
          m_body(body) {}

    let get_idx_ident() const { return m_idx_ident; }
    let get_val_ident() const { return m_val_ident; }
    let get_expr() const { return m_expr; }
    let get_body() const { return m_body; }
  };

  class BreakStmt : public Stmt {
  public:
    constexpr BreakStmt() : Stmt(QAST_NODE_BREAK){};
  };

  class ContinueStmt : public Stmt {
  public:
    constexpr ContinueStmt() : Stmt(QAST_NODE_CONTINUE){};
  };

  class ReturnStmt : public Stmt {
    std::optional<Expr *> m_value;

  public:
    constexpr ReturnStmt(std::optional<Expr *> value)
        : Stmt(QAST_NODE_RETURN), m_value(value) {}

    let get_value() const { return m_value; }
  };

  class ReturnIfStmt : public Stmt {
    Expr *m_cond;
    Expr *m_value;

  public:
    constexpr ReturnIfStmt(Expr *cond, Expr *value)
        : Stmt(QAST_NODE_RETIF), m_cond(cond), m_value(value) {}

    let get_cond() const { return m_cond; }
    let get_value() const { return m_value; }
  };

  class CaseStmt : public Stmt {
    Expr *m_cond;
    Stmt *m_body;

  public:
    constexpr CaseStmt(Expr *cond, Stmt *body)
        : Stmt(QAST_NODE_CASE), m_cond(cond), m_body(body) {}

    let get_cond() const { return m_cond; }
    let get_body() const { return m_body; }
  };

  typedef std::vector<CaseStmt *, Arena<CaseStmt *>> SwitchCases;
  class SwitchStmt : public Stmt {
    Expr *m_cond;
    SwitchCases m_cases;
    Stmt *m_default;

  public:
    SwitchStmt(Expr *cond, const SwitchCases &cases, Stmt *default_)
        : Stmt(QAST_NODE_SWITCH),
          m_cond(cond),
          m_cases(cases),
          m_default(default_) {}

    let get_cond() const { return m_cond; }
    let get_cases() const { return m_cases; }
    let get_default() const { return m_default; }
  };

  using SymbolAttributes = std::set<Expr *, std::less<Expr *>, Arena<Expr *>>;

  class ExportStmt : public Stmt {
    SymbolAttributes m_attrs;
    SmallString m_abi_name;
    Stmt *m_body;
    Vis m_vis;

  public:
    ExportStmt(Stmt *content, SmallString abi_name, Vis vis,
               SymbolAttributes attrs)
        : Stmt(QAST_NODE_EXPORT),
          m_attrs(attrs),
          m_abi_name(abi_name),
          m_body(content),
          m_vis(vis) {}

    let get_body() const { return m_body; }
    let get_abi_name() const { return m_abi_name; }
    let get_vis() const { return m_vis; }
    let get_attrs() const { return m_attrs; }
  };

  typedef std::set<SmallString, std::less<SmallString>, Arena<SmallString>>
      ScopeDeps;

  class ScopeStmt : public Stmt {
    ScopeDeps m_deps;
    SmallString m_name;
    Stmt *m_body;

  public:
    ScopeStmt(SmallString name, Stmt *body, ScopeDeps deps = {})
        : Stmt(QAST_NODE_SCOPE), m_deps(deps), m_name(name), m_body(body) {}

    let get_name() const { return m_name; }
    let get_body() const { return m_body; }
    let get_deps() const { return m_deps; }
  };

  class TypedefStmt : public Stmt {
    SmallString m_name;
    Type *m_type;

  public:
    TypedefStmt(SmallString name, Type *type)
        : Stmt(QAST_NODE_TYPEDEF), m_name(name), m_type(type) {}

    let get_name() const { return m_name; }
    let get_type() const { return m_type; }
  };

  class StructField : public Stmt {
    SmallString m_name;
    Type *m_type;
    Expr *m_value;
    Vis m_visibility;

  public:
    StructField(SmallString name, Type *type, Expr *value, Vis visibility)
        : Stmt(QAST_NODE_STRUCT_FIELD),
          m_name(name),
          m_type(type),
          m_value(value),
          m_visibility(visibility) {}

    let get_name() const { return m_name; }
    let get_type() const { return m_type; }
    let get_value() const { return m_value; }
    let get_visibility() const { return m_visibility; }

    void set_visibility(Vis visibility) { m_visibility = visibility; }
  };

  typedef std::pair<SmallString, Expr *> EnumItem;
  typedef std::vector<EnumItem, Arena<EnumItem>> EnumDefItems;

  class EnumDef : public Stmt {
    EnumDefItems m_items;
    SmallString m_name;
    Type *m_type;

  public:
    EnumDef(SmallString name, Type *type, const EnumDefItems &items)
        : Stmt(QAST_NODE_ENUM), m_items(items), m_name(name), m_type(type) {}

    let get_items() const { return m_items; }
    let get_name() const { return m_name; }
    let get_type() const { return m_type; }
  };

  class FnDecl : public Stmt {
    std::optional<TemplateParameters> m_template_parameters;
    SmallString m_name;
    FuncTy *m_type;

  public:
    FnDecl(SmallString name, FuncTy *type,
           std::optional<TemplateParameters> params = std::nullopt)
        : Stmt(QAST_NODE_FNDECL),
          m_template_parameters(params),
          m_name(name),
          m_type(type) {}

    let get_name() const { return m_name; }
    let get_type() const { return m_type; }
    let get_template_params() const { return m_template_parameters; }

    void set_type(FuncTy *type) { m_type = type; }
    void set_name(SmallString name) { m_name = name; }
    auto &get_template_params() { return m_template_parameters; }
  };

  typedef std::vector<std::pair<SmallString, bool>,
                      Arena<std::pair<SmallString, bool>>>
      FnCaptures;

  class FnDef : public Stmt {
    FnCaptures m_captures;
    std::optional<TemplateParameters> m_template_parameters;
    SmallString m_name;
    FuncTy *m_type;
    Stmt *m_body;
    Expr *m_precond;
    Expr *m_postcond;

  public:
    FnDef(FnDecl *decl, Stmt *body, Expr *precond, Expr *postcond,
          FnCaptures captures = {})
        : Stmt(QAST_NODE_FN),
          m_captures(captures),
          m_template_parameters(decl->get_template_params()),
          m_name(decl->get_name()),
          m_type(decl->get_type()),
          m_body(body),
          m_precond(precond),
          m_postcond(postcond) {}

    let get_captures() const { return m_captures; }
    let get_name() const { return m_name; }
    let get_body() const { return m_body; }
    let get_precond() const { return m_precond; }
    let get_postcond() const { return m_postcond; }
    let get_type() const { return m_type; }
    let get_template_params() const { return m_template_parameters; }

    void set_body(Stmt *body) { m_body = body; }
    void set_precond(Expr *precond) { m_precond = precond; }
    void set_postcond(Expr *postcond) { m_postcond = postcond; }

    std::optional<TemplateParameters> &get_template_params() {
      return m_template_parameters;
    }
  };

  enum class CompositeType { Region, Struct, Group, Class, Union };

  typedef std::vector<Stmt *, Arena<Stmt *>> StructDefFields;
  typedef std::vector<FnDecl *, Arena<FnDecl *>> StructDefMethods;
  typedef std::vector<FnDecl *, Arena<FnDecl *>> StructDefStaticMethods;

  class StructDef : public Stmt {
    StructDefMethods m_methods;
    StructDefStaticMethods m_static_methods;
    StructDefFields m_fields;
    std::optional<TemplateParameters> m_template_parameters;
    CompositeType m_comp_type;
    SmallString m_name;

  public:
    StructDef(SmallString name, const StructDefFields &fields = {},
              const StructDefMethods &methods = {},
              const StructDefStaticMethods &static_methods = {},
              std::optional<TemplateParameters> params = std::nullopt,
              CompositeType t = CompositeType::Struct)
        : Stmt(QAST_NODE_STRUCT),
          m_methods(methods),
          m_static_methods(static_methods),
          m_fields(fields),
          m_template_parameters(params),
          m_comp_type(t),
          m_name(name) {}

    let get_name() const { return m_name; }
    let get_methods() const { return m_methods; }
    let get_static_methods() const { return m_static_methods; }
    let get_fields() const { return m_fields; }
    let get_composite_type() const { return m_comp_type; }
    let get_template_params() const { return m_template_parameters; }

    void set_name(SmallString name) { m_name = name; }
    void set_composite_type(CompositeType t) { m_comp_type = t; }
    auto &get_methods() { return m_methods; }
    auto &get_static_methods() { return m_static_methods; }
    auto &get_fields() { return m_fields; }
    auto &get_template_params() { return m_template_parameters; }
  };

  template <typename T, typename... Args>
  static inline T *make(Args &&...args) {
    T *new_obj = new (Arena<T>().allocate(1)) T(std::forward<Args>(args)...);

    //  /// TODO: Cache nodes

    return new_obj;
  }

  ///=============================================================================

  Stmt *mock_stmt(npar_ty_t expected);
  Expr *mock_expr(npar_ty_t expected);
  Type *mock_type();
}  // namespace npar

constexpr std::string_view npar_node_t::getKindName(npar_ty_t type) {
  const std::array<std::string_view, QAST_NODE_COUNT> names = []() {
    std::array<std::string_view, QAST_NODE_COUNT> R;
    R.fill("");

    R[QAST_NODE_NODE] = "Node";
    R[QAST_NODE_BINEXPR] = "Binexpr";
    R[QAST_NODE_UNEXPR] = "Unexpr";
    R[QAST_NODE_TEREXPR] = "Terexpr";
    R[QAST_NODE_INT] = "Int";
    R[QAST_NODE_FLOAT] = "Float";
    R[QAST_NODE_STRING] = "String";
    R[QAST_NODE_CHAR] = "Char";
    R[QAST_NODE_BOOL] = "Bool";
    R[QAST_NODE_NULL] = "Null";
    R[QAST_NODE_UNDEF] = "Undef";
    R[QAST_NODE_CALL] = "Call";
    R[QAST_NODE_LIST] = "List";
    R[QAST_NODE_ASSOC] = "Assoc";
    R[QAST_NODE_FIELD] = "Field";
    R[QAST_NODE_INDEX] = "Index";
    R[QAST_NODE_SLICE] = "Slice";
    R[QAST_NODE_FSTRING] = "Fstring";
    R[QAST_NODE_IDENT] = "Ident";
    R[QAST_NODE_SEQ] = "SeqPoint";
    R[QAST_NODE_POST_UNEXPR] = "PostUnexpr";
    R[QAST_NODE_STMT_EXPR] = "StmtExpr";
    R[QAST_NODE_TYPE_EXPR] = "TypeExpr";
    R[QAST_NODE_TEMPL_CALL] = "TemplCall";
    R[QAST_NODE_REF_TY] = "Ref";
    R[QAST_NODE_U1_TY] = "U1";
    R[QAST_NODE_U8_TY] = "U8";
    R[QAST_NODE_U16_TY] = "U16";
    R[QAST_NODE_U32_TY] = "U32";
    R[QAST_NODE_U64_TY] = "U64";
    R[QAST_NODE_U128_TY] = "U128";
    R[QAST_NODE_I8_TY] = "I8";
    R[QAST_NODE_I16_TY] = "I16";
    R[QAST_NODE_I32_TY] = "I32";
    R[QAST_NODE_I64_TY] = "I64";
    R[QAST_NODE_I128_TY] = "I128";
    R[QAST_NODE_F16_TY] = "F16";
    R[QAST_NODE_F32_TY] = "F32";
    R[QAST_NODE_F64_TY] = "F64";
    R[QAST_NODE_F128_TY] = "F128";
    R[QAST_NODE_VOID_TY] = "Void";
    R[QAST_NODE_PTR_TY] = "Ptr";
    R[QAST_NODE_OPAQUE_TY] = "Opaque";
    R[QAST_NODE_ARRAY_TY] = "Array";
    R[QAST_NODE_TUPLE_TY] = "Tuple";
    R[QAST_NODE_FN_TY] = "FuncTy";
    R[QAST_NODE_UNRES_TY] = "Unres";
    R[QAST_NODE_INFER_TY] = "Infer";
    R[QAST_NODE_TEMPL_TY] = "Templ";
    R[QAST_NODE_TYPEDEF] = "Typedef";
    R[QAST_NODE_FNDECL] = "FnDecl";
    R[QAST_NODE_STRUCT] = "Struct";
    R[QAST_NODE_ENUM] = "Enum";
    R[QAST_NODE_FN] = "FnDef";
    R[QAST_NODE_SCOPE] = "Scope";
    R[QAST_NODE_EXPORT] = "Export";
    R[QAST_NODE_STRUCT_FIELD] = "StructField";
    R[QAST_NODE_BLOCK] = "Block";
    R[QAST_NODE_VAR] = "Let";
    R[QAST_NODE_INLINE_ASM] = "InlineAsm";
    R[QAST_NODE_RETURN] = "Return";
    R[QAST_NODE_RETIF] = "Retif";
    R[QAST_NODE_BREAK] = "Break";
    R[QAST_NODE_CONTINUE] = "Continue";
    R[QAST_NODE_IF] = "If";
    R[QAST_NODE_WHILE] = "While";
    R[QAST_NODE_FOR] = "For";
    R[QAST_NODE_FOREACH] = "Foreach";
    R[QAST_NODE_CASE] = "Case";
    R[QAST_NODE_SWITCH] = "Switch";
    R[QAST_NODE_EXPR_STMT] = "ExprStmt";

    return R;
  }();

  return names[type];
}

constexpr bool npar_node_t::is_expr_stmt(npar_ty_t type) const {
  return is(QAST_NODE_EXPR_STMT) && as<npar::ExprStmt>()->get_expr()->is(type);
}

constexpr bool npar_node_t::is_stmt_expr(npar_ty_t type) const {
  return is(QAST_NODE_STMT_EXPR) && as<npar::StmtExpr>()->get_stmt()->is(type);
}

constexpr bool npar_node_t::isSame(const npar_node_t *o) const {
  if (this == o) {
    return true;
  }

  if (getKind() != o->getKind()) {
    return false;
  }

  using namespace npar;

  switch (getKind()) {
    case QAST_NODE_NODE: {
      let us = as<npar_node_t>();
      let them = o->as<npar_node_t>();

      return us->m_mock == them->m_mock;
    }

    case QAST_NODE_BINEXPR: {
      let us = as<BinExpr>();
      let them = o->as<BinExpr>();

      return us->get_op() == them->get_op() &&
             us->get_lhs()->isSame(them->get_lhs()) &&
             us->get_rhs()->isSame(them->get_rhs());
    }

    case QAST_NODE_UNEXPR: {
      let us = as<UnaryExpr>();
      let them = o->as<UnaryExpr>();

      return us->get_op() == them->get_op() &&
             us->get_rhs()->isSame(them->get_rhs());
    }

    case QAST_NODE_TEREXPR: {
      let us = as<TernaryExpr>();
      let them = o->as<TernaryExpr>();

      return us->get_cond()->isSame(them->get_cond()) &&
             us->get_lhs()->isSame(them->get_lhs()) &&
             us->get_rhs()->isSame(them->get_rhs());
    }

    case QAST_NODE_INT: {
      let us = as<ConstInt>();
      let them = o->as<ConstInt>();

      return us->get_value() == them->get_value();
    }

    case QAST_NODE_FLOAT: {
      let us = as<ConstFloat>();
      let them = o->as<ConstFloat>();

      (void)us;
      (void)them;

      /// TODO: Implement check Float
      qcore_implement();

      return true;
    }

    case QAST_NODE_STRING: {
      let us = as<ConstString>();
      let them = o->as<ConstString>();

      (void)us;
      (void)them;

      /// TODO: Implement check String
      qcore_implement();

      return true;
    }

    case QAST_NODE_CHAR: {
      let us = as<ConstChar>();
      let them = o->as<ConstChar>();

      (void)us;
      (void)them;

      /// TODO: Implement check Char
      qcore_implement();

      return true;
    }

    case QAST_NODE_BOOL: {
      let us = as<ConstBool>();
      let them = o->as<ConstBool>();

      return us->get_value() == them->get_value();
    }

    case QAST_NODE_NULL: {
      let us = as<ConstNull>();
      let them = o->as<ConstNull>();

      (void)us;
      (void)them;

      /// TODO: Implement check Null
      qcore_implement();

      return true;
    }

    case QAST_NODE_UNDEF: {
      let us = as<ConstUndef>();
      let them = o->as<ConstUndef>();

      (void)us;
      (void)them;

      /// TODO: Implement check Undef
      qcore_implement();

      return true;
    }

    case QAST_NODE_CALL: {
      let us = as<Call>();
      let them = o->as<Call>();

      (void)us;
      (void)them;

      /// TODO: Implement check Call
      qcore_implement();

      return true;
    }

    case QAST_NODE_LIST: {
      let us = as<List>();
      let them = o->as<List>();

      (void)us;
      (void)them;

      /// TODO: Implement check List
      qcore_implement();

      return true;
    }

    case QAST_NODE_ASSOC: {
      let us = as<Assoc>();
      let them = o->as<Assoc>();

      (void)us;
      (void)them;

      /// TODO: Implement check Assoc
      qcore_implement();

      return true;
    }

    case QAST_NODE_FIELD: {
      let us = as<Field>();
      let them = o->as<Field>();

      (void)us;
      (void)them;

      /// TODO: Implement check Field
      qcore_implement();

      return true;
    }

    case QAST_NODE_INDEX: {
      let us = as<Index>();
      let them = o->as<Index>();

      (void)us;
      (void)them;

      /// TODO: Implement check Index
      qcore_implement();

      return true;
    }

    case QAST_NODE_SLICE: {
      let us = as<Slice>();
      let them = o->as<Slice>();

      (void)us;
      (void)them;

      /// TODO: Implement check Slice
      qcore_implement();

      return true;
    }

    case QAST_NODE_FSTRING: {
      let us = as<FString>();
      let them = o->as<FString>();

      (void)us;
      (void)them;

      /// TODO: Implement check Fstring
      qcore_implement();

      return true;
    }

    case QAST_NODE_IDENT: {
      let us = as<Ident>();
      let them = o->as<Ident>();

      (void)us;
      (void)them;

      /// TODO: Implement check Ident
      qcore_implement();

      return true;
    }

    case QAST_NODE_SEQ: {
      let us = as<SeqPoint>();
      let them = o->as<SeqPoint>();

      (void)us;
      (void)them;

      /// TODO: Implement check SeqPoint
      qcore_implement();

      return true;
    }

    case QAST_NODE_POST_UNEXPR: {
      let us = as<PostUnaryExpr>();
      let them = o->as<PostUnaryExpr>();

      (void)us;
      (void)them;

      /// TODO: Implement check PostUnexpr
      qcore_implement();

      return true;
    }

    case QAST_NODE_STMT_EXPR: {
      let us = as<StmtExpr>();
      let them = o->as<StmtExpr>();

      (void)us;
      (void)them;

      /// TODO: Implement check StmtExpr
      qcore_implement();

      return true;
    }

    case QAST_NODE_TYPE_EXPR: {
      let us = as<TypeExpr>();
      let them = o->as<TypeExpr>();

      (void)us;
      (void)them;

      /// TODO: Implement check TypeExpr
      qcore_implement();

      return true;
    }

    case QAST_NODE_TEMPL_CALL: {
      let us = as<TemplCall>();
      let them = o->as<TemplCall>();

      (void)us;
      (void)them;

      /// TODO: Implement check TemplCall
      qcore_implement();

      return true;
    }

    case QAST_NODE_REF_TY: {
      let us = as<RefTy>();
      let them = o->as<RefTy>();

      (void)us;
      (void)them;

      /// TODO: Implement check Ref
      qcore_implement();

      return true;
    }

    case QAST_NODE_U1_TY: {
      let us = as<U1>();
      let them = o->as<U1>();

      (void)us;
      (void)them;

      /// TODO: Implement check U1
      qcore_implement();

      return true;
    }

    case QAST_NODE_U8_TY: {
      let us = as<U8>();
      let them = o->as<U8>();

      (void)us;
      (void)them;

      /// TODO: Implement check U8
      qcore_implement();

      return true;
    }

    case QAST_NODE_U16_TY: {
      let us = as<U16>();
      let them = o->as<U16>();

      (void)us;
      (void)them;

      /// TODO: Implement check U16
      qcore_implement();

      return true;
    }

    case QAST_NODE_U32_TY: {
      let us = as<U32>();
      let them = o->as<U32>();

      (void)us;
      (void)them;

      /// TODO: Implement check U32
      qcore_implement();

      return true;
    }

    case QAST_NODE_U64_TY: {
      let us = as<U64>();
      let them = o->as<U64>();

      (void)us;
      (void)them;

      /// TODO: Implement check U64
      qcore_implement();

      return true;
    }

    case QAST_NODE_U128_TY: {
      let us = as<U128>();
      let them = o->as<U128>();

      (void)us;
      (void)them;

      /// TODO: Implement check U128
      qcore_implement();

      return true;
    }

    case QAST_NODE_I8_TY: {
      let us = as<I8>();
      let them = o->as<I8>();

      (void)us;
      (void)them;

      /// TODO: Implement check I8
      qcore_implement();

      return true;
    }

    case QAST_NODE_I16_TY: {
      let us = as<I16>();
      let them = o->as<I16>();

      (void)us;
      (void)them;

      /// TODO: Implement check I16
      qcore_implement();

      return true;
    }

    case QAST_NODE_I32_TY: {
      let us = as<I32>();
      let them = o->as<I32>();

      (void)us;
      (void)them;

      /// TODO: Implement check I32
      qcore_implement();

      return true;
    }

    case QAST_NODE_I64_TY: {
      let us = as<I64>();
      let them = o->as<I64>();

      (void)us;
      (void)them;

      /// TODO: Implement check I64
      qcore_implement();

      return true;
    }

    case QAST_NODE_I128_TY: {
      let us = as<I128>();
      let them = o->as<I128>();

      (void)us;
      (void)them;

      /// TODO: Implement check I128
      qcore_implement();

      return true;
    }

    case QAST_NODE_F16_TY: {
      let us = as<F16>();
      let them = o->as<F16>();

      (void)us;
      (void)them;

      /// TODO: Implement check F16
      qcore_implement();

      return true;
    }

    case QAST_NODE_F32_TY: {
      let us = as<F32>();
      let them = o->as<F32>();

      (void)us;
      (void)them;

      /// TODO: Implement check F32
      qcore_implement();

      return true;
    }

    case QAST_NODE_F64_TY: {
      let us = as<F64>();
      let them = o->as<F64>();

      (void)us;
      (void)them;

      /// TODO: Implement check F64
      qcore_implement();

      return true;
    }

    case QAST_NODE_F128_TY: {
      let us = as<F128>();
      let them = o->as<F128>();

      (void)us;
      (void)them;

      /// TODO: Implement check F128
      qcore_implement();

      return true;
    }

    case QAST_NODE_VOID_TY: {
      let us = as<VoidTy>();
      let them = o->as<VoidTy>();

      (void)us;
      (void)them;

      /// TODO: Implement check Void
      qcore_implement();

      return true;
    }

    case QAST_NODE_PTR_TY: {
      let us = as<PtrTy>();
      let them = o->as<PtrTy>();

      (void)us;
      (void)them;

      /// TODO: Implement check Ptr
      qcore_implement();

      return true;
    }

    case QAST_NODE_OPAQUE_TY: {
      let us = as<OpaqueTy>();
      let them = o->as<OpaqueTy>();

      (void)us;
      (void)them;

      /// TODO: Implement check Opaque
      qcore_implement();

      return true;
    }

    case QAST_NODE_ARRAY_TY: {
      let us = as<ArrayTy>();
      let them = o->as<ArrayTy>();

      (void)us;
      (void)them;

      /// TODO: Implement check Array
      qcore_implement();

      return true;
    }

    case QAST_NODE_TUPLE_TY: {
      let us = as<TupleTy>();
      let them = o->as<TupleTy>();

      (void)us;
      (void)them;

      /// TODO: Implement check Tuple
      qcore_implement();

      return true;
    }

    case QAST_NODE_FN_TY: {
      let us = as<FuncTy>();
      let them = o->as<FuncTy>();

      (void)us;
      (void)them;

      /// TODO: Implement check FuncTy
      qcore_implement();

      return true;
    }

    case QAST_NODE_UNRES_TY: {
      let us = as<NamedTy>();
      let them = o->as<NamedTy>();

      (void)us;
      (void)them;

      /// TODO: Implement check Unres
      qcore_implement();

      return true;
    }

    case QAST_NODE_INFER_TY: {
      let us = as<InferTy>();
      let them = o->as<InferTy>();

      (void)us;
      (void)them;

      /// TODO: Implement check Infer
      qcore_implement();

      return true;
    }

    case QAST_NODE_TEMPL_TY: {
      let us = as<TemplType>();
      let them = o->as<TemplType>();

      (void)us;
      (void)them;

      /// TODO: Implement check Templ
      qcore_implement();

      return true;
    }

    case QAST_NODE_TYPEDEF: {
      let us = as<TypedefStmt>();
      let them = o->as<TypedefStmt>();

      (void)us;
      (void)them;

      /// TODO: Implement check Typedef
      qcore_implement();

      return true;
    }

    case QAST_NODE_FNDECL: {
      let us = as<FnDecl>();
      let them = o->as<FnDecl>();

      (void)us;
      (void)them;

      /// TODO: Implement check FnDecl
      qcore_implement();

      return true;
    }

    case QAST_NODE_STRUCT: {
      let us = as<StructDef>();
      let them = o->as<StructDef>();

      (void)us;
      (void)them;

      /// TODO: Implement check Struct
      qcore_implement();

      return true;
    }

    case QAST_NODE_ENUM: {
      let us = as<EnumDef>();
      let them = o->as<EnumDef>();

      (void)us;
      (void)them;

      /// TODO: Implement check Enum
      qcore_implement();

      return true;
    }

    case QAST_NODE_FN: {
      let us = as<FnDef>();
      let them = o->as<FnDef>();

      (void)us;
      (void)them;

      /// TODO: Implement check FnDef
      qcore_implement();

      return true;
    }

    case QAST_NODE_SCOPE: {
      let us = as<ScopeStmt>();
      let them = o->as<ScopeStmt>();

      (void)us;
      (void)them;

      /// TODO: Implement check Scope
      qcore_implement();

      return true;
    }

    case QAST_NODE_EXPORT: {
      let us = as<ExportStmt>();
      let them = o->as<ExportStmt>();

      (void)us;
      (void)them;

      /// TODO: Implement check Export
      qcore_implement();

      return true;
    }

    case QAST_NODE_STRUCT_FIELD: {
      let us = as<StructField>();
      let them = o->as<StructField>();

      (void)us;
      (void)them;

      /// TODO: Implement check StructField
      qcore_implement();

      return true;
    }

    case QAST_NODE_BLOCK: {
      let us = as<Block>();
      let them = o->as<Block>();

      (void)us;
      (void)them;

      /// TODO: Implement check Block
      qcore_implement();

      return true;
    }

    case QAST_NODE_VAR: {
      let us = as<VarDecl>();
      let them = o->as<VarDecl>();

      (void)us;
      (void)them;

      /// TODO: Implement check Let
      qcore_implement();

      return true;
    }

    case QAST_NODE_INLINE_ASM: {
      let us = as<InlineAsm>();
      let them = o->as<InlineAsm>();

      (void)us;
      (void)them;

      /// TODO: Implement check InlineAsm
      qcore_implement();

      return true;
    }

    case QAST_NODE_RETURN: {
      let us = as<ReturnStmt>();
      let them = o->as<ReturnStmt>();

      (void)us;
      (void)them;

      /// TODO: Implement check Return
      qcore_implement();

      return true;
    }

    case QAST_NODE_RETIF: {
      let us = as<ReturnIfStmt>();
      let them = o->as<ReturnIfStmt>();

      (void)us;
      (void)them;

      /// TODO: Implement check Retif
      qcore_implement();

      return true;
    }

    case QAST_NODE_BREAK: {
      let us = as<BreakStmt>();
      let them = o->as<BreakStmt>();

      (void)us;
      (void)them;

      /// TODO: Implement check Break
      qcore_implement();

      return true;
    }

    case QAST_NODE_CONTINUE: {
      let us = as<ContinueStmt>();
      let them = o->as<ContinueStmt>();

      (void)us;
      (void)them;

      /// TODO: Implement check Continue
      qcore_implement();

      return true;
    }

    case QAST_NODE_IF: {
      let us = as<IfStmt>();
      let them = o->as<IfStmt>();

      (void)us;
      (void)them;

      /// TODO: Implement check If
      qcore_implement();

      return true;
    }

    case QAST_NODE_WHILE: {
      let us = as<WhileStmt>();
      let them = o->as<WhileStmt>();

      (void)us;
      (void)them;

      /// TODO: Implement check While
      qcore_implement();

      return true;
    }

    case QAST_NODE_FOR: {
      let us = as<ForStmt>();
      let them = o->as<ForStmt>();

      (void)us;
      (void)them;

      /// TODO: Implement check For
      qcore_implement();

      return true;
    }

    case QAST_NODE_FOREACH: {
      let us = as<ForeachStmt>();
      let them = o->as<ForeachStmt>();

      (void)us;
      (void)them;

      /// TODO: Implement check Foreach
      qcore_implement();

      return true;
    }

    case QAST_NODE_CASE: {
      let us = as<CaseStmt>();
      let them = o->as<CaseStmt>();

      (void)us;
      (void)them;

      /// TODO: Implement check Case
      qcore_implement();

      return true;
    }

    case QAST_NODE_SWITCH: {
      let us = as<SwitchStmt>();
      let them = o->as<SwitchStmt>();

      (void)us;
      (void)them;

      /// TODO: Implement check Switch
      qcore_implement();

      return true;
    }

    case QAST_NODE_EXPR_STMT: {
      let us = as<ExprStmt>();
      let them = o->as<ExprStmt>();

      (void)us;
      (void)them;

      /// TODO: Implement check ExprStmt
      qcore_implement();

      return true;
    }
  }

  /// TODO: Implement
  qcore_implement();
}

#endif  // __NITRATE_PARSER_NODE_H__
