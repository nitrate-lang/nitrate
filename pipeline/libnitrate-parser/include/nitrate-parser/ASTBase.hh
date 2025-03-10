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

#ifndef __NITRATE_AST_BASE_H__
#define __NITRATE_AST_BASE_H__

#include <array>
#include <nitrate-core/FlowPtr.hh>
#include <nitrate-core/Macro.hh>
#include <nitrate-core/NullableFlowPtr.hh>
#include <nitrate-lexer/Location.hh>
#include <nitrate-lexer/ScannerFwd.hh>
#include <nitrate-parser/AST.hh>
#include <nitrate-parser/ASTVisitor.hh>
#include <type_traits>
#include <utility>

namespace ncc::parse {
  class NCC_EXPORT ASTExtension {
    uint64_t m_key : 56 = 0;

    constexpr ASTExtension(uint64_t key) : m_key(key) {}
    void LazyInitialize();

  public:
    constexpr ASTExtension() = default;

    static void ResetStorage();

    void SetSourceLocationBound(lex::LocationID begin, lex::LocationID end);
    void SetComments(std::span<const string> comments);
    void SetParenthesisDepth(size_t depth);

    [[nodiscard]] auto GetSourceLocationBound() const -> std::pair<lex::LocationID, lex::LocationID>;
    [[nodiscard]] auto GetComments() const -> std::span<const string>;
    [[nodiscard]] auto GetParenthesisDepth() const -> size_t;

    [[nodiscard]] constexpr auto IsNull() const -> bool { return m_key == 0; }
    [[nodiscard]] constexpr auto Key() const { return m_key; }
  } __attribute__((packed));

  auto operator<<(std::ostream &os, const ASTExtension &idx) -> std::ostream &;

  class NCC_EXPORT Expr {
  private:
    ASTNodeKind m_node_type : 7;
    bool m_mock : 1;
    ASTExtension m_data;

  protected:
    constexpr Expr(ASTNodeKind ty, bool mock = false) : m_node_type(ty), m_mock(mock) {}
    constexpr Expr(ASTNodeKind ty, bool mock, lex::LocationID begin, lex::LocationID end)
        : m_node_type(ty), m_mock(mock) {
      m_data.SetSourceLocationBound(begin, end);
    }

  public:
    ///======================================================================
    /// Efficient LLVM-Style reflection

    [[gnu::const, nodiscard]] static constexpr auto GetKindSize(ASTNodeKind kind) -> uint32_t;
    [[gnu::const, nodiscard]] static constexpr auto GetKindName(ASTNodeKind type) -> std::string_view;

    template <typename T>
    [[gnu::const, nodiscard]] static constexpr auto GetTypeCode() -> ASTNodeKind {
      using namespace ncc::parse;

      if constexpr (std::is_same_v<T, Binary>) {
        return QAST_BINEXPR;
      } else if constexpr (std::is_same_v<T, Unary>) {
        return QAST_UNEXPR;
      } else if constexpr (std::is_same_v<T, Ternary>) {
        return QAST_TEREXPR;
      } else if constexpr (std::is_same_v<T, Integer>) {
        return QAST_INT;
      } else if constexpr (std::is_same_v<T, Float>) {
        return QAST_FLOAT;
      } else if constexpr (std::is_same_v<T, String>) {
        return QAST_STRING;
      } else if constexpr (std::is_same_v<T, Character>) {
        return QAST_CHAR;
      } else if constexpr (std::is_same_v<T, Boolean>) {
        return QAST_BOOL;
      } else if constexpr (std::is_same_v<T, Null>) {
        return QAST_NULL;
      } else if constexpr (std::is_same_v<T, Undefined>) {
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
      } else if constexpr (std::is_same_v<T, Identifier>) {
        return QAST_IDENT;
      } else if constexpr (std::is_same_v<T, PostUnary>) {
        return QAST_POST_UNEXPR;
      } else if constexpr (std::is_same_v<T, TemplateCall>) {
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
      } else if constexpr (std::is_same_v<T, TemplateType>) {
        return QAST_TEMPLATE;
      } else if constexpr (std::is_same_v<T, Typedef>) {
        return QAST_TYPEDEF;
      } else if constexpr (std::is_same_v<T, Struct>) {
        return QAST_STRUCT;
      } else if constexpr (std::is_same_v<T, Enum>) {
        return QAST_ENUM;
      } else if constexpr (std::is_same_v<T, Function>) {
        return QAST_FUNCTION;
      } else if constexpr (std::is_same_v<T, Scope>) {
        return QAST_SCOPE;
      } else if constexpr (std::is_same_v<T, Export>) {
        return QAST_EXPORT;
      } else if constexpr (std::is_same_v<T, Block>) {
        return QAST_BLOCK;
      } else if constexpr (std::is_same_v<T, Variable>) {
        return QAST_VAR;
      } else if constexpr (std::is_same_v<T, Assembly>) {
        return QAST_INLINE_ASM;
      } else if constexpr (std::is_same_v<T, Return>) {
        return QAST_RETURN;
      } else if constexpr (std::is_same_v<T, ReturnIf>) {
        return QAST_RETIF;
      } else if constexpr (std::is_same_v<T, Break>) {
        return QAST_BREAK;
      } else if constexpr (std::is_same_v<T, Continue>) {
        return QAST_CONTINUE;
      } else if constexpr (std::is_same_v<T, If>) {
        return QAST_IF;
      } else if constexpr (std::is_same_v<T, While>) {
        return QAST_WHILE;
      } else if constexpr (std::is_same_v<T, For>) {
        return QAST_FOR;
      } else if constexpr (std::is_same_v<T, Foreach>) {
        return QAST_FOREACH;
      } else if constexpr (std::is_same_v<T, Case>) {
        return QAST_CASE;
      } else if constexpr (std::is_same_v<T, Switch>) {
        return QAST_SWITCH;
      }
    }

