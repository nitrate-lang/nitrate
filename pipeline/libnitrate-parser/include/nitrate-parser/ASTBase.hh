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

#ifndef __NITRATE_AST_ASTBASE_H__
#define __NITRATE_AST_ASTBASE_H__

#include <nitrate-core/Error.h>
#include <nitrate-core/Macro.h>

#include <iostream>
#include <nitrate-lexer/Token.hh>
#include <nitrate-parser/ASTCommon.hh>
#include <nitrate-parser/ASTData.hh>
#include <nitrate-parser/ASTVisitor.hh>

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
      case QAST_BASE: {
        v.visit(*as<npar_node_t>());
        break;
      }
      case QAST_BINEXPR: {
        v.visit(*as<BinExpr>());
        break;
      }
      case QAST_UNEXPR: {
        v.visit(*as<UnaryExpr>());
        break;
      }
      case QAST_TEREXPR: {
        v.visit(*as<TernaryExpr>());
        break;
      }
      case QAST_INT: {
        v.visit(*as<ConstInt>());
        break;
      }
      case QAST_FLOAT: {
        v.visit(*as<ConstFloat>());
        break;
      }
      case QAST_STRING: {
        v.visit(*as<ConstString>());
        break;
      }
      case QAST_CHAR: {
        v.visit(*as<ConstChar>());
        break;
      }
      case QAST_BOOL: {
        v.visit(*as<ConstBool>());
        break;
      }
      case QAST_NULL: {
        v.visit(*as<ConstNull>());
        break;
      }
      case QAST_UNDEF: {
        v.visit(*as<ConstUndef>());
        break;
      }
      case QAST_CALL: {
        v.visit(*as<Call>());
        break;
      }
      case QAST_LIST: {
        v.visit(*as<List>());
        break;
      }
      case QAST_ASSOC: {
        v.visit(*as<Assoc>());
        break;
      }
      case QAST_FIELD: {
        v.visit(*as<Field>());
        break;
      }
      case QAST_INDEX: {
        v.visit(*as<Index>());
        break;
      }
      case QAST_SLICE: {
        v.visit(*as<Slice>());
        break;
      }
      case QAST_FSTRING: {
        v.visit(*as<FString>());
        break;
      }
      case QAST_IDENT: {
        v.visit(*as<Ident>());
        break;
      }
      case QAST_SEQ: {
        v.visit(*as<SeqPoint>());
        break;
      }
      case QAST_POST_UNEXPR: {
        v.visit(*as<PostUnaryExpr>());
        break;
      }
      case QAST_SEXPR: {
        v.visit(*as<StmtExpr>());
        break;
      }
      case QAST_TEXPR: {
        v.visit(*as<TypeExpr>());
        break;
      }
      case QAST_TEMPL_CALL: {
        v.visit(*as<TemplCall>());
        break;
      }
      case QAST_REF: {
        v.visit(*as<RefTy>());
        break;
      }
      case QAST_U1: {
        v.visit(*as<U1>());
        break;
      }
      case QAST_U8: {
        v.visit(*as<U8>());
        break;
      }
      case QAST_U16: {
        v.visit(*as<U16>());
        break;
      }
      case QAST_U32: {
        v.visit(*as<U32>());
        break;
      }
      case QAST_U64: {
        v.visit(*as<U64>());
        break;
      }
      case QAST_U128: {
        v.visit(*as<U128>());
        break;
      }
      case QAST_I8: {
        v.visit(*as<I8>());
        break;
      }
      case QAST_I16: {
        v.visit(*as<I16>());
        break;
      }
      case QAST_I32: {
        v.visit(*as<I32>());
        break;
      }
      case QAST_I64: {
        v.visit(*as<I64>());
        break;
      }
      case QAST_I128: {
        v.visit(*as<I128>());
        break;
      }
      case QAST_F16: {
        v.visit(*as<F16>());
        break;
      }
      case QAST_F32: {
        v.visit(*as<F32>());
        break;
      }
      case QAST_F64: {
        v.visit(*as<F64>());
        break;
      }
      case QAST_F128: {
        v.visit(*as<F128>());
        break;
      }
      case QAST_VOID: {
        v.visit(*as<VoidTy>());
        break;
      }
      case QAST_PTR: {
        v.visit(*as<PtrTy>());
        break;
      }
      case QAST_OPAQUE: {
        v.visit(*as<OpaqueTy>());
        break;
      }
      case QAST_ARRAY: {
        v.visit(*as<ArrayTy>());
        break;
      }
      case QAST_TUPLE: {
        v.visit(*as<TupleTy>());
        break;
      }
      case QAST_FUNCTOR: {
        v.visit(*as<FuncTy>());
        break;
      }
      case QAST_NAMED: {
        v.visit(*as<NamedTy>());
        break;
      }
      case QAST_INFER: {
        v.visit(*as<InferTy>());
        break;
      }
      case QAST_TEMPLATE: {
        v.visit(*as<TemplType>());
        break;
      }
      case QAST_TYPEDEF: {
        v.visit(*as<TypedefStmt>());
        break;
      }
      case QAST_STRUCT: {
        v.visit(*as<StructDef>());
        break;
      }
      case QAST_ENUM: {
        v.visit(*as<EnumDef>());
        break;
      }
      case QAST_FUNCTION: {
        v.visit(*as<Function>());
        break;
      }
      case QAST_SCOPE: {
        v.visit(*as<ScopeStmt>());
        break;
      }
      case QAST_EXPORT: {
        v.visit(*as<ExportStmt>());
        break;
      }
      case QAST_BLOCK: {
        v.visit(*as<Block>());
        break;
      }
      case QAST_VAR: {
        v.visit(*as<VarDecl>());
        break;
      }
      case QAST_INLINE_ASM: {
        v.visit(*as<InlineAsm>());
        break;
      }
      case QAST_RETURN: {
        v.visit(*as<ReturnStmt>());
        break;
      }
      case QAST_RETIF: {
        v.visit(*as<ReturnIfStmt>());
        break;
      }
      case QAST_BREAK: {
        v.visit(*as<BreakStmt>());
        break;
      }
      case QAST_CONTINUE: {
        v.visit(*as<ContinueStmt>());
        break;
      }
      case QAST_IF: {
        v.visit(*as<IfStmt>());
        break;
      }
      case QAST_WHILE: {
        v.visit(*as<WhileStmt>());
        break;
      }
      case QAST_FOR: {
        v.visit(*as<ForStmt>());
        break;
      }
      case QAST_FOREACH: {
        v.visit(*as<ForeachStmt>());
        break;
      }
      case QAST_CASE: {
        v.visit(*as<CaseStmt>());
        break;
      }
      case QAST_SWITCH: {
        v.visit(*as<SwitchStmt>());
        break;
      }
      case QAST_ESTMT: {
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
      return QAST_BASE;
    } else if constexpr (std::is_same_v<T, Stmt>) {
      return QAST_BASE;
    } else if constexpr (std::is_same_v<T, Type>) {
      return QAST_BASE;
    } else if constexpr (std::is_same_v<T, BinExpr>) {
      return QAST_BINEXPR;
    } else if constexpr (std::is_same_v<T, UnaryExpr>) {
      return QAST_UNEXPR;
    } else if constexpr (std::is_same_v<T, TernaryExpr>) {
      return QAST_TEREXPR;
    } else if constexpr (std::is_same_v<T, ConstInt>) {
      return QAST_INT;
    } else if constexpr (std::is_same_v<T, ConstFloat>) {
      return QAST_FLOAT;
    } else if constexpr (std::is_same_v<T, ConstString>) {
      return QAST_STRING;
    } else if constexpr (std::is_same_v<T, ConstChar>) {
      return QAST_CHAR;
    } else if constexpr (std::is_same_v<T, ConstBool>) {
      return QAST_BOOL;
    } else if constexpr (std::is_same_v<T, ConstNull>) {
      return QAST_NULL;
    } else if constexpr (std::is_same_v<T, ConstUndef>) {
      return QAST_UNDEF;
    } else if constexpr (std::is_same_v<T, Call>) {
      return QAST_CALL;
    } else if constexpr (std::is_same_v<T, List>) {
      return QAST_LIST;
    } else if constexpr (std::is_same_v<T, Assoc>) {
      return QAST_ASSOC;
    } else if constexpr (std::is_same_v<T, Field>) {
      return QAST_FIELD;
    } else if constexpr (std::is_same_v<T, Index>) {
      return QAST_INDEX;
    } else if constexpr (std::is_same_v<T, Slice>) {
      return QAST_SLICE;
    } else if constexpr (std::is_same_v<T, FString>) {
      return QAST_FSTRING;
    } else if constexpr (std::is_same_v<T, Ident>) {
      return QAST_IDENT;
    } else if constexpr (std::is_same_v<T, SeqPoint>) {
      return QAST_SEQ;
    } else if constexpr (std::is_same_v<T, PostUnaryExpr>) {
      return QAST_POST_UNEXPR;
    } else if constexpr (std::is_same_v<T, StmtExpr>) {
      return QAST_SEXPR;
    } else if constexpr (std::is_same_v<T, TypeExpr>) {
      return QAST_TEXPR;
    } else if constexpr (std::is_same_v<T, TemplCall>) {
      return QAST_TEMPL_CALL;
    } else if constexpr (std::is_same_v<T, RefTy>) {
      return QAST_REF;
    } else if constexpr (std::is_same_v<T, U1>) {
      return QAST_U1;
    } else if constexpr (std::is_same_v<T, U8>) {
      return QAST_U8;
    } else if constexpr (std::is_same_v<T, U16>) {
      return QAST_U16;
    } else if constexpr (std::is_same_v<T, U32>) {
      return QAST_U32;
    } else if constexpr (std::is_same_v<T, U64>) {
      return QAST_U64;
    } else if constexpr (std::is_same_v<T, U128>) {
      return QAST_U128;
    } else if constexpr (std::is_same_v<T, I8>) {
      return QAST_I8;
    } else if constexpr (std::is_same_v<T, I16>) {
      return QAST_I16;
    } else if constexpr (std::is_same_v<T, I32>) {
      return QAST_I32;
    } else if constexpr (std::is_same_v<T, I64>) {
      return QAST_I64;
    } else if constexpr (std::is_same_v<T, I128>) {
      return QAST_I128;
    } else if constexpr (std::is_same_v<T, F16>) {
      return QAST_F16;
    } else if constexpr (std::is_same_v<T, F32>) {
      return QAST_F32;
    } else if constexpr (std::is_same_v<T, F64>) {
      return QAST_F64;
    } else if constexpr (std::is_same_v<T, F128>) {
      return QAST_F128;
    } else if constexpr (std::is_same_v<T, VoidTy>) {
      return QAST_VOID;
    } else if constexpr (std::is_same_v<T, PtrTy>) {
      return QAST_PTR;
    } else if constexpr (std::is_same_v<T, OpaqueTy>) {
      return QAST_OPAQUE;
    } else if constexpr (std::is_same_v<T, ArrayTy>) {
      return QAST_ARRAY;
    } else if constexpr (std::is_same_v<T, TupleTy>) {
      return QAST_TUPLE;
    } else if constexpr (std::is_same_v<T, FuncTy>) {
      return QAST_FUNCTOR;
    } else if constexpr (std::is_same_v<T, NamedTy>) {
      return QAST_NAMED;
    } else if constexpr (std::is_same_v<T, InferTy>) {
      return QAST_INFER;
    } else if constexpr (std::is_same_v<T, TemplType>) {
      return QAST_TEMPLATE;
    } else if constexpr (std::is_same_v<T, TypedefStmt>) {
      return QAST_TYPEDEF;
    } else if constexpr (std::is_same_v<T, StructDef>) {
      return QAST_STRUCT;
    } else if constexpr (std::is_same_v<T, EnumDef>) {
      return QAST_ENUM;
    } else if constexpr (std::is_same_v<T, Function>) {
      return QAST_FUNCTION;
    } else if constexpr (std::is_same_v<T, ScopeStmt>) {
      return QAST_SCOPE;
    } else if constexpr (std::is_same_v<T, ExportStmt>) {
      return QAST_EXPORT;
    } else if constexpr (std::is_same_v<T, Block>) {
      return QAST_BLOCK;
    } else if constexpr (std::is_same_v<T, VarDecl>) {
      return QAST_VAR;
    } else if constexpr (std::is_same_v<T, InlineAsm>) {
      return QAST_INLINE_ASM;
    } else if constexpr (std::is_same_v<T, ReturnStmt>) {
      return QAST_RETURN;
    } else if constexpr (std::is_same_v<T, ReturnIfStmt>) {
      return QAST_RETIF;
    } else if constexpr (std::is_same_v<T, BreakStmt>) {
      return QAST_BREAK;
    } else if constexpr (std::is_same_v<T, ContinueStmt>) {
      return QAST_CONTINUE;
    } else if constexpr (std::is_same_v<T, IfStmt>) {
      return QAST_IF;
    } else if constexpr (std::is_same_v<T, WhileStmt>) {
      return QAST_WHILE;
    } else if constexpr (std::is_same_v<T, ForStmt>) {
      return QAST_FOR;
    } else if constexpr (std::is_same_v<T, ForeachStmt>) {
      return QAST_FOREACH;
    } else if constexpr (std::is_same_v<T, CaseStmt>) {
      return QAST_CASE;
    } else if constexpr (std::is_same_v<T, SwitchStmt>) {
      return QAST_SWITCH;
    } else if constexpr (std::is_same_v<T, ExprStmt>) {
      return QAST_ESTMT;
    }
  }

  constexpr npar_ty_t getKind() const { return m_node_type; }
  constexpr auto getKindName() const { return getKindName(m_node_type); }

  ///======================================================================

  constexpr bool is_type() const {
    auto kind = getKind();
    return kind >= QAST__TYPE_FIRST && kind <= QAST__TYPE_LAST;
  }

  constexpr bool is_stmt() const {
    auto kind = getKind();
    return kind >= QAST__STMT_FIRST && kind <= QAST__STMT_LAST;
  }

  constexpr bool is_expr() const {
    auto kind = getKind();
    return kind >= QAST__EXPR_FIRST && kind <= QAST__EXPR_LAST;
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

    return reinterpret_cast<T *>(ptr);
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

  bool isSame(const npar_node_t *o) const;

  uint64_t hash64() const;

  std::ostream &dump(std::ostream &os = std::cerr,
                     bool isForDebug = false) const;

  constexpr void set_offset(uint32_t pos) { m_offset = pos; }

  constexpr uint32_t get_offset() const { return m_offset; }
  constexpr uint32_t get_fileid() const { return m_fileid; }
  constexpr bool is_mock() const { return m_mock; }

  constexpr std::tuple<uint32_t, uint32_t> get_pos() const {
    return {m_offset, m_fileid};
  }
} __attribute__((packed));

