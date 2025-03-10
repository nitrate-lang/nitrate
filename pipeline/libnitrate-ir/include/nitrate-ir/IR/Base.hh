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

#ifndef __NITRATE_IR_GRAPH_BASE_H__
#define __NITRATE_IR_GRAPH_BASE_H__

#include <boost/multiprecision/cpp_int.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <cstdint>
#include <iostream>
#include <nitrate-core/Allocate.hh>
#include <nitrate-core/FlowPtr.hh>
#include <nitrate-core/Logger.hh>
#include <nitrate-core/NullableFlowPtr.hh>
#include <nitrate-ir/IR/Common.hh>
#include <nitrate-ir/IR/Fwd.hh>
#include <nitrate-ir/IR/Visitor.hh>
#include <nitrate-parser/ASTBase.hh>
#include <optional>
#include <ostream>
#include <string>

namespace ncc::ir {
  namespace detail {
    void NodeDumpImpl(const Expr *e, std::ostream &os, bool is_for_debug);
    auto IsAcyclicImpl(FlowPtr<Expr> e) -> bool;

    auto ExprGetType(Expr *e) -> std::optional<FlowPtr<Type>>;
    auto TypeGetAlignBitsImpl(const Type *self) -> std::optional<uint64_t>;
    auto TypeGetSizeBitsImpl(const Type *self) -> std::optional<uint64_t>;
    auto ExprGetCloneImpl(Expr *self) -> Expr *;
  };  // namespace detail

  using SrcLoc = parse::ASTExtension;

  template <class A>
  class GenericExpr {
    friend A;

    nr_ty_t m_node_type : 6; /* This node kind */
    uint8_t m_pad : 2;       /* Padding */
    SrcLoc m_loc;            /* Source location alias */

  public:
    GenericExpr(const GenericExpr &) = delete;
    auto operator=(const GenericExpr &) -> GenericExpr & = delete;

    constexpr GenericExpr(nr_ty_t ty, lex::LocationID begin = lex::LocationID(),
                          lex::LocationID end = lex::LocationID())
        : m_node_type(ty) {
      m_loc.SetSourceLocationBound(begin, end);
    }

    static constexpr auto GetKindSize(nr_ty_t type) -> uint32_t;
    static constexpr auto GetKindName(nr_ty_t type) -> const char *;

    [[nodiscard]] constexpr auto GetKind() const { return m_node_type; }
    [[nodiscard]] constexpr auto GetKindName() const -> const char * { return GetKindName(m_node_type); }

