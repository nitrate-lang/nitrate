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
  using OptionalSourceProvider = std::optional<std::reference_wrapper<lex::IScanner>>;

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

    [[nodiscard, gnu::const]] static constexpr auto GetKindSize(ASTNodeKind kind) -> uint32_t;
    [[nodiscard, gnu::const]] static constexpr auto GetKindName(ASTNodeKind type) -> std::string_view;

    [[nodiscard, gnu::pure]] constexpr auto GetKind() const -> ASTNodeKind { return m_node_type; }
    [[nodiscard, gnu::pure]] constexpr auto GetKindName() const -> std::string_view { return GetKindName(m_node_type); }
    [[nodiscard, gnu::pure]] constexpr auto Is(ASTNodeKind type) const -> bool { return type == GetKind(); }
    [[nodiscard, gnu::pure]] constexpr auto IsMock() const -> bool { return m_mock; }
    [[nodiscard, gnu::pure]] constexpr auto IsDiscarded() const -> bool { return Is(AST_DISCARDED); }

    template <typename T>
    [[nodiscard, gnu::const]] static constexpr auto GetTypeCode() -> ASTNodeKind {
      using namespace ncc::parse;

      if constexpr (std::is_same_v<T, Binary>) {
        return AST_eBIN;
      } else if constexpr (std::is_same_v<T, Unary>) {
        return AST_eUNARY;
      } else if constexpr (std::is_same_v<T, Integer>) {
        return AST_eINT;
      } else if constexpr (std::is_same_v<T, Float>) {
        return AST_eFLOAT;
      } else if constexpr (std::is_same_v<T, String>) {
        return AST_eSTRING;
      } else if constexpr (std::is_same_v<T, Character>) {
        return AST_eCHAR;
      } else if constexpr (std::is_same_v<T, Boolean>) {
        return AST_eBOOL;
      } else if constexpr (std::is_same_v<T, Null>) {
        return AST_eNULL;
      } else if constexpr (std::is_same_v<T, Undefined>) {
        return AST_eUNDEF;
      } else if constexpr (std::is_same_v<T, Call>) {
        return AST_eCALL;
      } else if constexpr (std::is_same_v<T, List>) {
        return AST_LIST;
      } else if constexpr (std::is_same_v<T, Assoc>) {
        return AST_ASSOC;
      } else if constexpr (std::is_same_v<T, Index>) {
        return AST_INDEX;
      } else if constexpr (std::is_same_v<T, Slice>) {
        return AST_SLICE;
      } else if constexpr (std::is_same_v<T, FString>) {
        return AST_FSTRING;
      } else if constexpr (std::is_same_v<T, Identifier>) {
        return AST_IDENT;
      } else if constexpr (std::is_same_v<T, TemplateCall>) {
        return AST_TEMPL_CALL;
      } else if constexpr (std::is_same_v<T, Import>) {
        return AST_IMPORT;
      } else if constexpr (std::is_same_v<T, RefTy>) {
        return AST_REF;
      } else if constexpr (std::is_same_v<T, U1>) {
        return AST_tU1;
      } else if constexpr (std::is_same_v<T, U8>) {
        return AST_tU8;
      } else if constexpr (std::is_same_v<T, U16>) {
        return AST_tU16;
      } else if constexpr (std::is_same_v<T, U32>) {
        return AST_tU32;
      } else if constexpr (std::is_same_v<T, U64>) {
        return AST_tU64;
      } else if constexpr (std::is_same_v<T, U128>) {
        return AST_tU128;
      } else if constexpr (std::is_same_v<T, I8>) {
        return AST_tI8;
      } else if constexpr (std::is_same_v<T, I16>) {
        return AST_tI16;
      } else if constexpr (std::is_same_v<T, I32>) {
        return AST_tI32;
      } else if constexpr (std::is_same_v<T, I64>) {
        return AST_tI64;
      } else if constexpr (std::is_same_v<T, I128>) {
        return AST_tI128;
      } else if constexpr (std::is_same_v<T, F16>) {
        return AST_tF16;
      } else if constexpr (std::is_same_v<T, F32>) {
        return AST_tF32;
      } else if constexpr (std::is_same_v<T, F64>) {
        return AST_tF64;
      } else if constexpr (std::is_same_v<T, F128>) {
        return AST_tF128;
      } else if constexpr (std::is_same_v<T, VoidTy>) {
        return AST_VOID;
      } else if constexpr (std::is_same_v<T, PtrTy>) {
        return AST_PTR;
      } else if constexpr (std::is_same_v<T, OpaqueTy>) {
        return AST_OPAQUE;
      } else if constexpr (std::is_same_v<T, ArrayTy>) {
        return AST_ARRAY;
      } else if constexpr (std::is_same_v<T, TupleTy>) {
        return AST_TUPLE;
      } else if constexpr (std::is_same_v<T, FuncTy>) {
        return AST_FUNCTOR;
      } else if constexpr (std::is_same_v<T, NamedTy>) {
        return AST_NAMED;
      } else if constexpr (std::is_same_v<T, InferTy>) {
        return AST_INFER;
      } else if constexpr (std::is_same_v<T, TemplateType>) {
        return AST_TEMPLATE;
      } else if constexpr (std::is_same_v<T, Typedef>) {
        return AST_TYPEDEF;
      } else if constexpr (std::is_same_v<T, Struct>) {
        return AST_STRUCT;
      } else if constexpr (std::is_same_v<T, Enum>) {
        return AST_ENUM;
      } else if constexpr (std::is_same_v<T, Function>) {
        return AST_FUNCTION;
      } else if constexpr (std::is_same_v<T, Scope>) {
        return AST_SCOPE;
      } else if constexpr (std::is_same_v<T, Export>) {
        return AST_EXPORT;
      } else if constexpr (std::is_same_v<T, Block>) {
        return AST_BLOCK;
      } else if constexpr (std::is_same_v<T, Variable>) {
        return AST_VAR;
      } else if constexpr (std::is_same_v<T, Assembly>) {
        return AST_INLINE_ASM;
      } else if constexpr (std::is_same_v<T, Return>) {
        return AST_RETURN;
      } else if constexpr (std::is_same_v<T, ReturnIf>) {
        return AST_RETIF;
      } else if constexpr (std::is_same_v<T, Break>) {
        return AST_BREAK;
      } else if constexpr (std::is_same_v<T, Continue>) {
        return AST_CONTINUE;
      } else if constexpr (std::is_same_v<T, If>) {
        return AST_IF;
      } else if constexpr (std::is_same_v<T, While>) {
        return AST_WHILE;
      } else if constexpr (std::is_same_v<T, For>) {
        return AST_FOR;
      } else if constexpr (std::is_same_v<T, Foreach>) {
        return AST_FOREACH;
      } else if constexpr (std::is_same_v<T, Case>) {
        return AST_CASE;
      } else if constexpr (std::is_same_v<T, Switch>) {
        return AST_SWITCH;
      }
    }

    [[nodiscard, gnu::pure]] constexpr auto IsType() const -> bool {
      const auto kind = GetKind();
      return kind >= AST__TYPE_FIRST && kind <= AST__TYPE_LAST;
    }

    [[nodiscard, gnu::pure]] constexpr auto IsStmt() const -> bool {
      const auto kind = GetKind();
      return kind >= AST__STMT_FIRST && kind <= AST__STMT_LAST;
    }

    [[nodiscard, gnu::pure]] constexpr auto IsExpr() const -> bool {
      const auto kind = GetKind();
      return kind >= AST__EXPR_FIRST && kind <= AST__EXPR_LAST;
    }

    template <typename T>
    [[nodiscard, gnu::pure]] constexpr auto Is() const -> bool {
      return GetTypeCode<T>() == GetKind();
    }

    ///======================================================================
    /// Visitation

    template <typename Visitor>
    constexpr void Accept(Visitor &&v) {
      v.Dispatch(MakeFlowPtr(this));
    }

    constexpr void Accept(ASTVisitor &v) { v.Dispatch(MakeFlowPtr(this)); }

    ///======================================================================
    /// Debug-mode checked type casting

    template <typename T>
    [[nodiscard, gnu::const]] constexpr auto As() -> T * {
      return SafeCastAs<T>(this);
    }

    template <typename T>
    [[nodiscard, gnu::const]] constexpr auto As() const -> const T * {
      return SafeCastAs<T>(const_cast<Expr *>(this));
    }

    template <typename T>
    [[nodiscard, gnu::const]] static constexpr auto SafeCastAs(Expr *ptr) -> T * {
#ifndef NDEBUG
      if (!ptr) [[unlikely]] {
        return nullptr;
      }

      if constexpr (std::is_same_v<T, Type>) {
        auto kind = ptr->GetKind();
        if (kind < AST__TYPE_FIRST || kind > AST__TYPE_LAST) [[unlikely]] {
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

    ///======================================================================
    /// Serialization

    auto PrettyPrint(std::ostream &os, OptionalSourceProvider rd = std::nullopt) const -> std::ostream &;
    auto Serialize(std::ostream &os) const -> std::ostream &;

    [[nodiscard]] auto PrettyPrint(OptionalSourceProvider rd = std::nullopt) const -> std::string;
    [[nodiscard]] auto Serialize() const -> std::string;

    ///======================================================================
    /// Identity

    [[nodiscard]] auto IsEq(const FlowPtr<Expr> &o) const -> bool;
    [[nodiscard]] auto Hash64() const -> uint64_t;
    [[nodiscard]] auto RecursiveChildCount() -> size_t;

    ///======================================================================
    /// AST Extension Data

    [[nodiscard]] auto SourceBegin() const -> lex::LocationID;
    [[nodiscard]] auto SourceBegin(lex::IScanner &rd) const -> lex::Location;
    [[nodiscard]] auto SourceEnd() const -> lex::LocationID;
    [[nodiscard]] auto SourceEnd(lex::IScanner &rd) const -> lex::Location;
    [[nodiscard]] auto GetSourcePosition() const -> std::pair<lex::LocationID, lex::LocationID>;
    [[nodiscard]] auto Comments() const -> std::span<const string>;
    [[nodiscard]] auto GetParenthesisDepth() const -> size_t;

    ///======================================================================
    /// Setters

    void SetSourcePosition(lex::LocationID begin, lex::LocationID end);
    void SetComments(std::span<const string> comments);
    void SetOffset(lex::LocationID pos);
    void SetParenthesisDepth(size_t depth);
    void SetMock(bool mock);
    void Discard() { m_node_type = AST_DISCARDED; };
  } __attribute__((packed));

  static_assert(sizeof(Expr) == 8);

  ///======================================================================

  namespace detail {
    constexpr static auto kGetKindNames = []() {
      std::array<std::string_view, AST__RANGE + 1> r;
      r.fill("");

      r[AST_eBIN] = "Binexpr";
      r[AST_eUNARY] = "Unexpr";
      r[AST_eINT] = "Int";
      r[AST_eFLOAT] = "Float";
      r[AST_eSTRING] = "String";
      r[AST_eCHAR] = "Char";
      r[AST_eBOOL] = "Bool";
      r[AST_eNULL] = "Null";
      r[AST_eUNDEF] = "Undef";
      r[AST_eCALL] = "Call";
      r[AST_LIST] = "List";
      r[AST_ASSOC] = "Assoc";
      r[AST_INDEX] = "Index";
      r[AST_SLICE] = "Slice";
      r[AST_FSTRING] = "Fstring";
      r[AST_IDENT] = "Ident";
      r[AST_TEMPL_CALL] = "TemplateCall";
      r[AST_IMPORT] = "Import";
      r[AST_REF] = "Ref";
      r[AST_tU1] = "U1";
      r[AST_tU8] = "U8";
      r[AST_tU16] = "U16";
      r[AST_tU32] = "U32";
      r[AST_tU64] = "U64";
      r[AST_tU128] = "U128";
      r[AST_tI8] = "I8";
      r[AST_tI16] = "I16";
      r[AST_tI32] = "I32";
      r[AST_tI64] = "I64";
      r[AST_tI128] = "I128";
      r[AST_tF16] = "F16";
      r[AST_tF32] = "F32";
      r[AST_tF64] = "F64";
      r[AST_tF128] = "F128";
      r[AST_VOID] = "Void";
      r[AST_PTR] = "Ptr";
      r[AST_OPAQUE] = "Opaque";
      r[AST_ARRAY] = "Array";
      r[AST_TUPLE] = "Tuple";
      r[AST_FUNCTOR] = "FuncTy";
      r[AST_NAMED] = "Unres";
      r[AST_INFER] = "Infer";
      r[AST_TEMPLATE] = "Templ";
      r[AST_TYPEDEF] = "Typedef";
      r[AST_STRUCT] = "Struct";
      r[AST_ENUM] = "Enum";
      r[AST_FUNCTION] = "Function";
      r[AST_SCOPE] = "Scope";
      r[AST_EXPORT] = "Export";
      r[AST_BLOCK] = "Block";
      r[AST_VAR] = "Let";
      r[AST_INLINE_ASM] = "Assembly";
      r[AST_RETURN] = "Return";
      r[AST_RETIF] = "Retif";
      r[AST_BREAK] = "Break";
      r[AST_CONTINUE] = "Continue";
      r[AST_IF] = "If";
      r[AST_WHILE] = "While";
      r[AST_FOR] = "For";
      r[AST_FOREACH] = "Foreach";
      r[AST_CASE] = "Case";
      r[AST_SWITCH] = "Switch";

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
        case AST_tU1:
        case AST_tU8:
        case AST_tU16:
        case AST_tU32:
        case AST_tU64:
        case AST_tU128:
        case AST_tI8:
        case AST_tI16:
        case AST_tI32:
        case AST_tI64:
        case AST_tI128:
        case AST_tF16:
        case AST_tF32:
        case AST_tF64:
        case AST_tF128:
        case AST_VOID:
          return true;
        default:
          return false;
      }
    }

    [[nodiscard, gnu::pure]] constexpr auto IsArray() const -> bool { return GetKind() == AST_ARRAY; };
    [[nodiscard, gnu::pure]] constexpr auto IsTuple() const -> bool { return GetKind() == AST_TUPLE; }
    [[nodiscard, gnu::pure]] constexpr auto IsPointer() const -> bool { return GetKind() == AST_PTR; }
    [[nodiscard, gnu::pure]] constexpr auto IsFunction() const -> bool { return GetKind() == AST_FUNCTOR; }
    [[nodiscard, gnu::pure]] constexpr auto IsComposite() const -> bool { return IsArray() || IsTuple(); }
    [[nodiscard, gnu::pure]] constexpr auto IsVoid() const -> bool { return GetKind() == AST_VOID; }
    [[nodiscard, gnu::pure]] constexpr auto IsBool() const -> bool { return GetKind() == AST_tU1; }
    [[nodiscard, gnu::pure]] constexpr auto IsRef() const -> bool { return GetKind() == AST_REF; }
    [[nodiscard, gnu::pure]] constexpr auto IsNumeric() const -> bool {
      return GetKind() >= AST_tU1 && GetKind() <= AST_tF128;
    }
    [[nodiscard, gnu::pure]] constexpr auto IsIntegral() const -> bool {
      return GetKind() >= AST_tU1 && GetKind() <= AST_tI128;
    }
    [[nodiscard, gnu::pure]] constexpr auto IsFloatingPoint() const -> bool {
      return GetKind() >= AST_tF16 && GetKind() <= AST_tF128;
    }
    [[nodiscard, gnu::pure]] constexpr auto IsSigned() const -> bool {
      return GetKind() >= AST_tI8 && GetKind() <= AST_tI128;
    }
    [[nodiscard, gnu::pure]] constexpr auto IsUnsigned() const -> bool {
      return GetKind() >= AST_tU1 && GetKind() <= AST_tU128;
    }

    [[nodiscard, gnu::pure]] constexpr auto GetWidth() const { return m_width; }
    [[nodiscard, gnu::pure]] constexpr auto GetRangeBegin() const { return m_range_begin; }
    [[nodiscard, gnu::pure]] constexpr auto GetRangeEnd() const { return m_range_end; }

    constexpr void SetRangeBegin(NullableFlowPtr<Expr> start) { m_range_begin = std::move(start); }
    constexpr void SetRangeEnd(NullableFlowPtr<Expr> end) { m_range_end = std::move(end); }
    constexpr void SetWidth(NullableFlowPtr<Expr> width) { m_width = std::move(width); }
  };
}  // namespace ncc::parse

#endif
