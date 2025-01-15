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
#include <utility>

namespace ncc::parse {
  class ASTExtensionKey {
    friend class ASTExtension;
    uint64_t m_key : 56 = 0;

    constexpr ASTExtensionKey(uint64_t key) : m_key(key) {}

  public:
    constexpr ASTExtensionKey() = default;

    [[nodiscard]] constexpr auto Key() const { return m_key; }
  } __attribute__((packed));

  class ASTExtensionPackage {
    friend class ASTExtension;

    lex::LocationID m_begin;
    lex::LocationID m_end;
    std::vector<lex::Token> m_comments;

    ASTExtensionPackage(lex::LocationID begin, lex::LocationID end)
        : m_begin(begin), m_end(end) {}

  public:
    [[nodiscard]] auto Begin() const { return m_begin; }
    [[nodiscard]] auto End() const { return m_end; }
    [[nodiscard]] std::span<const lex::Token> Comments() const {
      return m_comments;
    }

    void AddComments(std::span<const lex::Token> comments) {
      m_comments.insert(m_comments.end(), comments.begin(), comments.end());
    }
  };

  class NCC_EXPORT ASTExtension {
    std::vector<ASTExtensionPackage> m_pairs;
    std::mutex m_mutex;

  public:
    ASTExtension() { Reset(); }

    void Reset() {
      m_pairs.clear();
      m_pairs.shrink_to_fit();
      m_pairs.reserve(4096);

      m_pairs.push_back({lex::LocationID(), lex::LocationID()});
    }

    ASTExtensionKey Add(lex::LocationID begin, lex::LocationID end);
    const ASTExtensionPackage &Get(ASTExtensionKey loc);
    void Set(ASTExtensionKey id, ASTExtensionPackage &&data);
  };

  std::ostream &operator<<(std::ostream &os, const ASTExtensionKey &idx);

  extern ASTExtension ExtensionDataStore;

  class Base {
  private:
    NparTyT m_node_type : 7;
    bool m_mock : 1;
    ASTExtensionKey m_data;

  public:
    constexpr Base(NparTyT ty, bool mock = false,
                   lex::LocationID begin = lex::LocationID(),
                   lex::LocationID end = lex::LocationID())
        : m_node_type(ty),
          m_mock(mock),
          m_data(ExtensionDataStore.Add(begin, end)) {}

    ///======================================================================
    /// Efficient LLVM-Style reflection

    static constexpr uint32_t GetKindSize(NparTyT kind);
    static constexpr std::string_view GetKindName(NparTyT type);