    template <typename T>
    static constexpr auto GetTypeCode() -> nr_ty_t {
      if constexpr (std::is_same_v<T, GenericBinary<A>>) {
        return IR_eBIN;
      } else if constexpr (std::is_same_v<T, GenericUnary<A>>) {
        return IR_eUNARY;
      } else if constexpr (std::is_same_v<T, GenericInt<A>>) {
        return IR_eINT;
      } else if constexpr (std::is_same_v<T, GenericFloat<A>>) {
        return IR_eFLOAT;
      } else if constexpr (std::is_same_v<T, GenericList<A>>) {
        return IR_eLIST;
      } else if constexpr (std::is_same_v<T, GenericCall<A>>) {
        return IR_eCALL;
      } else if constexpr (std::is_same_v<T, GenericSeq<A>>) {
        return IR_eSEQ;
      } else if constexpr (std::is_same_v<T, GenericIndex<A>>) {
        return IR_eINDEX;
      } else if constexpr (std::is_same_v<T, GenericIdentifier<A>>) {
        return IR_eIDENT;
      } else if constexpr (std::is_same_v<T, GenericExtern<A>>) {
        return IR_eEXTERN;
      } else if constexpr (std::is_same_v<T, GenericLocal<A>>) {
        return IR_eLOCAL;
      } else if constexpr (std::is_same_v<T, GenericRet<A>>) {
        return IR_eRET;
      } else if constexpr (std::is_same_v<T, GenericBrk<A>>) {
        return IR_eBRK;
      } else if constexpr (std::is_same_v<T, GenericCont<A>>) {
        return IR_eSKIP;
      } else if constexpr (std::is_same_v<T, GenericIf<A>>) {
        return IR_eIF;
      } else if constexpr (std::is_same_v<T, GenericWhile<A>>) {
        return IR_eWHILE;
      } else if constexpr (std::is_same_v<T, GenericFor<A>>) {
        return IR_eFOR;
      } else if constexpr (std::is_same_v<T, GenericCase<A>>) {
        return IR_eCASE;
      } else if constexpr (std::is_same_v<T, GenericSwitch<A>>) {
        return IR_eSWITCH;
      } else if constexpr (std::is_same_v<T, GenericFunction<A>>) {
        return IR_eFUNCTION;
      } else if constexpr (std::is_same_v<T, GenericAsm<A>>) {
        return IR_eASM;
      } else if constexpr (std::is_same_v<T, GenericExpr<A>>) {
        return IR_eIGN;
      } else if constexpr (std::is_same_v<T, GenericU1Ty<A>>) {
        return IR_tU1;
      } else if constexpr (std::is_same_v<T, GenericU8Ty<A>>) {
        return IR_tU8;
      } else if constexpr (std::is_same_v<T, GenericU16Ty<A>>) {
        return IR_tU16;
      } else if constexpr (std::is_same_v<T, GenericU32Ty<A>>) {
        return IR_tU32;
      } else if constexpr (std::is_same_v<T, GenericU64Ty<A>>) {
        return IR_tU64;
      } else if constexpr (std::is_same_v<T, GenericU128Ty<A>>) {
        return IR_tU128;
      } else if constexpr (std::is_same_v<T, GenericI8Ty<A>>) {
        return IR_tI8;
      } else if constexpr (std::is_same_v<T, GenericI16Ty<A>>) {
        return IR_tI16;
      } else if constexpr (std::is_same_v<T, GenericI32Ty<A>>) {
        return IR_tI32;
      } else if constexpr (std::is_same_v<T, GenericI64Ty<A>>) {
        return IR_tI64;
      } else if constexpr (std::is_same_v<T, GenericI128Ty<A>>) {
        return IR_tI128;
      } else if constexpr (std::is_same_v<T, GenericF16Ty<A>>) {
        return IR_tF16_TY;
      } else if constexpr (std::is_same_v<T, GenericF32Ty<A>>) {
        return IR_tF32_TY;
      } else if constexpr (std::is_same_v<T, GenericF64Ty<A>>) {
        return IR_tF64_TY;
      } else if constexpr (std::is_same_v<T, GenericF128Ty<A>>) {
        return IR_tF128_TY;
      } else if constexpr (std::is_same_v<T, GenericVoidTy<A>>) {
        return IR_tVOID;
      } else if constexpr (std::is_same_v<T, GenericPtrTy<A>>) {
        return IR_tPTR;
      } else if constexpr (std::is_same_v<T, GenericConstTy<A>>) {
        return IR_tCONST;
      } else if constexpr (std::is_same_v<T, GenericOpaqueTy<A>>) {
        return IR_tOPAQUE;
      } else if constexpr (std::is_same_v<T, GenericStructTy<A>>) {
        return IR_tSTRUCT;
      } else if constexpr (std::is_same_v<T, GenericUnionTy<A>>) {
        return IR_tUNION;
      } else if constexpr (std::is_same_v<T, GenericArrayTy<A>>) {
        return IR_tARRAY;
      } else if constexpr (std::is_same_v<T, GenericFnTy<A>>) {
        return IR_tFUNC;
      } else if constexpr (std::is_same_v<T, GenericTmp<A>>) {
        return IR_tTMP;
      } else {
        static_assert(!std::is_same_v<T, T>, "The requested type target is not supported by this function.");
      }
    }

    [[nodiscard]] constexpr auto IsType() const -> bool {
      switch (GetKind()) {
        case IR_tU1:
        case IR_tU8:
        case IR_tU16:
        case IR_tU32:
        case IR_tU64:
        case IR_tU128:
        case IR_tI8:
        case IR_tI16:
        case IR_tI32:
        case IR_tI64:
        case IR_tI128:
        case IR_tF16_TY:
        case IR_tF32_TY:
        case IR_tF64_TY:
        case IR_tF128_TY:
        case IR_tVOID:
        case IR_tPTR:
        case IR_tCONST:
        case IR_tOPAQUE:
        case IR_tSTRUCT:
        case IR_tUNION:
        case IR_tARRAY:
        case IR_tFUNC:
        case IR_tTMP:
          return true;
        default:
          return false;
      }
    }

    [[nodiscard]] constexpr auto IsLiteral() const -> bool {
      return m_node_type == IR_eINT || m_node_type == IR_eFLOAT;
    }

