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
    void NodeDumpImpl(const Expr *E, std::ostream &os, bool isForDebug);
    bool IsAcyclicImpl(FlowPtr<Expr> E);

    std::optional<FlowPtr<Type>> Expr_getType(Expr *E);
    std::optional<uint64_t> Type_getAlignBitsImpl(const Type *self);
    std::optional<uint64_t> Type_getSizeBitsImpl(const Type *self);
    Expr *Expr_getCloneImpl(Expr *self);
  };  // namespace detail

  using SrcLoc = parse::LocationPairAlias::Index;

  template <class A>
  class IR_Vertex_Expr {
    friend A;

    nr_ty_t m_node_type : 6; /* This node kind */
    uint8_t pad : 2;         /* Padding */
    SrcLoc m_loc;            /* Source location alias */

    IR_Vertex_Expr(const IR_Vertex_Expr &) = delete;
    IR_Vertex_Expr &operator=(const IR_Vertex_Expr &) = delete;

  public:
    constexpr IR_Vertex_Expr(nr_ty_t ty,
                             lex::LocationID begin = lex::LocationID(),
                             lex::LocationID end = lex::LocationID())
        : m_node_type(ty) {
      m_loc = parse::g_location_pairs.Add(begin, end);
    }

    static constexpr uint32_t getKindSize(nr_ty_t kind);
    static constexpr const char *getKindName(nr_ty_t kind);

    constexpr auto getKind() const { return m_node_type; }
    constexpr const char *getKindName() const {
      return getKindName(m_node_type);
    }

    template <typename T>
    static constexpr nr_ty_t getTypeCode() {
      if constexpr (std::is_same_v<T, IR_Vertex_BinExpr<A>>) {
        return IR_eBIN;
      } else if constexpr (std::is_same_v<T, IR_Vertex_Unary<A>>) {
        return IR_eUNARY;
      } else if constexpr (std::is_same_v<T, IR_Vertex_Int<A>>) {
        return IR_eINT;
      } else if constexpr (std::is_same_v<T, IR_Vertex_Float<A>>) {
        return IR_eFLOAT;
      } else if constexpr (std::is_same_v<T, IR_Vertex_List<A>>) {
        return IR_eLIST;
      } else if constexpr (std::is_same_v<T, IR_Vertex_Call<A>>) {
        return IR_eCALL;
      } else if constexpr (std::is_same_v<T, IR_Vertex_Seq<A>>) {
        return IR_eSEQ;
      } else if constexpr (std::is_same_v<T, IR_Vertex_Index<A>>) {
        return IR_eINDEX;
      } else if constexpr (std::is_same_v<T, IR_Vertex_Ident<A>>) {
        return IR_eIDENT;
      } else if constexpr (std::is_same_v<T, IR_Vertex_Extern<A>>) {
        return IR_eEXTERN;
      } else if constexpr (std::is_same_v<T, IR_Vertex_Local<A>>) {
        return IR_eLOCAL;
      } else if constexpr (std::is_same_v<T, IR_Vertex_Ret<A>>) {
        return IR_eRET;
      } else if constexpr (std::is_same_v<T, IR_Vertex_Brk<A>>) {
        return IR_eBRK;
      } else if constexpr (std::is_same_v<T, IR_Vertex_Cont<A>>) {
        return IR_eSKIP;
      } else if constexpr (std::is_same_v<T, IR_Vertex_If<A>>) {
        return IR_eIF;
      } else if constexpr (std::is_same_v<T, IR_Vertex_While<A>>) {
        return IR_eWHILE;
      } else if constexpr (std::is_same_v<T, IR_Vertex_For<A>>) {
        return IR_eFOR;
      } else if constexpr (std::is_same_v<T, IR_Vertex_Case<A>>) {
        return IR_eCASE;
      } else if constexpr (std::is_same_v<T, IR_Vertex_Switch<A>>) {
        return IR_eSWITCH;
      } else if constexpr (std::is_same_v<T, IR_Vertex_Function<A>>) {
        return IR_eFUNCTION;
      } else if constexpr (std::is_same_v<T, IR_Vertex_Asm<A>>) {
        return IR_eASM;
      } else if constexpr (std::is_same_v<T, IR_Vertex_Expr<A>>) {
        return IR_eIGN;
      } else if constexpr (std::is_same_v<T, IR_Vertex_U1Ty<A>>) {
        return IR_tU1;
      } else if constexpr (std::is_same_v<T, IR_Vertex_U8Ty<A>>) {
        return IR_tU8;
      } else if constexpr (std::is_same_v<T, IR_Vertex_U16Ty<A>>) {
        return IR_tU16;
      } else if constexpr (std::is_same_v<T, IR_Vertex_U32Ty<A>>) {
        return IR_tU32;
      } else if constexpr (std::is_same_v<T, IR_Vertex_U64Ty<A>>) {
        return IR_tU64;
      } else if constexpr (std::is_same_v<T, IR_Vertex_U128Ty<A>>) {
        return IR_tU128;
      } else if constexpr (std::is_same_v<T, IR_Vertex_I8Ty<A>>) {
        return IR_tI8;
      } else if constexpr (std::is_same_v<T, IR_Vertex_I16Ty<A>>) {
        return IR_tI16;
      } else if constexpr (std::is_same_v<T, IR_Vertex_I32Ty<A>>) {
        return IR_tI32;
      } else if constexpr (std::is_same_v<T, IR_Vertex_I64Ty<A>>) {
        return IR_tI64;
      } else if constexpr (std::is_same_v<T, IR_Vertex_I128Ty<A>>) {
        return IR_tI128;
      } else if constexpr (std::is_same_v<T, IR_Vertex_F16Ty<A>>) {
        return IR_tF16_TY;
      } else if constexpr (std::is_same_v<T, IR_Vertex_F32Ty<A>>) {
        return IR_tF32_TY;
      } else if constexpr (std::is_same_v<T, IR_Vertex_F64Ty<A>>) {
        return IR_tF64_TY;
      } else if constexpr (std::is_same_v<T, IR_Vertex_F128Ty<A>>) {
        return IR_tF128_TY;
      } else if constexpr (std::is_same_v<T, IR_Vertex_VoidTy<A>>) {
        return IR_tVOID;
      } else if constexpr (std::is_same_v<T, IR_Vertex_PtrTy<A>>) {
        return IR_tPTR;
      } else if constexpr (std::is_same_v<T, IR_Vertex_ConstTy<A>>) {
        return IR_tCONST;
      } else if constexpr (std::is_same_v<T, IR_Vertex_OpaqueTy<A>>) {
        return IR_tOPAQUE;
      } else if constexpr (std::is_same_v<T, IR_Vertex_StructTy<A>>) {
        return IR_tSTRUCT;
      } else if constexpr (std::is_same_v<T, IR_Vertex_UnionTy<A>>) {
        return IR_tUNION;
      } else if constexpr (std::is_same_v<T, IR_Vertex_ArrayTy<A>>) {
        return IR_tARRAY;
      } else if constexpr (std::is_same_v<T, IR_Vertex_FnTy<A>>) {
        return IR_tFUNC;
      } else if constexpr (std::is_same_v<T, IR_Vertex_Tmp<A>>) {
        return IR_tTMP;
      } else {
        static_assert(
            !std::is_same_v<T, T>,
            "The requested type target is not supported by this function.");
      }
    }

    constexpr bool isType() const {
      switch (getKind()) {
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

    constexpr bool isLiteral() const {
      return m_node_type == IR_eINT || m_node_type == IR_eFLOAT;
    }

    constexpr std::string_view getName() const;

    constexpr auto begin() const {
      return parse::g_location_pairs.Get(m_loc).first;
    }
    constexpr auto begin(lex::IScanner &rd) const { return begin().Get(rd); }

    constexpr auto end() const {
      return parse::g_location_pairs.Get(m_loc).second;
    }
    constexpr auto end(lex::IScanner &rd) const { return end().Get(rd); }

    constexpr auto getLoc() const { return m_loc; }
    constexpr void setLoc(SrcLoc loc) { m_loc = loc; }

    constexpr void setLoc(lex::LocationID begin, lex::LocationID end) {
      m_loc = parse::g_location_pairs.Add(begin, end);
    }

    std::optional<FlowPtr<Type>> getType() {
      return detail::Expr_getType(reinterpret_cast<Expr *>(this));
    }

    template <typename T>
    static constexpr T *safeCastAs(IR_Vertex_Expr<A> *ptr) {
      if (!ptr) {
        return nullptr;
      }

#ifndef NDEBUG
      if constexpr (std::is_same_v<IR_Vertex_Type<A>, T>) {
        if (!ptr->isType()) [[unlikely]] {
          qcore_panicf("Invalid cast from non-type %s to type",
                       ptr->getKindName());
        }
      } else if constexpr (!std::is_same_v<IR_Vertex_Expr<A>, T>) {
        if (getTypeCode<T>() != ptr->getKind()) [[unlikely]] {
          qcore_panicf("Invalid cast from %s to %s", ptr->getKindName(),
                       getKindName(getTypeCode<T>()));
        }
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
      return safeCastAs<T>(const_cast<IR_Vertex_Expr<A> *>(this));
    }

    template <class T = IR_Vertex_Expr<A>>
    constexpr T *clone() {
      return detail::Expr_getCloneImpl(reinterpret_cast<Expr *>(this))
          ->template as<T>();
    }

    constexpr IR_Vertex_Expr *asExpr() { return this; }
    constexpr IR_Vertex_Type<A> *asType();
    constexpr const IR_Vertex_Type<A> *asType() const {
      return const_cast<IR_Vertex_Expr<A> *>(this)->asType();
    }

    constexpr bool is(nr_ty_t type) const { return type == getKind(); }
    constexpr bool isSame(const IR_Vertex_Expr<A> *other) const;

    constexpr void accept(IRVisitor<A> &v) {
      v.template dispatch(MakeFlowPtr(this));
    }

    void dump(std::ostream &os = std::cout, bool isForDebug = false) const {
      detail::NodeDumpImpl(reinterpret_cast<const Expr *>(this), os,
                           isForDebug);
    }

    std::string toString() const {
      std::stringstream ss;
      dump(ss, false);
      return ss.str();
    };
  } __attribute__((packed)) __attribute__((aligned(1)));

  static_assert(sizeof(IR_Vertex_Expr<void>) == 8,
                "IR_Vertex_Expr<void> is not 8 bytes in size.");

  template <class A>
  class IR_Vertex_Type : public IR_Vertex_Expr<A> {
    friend A;

  public:
    constexpr IR_Vertex_Type(nr_ty_t ty) : IR_Vertex_Expr<A>(ty) {}

    std::optional<uint64_t> getSizeBits() const {
      return detail::Type_getSizeBitsImpl(reinterpret_cast<const Type *>(this));
    }

    std::optional<uint64_t> getSizeBytes() const {
      if (auto size = getSizeBits()) [[likely]] {
        return std::ceil(size.value() / 8.0);
      } else {
        return std::nullopt;
      }
    }

    std::optional<uint64_t> getAlignBits() const {
      return detail::Type_getAlignBitsImpl(
          reinterpret_cast<const Type *>(this));
    }

    std::optional<uint64_t> getAlignBytes() const {
      if (auto align = getAlignBits()) [[likely]] {
        return std::ceil(align.value() / 8.0);
      } else {
        return std::nullopt;
      }
    }

    constexpr bool is_primitive() const {
      switch (this->getKind()) {
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

    constexpr bool is_array() const { return this->getKind() == IR_tARRAY; }
    constexpr bool is_pointer() const { return this->getKind() == IR_tPTR; }
    constexpr bool is_readonly() const { return this->getKind() == IR_tCONST; }
    constexpr bool is_function() const { return this->getKind() == IR_tFUNC; }

    constexpr bool is_composite() const {
      switch (this->getKind()) {
        case IR_tSTRUCT:
        case IR_tUNION:
        case IR_tARRAY:
          return true;
        default:
          return false;
      }
    }

    constexpr bool is_union() const { return this->getKind() == IR_tUNION; }

    constexpr bool is_numeric() const {
      switch (this->getKind()) {
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

    constexpr bool is_integral() const {
      switch (this->getKind()) {
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

    constexpr bool is_floating_point() const {
      switch (this->getKind()) {
        case IR_tF16_TY:
        case IR_tF32_TY:
        case IR_tF64_TY:
        case IR_tF128_TY:
          return true;
        default:
          return false;
      }
    }

    constexpr bool is_signed() const {
      switch (this->getKind()) {
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

    constexpr bool is_unsigned() const {
      switch (this->getKind()) {
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

    constexpr bool is_void() const { return this->getKind() == IR_tVOID; }
    constexpr bool is_bool() const { return this->getKind() == IR_tU1; }
  };

  template <class A>
  constexpr IR_Vertex_Type<A> *IR_Vertex_Expr<A>::asType() {
#ifndef NDEBUG
    if (!isType()) {
      qcore_panicf("Failed to cast a non-type node `%s` to a type node",
                   getKindName());
    }
#endif
    return static_cast<IR_Vertex_Type<A> *>(this);
  }

  template <class A>
  constexpr std::string_view IR_Vertex_Expr<A>::getName() const {
    std::string_view R = "";

    switch (this->getKind()) {
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
        R = as<IR_Vertex_Ident<A>>()->getName();
        break;
      }

      case IR_eEXTERN: {
        break;
      }

      case IR_eLOCAL: {
        R = as<IR_Vertex_Local<A>>()->getName();
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
        R = as<IR_Vertex_Function<A>>()->getName();
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
        R = as<IR_Vertex_ConstTy<A>>()->getItem()->getName();
        break;
      }

      case IR_tOPAQUE: {
        R = as<IR_Vertex_OpaqueTy<A>>()->getName();
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

    return R;
  }

  template <class A>
  constexpr uint32_t IR_Vertex_Expr<A>::getKindSize(nr_ty_t type) {
    const std::array<size_t, IR_COUNT> sizes = []() {
      std::array<size_t, IR_COUNT> R;
      R.fill(0);

      R[IR_eBIN] = sizeof(IR_Vertex_BinExpr<A>);
      R[IR_eUNARY] = sizeof(IR_Vertex_Unary<A>);
      R[IR_eINT] = sizeof(IR_Vertex_Int<A>);
      R[IR_eFLOAT] = sizeof(IR_Vertex_Float<A>);
      R[IR_eLIST] = sizeof(IR_Vertex_List<A>);
      R[IR_eCALL] = sizeof(IR_Vertex_Call<A>);
      R[IR_eSEQ] = sizeof(IR_Vertex_Seq<A>);
      R[IR_eINDEX] = sizeof(IR_Vertex_Index<A>);
      R[IR_eIDENT] = sizeof(IR_Vertex_Ident<A>);
      R[IR_eEXTERN] = sizeof(IR_Vertex_Extern<A>);
      R[IR_eLOCAL] = sizeof(IR_Vertex_Local<A>);
      R[IR_eRET] = sizeof(IR_Vertex_Ret<A>);
      R[IR_eBRK] = sizeof(IR_Vertex_Brk<A>);
      R[IR_eSKIP] = sizeof(IR_Vertex_Cont<A>);
      R[IR_eIF] = sizeof(IR_Vertex_If<A>);
      R[IR_eWHILE] = sizeof(IR_Vertex_While<A>);
      R[IR_eFOR] = sizeof(IR_Vertex_For<A>);
      R[IR_eCASE] = sizeof(IR_Vertex_Case<A>);
      R[IR_eSWITCH] = sizeof(IR_Vertex_Switch<A>);
      R[IR_eFUNCTION] = sizeof(IR_Vertex_Function<A>);
      R[IR_eASM] = sizeof(IR_Vertex_Asm<A>);
      R[IR_eIGN] = sizeof(IR_Vertex_Expr<A>);
      R[IR_tU1] = sizeof(IR_Vertex_U1Ty<A>);
      R[IR_tU8] = sizeof(IR_Vertex_U8Ty<A>);
      R[IR_tU16] = sizeof(IR_Vertex_U16Ty<A>);
      R[IR_tU32] = sizeof(IR_Vertex_U32Ty<A>);
      R[IR_tU64] = sizeof(IR_Vertex_U64Ty<A>);
      R[IR_tU128] = sizeof(IR_Vertex_U128Ty<A>);
      R[IR_tI8] = sizeof(IR_Vertex_I8Ty<A>);
      R[IR_tI16] = sizeof(IR_Vertex_I16Ty<A>);
      R[IR_tI32] = sizeof(IR_Vertex_I32Ty<A>);
      R[IR_tI64] = sizeof(IR_Vertex_I64Ty<A>);
      R[IR_tI128] = sizeof(IR_Vertex_I128Ty<A>);
      R[IR_tF16_TY] = sizeof(IR_Vertex_F16Ty<A>);
      R[IR_tF32_TY] = sizeof(IR_Vertex_F32Ty<A>);
      R[IR_tF64_TY] = sizeof(IR_Vertex_F64Ty<A>);
      R[IR_tF128_TY] = sizeof(IR_Vertex_F128Ty<A>);
      R[IR_tVOID] = sizeof(IR_Vertex_VoidTy<A>);
      R[IR_tPTR] = sizeof(IR_Vertex_PtrTy<A>);
      R[IR_tCONST] = sizeof(IR_Vertex_ConstTy<A>);
      R[IR_tOPAQUE] = sizeof(IR_Vertex_OpaqueTy<A>);
      R[IR_tSTRUCT] = sizeof(IR_Vertex_StructTy<A>);
      R[IR_tUNION] = sizeof(IR_Vertex_UnionTy<A>);
      R[IR_tARRAY] = sizeof(IR_Vertex_ArrayTy<A>);
      R[IR_tFUNC] = sizeof(IR_Vertex_FnTy<A>);
      R[IR_tTMP] = sizeof(IR_Vertex_Tmp<A>);

      return R;
    }();

    return sizes[type];
  }

  template <class A>
  constexpr const char *IR_Vertex_Expr<A>::getKindName(nr_ty_t type) {
    const std::array<const char *, IR_COUNT> names = []() {
      std::array<const char *, IR_COUNT> R;
      R.fill("");

      R[IR_eBIN] = "bin_expr";
      R[IR_eUNARY] = "unary_expr";
      R[IR_eINT] = "int";
      R[IR_eFLOAT] = "float";
      R[IR_eLIST] = "list";
      R[IR_eCALL] = "call";
      R[IR_eSEQ] = "seq";
      R[IR_eINDEX] = "index";
      R[IR_eIDENT] = "ident";
      R[IR_eEXTERN] = "extern";
      R[IR_eLOCAL] = "local";
      R[IR_eRET] = "return";
      R[IR_eBRK] = "break";
      R[IR_eSKIP] = "continue";
      R[IR_eIF] = "if";
      R[IR_eWHILE] = "while";
      R[IR_eFOR] = "for";
      R[IR_eCASE] = "case";
      R[IR_eSWITCH] = "switch";
      R[IR_eFUNCTION] = "fn";
      R[IR_eASM] = "asm";
      R[IR_eIGN] = "ignore";
      R[IR_tU1] = "u1";
      R[IR_tU8] = "u8";
      R[IR_tU16] = "u16";
      R[IR_tU32] = "u32";
      R[IR_tU64] = "u64";
      R[IR_tU128] = "u128";
      R[IR_tI8] = "i8";
      R[IR_tI16] = "i16";
      R[IR_tI32] = "i32";
      R[IR_tI64] = "i64";
      R[IR_tI128] = "i128";
      R[IR_tF16_TY] = "f16";
      R[IR_tF32_TY] = "f32";
      R[IR_tF64_TY] = "f64";
      R[IR_tF128_TY] = "f128";
      R[IR_tVOID] = "void";
      R[IR_tPTR] = "ptr";
      R[IR_tCONST] = "const";
      R[IR_tOPAQUE] = "opaque";
      R[IR_tSTRUCT] = "struct";
      R[IR_tUNION] = "union";
      R[IR_tARRAY] = "array";
      R[IR_tFUNC] = "fn_ty";
      R[IR_tTMP] = "tmp";

      return R;
    }();

    return names[type];
  }

  template <class A>
  constexpr bool IR_Vertex_Expr<A>::isSame(
      const IR_Vertex_Expr<A> *other) const {
    nr_ty_t kind = getKind();

    if (kind != other->getKind()) {
      return false;
    }

    qcore_implement();

    qcore_panic("unreachable");
  }
}  // namespace ncc::ir

#endif