static_assert(sizeof(npar_node_t) == 8);

///======================================================================

constexpr std::string_view npar_node_t::getKindName(npar_ty_t type) {
  const std::array<std::string_view, QAST_COUNT> names = []() {
    std::array<std::string_view, QAST_COUNT> R;
    R.fill("");

    R[QAST_BASE] = "Node";
    R[QAST_BINEXPR] = "Binexpr";
    R[QAST_UNEXPR] = "Unexpr";
    R[QAST_TEREXPR] = "Terexpr";
    R[QAST_INT] = "Int";
    R[QAST_FLOAT] = "Float";
    R[QAST_STRING] = "String";
    R[QAST_CHAR] = "Char";
    R[QAST_BOOL] = "Bool";
    R[QAST_NULL] = "Null";
    R[QAST_UNDEF] = "Undef";
    R[QAST_CALL] = "Call";
    R[QAST_LIST] = "List";
    R[QAST_ASSOC] = "Assoc";
    R[QAST_FIELD] = "Field";
    R[QAST_INDEX] = "Index";
    R[QAST_SLICE] = "Slice";
    R[QAST_FSTRING] = "Fstring";
    R[QAST_IDENT] = "Ident";
    R[QAST_SEQ] = "SeqPoint";
    R[QAST_POST_UNEXPR] = "PostUnexpr";
    R[QAST_SEXPR] = "StmtExpr";
    R[QAST_TEXPR] = "TypeExpr";
    R[QAST_TEMPL_CALL] = "TemplCall";
    R[QAST_REF] = "Ref";
    R[QAST_U1] = "U1";
    R[QAST_U8] = "U8";
    R[QAST_U16] = "U16";
    R[QAST_U32] = "U32";
    R[QAST_U64] = "U64";
    R[QAST_U128] = "U128";
    R[QAST_I8] = "I8";
    R[QAST_I16] = "I16";
    R[QAST_I32] = "I32";
    R[QAST_I64] = "I64";
    R[QAST_I128] = "I128";
    R[QAST_F16] = "F16";
    R[QAST_F32] = "F32";
    R[QAST_F64] = "F64";
    R[QAST_F128] = "F128";
    R[QAST_VOID] = "Void";
    R[QAST_PTR] = "Ptr";
    R[QAST_OPAQUE] = "Opaque";
    R[QAST_ARRAY] = "Array";
    R[QAST_TUPLE] = "Tuple";
    R[QAST_FUNCTOR] = "FuncTy";
    R[QAST_NAMED] = "Unres";
    R[QAST_INFER] = "Infer";
    R[QAST_TEMPLATE] = "Templ";
    R[QAST_TYPEDEF] = "Typedef";
    R[QAST_STRUCT] = "Struct";
    R[QAST_ENUM] = "Enum";
    R[QAST_FUNCTION] = "Function";
    R[QAST_SCOPE] = "Scope";
    R[QAST_EXPORT] = "Export";
    R[QAST_BLOCK] = "Block";
    R[QAST_VAR] = "Let";
    R[QAST_INLINE_ASM] = "InlineAsm";
    R[QAST_RETURN] = "Return";
    R[QAST_RETIF] = "Retif";
    R[QAST_BREAK] = "Break";
    R[QAST_CONTINUE] = "Continue";
    R[QAST_IF] = "If";
    R[QAST_WHILE] = "While";
    R[QAST_FOR] = "For";
    R[QAST_FOREACH] = "Foreach";
    R[QAST_CASE] = "Case";
    R[QAST_SWITCH] = "Switch";
    R[QAST_ESTMT] = "ExprStmt";

    return R;
  }();

  return names[type];
}