    [[nodiscard]] constexpr auto GetName() const -> std::string_view;

    [[nodiscard]] constexpr auto Begin() const { return m_loc.GetSourceLocationBound().first; }
    constexpr auto Begin(lex::IScanner &rd) const { return Begin().Get(rd); }

    [[nodiscard]] constexpr auto End() const { return m_loc.GetSourceLocationBound().second; }
    constexpr auto End(lex::IScanner &rd) const { return End().Get(rd); }

    [[nodiscard]] constexpr auto GetLoc() const { return m_loc; }
    constexpr void SetLoc(SrcLoc loc) { m_loc = loc; }

    constexpr void SetLoc(lex::LocationID begin, lex::LocationID end) { m_loc.SetSourceLocationBound(begin, end); }

    auto GetType() -> std::optional<FlowPtr<Type>> { return detail::ExprGetType(reinterpret_cast<Expr *>(this)); }

    template <typename T>
    static constexpr auto SafeCastAs(GenericExpr<A> *ptr) -> T * {
      if (!ptr) {
        return nullptr;
      }

#ifndef NDEBUG
      if constexpr (std::is_same_v<GenericType<A>, T>) {
        if (!ptr->IsType()) [[unlikely]] {
          qcore_panicf("Invalid cast from non-type %s to type", ptr->GetKindName());
        }
      } else if constexpr (!std::is_same_v<GenericExpr<A>, T>) {
        if (GetTypeCode<T>() != ptr->GetKind()) [[unlikely]] {
          qcore_panicf("Invalid cast from %s to %s", ptr->GetKindName(), GetKindName(GetTypeCode<T>()));
        }
      }
#endif

      return reinterpret_cast<T *>(ptr);
    }

    template <typename T>
    [[nodiscard]] constexpr T *As() {
      return SafeCastAs<T>(this);
    }

    template <typename T>
    constexpr const T *As() const {
      return SafeCastAs<T>(const_cast<GenericExpr<A> *>(this));
    }

    template <class T = GenericExpr<A>>
    constexpr auto Clone() -> T * {
      return detail::ExprGetCloneImpl(reinterpret_cast<Expr *>(this))->template As<T>();
    }

    constexpr auto AsExpr() -> GenericExpr * { return this; }
    constexpr auto AsType() -> GenericType<A> *;
    [[nodiscard]] constexpr auto AsType() const -> const GenericType<A> * {
      return const_cast<GenericExpr<A> *>(this)->AsType();
    }

    [[nodiscard]] constexpr bool Is(nr_ty_t type) const { return type == GetKind(); }

    [[nodiscard]] constexpr auto IsEq(const GenericExpr<A> *other) const -> bool;

    constexpr void Accept(IRVisitor<A> &v) { v.template Dispatch(MakeFlowPtr(this)); }

    void Dump(std::ostream &os = std::cout, bool is_for_debug = false) const {
      detail::NodeDumpImpl(reinterpret_cast<const Expr *>(this), os, is_for_debug);
    }

    [[nodiscard]] auto ToString() const -> std::string {
      std::stringstream ss;
      Dump(ss, false);
      return ss.str();
    };
  } __attribute__((packed)) __attribute__((aligned(1)));

  static_assert(sizeof(GenericExpr<void>) == 8, "GenericExpr<void> is not 8 bytes in size.");

  template <class A>
  class GenericType : public GenericExpr<A> {
    friend A;

  public:
    constexpr GenericType(nr_ty_t ty) : GenericExpr<A>(ty) {}

    [[nodiscard]] auto GetSizeBits() const -> std::optional<uint64_t> {
      return detail::TypeGetSizeBitsImpl(reinterpret_cast<const Type *>(this));
    }

    [[nodiscard]] auto GetSizeBytes() const -> std::optional<uint64_t> {
      if (auto size = GetSizeBits()) [[likely]] {
        return std::ceil(size.value() / 8.0);
      } else {
        return std::nullopt;
      }
    }

    [[nodiscard]] auto GetAlignBits() const -> std::optional<uint64_t> {
      return detail::TypeGetAlignBitsImpl(reinterpret_cast<const Type *>(this));
    }

    [[nodiscard]] auto GetAlignBytes() const -> std::optional<uint64_t> {
      if (auto align = GetAlignBits()) [[likely]] {
        return std::ceil(align.value() / 8.0);
      } else {
        return std::nullopt;
      }
    }