    [[gnu::const, nodiscard]] constexpr auto GetKind() const -> ASTNodeKind { return m_node_type; }
    [[gnu::const, nodiscard]] constexpr auto GetKindName() const { return GetKindName(m_node_type); }

    [[gnu::const, nodiscard]] constexpr auto IsType() const -> bool {
      auto kind = GetKind();
      return kind >= QAST__TYPE_FIRST && kind <= QAST__TYPE_LAST;
    }

    [[gnu::const, nodiscard]] constexpr auto IsStmt() const -> bool {
      auto kind = GetKind();
      return kind >= QAST__STMT_FIRST && kind <= QAST__STMT_LAST;
    }

    [[gnu::const, nodiscard]] constexpr auto IsExpr() const -> bool {
      auto kind = GetKind();
      return kind >= QAST__EXPR_FIRST && kind <= QAST__EXPR_LAST;
    }

    template <typename T>
    [[gnu::const, nodiscard]] constexpr bool Is() const {
      return Expr::GetTypeCode<T>() == GetKind();
    }

    [[gnu::const, nodiscard]] constexpr bool Is(ASTNodeKind type) const { return type == GetKind(); }
    [[gnu::const, nodiscard]] constexpr auto IsMock() const -> bool { return m_mock; }
    constexpr void SetMock(bool mock) { m_mock = mock; }
    [[gnu::pure, nodiscard]] auto IsEq(FlowPtr<Expr> o) const -> bool;
    [[gnu::const, nodiscard]] auto Hash64() const -> uint64_t;
    [[gnu::const, nodiscard]] auto RecursiveChildCount() -> size_t;

    ///======================================================================
    /// Visitation

    constexpr void Accept(ASTVisitor &v) { v.Dispatch(MakeFlowPtr(this)); }

    template <typename Visitor>
    constexpr void Accept(Visitor &&v) {
      v.Dispatch(MakeFlowPtr(this));
    }

    ///======================================================================
    /// Debug-mode checked type casting

    template <typename T>
    [[gnu::const, nodiscard]] static constexpr auto SafeCastAs(Expr *ptr) -> T * {
#ifndef NDEBUG
      if (!ptr) {
        return nullptr;
      }

      if constexpr (std::is_same_v<T, Type>) {
        auto kind = ptr->GetKind();
        if (kind < QAST__TYPE_FIRST || kind > QAST__TYPE_LAST) [[unlikely]] {
          qcore_panicf("Invalid cast from %s to Type", ptr->GetKindName());
        }
      } else {
        if (GetTypeCode<T>() != ptr->GetKind()) [[unlikely]] {
          qcore_panicf("Invalid cast from %s to %s", ptr->GetKindName(), GetKindName(GetTypeCode<T>()));
        }
      }
#endif

      return reinterpret_cast<T *>(ptr);
    }

    template <typename T>
    [[gnu::const, nodiscard]] constexpr T *As() {
      return SafeCastAs<T>(this);
    }

    template <typename T>
    [[gnu::const, nodiscard]] constexpr const T *As() const {
      return SafeCastAs<T>(const_cast<Expr *>(this));
    }

    ///======================================================================
    /// Serialization

    void DebugString(std::ostream &os, std::optional<std::reference_wrapper<lex::IScanner>> rd = std::nullopt) const;
    [[gnu::pure, nodiscard]] std::string DebugString(
        std::optional<std::reference_wrapper<lex::IScanner>> rd = std::nullopt) const;

    void Serialize(std::ostream &os) const;
    [[gnu::pure, nodiscard]] std::string Serialize() const;

    ///======================================================================
    /// AST Extension Data

