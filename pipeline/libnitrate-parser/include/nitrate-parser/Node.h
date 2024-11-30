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
#include <nitrate-core/Memory.h>
#include <nitrate-lexer/Token.h>

#include <cassert>
#include <iostream>
#include <map>
#include <nitrate-core/Classes.hh>
#include <optional>
#include <ostream>
#include <set>
#include <sstream>
#include <string>
#include <tuple>
#include <variant>
#include <vector>

typedef struct qparse_node_t qparse_node_t;

typedef enum qparse_ty_t {
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
  QAST_NODE_STRUCT_TY,
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
  QAST_NODE_SUBSYSTEM,
  QAST_NODE_EXPORT,
  QAST_NODE_STRUCT_FIELD,

  QAST_NODE__DECL_FIRST = QAST_NODE_TYPEDEF,
  QAST_NODE__DECL_LAST = QAST_NODE_STRUCT_FIELD,

  QAST_NODE_BLOCK,
  QAST_NODE_CONST,
  QAST_NODE_VAR,
  QAST_NODE_LET,
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
  QAST_NODE_VOLATILE,

  QAST_NODE__STMT_FIRST = QAST_NODE__DECL_FIRST,
  QAST_NODE__STMT_LAST = QAST_NODE_VOLATILE,

  QAST_NODE__FIRST = QAST_NODE_NODE,
  QAST_NODE__LAST = QAST_NODE_VOLATILE,
} qparse_ty_t;

#define QAST_NODE_COUNT (QAST_NODE__LAST - QAST_NODE__FIRST + 1)

namespace qparse {
  class ArenaAllocatorImpl {
    qcore_arena m_arena;

  public:
    ArenaAllocatorImpl() = default;

    void *allocate(std::size_t bytes);
    void deallocate(void *ptr) noexcept;

    void swap(qcore_arena_t &arena);

    qcore_arena_t &get() { return *m_arena.get(); }
  };

  extern thread_local ArenaAllocatorImpl qparse_arena;

  template <class T>
  struct Arena {
    typedef T value_type;

    Arena() = default;

    template <class U>
    constexpr Arena(const Arena<U> &) noexcept {}

    [[nodiscard]] T *allocate(std::size_t n) {
      return static_cast<T *>(qparse_arena.allocate(sizeof(T) * n));
    }

    void deallocate(T *p, std::size_t n) noexcept {
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
};  // namespace qparse

#define PNODE_IMPL_CORE(__typename)                           \
public:                                                       \
public:                                                       \
  template <typename T = __typename, typename... Args>        \
  static __typename *get(Args &&...args) {                    \
    void *ptr = Arena<__typename>().allocate(1);              \
    return new (ptr) __typename(std::forward<Args>(args)...); \
  }                                                           \
                                                              \
public:

struct qparse_node_t {};

namespace qparse {
  class Node;
  class Stmt;
  class Type;
  class Decl;
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
  class StructTy;
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
  class VolStmt;
  class ConstDecl;
  class VarDecl;
  class LetDecl;
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
  class TypedefDecl;
  class FnDecl;
  class FnDef;
  class StructField;
  class StructDef;
  class EnumDef;
  class SubsystemDecl;
  class ExportDecl;

}  // namespace qparse

namespace qparse {
  enum class Vis {
    PUBLIC,
    PRIVATE,
    PROTECTED,
  };

  class String
      : public std::basic_string<char, std::char_traits<char>, Arena<char>> {
  public:
    String() = default;
    String(const char *str)
        : std::basic_string<char, std::char_traits<char>, Arena<char>>(str) {}
    String(const std::string &str)
        : std::basic_string<char, std::char_traits<char>, Arena<char>>(
              str.c_str(), str.size()) {}

    std::string_view view() { return std::string_view(data(), size()); }
  };

  class Node : public qparse_node_t {
    qparse_ty_t m_node_type;
    uint32_t m_pos_start, m_pos_end;

  public:
    constexpr Node(qparse_ty_t ty)
        : m_node_type(ty), m_pos_start(0), m_pos_end(0){};

    ///======================================================================
    /* Efficient LLVM reflection */

    static constexpr uint32_t getKindSize(qparse_ty_t kind) noexcept;
    static constexpr std::string_view getKindName(qparse_ty_t kind) noexcept;

    template <typename T>
    static constexpr qparse_ty_t getTypeCode() noexcept {
      if constexpr (std::is_same_v<T, Node>) {
        return QAST_NODE_NODE;
      } else if constexpr (std::is_same_v<T, Decl>) {
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
      } else if constexpr (std::is_same_v<T, StructTy>) {
        return QAST_NODE_STRUCT_TY;
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
      } else if constexpr (std::is_same_v<T, TypedefDecl>) {
        return QAST_NODE_TYPEDEF;
      } else if constexpr (std::is_same_v<T, FnDecl>) {
        return QAST_NODE_FNDECL;
      } else if constexpr (std::is_same_v<T, StructDef>) {
        return QAST_NODE_STRUCT;
      } else if constexpr (std::is_same_v<T, EnumDef>) {
        return QAST_NODE_ENUM;
      } else if constexpr (std::is_same_v<T, FnDef>) {
        return QAST_NODE_FN;
      } else if constexpr (std::is_same_v<T, SubsystemDecl>) {
        return QAST_NODE_SUBSYSTEM;
      } else if constexpr (std::is_same_v<T, ExportDecl>) {
        return QAST_NODE_EXPORT;
      } else if constexpr (std::is_same_v<T, StructField>) {
        return QAST_NODE_STRUCT_FIELD;
      } else if constexpr (std::is_same_v<T, Block>) {
        return QAST_NODE_BLOCK;
      } else if constexpr (std::is_same_v<T, ConstDecl>) {
        return QAST_NODE_CONST;
      } else if constexpr (std::is_same_v<T, VarDecl>) {
        return QAST_NODE_VAR;
      } else if constexpr (std::is_same_v<T, LetDecl>) {
        return QAST_NODE_LET;
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
      } else if constexpr (std::is_same_v<T, VolStmt>) {
        return QAST_NODE_VOLATILE;
      }
    }

    constexpr qparse_ty_t getKind() const noexcept { return m_node_type; }
    constexpr std::string_view getKindName() const noexcept {
      return getKindName(m_node_type);
    }