    [[nodiscard]] constexpr auto IsPrimitive() const -> bool {
      switch (this->GetKind()) {
        case IR_tU1:
        case IR_tU8:
        case IR_tU16:
        case IR_tU32:
        case IR_tU64:
        case IR_tU128:
        case IR_tI8:
        case IR_tI16:
        case IR_tI32:
        case IR_tI64:
        case IR_tI128:
        case IR_tF16_TY:
        case IR_tF32_TY:
        case IR_tF64_TY:
        case IR_tF128_TY:
        case IR_tVOID:
          return true;
        default:
          return false;
      }
    }

    [[nodiscard]] constexpr auto IsArray() const -> bool { return this->GetKind() == IR_tARRAY; }
    [[nodiscard]] constexpr auto IsPointer() const -> bool { return this->GetKind() == IR_tPTR; }
    [[nodiscard]] constexpr auto IsReadonly() const -> bool { return this->GetKind() == IR_tCONST; }
    [[nodiscard]] constexpr auto IsFunction() const -> bool { return this->GetKind() == IR_tFUNC; }

    [[nodiscard]] constexpr auto IsComposite() const -> bool {
      switch (this->GetKind()) {
        case IR_tSTRUCT:
        case IR_tUNION:
        case IR_tARRAY:
          return true;
        default:
          return false;
      }
    }

    [[nodiscard]] constexpr auto IsUnion() const -> bool { return this->GetKind() == IR_tUNION; }

    [[nodiscard]] constexpr auto IsNumeric() const -> bool {
      switch (this->GetKind()) {
        case IR_tU1:
        case IR_tU8:
        case IR_tU16:
        case IR_tU32:
        case IR_tU64:
        case IR_tU128:
        case IR_tI8:
        case IR_tI16:
        case IR_tI32:
        case IR_tI64:
        case IR_tI128:
        case IR_tF16_TY:
        case IR_tF32_TY:
        case IR_tF64_TY:
        case IR_tF128_TY:
          return true;
        default:
          return false;
      }
    }

    [[nodiscard]] constexpr auto IsIntegral() const -> bool {
      switch (this->GetKind()) {
        case IR_tU1:
        case IR_tU8:
        case IR_tU16:
        case IR_tU32:
        case IR_tU64:
        case IR_tU128:
        case IR_tI8:
        case IR_tI16:
        case IR_tI32:
        case IR_tI64:
        case IR_tI128:
          return true;
        default:
          return false;
      }
    }

    [[nodiscard]] constexpr auto IsFloatingPoint() const -> bool {
      switch (this->GetKind()) {
        case IR_tF16_TY:
        case IR_tF32_TY:
        case IR_tF64_TY:
        case IR_tF128_TY:
          return true;
        default:
          return false;
      }
    }

    [[nodiscard]] constexpr auto IsSigned() const -> bool {
      switch (this->GetKind()) {
        case IR_tI8:
        case IR_tI16:
        case IR_tI32:
        case IR_tI64:
        case IR_tI128:
        case IR_tF16_TY:
        case IR_tF32_TY:
        case IR_tF64_TY:
        case IR_tF128_TY:
          return true;
        default:
          return false;
      }
    }

    [[nodiscard]] constexpr auto IsUnsigned() const -> bool {
      switch (this->GetKind()) {
        case IR_tU1:
        case IR_tU8:
        case IR_tU16:
        case IR_tU32:
        case IR_tU64:
        case IR_tU128:
          return true;
        default:
          return false;
      }
    }

    [[nodiscard]] constexpr auto IsVoid() const -> bool { return this->GetKind() == IR_tVOID; }
    [[nodiscard]] constexpr auto IsBool() const -> bool { return this->GetKind() == IR_tU1; }
  };

  template <class A>
  constexpr auto GenericExpr<A>::AsType() -> GenericType<A> * {
#ifndef NDEBUG
    if (!IsType()) {
      qcore_panicf("Failed to cast a non-type node `%s` to a type node", GetKindName());
    }
#endif
    return static_cast<GenericType<A> *>(this);
  }

