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
        return AST_eLIST;
      } else if constexpr (std::is_same_v<T, Assoc>) {
        return AST_ePAIR;
      } else if constexpr (std::is_same_v<T, Index>) {
        return AST_eINDEX;
      } else if constexpr (std::is_same_v<T, Slice>) {
        return AST_eSLICE;
      } else if constexpr (std::is_same_v<T, FString>) {
        return AST_eFSTRING;
      } else if constexpr (std::is_same_v<T, Identifier>) {
        return AST_eIDENT;
      } else if constexpr (std::is_same_v<T, TemplateCall>) {
        return AST_eTEMPLATE_CALL;
      } else if constexpr (std::is_same_v<T, Import>) {
        return AST_eIMPORT;
      } else if constexpr (std::is_same_v<T, RefTy>) {
        return AST_tREF;
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
        return AST_tVOID;
      } else if constexpr (std::is_same_v<T, PtrTy>) {
        return AST_tPTR;
      } else if constexpr (std::is_same_v<T, OpaqueTy>) {
        return AST_tOPAQUE;
      } else if constexpr (std::is_same_v<T, ArrayTy>) {
        return AST_tARRAY;
      } else if constexpr (std::is_same_v<T, TupleTy>) {
        return AST_tTUPLE;
      } else if constexpr (std::is_same_v<T, FuncTy>) {
        return AST_tFUNCTION;
      } else if constexpr (std::is_same_v<T, NamedTy>) {
        return AST_tNAMED;
      } else if constexpr (std::is_same_v<T, InferTy>) {
        return AST_tINFER;
      } else if constexpr (std::is_same_v<T, TemplateType>) {
        return AST_tTEMPLATE;
      } else if constexpr (std::is_same_v<T, Typedef>) {
        return AST_sTYPEDEF;
      } else if constexpr (std::is_same_v<T, Struct>) {
        return AST_sSTRUCT;
      } else if constexpr (std::is_same_v<T, Enum>) {
        return AST_sENUM;
      } else if constexpr (std::is_same_v<T, Function>) {
        return AST_sFUNCTION;
      } else if constexpr (std::is_same_v<T, Scope>) {
        return AST_sSCOPE;
      } else if constexpr (std::is_same_v<T, Export>) {
        return AST_sEXPORT;
      } else if constexpr (std::is_same_v<T, Block>) {
        return AST_sBLOCK;
      } else if constexpr (std::is_same_v<T, Variable>) {
        return AST_sVAR;
      } else if constexpr (std::is_same_v<T, Assembly>) {
        return AST_sASM;
      } else if constexpr (std::is_same_v<T, Return>) {
        return AST_sRET;
      } else if constexpr (std::is_same_v<T, ReturnIf>) {
        return AST_sRETIF;
      } else if constexpr (std::is_same_v<T, Break>) {
        return AST_sBRK;
      } else if constexpr (std::is_same_v<T, Continue>) {
        return AST_sCONT;
      } else if constexpr (std::is_same_v<T, If>) {
        return AST_sIF;
      } else if constexpr (std::is_same_v<T, While>) {
        return AST_sWHILE;
      } else if constexpr (std::is_same_v<T, For>) {
        return AST_sFOR;
      } else if constexpr (std::is_same_v<T, Foreach>) {
        return AST_sFOREACH;
      } else if constexpr (std::is_same_v<T, Case>) {
        return AST_sCASE;
      } else if constexpr (std::is_same_v<T, Switch>) {
        return AST_sSWITCH;
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
      r[AST_eLIST] = "List";
      r[AST_ePAIR] = "Assoc";
      r[AST_eINDEX] = "Index";
      r[AST_eSLICE] = "Slice";
      r[AST_eFSTRING] = "Fstring";
      r[AST_eIDENT] = "Ident";
      r[AST_eTEMPLATE_CALL] = "TemplateCall";
      r[AST_eIMPORT] = "Import";
      r[AST_tREF] = "Ref";
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
      r[AST_tVOID] = "Void";
      r[AST_tPTR] = "Ptr";
      r[AST_tOPAQUE] = "Opaque";
      r[AST_tARRAY] = "Array";
      r[AST_tTUPLE] = "Tuple";
      r[AST_tFUNCTION] = "FuncTy";
      r[AST_tNAMED] = "Unres";
      r[AST_tINFER] = "Infer";
      r[AST_tTEMPLATE] = "Templ";
      r[AST_sTYPEDEF] = "Typedef";
      r[AST_sSTRUCT] = "Struct";
      r[AST_sENUM] = "Enum";
      r[AST_sFUNCTION] = "Function";
      r[AST_sSCOPE] = "Scope";
      r[AST_sEXPORT] = "Export";
      r[AST_sBLOCK] = "Block";
      r[AST_sVAR] = "Let";
      r[AST_sASM] = "Assembly";
      r[AST_sRET] = "Return";
      r[AST_sRETIF] = "Retif";
      r[AST_sBRK] = "Break";
      r[AST_sCONT] = "Continue";
      r[AST_sIF] = "If";
      r[AST_sWHILE] = "While";
      r[AST_sFOR] = "For";
      r[AST_sFOREACH] = "Foreach";
      r[AST_sCASE] = "Case";
      r[AST_sSWITCH] = "Switch";

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
        case AST_tVOID:
          return true;
        default:
          return false;
      }
    }

    [[nodiscard, gnu::pure]] constexpr auto IsArray() const -> bool { return GetKind() == AST_tARRAY; };
    [[nodiscard, gnu::pure]] constexpr auto IsTuple() const -> bool { return GetKind() == AST_tTUPLE; }
    [[nodiscard, gnu::pure]] constexpr auto IsPointer() const -> bool { return GetKind() == AST_tPTR; }
    [[nodiscard, gnu::pure]] constexpr auto IsFunction() const -> bool { return GetKind() == AST_tFUNCTION; }
    [[nodiscard, gnu::pure]] constexpr auto IsComposite() const -> bool { return IsArray() || IsTuple(); }
    [[nodiscard, gnu::pure]] constexpr auto IsVoid() const -> bool { return GetKind() == AST_tVOID; }
    [[nodiscard, gnu::pure]] constexpr auto IsBool() const -> bool { return GetKind() == AST_tU1; }
    [[nodiscard, gnu::pure]] constexpr auto IsRef() const -> bool { return GetKind() == AST_tREF; }
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