    [[nodiscard]] constexpr auto SourceBegin() const {
      return m_data.IsNull() ? lex::LocationID() : m_data.GetSourceLocationBound().first;
    }
    [[nodiscard]] constexpr auto SourceBegin(lex::IScanner &rd) const { return SourceBegin().Get(rd); }
    [[nodiscard]] constexpr auto SourceEnd() const {
      return m_data.IsNull() ? lex::LocationID() : m_data.GetSourceLocationBound().second;
    }
    [[nodiscard]] constexpr auto SourceEnd(lex::IScanner &rd) const { return SourceEnd().Get(rd); }
    [[nodiscard]] constexpr auto GetPos() const {
      return std::pair<lex::LocationID, lex::LocationID>(SourceBegin(), SourceEnd());
    }

    [[nodiscard]] constexpr auto Comments() const {
      return m_data.IsNull() ? std::span<const string>() : m_data.GetComments();
    }

    [[nodiscard]] constexpr auto GetParenthesisDepth() const { return m_data.GetParenthesisDepth(); }

    ///======================================================================
    /// Setters

    constexpr void SetOffset(lex::LocationID pos) { m_data.SetSourceLocationBound(pos, SourceEnd()); }
    constexpr void SetLoc(lex::LocationID begin, lex::LocationID end) { m_data.SetSourceLocationBound(begin, end); }
    void SetComments(std::span<const string> comments);
    void SetParenthesisDepth(size_t depth) { m_data.SetParenthesisDepth(depth); }
  } __attribute__((packed));

  static_assert(sizeof(Expr) == 8);

  ///======================================================================

  namespace detail {
    constexpr static auto kGetKindNames = []() {
      std::array<std::string_view, QAST__RANGE + 1> r;
      r.fill("");

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
      r[QAST_POST_UNEXPR] = "PostUnexpr";
      r[QAST_TEMPL_CALL] = "TemplateCall";
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
      r[QAST_INLINE_ASM] = "Assembly";
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

      return r;
    }();
  }  // namespace detail

  constexpr auto Expr::GetKindName(ASTNodeKind type) -> std::string_view { return detail::kGetKindNames[type]; }

  class Type : public Expr {
    NullableFlowPtr<Expr> m_range_begin, m_range_end, m_width;

  protected:
    constexpr Type(ASTNodeKind ty) : Expr(ty) {}

  public:
    [[nodiscard]] constexpr auto IsPrimitive() const -> bool {
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

    [[gnu::const, nodiscard]] constexpr auto IsArray() const -> bool { return GetKind() == QAST_ARRAY; };
    [[gnu::const, nodiscard]] constexpr auto IsTuple() const -> bool { return GetKind() == QAST_TUPLE; }
    [[gnu::const, nodiscard]] constexpr auto IsPointer() const -> bool { return GetKind() == QAST_PTR; }
    [[gnu::const, nodiscard]] constexpr auto IsFunction() const -> bool { return GetKind() == QAST_FUNCTOR; }
    [[gnu::const, nodiscard]] constexpr auto IsComposite() const -> bool { return IsArray() || IsTuple(); }
    [[gnu::const, nodiscard]] constexpr auto IsVoid() const -> bool { return GetKind() == QAST_VOID; }
    [[gnu::const, nodiscard]] constexpr auto IsBool() const -> bool { return GetKind() == QAST_U1; }
    [[gnu::const, nodiscard]] constexpr auto IsRef() const -> bool { return GetKind() == QAST_REF; }
    [[gnu::const, nodiscard]] constexpr auto IsNumeric() const -> bool {
      return GetKind() >= QAST_U1 && GetKind() <= QAST_F128;
    }
    [[gnu::const, nodiscard]] constexpr auto IsIntegral() const -> bool {
      return GetKind() >= QAST_U1 && GetKind() <= QAST_I128;
    }
    [[gnu::const, nodiscard]] constexpr auto IsFloatingPoint() const -> bool {
      return GetKind() >= QAST_F16 && GetKind() <= QAST_F128;
    }
    [[gnu::const, nodiscard]] constexpr auto IsSigned() const -> bool {
      return GetKind() >= QAST_I8 && GetKind() <= QAST_I128;
    }
    [[gnu::const, nodiscard]] constexpr auto IsUnsigned() const -> bool {
      return GetKind() >= QAST_U1 && GetKind() <= QAST_U128;
    }

    [[gnu::const, nodiscard]] constexpr auto GetWidth() const { return m_width; }
    [[gnu::const, nodiscard]] constexpr auto GetRangeBegin() const { return m_range_begin; }
    [[gnu::const, nodiscard]] constexpr auto GetRangeEnd() const { return m_range_end; }

    constexpr void SetRangeBegin(NullableFlowPtr<Expr> start) { m_range_begin = std::move(start); }
    constexpr void SetRangeEnd(NullableFlowPtr<Expr> end) { m_range_end = std::move(end); }
    constexpr void SetWidth(NullableFlowPtr<Expr> width) { m_width = std::move(width); }
  };
}  // namespace ncc::parse

#endif