  template <class A>
  constexpr auto GenericExpr<A>::GetName() const -> std::string_view {
    std::string_view r;

    switch (this->GetKind()) {
      case IR_eBIN: {
        break;
      }

      case IR_eUNARY: {
        break;
      }

      case IR_eINT: {
        break;
      }

      case IR_eFLOAT: {
        break;
      }

      case IR_eLIST: {
        break;
      }

      case IR_eCALL: {
        break;
      }

      case IR_eSEQ: {
        break;
      }

      case IR_eINDEX: {
        break;
      }

      case IR_eIDENT: {
        r = As<GenericIdentifier<A>>()->GetName();
        break;
      }

      case IR_eEXTERN: {
        break;
      }

      case IR_eLOCAL: {
        r = As<GenericLocal<A>>()->GetName();
        break;
      }

      case IR_eRET: {
        break;
      }

      case IR_eBRK: {
        break;
      }

      case IR_eSKIP: {
        break;
      }

      case IR_eIF: {
        break;
      }

      case IR_eWHILE: {
        break;
      }

      case IR_eFOR: {
        break;
      }

      case IR_eCASE: {
        break;
      }

      case IR_eSWITCH: {
        break;
      }

      case IR_eIGN: {
        break;
      }

      case IR_eFUNCTION: {
        r = As<GenericFunction<A>>()->GetName();
        break;
      }

      case IR_eASM: {
        break;
      }

      case IR_tU1: {
        break;
      }

      case IR_tU8: {
        break;
      }

      case IR_tU16: {
        break;
      }

      case IR_tU32: {
        break;
      }

      case IR_tU64: {
        break;
      }

      case IR_tU128: {
        break;
      }

      case IR_tI8: {
        break;
      }

      case IR_tI16: {
        break;
      }

      case IR_tI32: {
        break;
      }

      case IR_tI64: {
        break;
      }

      case IR_tI128: {
        break;
      }

      case IR_tF16_TY: {
        break;
      }

      case IR_tF32_TY: {
        break;
      }

      case IR_tF64_TY: {
        break;
      }

      case IR_tF128_TY: {
        break;
      }

      case IR_tVOID: {
        break;
      }

      case IR_tPTR: {
        break;
      }

      case IR_tCONST: {
        r = As<GenericConstTy<A>>()->GetItem()->GetName();
        break;
      }

      case IR_tOPAQUE: {
        r = As<GenericOpaqueTy<A>>()->GetName();
        break;
      }

      case IR_tSTRUCT: {
        break;
      }

      case IR_tUNION: {
        break;
      }

      case IR_tARRAY: {
        break;
      }

      case IR_tFUNC: {
        break;
      }

      case IR_tTMP: {
        break;
      }
    }

    return r;
  }

  template <class A>
  constexpr auto GenericExpr<A>::GetKindSize(nr_ty_t type) -> uint32_t {
    const std::array<size_t, kIRNodeKindCount> sizes = []() {
      std::array<size_t, kIRNodeKindCount> r;
      r.fill(0);

      r[IR_eBIN] = sizeof(GenericBinary<A>);
      r[IR_eUNARY] = sizeof(GenericUnary<A>);
      r[IR_eINT] = sizeof(GenericInt<A>);
      r[IR_eFLOAT] = sizeof(GenericFloat<A>);
      r[IR_eLIST] = sizeof(GenericList<A>);
      r[IR_eCALL] = sizeof(GenericCall<A>);
      r[IR_eSEQ] = sizeof(GenericSeq<A>);
      r[IR_eINDEX] = sizeof(GenericIndex<A>);
      r[IR_eIDENT] = sizeof(GenericIdentifier<A>);
      r[IR_eEXTERN] = sizeof(GenericExtern<A>);
      r[IR_eLOCAL] = sizeof(GenericLocal<A>);
      r[IR_eRET] = sizeof(GenericRet<A>);
      r[IR_eBRK] = sizeof(GenericBrk<A>);
      r[IR_eSKIP] = sizeof(GenericCont<A>);
      r[IR_eIF] = sizeof(GenericIf<A>);
      r[IR_eWHILE] = sizeof(GenericWhile<A>);
      r[IR_eFOR] = sizeof(GenericFor<A>);
      r[IR_eCASE] = sizeof(GenericCase<A>);
      r[IR_eSWITCH] = sizeof(GenericSwitch<A>);
      r[IR_eFUNCTION] = sizeof(GenericFunction<A>);
      r[IR_eASM] = sizeof(GenericAsm<A>);
      r[IR_eIGN] = sizeof(GenericExpr<A>);
      r[IR_tU1] = sizeof(GenericU1Ty<A>);
      r[IR_tU8] = sizeof(GenericU8Ty<A>);
      r[IR_tU16] = sizeof(GenericU16Ty<A>);
      r[IR_tU32] = sizeof(GenericU32Ty<A>);
      r[IR_tU64] = sizeof(GenericU64Ty<A>);
      r[IR_tU128] = sizeof(GenericU128Ty<A>);
      r[IR_tI8] = sizeof(GenericI8Ty<A>);
      r[IR_tI16] = sizeof(GenericI16Ty<A>);
      r[IR_tI32] = sizeof(GenericI32Ty<A>);
      r[IR_tI64] = sizeof(GenericI64Ty<A>);
      r[IR_tI128] = sizeof(GenericI128Ty<A>);
      r[IR_tF16_TY] = sizeof(GenericF16Ty<A>);
      r[IR_tF32_TY] = sizeof(GenericF32Ty<A>);
      r[IR_tF64_TY] = sizeof(GenericF64Ty<A>);
      r[IR_tF128_TY] = sizeof(GenericF128Ty<A>);
      r[IR_tVOID] = sizeof(GenericVoidTy<A>);
      r[IR_tPTR] = sizeof(GenericPtrTy<A>);
      r[IR_tCONST] = sizeof(GenericConstTy<A>);
      r[IR_tOPAQUE] = sizeof(GenericOpaqueTy<A>);
      r[IR_tSTRUCT] = sizeof(GenericStructTy<A>);
      r[IR_tUNION] = sizeof(GenericUnionTy<A>);
      r[IR_tARRAY] = sizeof(GenericArrayTy<A>);
      r[IR_tFUNC] = sizeof(GenericFnTy<A>);
      r[IR_tTMP] = sizeof(GenericTmp<A>);

      return r;
    }();

    return sizes[type];
  }