    template <typename T>
    static constexpr NparTyT GetTypeCode() {
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

    [[nodiscard]] constexpr NparTyT GetKind() const { return m_node_type; }
    [[nodiscard]] constexpr auto GetKindName() const {
      return GetKindName(m_node_type);
    }

    [[nodiscard]] constexpr bool IsType() const {
      auto kind = GetKind();
      return kind >= QAST__TYPE_FIRST && kind <= QAST__TYPE_LAST;
    }

    [[nodiscard]] constexpr bool IsStmt() const {
      auto kind = GetKind();
      return kind >= QAST__STMT_FIRST && kind <= QAST__STMT_LAST;
    }

    [[nodiscard]] constexpr bool IsExpr() const {
      auto kind = GetKind();
      return kind >= QAST__EXPR_FIRST && kind <= QAST__EXPR_LAST;
    }

    template <typename T>
    [[nodiscard]] constexpr bool is() const {  /// NOLINT
      return Base::GetTypeCode<T>() == GetKind();
    }

    [[nodiscard]] constexpr bool is(NparTyT type) const {  /// NOLINT
      return type == GetKind();
    }
    [[nodiscard]] constexpr bool IsMock() const { return m_mock; }

    [[nodiscard]] bool IsEq(FlowPtr<Base> o) const;

    [[nodiscard]] uint64_t Hash64() const;

    [[nodiscard]] size_t RecursiveChildCount();

    ///======================================================================
    /// Visitation

    constexpr void Accept(ASTVisitor &v) { v.Dispatch(MakeFlowPtr(this)); }
    constexpr void Accept(ASTVisitor &v) const {
      v.Dispatch(MakeFlowPtr(const_cast<Base *>(this)));
    }

    ///======================================================================
    /// Debug-mode checked type casting

    template <typename T>
    [[nodiscard]] static constexpr T *SafeCastAs(Base *ptr) {
      if (!ptr) {
        return nullptr;
      }

#ifndef NDEBUG
      if (GetTypeCode<T>() != ptr->GetKind()) [[unlikely]] {
        qcore_panicf("Invalid cast from %s to %s", ptr->GetKindName(),
                     GetKindName(GetTypeCode<T>()));
      }
#endif

      return reinterpret_cast<T *>(ptr);
    }

    template <typename T>
    constexpr T *as() {  /// NOLINT
      return SafeCastAs<T>(this);
    }

    template <typename T>
    constexpr const T *as() const {  /// NOLINT
      return SafeCastAs<T>(const_cast<Base *>(this));
    }

    ///======================================================================
    /// Debugging

    std::ostream &Dump(std::ostream &os = std::cerr,
                       WriterSourceProvider rd = std::nullopt) const;

    [[nodiscard]] std::string ToJson(
        WriterSourceProvider rd = std::nullopt) const;

    ///======================================================================
    /// AST Extension Data

    [[nodiscard]] constexpr auto Begin() const {
      return ExtensionDataStore.Get(m_data).Begin();
    }
    [[nodiscard]] constexpr auto Begin(lex::IScanner &rd) const {
      return Begin().Get(rd);
    }
    [[nodiscard]] constexpr auto End() const {
      return ExtensionDataStore.Get(m_data).End();
    }
    [[nodiscard]] constexpr auto End(lex::IScanner &rd) const {
      return End().Get(rd);
    }
    [[nodiscard]] constexpr auto GetPos() const {
      return std::pair<lex::LocationID, lex::LocationID>(Begin(), End());
    }

    [[nodiscard]] constexpr auto Comments() const {
      return ExtensionDataStore.Get(m_data).Comments();
    }

    ///======================================================================
    /// Setters

    constexpr void SetOffset(lex::LocationID pos) {
      m_data = ExtensionDataStore.Add(pos, End());
    }

    constexpr void SetLoc(lex::LocationID begin, lex::LocationID end) {
      m_data = ExtensionDataStore.Add(begin, end);
    }

    void BindCodeCommentData(std::span<const lex::Token> comment_tokens);
  } __attribute__((packed));

  static_assert(sizeof(Base) == 8);

  ///======================================================================

  namespace detail {
    constexpr static auto kGetKindNames = []() {
      std::array<std::string_view, QAST_COUNT> r;
      r.fill("");

      r[QAST_BASE] = "Node";
      r[QAST_BINEXPR] = "Binexpr";
      r[QAST_UNEXPR] = "Unexpr";
      r[QAST_TEREXPR] = "Terexpr";
      r[QAST_INT] = "Int";
      r[QAST_FLOAT] = "Float";
      r[QAST_STRING] = "String";
      r[QAST_CHAR] = "Char";
      r[QAST_BOOL] = "Bool";
      r[QAST_NULL] = "Null";
      r[QAST_UNDEF] = "Undef";
      r[QAST_CALL] = "Call";
      r[QAST_LIST] = "List";
      r[QAST_ASSOC] = "Assoc";
      r[QAST_INDEX] = "Index";
      r[QAST_SLICE] = "Slice";
      r[QAST_FSTRING] = "Fstring";
      r[QAST_IDENT] = "Ident";
      r[QAST_SEQ] = "SeqPoint";
      r[QAST_POST_UNEXPR] = "PostUnexpr";
      r[QAST_SEXPR] = "StmtExpr";
      r[QAST_TEXPR] = "TypeExpr";
      r[QAST_TEMPL_CALL] = "TemplCall";
      r[QAST_REF] = "Ref";
      r[QAST_U1] = "U1";
      r[QAST_U8] = "U8";
      r[QAST_U16] = "U16";
      r[QAST_U32] = "U32";
      r[QAST_U64] = "U64";
      r[QAST_U128] = "U128";
      r[QAST_I8] = "I8";
      r[QAST_I16] = "I16";
      r[QAST_I32] = "I32";
      r[QAST_I64] = "I64";
      r[QAST_I128] = "I128";
      r[QAST_F16] = "F16";
      r[QAST_F32] = "F32";
      r[QAST_F64] = "F64";
      r[QAST_F128] = "F128";
      r[QAST_VOID] = "Void";
      r[QAST_PTR] = "Ptr";
      r[QAST_OPAQUE] = "Opaque";
      r[QAST_ARRAY] = "Array";
      r[QAST_TUPLE] = "Tuple";
      r[QAST_FUNCTOR] = "FuncTy";
      r[QAST_NAMED] = "Unres";
      r[QAST_INFER] = "Infer";
      r[QAST_TEMPLATE] = "Templ";
      r[QAST_TYPEDEF] = "Typedef";
      r[QAST_STRUCT] = "Struct";
      r[QAST_ENUM] = "Enum";
      r[QAST_FUNCTION] = "Function";
      r[QAST_SCOPE] = "Scope";
      r[QAST_EXPORT] = "Export";
      r[QAST_BLOCK] = "Block";
      r[QAST_VAR] = "Let";
      r[QAST_INLINE_ASM] = "InlineAsm";
      r[QAST_RETURN] = "Return";
      r[QAST_RETIF] = "Retif";
      r[QAST_BREAK] = "Break";
      r[QAST_CONTINUE] = "Continue";
      r[QAST_IF] = "If";
      r[QAST_WHILE] = "While";
      r[QAST_FOR] = "For";
      r[QAST_FOREACH] = "Foreach";
      r[QAST_CASE] = "Case";
      r[QAST_SWITCH] = "Switch";
      r[QAST_ESTMT] = "ExprStmt";

      return r;
    }();
  }  // namespace detail