    ///======================================================================

    constexpr bool is_type() const noexcept {
      auto kind = getKind();
      return kind >= QAST_NODE__TYPE_FIRST && kind <= QAST_NODE__TYPE_LAST;
    }

    constexpr bool is_stmt() const noexcept {
      auto kind = getKind();
      return kind >= QAST_NODE__STMT_FIRST && kind <= QAST_NODE__STMT_LAST;
    }

    constexpr bool is_decl() const noexcept {
      auto kind = getKind();
      return kind >= QAST_NODE__DECL_FIRST && kind <= QAST_NODE__DECL_LAST;
    }

    constexpr bool is_expr() const noexcept {
      auto kind = getKind();
      return kind >= QAST_NODE__EXPR_FIRST && kind <= QAST_NODE__EXPR_LAST;
    }

    template <typename T>
    static constexpr T *safeCastAs(Node *ptr) noexcept {
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
    constexpr T *as() noexcept {
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
    constexpr const T *as() const noexcept {
      return safeCastAs<T>(const_cast<Node *>(this));
    }

    template <typename T>
    constexpr bool is() const noexcept {
      return Node::getTypeCode<T>() == getKind();
    }

    constexpr bool is(qparse_ty_t type) const { return type == getKind(); }

    std::ostream &dump(std::ostream &os = std::cerr,
                       bool isForDebug = false) const noexcept;

    std::string to_string() {
      std::stringstream ss;
      dump(ss, false);
      return ss.str();
    };

    constexpr void set_start_pos(uint32_t pos) { m_pos_start = pos; }
    constexpr Node *set_end_pos(uint32_t pos) {
      m_pos_end = pos;
      return this;
    }
    constexpr Node *set_pos(
        std::tuple<uint32_t, uint32_t, std::string_view> pos) {
      m_pos_start = std::get<0>(pos);
      m_pos_end = std::get<1>(pos);

      /// FIXME: Use the filename info
      return this;
    }

    constexpr uint32_t get_end_pos() { return m_pos_end; }
    constexpr uint32_t get_start_pos() { return m_pos_start; }
    constexpr std::tuple<uint32_t, uint32_t, std::string_view> get_pos() {
      return {m_pos_start, m_pos_end, ""};
    }

    PNODE_IMPL_CORE(Node)
  } __attribute__((packed));

  constexpr size_t PNODE_BASE_SIZE = sizeof(Node);

  class Stmt : public Node {
  public:
    constexpr Stmt(qparse_ty_t ty) : Node(ty){};
  };

  class Expr;

  class Type : public Node {
    Expr *m_width, *m_range_start, *m_range_end;
    bool m_volatile;

  public:
    constexpr Type(qparse_ty_t ty, bool is_volatile = false)
        : Node(ty),
          m_width(nullptr),
          m_range_start(nullptr),
          m_range_end(nullptr),
          m_volatile(is_volatile) {}

    constexpr bool is_primitive() const noexcept {
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
    constexpr bool is_tuple() const noexcept {
      return getKind() == QAST_NODE_TUPLE_TY;
    }
    constexpr bool is_pointer() const noexcept {
      return getKind() == QAST_NODE_PTR_TY;
    }
    constexpr bool is_function() const noexcept {
      return getKind() == QAST_NODE_FN_TY;
    }
    constexpr bool is_composite() const noexcept {
      return getKind() == QAST_NODE_STRUCT_TY || is_array() || is_tuple();
    }
    constexpr bool is_numeric() const noexcept {
      return getKind() >= QAST_NODE_U1_TY && getKind() <= QAST_NODE_F128_TY;
    }
    constexpr bool is_integral() const noexcept {
      return getKind() >= QAST_NODE_U1_TY && getKind() <= QAST_NODE_I128_TY;
    }
    constexpr bool is_floating_point() const noexcept {
      return getKind() >= QAST_NODE_F16_TY && getKind() <= QAST_NODE_F128_TY;
    }
    constexpr bool is_signed() const noexcept {
      return getKind() >= QAST_NODE_I8_TY && getKind() <= QAST_NODE_I128_TY;
    }
    constexpr bool is_unsigned() const noexcept {
      return getKind() >= QAST_NODE_U1_TY && getKind() <= QAST_NODE_U128_TY;
    }
    constexpr bool is_void() const noexcept {
      return getKind() == QAST_NODE_VOID_TY;
    }
    constexpr bool is_bool() const noexcept {
      return getKind() == QAST_NODE_U1_TY;
    }
    constexpr bool is_ref() const noexcept {
      return getKind() == QAST_NODE_REF_TY;
    }
    constexpr bool is_volatile() const noexcept { return m_volatile; }
    bool is_ptr_to(Type *type) noexcept;

    constexpr Expr *get_width() { return m_width; }
    constexpr void set_width(Expr *width) { m_width = width; }

    constexpr std::pair<Expr *, Expr *> get_range() {
      return {m_range_start, m_range_end};
    }
    constexpr void set_range(Expr *start, Expr *end) {
      m_range_start = start;
      m_range_end = end;
    }
  };

  typedef std::set<Expr *, std::less<Expr *>, Arena<Expr *>> DeclTags;

  typedef std::tuple<String, Type *, Expr *> TemplateParameter;
  typedef std::vector<TemplateParameter, Arena<TemplateParameter>>
      TemplateParameters;

  class Decl : public Stmt {
  protected:
    DeclTags m_tags;
    std::optional<TemplateParameters> m_template_parameters;
    String m_name;
    Type *m_type;
    Vis m_visibility;

  public:
    Decl(qparse_ty_t ty, String name = "", Type *type = nullptr,
         DeclTags tags = {},
         const std::optional<TemplateParameters> &params = std::nullopt,
         Vis visibility = Vis::PRIVATE)
        : Stmt(ty),
          m_tags(tags),
          m_template_parameters(params),
          m_name(name),
          m_type(type),
          m_visibility(visibility) {}

    String get_name() { return m_name; }
    void set_name(String name) { m_name = name; }

    Type *get_type() { return m_type; }
    void set_type(Type *type) { m_type = type; }

    DeclTags &get_tags() { return m_tags; }
    void set_tags(DeclTags t) { m_tags = t; }

    auto &get_template_params() { return m_template_parameters; }
    void set_template_params(std::optional<TemplateParameters> x) {
      m_template_parameters = x;
    }

    Vis get_visibility() { return m_visibility; }
    void set_visibility(Vis visibility) { m_visibility = visibility; }
  };

  class Expr : public Node {
  public:
    constexpr Expr(qparse_ty_t ty) : Node(ty) {}

    constexpr bool is_binexpr() const noexcept {
      return getKind() == QAST_NODE_BINEXPR;
    }
    constexpr bool is_unaryexpr() const noexcept {
      return getKind() == QAST_NODE_UNEXPR;
    }
    constexpr bool is_ternaryexpr() const noexcept {
      return getKind() == QAST_NODE_TEREXPR;
    }
  };

  class ExprStmt : public Stmt {
    Expr *m_expr;

  public:
    constexpr ExprStmt(Expr *expr = nullptr)
        : Stmt(QAST_NODE_EXPR_STMT), m_expr(expr) {}

    Expr *get_expr() { return m_expr; }
    void set_expr(Expr *expr) { m_expr = expr; }

    PNODE_IMPL_CORE(ExprStmt)
  };

  class StmtExpr : public Expr {
    Stmt *m_stmt;

  public:
    constexpr StmtExpr(Stmt *stmt = nullptr)
        : Expr(QAST_NODE_STMT_EXPR), m_stmt(stmt) {}

    Stmt *get_stmt() { return m_stmt; }
    void set_stmt(Stmt *stmt) { m_stmt = stmt; }

    PNODE_IMPL_CORE(StmtExpr)
  };

  class TypeExpr : public Expr {
    Type *m_type;

  public:
    constexpr TypeExpr(Type *type = nullptr)
        : Expr(QAST_NODE_TYPE_EXPR), m_type(type) {}

    Type *get_type() { return m_type; }
    void set_type(Type *type) { m_type = type; }

    PNODE_IMPL_CORE(TypeExpr)
  };

  class NamedTy : public Type {
    String m_name;

  public:
    NamedTy(String name = "") : Type(QAST_NODE_UNRES_TY), m_name(name) {}

    String get_name() { return m_name; }
    void set_name(String name) { m_name = name; }

    PNODE_IMPL_CORE(NamedTy)
  };

  class InferTy : public Type {
  public:
    constexpr InferTy() : Type(QAST_NODE_INFER_TY) {}

    PNODE_IMPL_CORE(InferTy)
  };

  typedef std::vector<Expr *, Arena<Expr *>> TemplTypeArgs;
  class TemplType : public Type {
    Type *m_template;
    TemplTypeArgs m_args;

  public:
    TemplType(Type *templ, const TemplTypeArgs &args)
        : Type(QAST_NODE_TEMPL_TY), m_template(templ), m_args(args) {}

    Type *get_template() { return m_template; }
    void set_template(Type *templ) { m_template = templ; }

    TemplTypeArgs &get_args() { return m_args; }

    PNODE_IMPL_CORE(TemplType)
  };

  class U1 : public Type {
  public:
    constexpr U1() : Type(QAST_NODE_U1_TY){};

    PNODE_IMPL_CORE(U1)
  };

  class U8 : public Type {
  public:
    constexpr U8() : Type(QAST_NODE_U8_TY){};

    PNODE_IMPL_CORE(U8)
  };

  class U16 : public Type {
  public:
    constexpr U16() : Type(QAST_NODE_U16_TY){};

    PNODE_IMPL_CORE(U16)
  };

  class U32 : public Type {
  public:
    constexpr U32() : Type(QAST_NODE_U32_TY){};

    PNODE_IMPL_CORE(U32)
  };

  class U64 : public Type {
  public:
    constexpr U64() : Type(QAST_NODE_U64_TY){};

    PNODE_IMPL_CORE(U64)
  };

  class U128 : public Type {
  public:
    constexpr U128() : Type(QAST_NODE_U128_TY){};

    PNODE_IMPL_CORE(U128)
  };

  class I8 : public Type {
  public:
    constexpr I8() : Type(QAST_NODE_I8_TY){};

    PNODE_IMPL_CORE(I8)
  };

  class I16 : public Type {
  public:
    constexpr I16() : Type(QAST_NODE_I16_TY){};

    PNODE_IMPL_CORE(I16)
  };

  class I32 : public Type {
  public:
    constexpr I32() : Type(QAST_NODE_I32_TY){};

    PNODE_IMPL_CORE(I32)
  };

  class I64 : public Type {
  public:
    constexpr I64() : Type(QAST_NODE_I64_TY){};

    PNODE_IMPL_CORE(I64)
  };

  class I128 : public Type {
  public:
    constexpr I128() : Type(QAST_NODE_I128_TY){};

    PNODE_IMPL_CORE(I128)
  };

  class F16 : public Type {
  public:
    constexpr F16() : Type(QAST_NODE_F16_TY){};

    PNODE_IMPL_CORE(F16)
  };

  class F32 : public Type {
  public:
    constexpr F32() : Type(QAST_NODE_F32_TY){};

    PNODE_IMPL_CORE(F32)
  };

  class F64 : public Type {
  public:
    constexpr F64() : Type(QAST_NODE_F64_TY){};

    PNODE_IMPL_CORE(F64)
  };

  class F128 : public Type {
  public:
    constexpr F128() : Type(QAST_NODE_F128_TY){};

    PNODE_IMPL_CORE(F128)
  };

  class VoidTy : public Type {
  public:
    constexpr VoidTy() : Type(QAST_NODE_VOID_TY) {}

    PNODE_IMPL_CORE(VoidTy)
  };

  class PtrTy : public Type {
    Type *m_item;
    bool m_is_volatile;

  public:
    constexpr PtrTy(Type *item = nullptr, bool is_volatile = false)
        : Type(QAST_NODE_PTR_TY), m_item(item), m_is_volatile(is_volatile) {}

    Type *get_item() { return m_item; }
    void set_item(Type *item) { m_item = item; }

    bool is_volatile() { return m_is_volatile; }
    void set_volatile(bool is_volatile) { m_is_volatile = is_volatile; }

    PNODE_IMPL_CORE(PtrTy)
  };

  class OpaqueTy : public Type {
    String m_name;

  public:
    OpaqueTy(String name = "") : Type(QAST_NODE_OPAQUE_TY), m_name(name) {}

    String get_name() { return m_name; }
    void set_name(String name) { m_name = name; }

    PNODE_IMPL_CORE(OpaqueTy)
  };

  typedef std::vector<Type *, Arena<Type *>> TupleTyItems;
  class TupleTy : public Type {
    TupleTyItems m_items;

  public:
    TupleTy(const TupleTyItems &items)
        : Type(QAST_NODE_TUPLE_TY), m_items(items) {}

    TupleTyItems &get_items() { return m_items; }

    PNODE_IMPL_CORE(TupleTy)
  };

  class ArrayTy : public Type {
    Type *m_item;
    Expr *m_size;

  public:
    constexpr ArrayTy(Type *item = nullptr, Expr *size = nullptr)
        : Type(QAST_NODE_ARRAY_TY), m_item(item), m_size(size) {}

    Type *get_item() { return m_item; }
    void set_item(Type *item) { m_item = item; }

    Expr *get_size() { return m_size; }
    void set_size(Expr *size) { m_size = size; }

    PNODE_IMPL_CORE(ArrayTy)
  };

  class RefTy : public Type {
    Type *m_item;

  public:
    constexpr RefTy(Type *item = nullptr)
        : Type(QAST_NODE_REF_TY), m_item(item) {}

    Type *get_item() { return m_item; }
    void set_item(Type *item) { m_item = item; }

    PNODE_IMPL_CORE(RefTy)
  };

  typedef std::pair<String, Type *> StructItem;
  typedef std::vector<StructItem, Arena<StructItem>> StructItems;

  class StructTy : public Type {
    StructItems m_items;

  public:
    StructTy(const StructItems &items)
        : Type(QAST_NODE_STRUCT_TY), m_items(items) {}

    StructItems &get_items() { return m_items; }

    PNODE_IMPL_CORE(StructTy)
  };

  enum class FuncPurity {
    IMPURE_THREAD_UNSAFE,
    IMPURE_THREAD_SAFE,
    PURE,
    QUASIPURE,
    RETROPURE,
  };

  typedef std::tuple<String, Type *, Expr *> FuncParam;
  typedef std::vector<FuncParam, Arena<FuncParam>> FuncParams;

  class FuncTy : public Type {
    FuncParams m_params;
    Type *m_return;
    FuncPurity m_purity;
    bool m_variadic;
    bool m_is_foreign;
    bool m_crashpoint;
    bool m_noexcept;
    bool m_noreturn;

  public:
    FuncTy()
        : Type(QAST_NODE_FN_TY),
          m_return(nullptr),
          m_purity(FuncPurity::IMPURE_THREAD_UNSAFE),
          m_variadic(false),
          m_is_foreign(false),
          m_crashpoint(false),
          m_noexcept(false),
          m_noreturn(false) {}

    FuncTy(Type *return_type, FuncParams parameters, bool variadic = false,
           FuncPurity purity = FuncPurity::IMPURE_THREAD_UNSAFE,
           bool is_foreign = false, bool crashpoint = false,
           bool noexcept_ = false, bool noreturn = false)
        : Type(QAST_NODE_FN_TY),
          m_params(parameters),
          m_return(return_type),
          m_purity(purity),
          m_variadic(variadic),
          m_is_foreign(is_foreign),
          m_crashpoint(crashpoint),
          m_noexcept(noexcept_),
          m_noreturn(noreturn) {
      assert(!noreturn || (purity == FuncPurity::IMPURE_THREAD_UNSAFE ||
                           purity == FuncPurity::IMPURE_THREAD_SAFE));
    }
    FuncTy(Type *return_type, std::vector<Type *, Arena<Type *>> parameters,
           bool variadic = false,
           FuncPurity purity = FuncPurity::IMPURE_THREAD_UNSAFE,
           bool is_foreign = false, bool crashpoint = false,
           bool noexcept_ = false, bool noreturn = false)
        : Type(QAST_NODE_FN_TY),
          m_return(return_type),
          m_purity(purity),
          m_variadic(variadic),
          m_is_foreign(is_foreign),
          m_crashpoint(crashpoint),
          m_noexcept(noexcept_),
          m_noreturn(noreturn) {
      assert(!noreturn || (purity == FuncPurity::IMPURE_THREAD_UNSAFE ||
                           purity == FuncPurity::IMPURE_THREAD_SAFE));

      for (size_t i = 0; i < parameters.size(); i++) {
        m_params.push_back(
            FuncParam("_" + std::to_string(i), parameters[i], nullptr));
      }
    }

    bool is_noreturn() { return m_noreturn; }
    void set_noreturn(bool noreturn);

    Type *get_return_ty() { return m_return; }
    void set_return_ty(Type *return_ty) { m_return = return_ty; }

    FuncParams &get_params() { return m_params; }

    FuncPurity get_purity() { return m_purity; }
    void set_purity(FuncPurity purity) { m_purity = purity; }

    bool is_variadic() { return m_variadic; }
    void set_variadic(bool variadic) { m_variadic = variadic; }

    bool is_foreign() { return m_is_foreign; }
    void set_foreign(bool is_foreign) { m_is_foreign = is_foreign; }

    bool is_crashpoint() { return m_crashpoint; }
    void set_crashpoint(bool crashpoint) { m_crashpoint = crashpoint; }

    bool is_noexcept() { return m_noexcept; }
    void set_noexcept(bool noexcept_) { m_noexcept = noexcept_; }

    PNODE_IMPL_CORE(FuncTy)
  };

  ///=============================================================================

  class UnaryExpr : public Expr {
    Expr *m_rhs;
    qlex_op_t m_op;

  public:
    constexpr UnaryExpr(qlex_op_t op = qOpTernary, Expr *rhs = nullptr)
        : Expr(QAST_NODE_UNEXPR), m_rhs(rhs), m_op(op) {}

    Expr *get_rhs() { return m_rhs; }
    void set_rhs(Expr *rhs) { m_rhs = rhs; }

    qlex_op_t get_op() { return m_op; }
    void set_op(qlex_op_t op) { m_op = op; }

    PNODE_IMPL_CORE(UnaryExpr)
  };

  class BinExpr : public Expr {
    Expr *m_lhs;
    Expr *m_rhs;
    qlex_op_t m_op;

  public:
    constexpr BinExpr(Expr *lhs = nullptr, qlex_op_t op = qOpTernary,
                      Expr *rhs = nullptr)
        : Expr(QAST_NODE_BINEXPR), m_lhs(lhs), m_rhs(rhs), m_op(op) {}

    Expr *get_lhs() { return m_lhs; }
    void set_lhs(Expr *lhs) { m_lhs = lhs; }

    Expr *get_rhs() { return m_rhs; }
    void set_rhs(Expr *rhs) { m_rhs = rhs; }

    qlex_op_t get_op() { return m_op; }
    void set_op(qlex_op_t op) { m_op = op; }

    PNODE_IMPL_CORE(BinExpr)
  };

  class PostUnaryExpr : public Expr {
    Expr *m_lhs;
    qlex_op_t m_op;

  public:
    constexpr PostUnaryExpr(Expr *lhs = nullptr, qlex_op_t op = qOpTernary)
        : Expr(QAST_NODE_POST_UNEXPR), m_lhs(lhs), m_op(op) {}

    Expr *get_lhs() { return m_lhs; }
    void set_lhs(Expr *lhs) { m_lhs = lhs; }

    qlex_op_t get_op() { return m_op; }
    void set_op(qlex_op_t op) { m_op = op; }

    PNODE_IMPL_CORE(PostUnaryExpr)
  };

  class TernaryExpr : public Expr {
    Expr *m_cond;
    Expr *m_lhs;
    Expr *m_rhs;

  public:
    constexpr TernaryExpr(Expr *cond = nullptr, Expr *lhs = nullptr,
                          Expr *rhs = nullptr)
        : Expr(QAST_NODE_TEREXPR), m_cond(cond), m_lhs(lhs), m_rhs(rhs) {}

    Expr *get_cond() { return m_cond; }
    void set_cond(Expr *cond) { m_cond = cond; }

    Expr *get_lhs() { return m_lhs; }
    void set_lhs(Expr *lhs) { m_lhs = lhs; }

    Expr *get_rhs() { return m_rhs; }
    void set_rhs(Expr *rhs) { m_rhs = rhs; }

    PNODE_IMPL_CORE(TernaryExpr)
  };

  ///=============================================================================

  class ConstInt : public Expr {
    String m_value;

  public:
    ConstInt(String value = "") : Expr(QAST_NODE_INT), m_value(value) {}

    ConstInt(uint64_t value)
        : Expr(QAST_NODE_INT), m_value(std::to_string(value)) {}

    String get_value() { return m_value; }

    PNODE_IMPL_CORE(ConstInt)
  };

  class ConstFloat : public Expr {
    String m_value;

  public:
    ConstFloat(String value = "") : Expr(QAST_NODE_FLOAT), m_value(value) {}
    ConstFloat(double value)
        : Expr(QAST_NODE_FLOAT), m_value(std::to_string(value)) {}

    String get_value() { return m_value; }

    PNODE_IMPL_CORE(ConstFloat)
  };

  class ConstBool : public Expr {
    bool m_value;

  public:
    constexpr ConstBool(bool value = false)
        : Expr(QAST_NODE_BOOL), m_value(value) {}

    bool get_value() { return m_value; }

    PNODE_IMPL_CORE(ConstBool)
  };

  class ConstString : public Expr {
    String m_value;

  public:
    ConstString(String value = "") : Expr(QAST_NODE_STRING), m_value(value) {}

    String get_value() { return m_value; }

    PNODE_IMPL_CORE(ConstString)
  };

  class ConstChar : public Expr {
    uint8_t m_value;

  public:
    constexpr ConstChar(uint8_t value = 0)
        : Expr(QAST_NODE_CHAR), m_value(value) {}

    uint8_t get_value() { return m_value; }

    PNODE_IMPL_CORE(ConstChar)
  };

  class ConstNull : public Expr {
  public:
    constexpr ConstNull() : Expr(QAST_NODE_NULL) {}

    PNODE_IMPL_CORE(ConstNull)
  };

  class ConstUndef : public Expr {
  public:
    constexpr ConstUndef() : Expr(QAST_NODE_UNDEF) {}

    PNODE_IMPL_CORE(ConstUndef)
  };

  ///=============================================================================

  typedef std::pair<String, Expr *> CallArg;
  typedef std::vector<CallArg, Arena<CallArg>> CallArgs;

  class Call : public Expr {
    Expr *m_func;
    CallArgs m_args;

  public:
    Call(Expr *func = nullptr, CallArgs args = {})
        : Expr(QAST_NODE_CALL), m_func(func), m_args(args) {}

    Expr *get_func() { return m_func; }
    void set_func(Expr *func) { m_func = func; }

    CallArgs &get_args() { return m_args; }

    PNODE_IMPL_CORE(Call)
  };

  typedef std::map<String, Expr *, std::less<String>,
                   Arena<std::pair<const String, Expr *>>>
      TemplateArgs;

  class TemplCall : public Expr {
    TemplateArgs m_template_args;
    Expr *m_func;
    CallArgs m_args;

  public:
    TemplCall(Expr *func = nullptr, CallArgs args = {},
              TemplateArgs template_args = {})
        : Expr(QAST_NODE_TEMPL_CALL),
          m_template_args(template_args),
          m_func(func),
          m_args(args) {}

    Expr *get_func() { return m_func; }
    void set_func(Expr *func) { m_func = func; }

    TemplateArgs &get_template_args() { return m_template_args; }
    CallArgs &get_args() { return m_args; }

    PNODE_IMPL_CORE(TemplCall)
  };

  typedef std::vector<Expr *, Arena<Expr *>> ListData;

  class List : public Expr {
    ListData m_items;

  public:
    List(const ListData &items) : Expr(QAST_NODE_LIST), m_items(items) {}

    ListData &get_items() { return m_items; }

    PNODE_IMPL_CORE(List)
  };

  class Assoc : public Expr {
    Expr *m_key;
    Expr *m_value;

  public:
    constexpr Assoc(Expr *key = nullptr, Expr *value = nullptr)
        : Expr(QAST_NODE_ASSOC), m_key(key), m_value(value) {}

    Expr *get_key() { return m_key; }
    void set_key(Expr *key) { m_key = key; }

    Expr *get_value() { return m_value; }
    void set_value(Expr *value) { m_value = value; }

    PNODE_IMPL_CORE(Assoc)
  };

  class Field : public Expr {
    Expr *m_base;
    String m_field;

  public:
    Field(Expr *base = nullptr, String field = "")
        : Expr(QAST_NODE_FIELD), m_base(base), m_field(field) {}

    Expr *get_base() { return m_base; }
    void set_base(Expr *base) { m_base = base; }

    String get_field() { return m_field; }
    void set_field(String field) { m_field = field; }

    PNODE_IMPL_CORE(Field)
  };

  class Index : public Expr {
    Expr *m_base;
    Expr *m_index;

  public:
    constexpr Index(Expr *base = nullptr, Expr *index = nullptr)
        : Expr(QAST_NODE_INDEX), m_base(base), m_index(index) {}

    Expr *get_base() { return m_base; }
    void set_base(Expr *base) { m_base = base; }

    Expr *get_index() { return m_index; }
    void set_index(Expr *index) { m_index = index; }

    PNODE_IMPL_CORE(Index)
  };

  class Slice : public Expr {
    Expr *m_base;
    Expr *m_start;
    Expr *m_end;

  public:
    constexpr Slice(Expr *base = nullptr, Expr *start = nullptr,
                    Expr *end = nullptr)
        : Expr(QAST_NODE_SLICE), m_base(base), m_start(start), m_end(end) {}

    Expr *get_base() { return m_base; }
    void set_base(Expr *base) { m_base = base; }

    Expr *get_start() { return m_start; }
    void set_start(Expr *start) { m_start = start; }

    Expr *get_end() { return m_end; }
    void set_end(Expr *end) { m_end = end; }

    PNODE_IMPL_CORE(Slice)
  };

  typedef std::vector<std::variant<String, Expr *>,
                      Arena<std::variant<String, Expr *>>>
      FStringItems;

  class FString : public Expr {
    FStringItems m_items;

  public:
    FString(FStringItems items = {})
        : Expr(QAST_NODE_FSTRING), m_items(items) {}

    FStringItems &get_items() { return m_items; }

    PNODE_IMPL_CORE(FString)
  };

  class Ident : public Expr {
    String m_name;

  public:
    Ident(String name = "") : Expr(QAST_NODE_IDENT), m_name(name) {}

    String get_name() { return m_name; }
    void set_name(String name) { m_name = name; }

    PNODE_IMPL_CORE(Ident)
  };

  typedef std::vector<Expr *, Arena<Expr *>> SeqPointItems;
  class SeqPoint : public Expr {
    SeqPointItems m_items;

  public:
    SeqPoint(const SeqPointItems &items)
        : Expr(QAST_NODE_SEQ), m_items(items) {}

    SeqPointItems &get_items() { return m_items; }

    PNODE_IMPL_CORE(SeqPoint)
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

    BlockItems &get_items() { return m_items; }

    SafetyMode get_safety() { return m_safety; }
    void set_safety(SafetyMode safety) { m_safety = safety; }

    PNODE_IMPL_CORE(Block)
  };

  class VolStmt : public Stmt {
    Stmt *m_stmt;

  public:
    constexpr VolStmt(Stmt *stmt = nullptr)
        : Stmt(QAST_NODE_VOLATILE), m_stmt(stmt) {}

    Stmt *get_stmt() { return m_stmt; }
    void set_stmt(Stmt *stmt) { m_stmt = stmt; }

    PNODE_IMPL_CORE(VolStmt)
  };

  class ConstDecl : public Decl {
    Expr *m_value;

  public:
    ConstDecl(String name = "", Type *type = nullptr, Expr *value = nullptr)
        : Decl(QAST_NODE_CONST, name, type), m_value(value) {}

    Expr *get_value() { return m_value; }
    void set_value(Expr *value) { m_value = value; }

    PNODE_IMPL_CORE(ConstDecl)
  };

  class VarDecl : public Decl {
    Expr *m_value;

  public:
    VarDecl(String name = "", Type *type = nullptr, Expr *value = nullptr)
        : Decl(QAST_NODE_VAR, name, type), m_value(value) {}

    Expr *get_value() { return m_value; }
    void set_value(Expr *value) { m_value = value; }

    PNODE_IMPL_CORE(VarDecl)
  };

  class LetDecl : public Decl {
    Expr *m_value;

  public:
    LetDecl(String name = "", Type *type = nullptr, Expr *value = nullptr)
        : Decl(QAST_NODE_LET, name, type), m_value(value) {}

    Expr *get_value() { return m_value; }
    void set_value(Expr *value) { m_value = value; }

    PNODE_IMPL_CORE(LetDecl)
  };

  typedef std::vector<Expr *, Arena<Expr *>> InlineAsmArgs;

  class InlineAsm : public Stmt {
    String m_code;
    InlineAsmArgs m_args;

  public:
    InlineAsm(String code, const InlineAsmArgs &args)
        : Stmt(QAST_NODE_INLINE_ASM), m_code(code), m_args(args) {}

    String get_code() { return m_code; }
    void set_code(String code) { m_code = code; }

    InlineAsmArgs &get_args() { return m_args; }

    PNODE_IMPL_CORE(InlineAsm)
  };

  class IfStmt : public Stmt {
    Expr *m_cond;
    Block *m_then;
    Block *m_else;

  public:
    constexpr IfStmt(Expr *cond = nullptr, Block *then = nullptr,
                     Block *else_ = nullptr)
        : Stmt(QAST_NODE_IF), m_cond(cond), m_then(then), m_else(else_) {}

    Expr *get_cond() { return m_cond; }
    void set_cond(Expr *cond) { m_cond = cond; }

    Block *get_then() { return m_then; }
    void set_then(Block *then) { m_then = then; }

    Block *get_else() { return m_else; }
    void set_else(Block *else_) { m_else = else_; }

    PNODE_IMPL_CORE(IfStmt)
  };

  class WhileStmt : public Stmt {
    Expr *m_cond;
    Block *m_body;

  public:
    constexpr WhileStmt(Expr *cond = nullptr, Block *body = nullptr)
        : Stmt(QAST_NODE_WHILE), m_cond(cond), m_body(body) {}

    Expr *get_cond() { return m_cond; }
    void set_cond(Expr *cond) { m_cond = cond; }

    Block *get_body() { return m_body; }
    void set_body(Block *body) { m_body = body; }

    PNODE_IMPL_CORE(WhileStmt)
  };

  class ForStmt : public Stmt {
    Expr *m_init;
    Expr *m_cond;
    Expr *m_step;
    Block *m_body;

  public:
    constexpr ForStmt(Expr *init = nullptr, Expr *cond = nullptr,
                      Expr *step = nullptr, Block *body = nullptr)
        : Stmt(QAST_NODE_FOR),
          m_init(init),
          m_cond(cond),
          m_step(step),
          m_body(body) {}

    Expr *get_init() { return m_init; }
    void set_init(Expr *init) { m_init = init; }

    Expr *get_cond() { return m_cond; }
    void set_cond(Expr *cond) { m_cond = cond; }

    Expr *get_step() { return m_step; }
    void set_step(Expr *step) { m_step = step; }

    Block *get_body() { return m_body; }
    void set_body(Block *body) { m_body = body; }

    PNODE_IMPL_CORE(ForStmt)
  };

  class ForeachStmt : public Stmt {
    String m_idx_ident;
    String m_val_ident;
    Expr *m_expr;
    Block *m_body;

  public:
    ForeachStmt(String idx_ident = "", String val_ident = "",
                Expr *expr = nullptr, Block *body = nullptr)
        : Stmt(QAST_NODE_FOREACH),
          m_idx_ident(idx_ident),
          m_val_ident(val_ident),
          m_expr(expr),
          m_body(body) {}

    String get_idx_ident() { return m_idx_ident; }
    void set_idx_ident(String idx_ident) { m_idx_ident = idx_ident; }

    String get_val_ident() { return m_val_ident; }
    void set_val_ident(String val_ident) { m_val_ident = val_ident; }

    Expr *get_expr() { return m_expr; }
    void set_expr(Expr *expr) { m_expr = expr; }

    Block *get_body() { return m_body; }
    void set_body(Block *body) { m_body = body; }

    PNODE_IMPL_CORE(ForeachStmt)
  };

  class BreakStmt : public Stmt {
  public:
    constexpr BreakStmt() : Stmt(QAST_NODE_BREAK){};

    PNODE_IMPL_CORE(BreakStmt)
  };

  class ContinueStmt : public Stmt {
  public:
    constexpr ContinueStmt() : Stmt(QAST_NODE_CONTINUE){};

    PNODE_IMPL_CORE(ContinueStmt)
  };

  class ReturnStmt : public Stmt {
    Expr *m_value;

  public:
    constexpr ReturnStmt(Expr *value = nullptr)
        : Stmt(QAST_NODE_RETURN), m_value(value) {}

    Expr *get_value() { return m_value; }
    void set_value(Expr *value) { m_value = value; }

    PNODE_IMPL_CORE(ReturnStmt)
  };

  class ReturnIfStmt : public Stmt {
    Expr *m_cond;
    Expr *m_value;

  public:
    constexpr ReturnIfStmt(Expr *cond = nullptr, Expr *value = nullptr)
        : Stmt(QAST_NODE_RETIF), m_cond(cond), m_value(value) {}

    Expr *get_cond() { return m_cond; }
    void set_cond(Expr *cond) { m_cond = cond; }

    Expr *get_value() { return m_value; }
    void set_value(Expr *value) { m_value = value; }

    PNODE_IMPL_CORE(ReturnIfStmt)
  };

  class CaseStmt : public Stmt {
    Expr *m_cond;
    Block *m_body;

  public:
    constexpr CaseStmt(Expr *cond = nullptr, Block *body = nullptr)
        : Stmt(QAST_NODE_CASE), m_cond(cond), m_body(body) {}

    Expr *get_cond() { return m_cond; }
    void set_cond(Expr *cond) { m_cond = cond; }

    Block *get_body() { return m_body; }
    void set_body(Block *body) { m_body = body; }

    PNODE_IMPL_CORE(CaseStmt)
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

    Expr *get_cond() { return m_cond; }
    void set_cond(Expr *cond) { m_cond = cond; }

    SwitchCases &get_cases() { return m_cases; }

    Stmt *get_default() { return m_default; }
    void set_default(Stmt *default_) { m_default = default_; }

    PNODE_IMPL_CORE(SwitchStmt)
  };

  ///=============================================================================

  class TypedefDecl : public Decl {
  public:
    TypedefDecl(String name = "", Type *type = nullptr)
        : Decl(QAST_NODE_TYPEDEF, name, type) {}

    PNODE_IMPL_CORE(TypedefDecl)
  };

  class FnDecl : public Decl {
  public:
    FnDecl(String name = "", FuncTy *type = nullptr)
        : Decl(QAST_NODE_FNDECL, name, type) {}

    FuncTy *get_type() { return static_cast<FuncTy *>(m_type); }

    PNODE_IMPL_CORE(FnDecl)
  };

  typedef std::vector<std::pair<String, bool>, Arena<std::pair<String, bool>>>
      FnCaptures;

  class FnDef : public Decl {
    FnCaptures m_captures;
    Block *m_body;
    Expr *m_precond;
    Expr *m_postcond;

  public:
    FnDef(FnDecl *decl = nullptr, Block *body = nullptr,
          Expr *precond = nullptr, Expr *postcond = nullptr,
          FnCaptures captures = {})
        : Decl(QAST_NODE_FN, decl->get_name(), decl->get_type()),
          m_captures(captures),
          m_body(body),
          m_precond(precond),
          m_postcond(postcond) {
      set_template_params(decl->get_template_params());
      set_visibility(decl->get_visibility());
      set_tags(decl->get_tags());
    }

    Block *get_body() { return m_body; }
    void set_body(Block *body) { m_body = body; }

    Expr *get_precond() { return m_precond; }
    void set_precond(Expr *precond) { m_precond = precond; }

    Expr *get_postcond() { return m_postcond; }
    void set_postcond(Expr *postcond) { m_postcond = postcond; }

    FnCaptures &get_captures() { return m_captures; }

    FuncTy *get_type() { return static_cast<FuncTy *>(m_type); }

    PNODE_IMPL_CORE(FnDef)
  };

  enum class CompositeType { Region, Struct, Group, Class, Union };

  class StructField : public Decl {
    Expr *m_value;

  public:
    StructField(String name = "", Type *type = nullptr, Expr *value = nullptr)
        : Decl(QAST_NODE_STRUCT_FIELD, name, type), m_value(value) {}

    Expr *get_value() { return m_value; }
    void set_value(Expr *value) { m_value = value; }

    PNODE_IMPL_CORE(StructField)
  };

  typedef std::vector<StructField *, Arena<StructField *>> StructDefFields;
  typedef std::vector<FnDecl *, Arena<FnDecl *>> StructDefMethods;
  typedef std::vector<FnDecl *, Arena<FnDecl *>> StructDefStaticMethods;

  class StructDef : public Decl {
    StructDefMethods m_methods;
    StructDefStaticMethods m_static_methods;
    StructDefFields m_fields;
    CompositeType m_comp_type;

  public:
    StructDef(String name = "", StructTy *type = nullptr,
              const StructDefFields &fields = {},
              const StructDefMethods &methods = {},
              const StructDefStaticMethods &static_methods = {})
        : Decl(QAST_NODE_STRUCT, name, type),
          m_methods(methods),
          m_static_methods(static_methods),
          m_fields(fields) {}

    StructTy *get_type() { return static_cast<StructTy *>(m_type); }

    StructDefMethods &get_methods() { return m_methods; }
    StructDefStaticMethods &get_static_methods() { return m_static_methods; }
    StructDefFields &get_fields() { return m_fields; }

    CompositeType get_composite_type() { return m_comp_type; }
    void set_composite_type(CompositeType t) { m_comp_type = t; }

    PNODE_IMPL_CORE(StructDef)
  };

  typedef std::pair<String, Expr *> EnumItem;
  typedef std::vector<EnumItem, Arena<EnumItem>> EnumDefItems;

  class EnumDef : public Decl {
    EnumDefItems m_items;

  public:
    EnumDef(String name, Type *type, const EnumDefItems &items)
        : Decl(QAST_NODE_ENUM, name, type), m_items(items) {}

    EnumDefItems &get_items() { return m_items; }

    PNODE_IMPL_CORE(EnumDef)
  };

  typedef std::set<String, std::less<String>, Arena<String>> SubsystemDeps;

  class SubsystemDecl : public Decl {
    Block *m_body;
    SubsystemDeps m_deps;

  public:
    SubsystemDecl(String name = "", Block *body = nullptr,
                  SubsystemDeps deps = {})
        : Decl(QAST_NODE_SUBSYSTEM, name, nullptr),
          m_body(body),
          m_deps(deps) {}

    Block *get_body() { return m_body; }
    void set_body(Block *body) { m_body = body; }

    SubsystemDeps &get_deps() { return m_deps; }

    PNODE_IMPL_CORE(SubsystemDecl)
  };

  class ExportDecl : public Decl {
    Block *m_body;
    String m_abi_name;

  public:
    ExportDecl(Block *content, String abi_name = "")
        : Decl(QAST_NODE_EXPORT, "", nullptr),
          m_body(content),
          m_abi_name(abi_name) {}

    Block *get_body() { return m_body; }
    void set_body(Block *body) { m_body = body; }

    String get_abi_name() { return m_abi_name; }
    void set_abi_name(String abi_name) { m_abi_name = abi_name; }

    PNODE_IMPL_CORE(ExportDecl)
  };

  template <typename T, typename... Args>
  static inline T *make(Args &&...args) {
    T *new_obj = new (Arena<T>().allocate(1)) T(std::forward<Args>(args)...);

    /// TODO: Cache nodes

    return new_obj;
  }

  ///=============================================================================

  constexpr std::string_view Node::getKindName(qparse_ty_t type) noexcept {
    const std::array<std::string_view, QAST_NODE_COUNT> names = []() {
      std::array<std::string_view, QAST_NODE_COUNT> R;
      R.fill("");

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
      R[QAST_NODE_TYPE_EXPR] = "peExpr";
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
      R[QAST_NODE_STRUCT_TY] = "Struct";
      R[QAST_NODE_ARRAY_TY] = "Array";
      R[QAST_NODE_TUPLE_TY] = "Tuple";
      R[QAST_NODE_FN_TY] = "Fn";
      R[QAST_NODE_UNRES_TY] = "Unres";
      R[QAST_NODE_INFER_TY] = "Infer";
      R[QAST_NODE_TEMPL_TY] = "Templ";
      R[QAST_NODE_TYPEDEF] = "pedef";
      R[QAST_NODE_FNDECL] = "Fndecl";
      R[QAST_NODE_STRUCT] = "Struct";
      R[QAST_NODE_ENUM] = "Enum";
      R[QAST_NODE_FN] = "Fn";
      R[QAST_NODE_SUBSYSTEM] = "Subsystem";
      R[QAST_NODE_EXPORT] = "Export";
      R[QAST_NODE_STRUCT_FIELD] = "StructField";
      R[QAST_NODE_BLOCK] = "Block";
      R[QAST_NODE_CONST] = "Const";
      R[QAST_NODE_VAR] = "Var";
      R[QAST_NODE_LET] = "Let";
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
      R[QAST_NODE_VOLATILE] = "Volstmt";

      return R;
    }();

    return names[type];
  }

  Stmt *mock_stmt(qparse_ty_t expected);
  Expr *mock_expr(qparse_ty_t expected);
  Type *mock_type(qparse_ty_t expected);
  Decl *mock_decl(qparse_ty_t expected);
  Block *mock_block(qparse_ty_t expected);

}  // namespace qparse

namespace std {
  std::ostream &operator<<(std::ostream &os, const qlex_op_t &op);
  std::ostream &operator<<(std::ostream &os, const qlex_op_t &expr);
  std::ostream &operator<<(std::ostream &os, const qlex_op_t &op);
  std::ostream &operator<<(std::ostream &os, const qparse::FuncPurity &purity);
}  // namespace std

#endif  // __NITRATE_PARSER_NODE_H__