  template <class A>
  constexpr auto GenericExpr<A>::GetKindName(nr_ty_t type) -> const char * {
    const std::array<const char *, kIRNodeKindCount> names = []() {
      std::array<const char *, kIRNodeKindCount> r;
      r.fill("");

      r[IR_eBIN] = "bin_expr";
      r[IR_eUNARY] = "unary_expr";
      r[IR_eINT] = "int";
      r[IR_eFLOAT] = "float";
      r[IR_eLIST] = "list";
      r[IR_eCALL] = "call";
      r[IR_eSEQ] = "seq";
      r[IR_eINDEX] = "index";
      r[IR_eIDENT] = "ident";
      r[IR_eEXTERN] = "extern";
      r[IR_eLOCAL] = "local";
      r[IR_eRET] = "return";
      r[IR_eBRK] = "break";
      r[IR_eSKIP] = "continue";
      r[IR_eIF] = "if";
      r[IR_eWHILE] = "while";
      r[IR_eFOR] = "for";
      r[IR_eCASE] = "case";
      r[IR_eSWITCH] = "switch";
      r[IR_eFUNCTION] = "fn";
      r[IR_eASM] = "asm";
      r[IR_eIGN] = "ignore";
      r[IR_tU1] = "u1";
      r[IR_tU8] = "u8";
      r[IR_tU16] = "u16";
      r[IR_tU32] = "u32";
      r[IR_tU64] = "u64";
      r[IR_tU128] = "u128";
      r[IR_tI8] = "i8";
      r[IR_tI16] = "i16";
      r[IR_tI32] = "i32";
      r[IR_tI64] = "i64";
      r[IR_tI128] = "i128";
      r[IR_tF16_TY] = "f16";
      r[IR_tF32_TY] = "f32";
      r[IR_tF64_TY] = "f64";
      r[IR_tF128_TY] = "f128";
      r[IR_tVOID] = "void";
      r[IR_tPTR] = "ptr";
      r[IR_tCONST] = "const";
      r[IR_tOPAQUE] = "opaque";
      r[IR_tSTRUCT] = "struct";
      r[IR_tUNION] = "union";
      r[IR_tARRAY] = "array";
      r[IR_tFUNC] = "fn_ty";
      r[IR_tTMP] = "tmp";

      return r;
    }();

    return names[type];
  }

  template <class A>
  constexpr auto GenericExpr<A>::IsEq(const GenericExpr<A> *other) const -> bool {
    nr_ty_t kind = GetKind();

    if (kind != other->GetKind()) {
      return false;
    }

    qcore_implement();

    qcore_panic("unreachable");
  }
}  // namespace ncc::ir

#endif