  constexpr std::string_view Base::GetKindName(NparTyT type) {
    return detail::kGetKindNames[type];
  }

  class Stmt : public Base {
  public:
    constexpr Stmt(NparTyT ty) : Base(ty){};

    [[nodiscard]] constexpr bool IsExprStmt(NparTyT type) const;
  };

  class Type : public Base {
    NullableFlowPtr<Expr> m_range_begin, m_range_end, m_width;

  public:
    constexpr Type(NparTyT ty) : Base(ty) {}

    [[nodiscard]] constexpr bool IsPrimitive() const {
      switch (GetKind()) {
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
    [[nodiscard]] constexpr bool IsArray() const {
      return GetKind() == QAST_ARRAY;
    };
    [[nodiscard]] constexpr bool IsTuple() const {
      return GetKind() == QAST_TUPLE;
    }
    [[nodiscard]] constexpr bool IsPointer() const {
      return GetKind() == QAST_PTR;
    }
    [[nodiscard]] constexpr bool IsFunction() const {
      return GetKind() == QAST_FUNCTOR;
    }
    [[nodiscard]] constexpr bool IsComposite() const {
      return IsArray() || IsTuple();
    }
    [[nodiscard]] constexpr bool IsNumeric() const {
      return GetKind() >= QAST_U1 && GetKind() <= QAST_F128;
    }
    [[nodiscard]] constexpr bool IsIntegral() const {
      return GetKind() >= QAST_U1 && GetKind() <= QAST_I128;
    }
    [[nodiscard]] constexpr bool IsFloatingPoint() const {
      return GetKind() >= QAST_F16 && GetKind() <= QAST_F128;
    }
    [[nodiscard]] constexpr bool IsSigned() const {
      return GetKind() >= QAST_I8 && GetKind() <= QAST_I128;
    }
    [[nodiscard]] constexpr bool IsUnsigned() const {
      return GetKind() >= QAST_U1 && GetKind() <= QAST_U128;
    }
    [[nodiscard]] constexpr bool IsVoid() const {
      return GetKind() == QAST_VOID;
    }
    [[nodiscard]] constexpr bool IsBool() const { return GetKind() == QAST_U1; }
    [[nodiscard]] constexpr bool IsRef() const { return GetKind() == QAST_REF; }
    bool IsPtrTo(const Type *type) const;

    [[nodiscard]] constexpr auto GetWidth() const { return m_width; }
    [[nodiscard]] constexpr auto GetRangeBegin() const { return m_range_begin; }
    [[nodiscard]] constexpr auto GetRangeEnd() const { return m_range_end; }

    constexpr void SetRangeBegin(NullableFlowPtr<Expr> start) {
      m_range_begin = std::move(start);
    }

    constexpr void SetRangeEnd(NullableFlowPtr<Expr> end) {
      m_range_end = std::move(end);
    }

    constexpr void SetWidth(NullableFlowPtr<Expr> width) {
      m_width = std::move(width);
    }
  };

  class Expr : public Base {
  public:
    constexpr Expr(NparTyT ty) : Base(ty) {}

    [[nodiscard]] constexpr bool IsStmtExpr(NparTyT type) const;
  };
}  // namespace ncc::parse

#endif
