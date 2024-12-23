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

#include <array>
#include <iostream>
#include <nitrate-core/Logger.hh>
#include <nitrate-core/Macro.hh>
#include <nitrate-lexer/Lexer.hh>
#include <nitrate-lexer/Token.hh>
#include <nitrate-parser/ASTCommon.hh>
#include <nitrate-parser/ASTData.hh>
#include <nitrate-parser/ASTWriter.hh>
#include <type_traits>

namespace ncc::parse {
  class npar_pack Base {
  private:
    npar_ty_t m_node_type : 7;
    bool m_mock : 1;
    lex::LocationID m_begin, m_end;

  public:
    constexpr Base(npar_ty_t ty, bool mock = false,
                   lex::LocationID begin = lex::LocationID(),
                   lex::LocationID end = lex::LocationID())
        : m_node_type(ty), m_mock(mock), m_begin(begin), m_end(end) {}

    ///======================================================================
    /// Efficient LLVM-Style reflection

    static constexpr uint32_t getKindSize(npar_ty_t kind);
    static constexpr std::string_view getKindName(npar_ty_t kind);

    template <typename T>
    static constexpr npar_ty_t getTypeCode() {
      using namespace ncc::parse;

      if constexpr (std::is_same_v<T, Base>) {
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
    constexpr bool is() const {
      return Base::getTypeCode<T>() == getKind();
    }

    constexpr bool is(npar_ty_t type) const { return type == getKind(); }
    constexpr bool is_mock() const { return m_mock; }

    bool isSame(const Base *o) const;

    uint64_t hash64() const;

    ///======================================================================
    /// Visitation

    constexpr void accept(ASTVisitor &v) const {
      v.dispatch(MakeFlowPtr(const_cast<Base *>(this)));
    }

    ///======================================================================
    /// Debug-mode checked type casting

    template <typename T>
    static constexpr T *safeCastAs(Base *ptr) {
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

    template <typename T>
    constexpr T *as() {
      return safeCastAs<T>(this);
    }

    template <typename T>
    constexpr const T *as() const {
      return safeCastAs<T>(const_cast<Base *>(this));
    }

    Base *&as_base() { return *reinterpret_cast<Base **>(this); }

    ///======================================================================
    /// Debugging

    std::ostream &dump(std::ostream &os = std::cerr,
                       WriterSourceProvider rd = std::nullopt) const;

    ///======================================================================
    /// Source location information

    constexpr lex::LocationID begin() const { return m_begin; }
    constexpr lex::Location begin(lex::IScanner &rd) const {
      return m_begin.Get(rd);
    }

    constexpr lex::LocationID end() const { return m_end; }
    constexpr lex::Location end(lex::IScanner &rd) const {
      return m_end.Get(rd);
    }

    constexpr std::tuple<uint32_t, uint32_t> get_pos() const {
      /// TODO: Implement this
      return {lex::QLEX_EOFF, lex::QLEX_NOFILE};
    }

    ///======================================================================
    /// Setters

    /**
     * @warning This function modifies the object's state.
     */
    constexpr void set_offset(lex::LocationID pos) { m_begin = pos; }
  } __attribute__((packed));

  static_assert(sizeof(Base) == 9);

  ///======================================================================

  constexpr std::string_view Base::getKindName(npar_ty_t type) {
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

  class Stmt : public Base {
  public:
    constexpr Stmt(npar_ty_t ty) : Base(ty){};

    constexpr bool is_expr_stmt(npar_ty_t type) const;
  };

  class npar_pack Type : public Base {
    std::optional<FlowPtr<Expr>> m_range_begin, m_range_end, m_width;

  public:
    constexpr Type(npar_ty_t ty) : Base(ty) {}

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
    constexpr let get_range_begin() const { return m_range_begin; }
    constexpr let get_range_end() const { return m_range_end; }

    constexpr void set_range_begin(std::optional<FlowPtr<Expr>> start) {
      m_range_begin = start;
    }

    constexpr void set_range_end(std::optional<FlowPtr<Expr>> end) {
      m_range_end = end;
    }

    constexpr void set_width(std::optional<FlowPtr<Expr>> width) {
      m_width = width;
    }
  };

  class Expr : public Base {
  public:
    constexpr Expr(npar_ty_t ty) : Base(ty) {}

    constexpr bool is_stmt_expr(npar_ty_t type) const;
  };
}  // namespace ncc::parse

#endif