namespace npar {
  class Stmt : public npar_node_t {
  public:
    constexpr Stmt(npar_ty_t ty) : npar_node_t(ty){};

    constexpr bool is_expr_stmt(npar_ty_t type) const;
  };

  class Type : public npar_node_t {
    std::pair<Expr *, Expr *> m_range;
    Expr *m_width;

  public:
    constexpr Type(npar_ty_t ty)
        : npar_node_t(ty), m_range({nullptr, nullptr}), m_width(nullptr) {}

    constexpr bool is_primitive() const {
      switch (getKind()) {
        case QAST_U1:
        case QAST_U8:
        case QAST_U16:
        case QAST_U32:
        case QAST_U64:
        case QAST_U128:
        case QAST_I8:
        case QAST_I16:
        case QAST_I32:
        case QAST_I64:
        case QAST_I128:
        case QAST_F16:
        case QAST_F32:
        case QAST_F64:
        case QAST_F128:
        case QAST_VOID:
          return true;
        default:
          return false;
      }
    }
    constexpr bool is_array() const { return getKind() == QAST_ARRAY; };
    constexpr bool is_tuple() const { return getKind() == QAST_TUPLE; }
    constexpr bool is_pointer() const { return getKind() == QAST_PTR; }
    constexpr bool is_function() const { return getKind() == QAST_FUNCTOR; }
    constexpr bool is_composite() const { return is_array() || is_tuple(); }
    constexpr bool is_numeric() const {
      return getKind() >= QAST_U1 && getKind() <= QAST_F128;
    }
    constexpr bool is_integral() const {
      return getKind() >= QAST_U1 && getKind() <= QAST_I128;
    }
    constexpr bool is_floating_point() const {
      return getKind() >= QAST_F16 && getKind() <= QAST_F128;
    }
    constexpr bool is_signed() const {
      return getKind() >= QAST_I8 && getKind() <= QAST_I128;
    }
    constexpr bool is_unsigned() const {
      return getKind() >= QAST_U1 && getKind() <= QAST_U128;
    }
    constexpr bool is_void() const { return getKind() == QAST_VOID; }
    constexpr bool is_bool() const { return getKind() == QAST_U1; }
    constexpr bool is_ref() const { return getKind() == QAST_REF; }
    bool is_ptr_to(Type *type) const;

    constexpr let get_width() const { return m_width; }
    constexpr let get_range() const { return m_range; }

    constexpr void set_range(Expr *start, Expr *end) { m_range = {start, end}; }
    constexpr void set_width(Expr *width) { m_width = width; }
  };

  class Expr : public npar_node_t {
  public:
    constexpr Expr(npar_ty_t ty) : npar_node_t(ty) {}

    constexpr bool is_stmt_expr(npar_ty_t type) const;
  };
}  // namespace npar

#endif
