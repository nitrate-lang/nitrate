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

#ifndef __NITRATE_IR_NODE_H__
#define __NITRATE_IR_NODE_H__

#include <boost/multiprecision/cpp_int.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <functional>
#include <iostream>
#include <memory>
#include <nitrate-core/Allocate.hh>
#include <nitrate-core/Logger.hh>
#include <nitrate-ir/IRFwd.hh>
#include <nitrate-ir/IRVisitor.hh>
#include <nitrate-lexer/Token.hh>
#include <optional>
#include <ostream>
#include <span>
#include <string>
#include <unordered_set>
#include <variant>
#include <vector>

namespace ncc::ir {
  using boost::multiprecision::uint128_t;

  extern "C" thread_local std::unique_ptr<ncc::IMemory> nr_allocator;

  template <class T>
  struct Arena {
    typedef T value_type;

    Arena() = default;

    template <class U>
    constexpr Arena(const Arena<U> &) {}

    [[nodiscard]] T *allocate(std::size_t n) {
      return static_cast<T *>(nr_allocator->alloc(sizeof(T) * n));
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
};  // namespace ncc::ir

namespace ncc::ir {
  enum class Purity {
    Impure = 0,
    Pure = 1,
    Quasi = 2,
    Retro = 3,
  };

  enum class Vis {
    Sec = 0,
    Pub = 1,
    Pro = 2,
  };

  enum class StorageClass {
    /* Automatic storeage duration */
    LLVM_StackAlloa,

    /* Static storage duration */
    LLVM_Static,

    /* Thread-local storage duration */
    LLVM_ThreadLocal,

    /* Dynamic allocation */
    Managed,
  };

}  // namespace ncc::ir

namespace ncc::ir {
  template <class Attorney>
  class IR_Vertex_Expr;
  template <class Attorney>
  class IR_Vertex_Type;
  template <class Attorney>
  class IR_Vertex_BinExpr;
  template <class Attorney>
  class IR_Vertex_Unary;
  template <class Attorney>
  class IR_Vertex_U1Ty;
  template <class Attorney>
  class IR_Vertex_U8Ty;
  template <class Attorney>
  class IR_Vertex_U16Ty;
  template <class Attorney>
  class IR_Vertex_U32Ty;
  template <class Attorney>
  class IR_Vertex_U64Ty;
  template <class Attorney>
  class IR_Vertex_U128Ty;
  template <class Attorney>
  class IR_Vertex_I8Ty;
  template <class Attorney>
  class IR_Vertex_I16Ty;
  template <class Attorney>
  class IR_Vertex_I32Ty;
  template <class Attorney>
  class IR_Vertex_I64Ty;
  template <class Attorney>
  class IR_Vertex_I128Ty;
  template <class Attorney>
  class IR_Vertex_F16Ty;
  template <class Attorney>
  class IR_Vertex_F32Ty;
  template <class Attorney>
  class IR_Vertex_F64Ty;
  template <class Attorney>
  class IR_Vertex_F128Ty;
  template <class Attorney>
  class IR_Vertex_VoidTy;
  template <class Attorney>
  class IR_Vertex_PtrTy;
  template <class Attorney>
  class IR_Vertex_ConstTy;
  template <class Attorney>
  class IR_Vertex_OpaqueTy;
  template <class Attorney>
  class IR_Vertex_StructTy;
  template <class Attorney>
  class IR_Vertex_UnionTy;
  template <class Attorney>
  class IR_Vertex_ArrayTy;
  template <class Attorney>
  class IR_Vertex_FnTy;
  template <class Attorney>
  class IR_Vertex_Int;
  template <class Attorney>
  class IR_Vertex_Float;
  template <class Attorney>
  class IR_Vertex_List;
  template <class Attorney>
  class IR_Vertex_Call;
  template <class Attorney>
  class IR_Vertex_Seq;
  template <class Attorney>
  class IR_Vertex_Index;
  template <class Attorney>
  class IR_Vertex_Ident;
  template <class Attorney>
  class IR_Vertex_Extern;
  template <class Attorney>
  class IR_Vertex_Local;
  template <class Attorney>
  class IR_Vertex_Ret;
  template <class Attorney>
  class IR_Vertex_Brk;
  template <class Attorney>
  class IR_Vertex_Cont;
  template <class Attorney>
  class IR_Vertex_If;
  template <class Attorney>
  class IR_Vertex_While;
  template <class Attorney>
  class IR_Vertex_For;
  template <class Attorney>
  class IR_Vertex_Case;
  template <class Attorney>
  class IR_Vertex_Switch;
  template <class Attorney>
  class IR_Vertex_Fn;
  template <class Attorney>
  class IR_Vertex_Asm;
  template <class Attorney>
  class IR_Vertex_Tmp;
}  // namespace ncc::ir

namespace ncc::ir {
  template <class A>
  class IR_Vertex_Expr {
    nr_ty_t m_node_type : 6; /* Typecode of this node. */
    uint32_t m_offset : 32;  /* Offset into source code where node starts. */
    uint32_t m_fileid : 24;  /* File ID of the source file. */

    IR_Vertex_Expr(const IR_Vertex_Expr &) = delete;
    IR_Vertex_Expr &operator=(const IR_Vertex_Expr &) = delete;

    IR_Vertex_Expr *cloneImpl() const;

  public:
    constexpr IR_Vertex_Expr(nr_ty_t ty, uint32_t offset = ncc::lex::QLEX_EOFF,
                             uint32_t fileid = ncc::lex::QLEX_NOFILE)
        : m_node_type(ty), m_offset(offset), m_fileid(fileid) {}

    static constexpr uint32_t getKindSize(nr_ty_t kind);
    constexpr auto getKind() const { return m_node_type; }
    static constexpr const char *getKindName(nr_ty_t kind);

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
      } else if constexpr (std::is_same_v<T, IR_Vertex_Fn<A>>) {
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

    // Returns "" if the construct is not named.
    constexpr std::string_view getName() const;

    constexpr std::tuple<uint32_t, uint32_t> getLoc() const {
      return {m_offset, m_fileid};
    }

    std::optional<IR_Vertex_Type<A> *> getType() const;

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
      return safeCastAs<T>(const_cast<IR_Vertex_Expr<A> *>(this));
    }

    template <class T = IR_Vertex_Expr<A>>
    constexpr T *clone() const {
      return cloneImpl()->template as<T>();
    }

    constexpr IR_Vertex_Expr *asExpr() { return this; }
    constexpr IR_Vertex_Type<A> *asType();
    constexpr const IR_Vertex_Type<A> *asType() const {
      return const_cast<IR_Vertex_Expr<A> *>(this)->asType();
    }

    /**
     * @brief Type check.
     *
     * @param type The type to check.
     * @return true If the type matches.
     * @return false If the type does not match.
     */
    constexpr bool is(nr_ty_t type) const { return type == getKind(); }

    /**
     * @brief Compare two nodes for equality.
     * @param other The other node.
     * @return true If the nodes are equivalent (deep comparison).
     * @note This compare will be insensitive to metadata like module, source
     * location, etc.
     */
    constexpr bool isSame(const IR_Vertex_Expr<A> *other) const;

    constexpr void accept(IRVisitor &v) {
      switch (getKind()) {
        case IR_eBIN: {
          v.visit(*as<IR_Vertex_BinExpr>());
          break;
        }

        case IR_eUNARY: {
          v.visit(*as<IR_Vertex_Unary>());
          break;
        }

        case IR_eINT: {
          v.visit(*as<IR_Vertex_Int>());
          break;
        }

        case IR_eFLOAT: {
          v.visit(*as<IR_Vertex_Float>());
          break;
        }

        case IR_eLIST: {
          v.visit(*as<IR_Vertex_List>());
          break;
        }

        case IR_eCALL: {
          v.visit(*as<IR_Vertex_Call>());
          break;
        }

        case IR_eSEQ: {
          v.visit(*as<IR_Vertex_Seq>());
          break;
        }

        case IR_eINDEX: {
          v.visit(*as<IR_Vertex_Index>());
          break;
        }

        case IR_eIDENT: {
          v.visit(*as<IR_Vertex_Ident>());
          break;
        }

        case IR_eEXTERN: {
          v.visit(*as<IR_Vertex_Extern>());
          break;
        }

        case IR_eLOCAL: {
          v.visit(*as<IR_Vertex_Local>());
          break;
        }

        case IR_eRET: {
          v.visit(*as<IR_Vertex_Ret>());
          break;
        }

        case IR_eBRK: {
          v.visit(*as<IR_Vertex_Brk>());
          break;
        }

        case IR_eSKIP: {
          v.visit(*as<IR_Vertex_Cont>());
          break;
        }

        case IR_eIF: {
          v.visit(*as<IR_Vertex_If>());
          break;
        }

        case IR_eWHILE: {
          v.visit(*as<IR_Vertex_While>());
          break;
        }

        case IR_eFOR: {
          v.visit(*as<IR_Vertex_For>());
          break;
        }

        case IR_eCASE: {
          v.visit(*as<IR_Vertex_Case>());
          break;
        }

        case IR_eSWITCH: {
          v.visit(*as<IR_Vertex_Switch>());
          break;
        }

        case IR_eFUNCTION: {
          v.visit(*as<IR_Vertex_Fn>());
          break;
        }

        case IR_eASM: {
          v.visit(*as<IR_Vertex_Asm>());
          break;
        }

        case IR_eIGN: {
          v.visit(*as<IR_Vertex_Expr>());
          break;
        }

        case IR_tU1: {
          v.visit(*as<IR_Vertex_U1Ty>());
          break;
        }

        case IR_tU8: {
          v.visit(*as<IR_Vertex_U8Ty>());
          break;
        }

        case IR_tU16: {
          v.visit(*as<IR_Vertex_U16Ty>());
          break;
        }

        case IR_tU32: {
          v.visit(*as<IR_Vertex_U32Ty>());
          break;
        }

        case IR_tU64: {
          v.visit(*as<IR_Vertex_U64Ty>());
          break;
        }

        case IR_tU128: {
          v.visit(*as<IR_Vertex_U128Ty>());
          break;
        }

        case IR_tI8: {
          v.visit(*as<IR_Vertex_I8Ty>());
          break;
        }

        case IR_tI16: {
          v.visit(*as<IR_Vertex_I16Ty>());
          break;
        }

        case IR_tI32: {
          v.visit(*as<IR_Vertex_I32Ty>());
          break;
        }

        case IR_tI64: {
          v.visit(*as<IR_Vertex_I64Ty>());
          break;
        }

        case IR_tI128: {
          v.visit(*as<IR_Vertex_I128Ty>());
          break;
        }

        case IR_tF16_TY: {
          v.visit(*as<IR_Vertex_F16Ty>());
          break;
        }

        case IR_tF32_TY: {
          v.visit(*as<IR_Vertex_F32Ty>());
          break;
        }

        case IR_tF64_TY: {
          v.visit(*as<IR_Vertex_F64Ty>());
          break;
        }

        case IR_tF128_TY: {
          v.visit(*as<IR_Vertex_F128Ty>());
          break;
        }

        case IR_tVOID: {
          v.visit(*as<IR_Vertex_VoidTy>());
          break;
        }

        case IR_tPTR: {
          v.visit(*as<IR_Vertex_PtrTy>());
          break;
        }

        case IR_tCONST: {
          v.visit(*as<IR_Vertex_ConstTy>());
          break;
        }

        case IR_tOPAQUE: {
          v.visit(*as<IR_Vertex_OpaqueTy>());
          break;
        }

        case IR_tSTRUCT: {
          v.visit(*as<IR_Vertex_StructTy>());
          break;
        }

        case IR_tUNION: {
          v.visit(*as<IR_Vertex_UnionTy>());
          break;
        }

        case IR_tARRAY: {
          v.visit(*as<IR_Vertex_ArrayTy>());
          break;
        }

        case IR_tFUNC: {
          v.visit(*as<IR_Vertex_FnTy>());
          break;
        }

        case IR_tTMP: {
          v.visit(*as<IR_Vertex_Tmp>());
          break;
        }
      }
    }

    bool isAcyclic() const;

    /**
     * @brief Print the node to the output stream.
     * @param os The output stream.
     * @param isForDebug Whether to print the node for debugging.
     */
    void dump(std::ostream &os = std::cout, bool isForDebug = false) const;

    std::string toString() const {
      std::stringstream ss;
      dump(ss, false);
      return ss.str();
    };

    /**
     * @brief Get a hashcode for the node. The code is unique its the nodes and
     * its childrens recursive state.
     * @return boost::uuids::uuid The hash.
     * @note This code will be the same on different compiler runs as long as
     * the compiler version is the same.
     */
    boost::uuids::uuid hash();

    /**
     * @brief Get a hashcode for the node. The code is unique its the nodes and
     * its childrens recursive state.
     * @return std::string The unique identifier.
     * @note Wrapper around hash()
     */
    std::string getStateUUID() { return boost::uuids::to_string(hash()); }

    /**
     * @brief Get a short code to uniquely identify the node.
     * @return uint64_t The unique identifier.
     * @note This code may be different for different compiler runs.
     */
    uint64_t getUniqId() const;

  } __attribute__((packed)) __attribute__((aligned(1)));

  // static_assert(sizeof(Expr) == 8);

  template <class A>
  class IR_Vertex_Type : public IR_Vertex_Expr<A> {
    std::optional<uint64_t> getAlignBits() const;

  public:
    constexpr IR_Vertex_Type(nr_ty_t ty) : IR_Vertex_Expr<A>(ty) {}

    std::optional<uint64_t> getSizeBits() const;

    std::optional<uint64_t> getSizeBytes() const {
      if (auto size = getSizeBits()) [[likely]] {
        return std::ceil(size.value() / 8.0);
      } else {
        return std::nullopt;
      }
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

  ///=============================================================================
  /// BEGIN: EXPRESSIONS CATEGORIES
  ///=============================================================================

  enum class Op {
    Plus,      /* '+': Addition operator */
    Minus,     /* '-': Subtraction operator */
    Times,     /* '*': Multiplication operator */
    Slash,     /* '/': Division operator */
    Percent,   /* '%': Modulus operator */
    BitAnd,    /* '&': Bitwise AND operator */
    BitOr,     /* '|': Bitwise OR operator */
    BitXor,    /* '^': Bitwise XOR operator */
    BitNot,    /* '~': Bitwise NOT operator */
    LogicAnd,  /* '&&': Logical AND operator */
    LogicOr,   /* '||': Logical OR operator */
    LogicNot,  /* '!': Logical NOT operator */
    LShift,    /* '<<': Left shift operator */
    RShift,    /* '>>': Right shift operator */
    Inc,       /* '++': Increment operator */
    Dec,       /* '--': Decrement operator */
    Set,       /* '=': Assignment operator */
    LT,        /* '<': Less than operator */
    GT,        /* '>': Greater than operator */
    LE,        /* '<=': Less than or equal to operator */
    GE,        /* '>=': Greater than or equal to operator */
    Eq,        /* '==': Equal to operator */
    NE,        /* '!=': Not equal to operator */
    Alignof,   /* 'alignof': Alignment of operator */
    BitcastAs, /* 'bitcast_as': Bitcast operator */
    CastAs,    /* 'cast_as': Common operator */
    Bitsizeof, /* 'bitsizeof': Bit size of operator */
  };

  enum class AbiTag {
    C,
    Nitrate,
    Internal,
    Default = Nitrate,
  };

  std::ostream &operator<<(std::ostream &os, Op op);

  template <class A>
  class IR_Vertex_BinExpr final : public IR_Vertex_Expr<A> {
    IR_Vertex_Expr<A> *m_lhs, *m_rhs;
    Op m_op;

  public:
    constexpr IR_Vertex_BinExpr(auto lhs, auto rhs, auto op)
        : IR_Vertex_Expr<A>(IR_eBIN), m_lhs(lhs), m_rhs(rhs), m_op(op) {}

    constexpr auto getLHS() const { return m_lhs; }
    constexpr auto getRHS() const { return m_rhs; }
    constexpr auto getOp() const { return m_op; }

    constexpr void setLHS(auto lhs) { m_lhs = lhs; }
    constexpr void setRHS(auto rhs) { m_rhs = rhs; }
    constexpr void setOp(auto op) { m_op = op; }
  };

  template <class A>
  class IR_Vertex_Unary final : public IR_Vertex_Expr<A> {
    IR_Vertex_Expr<A> *m_expr;
    Op m_op;
    bool m_postfix;

  public:
    constexpr IR_Vertex_Unary(auto expr, auto op, auto is_postfix)
        : IR_Vertex_Expr<A>(IR_eUNARY),
          m_expr(expr),
          m_op(op),
          m_postfix(is_postfix) {}

    constexpr auto getExpr() const { return m_expr; }
    constexpr auto getOp() const { return m_op; }
    constexpr auto isPostfix() const { return m_postfix; }

    constexpr void setExpr(IR_Vertex_Expr<A> *expr) { m_expr = expr; }
    constexpr void setOp(Op op) { m_op = op; }
    constexpr void setPostfix(bool is_postfix) { m_postfix = is_postfix; }
  };

  ///=============================================================================
  /// END: EXPRESSIONS CATEGORIES
  ///=============================================================================

  /// ===========================================================================
  /// BEGIN: PRIMITIVE TYPES
  /// ===========================================================================

  template <class A>
  class IR_Vertex_U1Ty final : public IR_Vertex_Type<A> {
  public:
    constexpr IR_Vertex_U1Ty() : IR_Vertex_Type<A>(IR_tU1) {}
  };

  template <class A>
  class IR_Vertex_U8Ty final : public IR_Vertex_Type<A> {
  public:
    constexpr IR_Vertex_U8Ty() : IR_Vertex_Type<A>(IR_tU8) {}
  };

  template <class A>
  class IR_Vertex_U16Ty final : public IR_Vertex_Type<A> {
  public:
    constexpr IR_Vertex_U16Ty() : IR_Vertex_Type<A>(IR_tU16) {}
  };

  template <class A>
  class IR_Vertex_U32Ty final : public IR_Vertex_Type<A> {
  public:
    constexpr IR_Vertex_U32Ty() : IR_Vertex_Type<A>(IR_tU32) {}
  };

  template <class A>
  class IR_Vertex_U64Ty final : public IR_Vertex_Type<A> {
  public:
    constexpr IR_Vertex_U64Ty() : IR_Vertex_Type<A>(IR_tU64) {}
  };

  template <class A>
  class IR_Vertex_U128Ty final : public IR_Vertex_Type<A> {
  public:
    constexpr IR_Vertex_U128Ty() : IR_Vertex_Type<A>(IR_tU128) {}
  };

  template <class A>
  class IR_Vertex_I8Ty final : public IR_Vertex_Type<A> {
  public:
    constexpr IR_Vertex_I8Ty() : IR_Vertex_Type<A>(IR_tI8) {}
  };

  template <class A>
  class IR_Vertex_I16Ty final : public IR_Vertex_Type<A> {
  public:
    constexpr IR_Vertex_I16Ty() : IR_Vertex_Type<A>(IR_tI16){};
  };

  template <class A>
  class IR_Vertex_I32Ty final : public IR_Vertex_Type<A> {
  public:
    constexpr IR_Vertex_I32Ty() : IR_Vertex_Type<A>(IR_tI32) {}
  };

  template <class A>
  class IR_Vertex_I64Ty final : public IR_Vertex_Type<A> {
  public:
    constexpr IR_Vertex_I64Ty() : IR_Vertex_Type<A>(IR_tI64) {}
  };

  template <class A>
  class IR_Vertex_I128Ty final : public IR_Vertex_Type<A> {
  public:
    constexpr IR_Vertex_I128Ty() : IR_Vertex_Type<A>(IR_tI128) {}
  };

  template <class A>
  class IR_Vertex_F16Ty final : public IR_Vertex_Type<A> {
  public:
    constexpr IR_Vertex_F16Ty() : IR_Vertex_Type<A>(IR_tF16_TY) {}
  };

  template <class A>
  class IR_Vertex_F32Ty final : public IR_Vertex_Type<A> {
  public:
    constexpr IR_Vertex_F32Ty() : IR_Vertex_Type<A>(IR_tF32_TY) {}
  };

  template <class A>
  class IR_Vertex_F64Ty final : public IR_Vertex_Type<A> {
  public:
    constexpr IR_Vertex_F64Ty() : IR_Vertex_Type<A>(IR_tF64_TY) {}
  };

  template <class A>
  class IR_Vertex_F128Ty final : public IR_Vertex_Type<A> {
  public:
    constexpr IR_Vertex_F128Ty() : IR_Vertex_Type<A>(IR_tF128_TY) {}
  };

  template <class A>
  class IR_Vertex_VoidTy final : public IR_Vertex_Type<A> {
  public:
    constexpr IR_Vertex_VoidTy() : IR_Vertex_Type<A>(IR_tVOID) {}
  };

  /// ===========================================================================
  /// END: PRIMITIVE TYPES
  /// ===========================================================================

  /// ===========================================================================
  /// BEGIN: COMPLEX TYPES
  /// ===========================================================================

  template <class A>
  class IR_Vertex_PtrTy final : public IR_Vertex_Type<A> {
    IR_Vertex_Type<A> *m_pointee;
    uint8_t m_native_size;

  public:
    constexpr IR_Vertex_PtrTy(auto pointee, auto platform_size_bytes = 8)
        : IR_Vertex_Type<A>(IR_tPTR),
          m_pointee(pointee),
          m_native_size(platform_size_bytes) {}

    constexpr auto getPointee() const { return m_pointee; }
    constexpr auto getPlatformPointerSizeBytes() const { return m_native_size; }
  };

  template <class A>
  class IR_Vertex_ConstTy final : public IR_Vertex_Type<A> {
    IR_Vertex_Type<A> *m_item;

  public:
    constexpr IR_Vertex_ConstTy(auto item)
        : IR_Vertex_Type<A>(IR_tCONST), m_item(item) {}

    constexpr auto getItem() const { return m_item; }
  };

  template <class A>
  class IR_Vertex_OpaqueTy final : public IR_Vertex_Type<A> {
    std::string_view m_name;

  public:
    constexpr IR_Vertex_OpaqueTy(std::string_view name)
        : IR_Vertex_Type<A>(IR_tOPAQUE), m_name(name) {}

    constexpr auto getName() const { return m_name; }
  };

  template <class A>
  using StructFields =
      std::vector<IR_Vertex_Type<A> *, Arena<IR_Vertex_Type<A> *>>;

  template <class A>
  class IR_Vertex_StructTy final : public IR_Vertex_Type<A> {
    std::span<IR_Vertex_Type<A>> m_fields;

  public:
    IR_Vertex_StructTy(auto fields)
        : IR_Vertex_Type<A>(IR_tSTRUCT), m_fields(fields) {}

    constexpr auto getFields() const { return m_fields; }
  };

  template <class A>
  using UnionFields =
      std::vector<IR_Vertex_Type<A> *, Arena<IR_Vertex_Type<A> *>>;

  template <class A>
  class IR_Vertex_UnionTy final : public IR_Vertex_Type<A> {
    UnionFields<A> m_fields;

  public:
    IR_Vertex_UnionTy(const UnionFields<A> &fields)
        : IR_Vertex_Type<A>(IR_tUNION), m_fields(fields) {}

    const UnionFields<A> &getFields() const { return m_fields; }
  };

  template <class A>
  class IR_Vertex_ArrayTy final : public IR_Vertex_Type<A> {
    IR_Vertex_Type<A> *m_element;
    size_t m_size;

  public:
    IR_Vertex_ArrayTy(IR_Vertex_Type<A> *element, size_t size)
        : IR_Vertex_Type<A>(IR_tARRAY), m_element(element), m_size(size) {}

    auto getElement() const { return m_element; }
    size_t getCount() const { return m_size; }
  };

  enum class FnAttr {
    Variadic,
  };

  template <class A>
  using FnParams = std::vector<IR_Vertex_Type<A> *, Arena<IR_Vertex_Type<A> *>>;

  typedef std::unordered_set<FnAttr, std::hash<FnAttr>, std::equal_to<FnAttr>,
                             Arena<FnAttr>>
      FnAttrs;

  template <class A>
  class IR_Vertex_FnTy final : public IR_Vertex_Type<A> {
    FnParams<A> m_params;
    FnAttrs m_attrs;
    IR_Vertex_Type<A> *m_return;
    uint8_t m_platform_ptr_size_bytes;

  public:
    IR_Vertex_FnTy(const FnParams<A> &params, IR_Vertex_Type<A> *ret,
                   const FnAttrs &attrs, uint8_t platform_ptr_size_bytes = 8)
        : IR_Vertex_Type<A>(IR_tFUNC),
          m_params(params),
          m_attrs(attrs),
          m_return(ret),
          m_platform_ptr_size_bytes(platform_ptr_size_bytes) {}

    const FnParams<A> &getParams() const { return m_params; }
    auto getReturn() const { return m_return; }
    const FnAttrs &getAttrs() const { return m_attrs; }

    bool isVariadic() const { return m_attrs.contains(FnAttr::Variadic); }

    uint8_t getPlatformPointerSizeBytes() const {
      return m_platform_ptr_size_bytes;
    }
  };

  ///=============================================================================
  /// END: COMPLEX TYPES
  ///=============================================================================

  ///=============================================================================
  /// BEGIN: LITERALS
  ///=============================================================================

  template <class A>
  class IR_Vertex_Int final : public IR_Vertex_Expr<A> {
    struct map_hash {
      std::size_t operator()(std::pair<uint128_t, uint8_t> const &v) const {
        return std::hash<uint128_t>()(v.first) ^ std::hash<uint8_t>()(v.second);
      }
    };

    static inline std::unordered_map<std::pair<uint128_t, uint8_t>,
                                     IR_Vertex_Int<A> *, map_hash>
        m_cache;

    unsigned __int128 m_value __attribute__((aligned(16)));
    uint8_t m_size;

    static uint128_t str2u128(std::string_view x);

  public:
    IR_Vertex_Int(uint128_t val, uint8_t size)
        : IR_Vertex_Expr<A>(IR_eINT), m_value(val), m_size(size) {}

    IR_Vertex_Int(std::string_view str, uint8_t size)
        : IR_Vertex_Expr<A>(IR_eINT), m_value(str2u128(str)) {
      m_size = size;
    }

    static IR_Vertex_Int *get(uint128_t val, uint8_t size);
    static IR_Vertex_Int *get(std::string_view str, uint8_t size) {
      return get(str2u128(str), size);
    }

    uint8_t getSize() const { return m_size; }
    uint128_t getValue() const { return m_value; }
    std::string getValueString() const;
  } __attribute__((packed));

  static_assert(sizeof(IR_Vertex_Int<void>) == 48);

  enum class FloatSize : uint8_t {
    F16,
    F32,
    F64,
    F128,
  };

  template <class A>
  class IR_Vertex_Float final : public IR_Vertex_Expr<A> {
    double m_data;
    FloatSize m_size;

    static_assert(sizeof(double) == 8);

  public:
    IR_Vertex_Float(double dec, FloatSize size)
        : IR_Vertex_Expr<A>(IR_eFLOAT), m_data{dec}, m_size(size) {}

    IR_Vertex_Float(std::string_view str) : IR_Vertex_Expr<A>(IR_eFLOAT) {
      m_data = std::stod(std::string(str));
      if (str.ends_with("f128")) {
        m_size = FloatSize::F128;
      } else if (str.ends_with("f32")) {
        m_size = FloatSize::F32;
      } else if (str.ends_with("f16")) {
        m_size = FloatSize::F16;
      } else {
        m_size = FloatSize::F64;
      }
    }

    FloatSize getSize() const { return m_size; }
    double getValue() const { return m_data; }
  } __attribute__((packed));

  template <class A>
  using IR_Vertex_ListItems =
      std::vector<IR_Vertex_Expr<A> *, Arena<IR_Vertex_Expr<A> *>>;

  template <class A>
  class IR_Vertex_List final : public IR_Vertex_Expr<A> {
    /// FIXME: Implement run-length compression

    IR_Vertex_ListItems<A> m_items;
    bool m_is_homogenous;

  public:
    IR_Vertex_List(const IR_Vertex_ListItems<A> &items, bool is_homogenous)
        : IR_Vertex_Expr<A>(IR_eLIST),
          m_items(items),
          m_is_homogenous(is_homogenous) {}

    auto begin() const { return m_items.begin(); }
    auto end() const { return m_items.end(); }
    size_t size() const { return m_items.size(); }

    IR_Vertex_Expr<A> *operator[](size_t idx) const { return m_items[idx]; }
    IR_Vertex_Expr<A> *at(size_t idx) const { return m_items.at(idx); }

    bool isHomogenous() const { return m_is_homogenous; }
  };

  ///=============================================================================
  /// END: LITERALS
  ///=============================================================================

  ///=============================================================================
  /// BEGIN: EXPRESSIONS
  ///=============================================================================

  template <class A>
  using IR_Vertex_CallArgs =
      std::vector<IR_Vertex_Expr<A> *, Arena<IR_Vertex_Expr<A> *>>;

  template <class A>
  class IR_Vertex_Call final : public IR_Vertex_Expr<A> {
    IR_Vertex_Expr<A> *m_iref; /* Possibly cyclic reference to the target. */
    IR_Vertex_CallArgs<A> m_args;

  public:
    IR_Vertex_Call(IR_Vertex_Expr<A> *ref, const IR_Vertex_CallArgs<A> &args)
        : IR_Vertex_Expr<A>(IR_eCALL), m_iref(ref), m_args(args) {}

    auto getTarget() const { return m_iref; }
    void setTarget(IR_Vertex_Expr<A> *ref) { m_iref = ref; }

    const IR_Vertex_CallArgs<A> &getArgs() const { return m_args; }
    IR_Vertex_CallArgs<A> &getArgs() { return m_args; }
    void setArgs(const IR_Vertex_CallArgs<A> &args) { m_args = args; }

    size_t getNumArgs() { return m_args.size(); }
  };

  template <class A>
  using IR_Vertex_SeqItems =
      std::vector<IR_Vertex_Expr<A> *, Arena<IR_Vertex_Expr<A> *>>;

  template <class A>
  class IR_Vertex_Seq final : public IR_Vertex_Expr<A> {
    IR_Vertex_SeqItems<A> m_items;

  public:
    IR_Vertex_Seq(const IR_Vertex_SeqItems<A> &items)
        : IR_Vertex_Expr<A>(IR_eSEQ), m_items(items) {}

    const IR_Vertex_SeqItems<A> &getItems() const { return m_items; }
    IR_Vertex_SeqItems<A> &getItems() { return m_items; }
    void setItems(const IR_Vertex_SeqItems<A> &items) { m_items = items; }
  };

  template <class A>
  class IR_Vertex_Index final : public IR_Vertex_Expr<A> {
    IR_Vertex_Expr<A> *m_expr;
    IR_Vertex_Expr<A> *m_index;

  public:
    IR_Vertex_Index(IR_Vertex_Expr<A> *expr, IR_Vertex_Expr<A> *index)
        : IR_Vertex_Expr<A>(IR_eINDEX), m_expr(expr), m_index(index) {}

    auto getExpr() const { return m_expr; }
    void setExpr(IR_Vertex_Expr<A> *expr) { m_expr = expr; }

    auto getIndex() const { return m_index; }
    void setIndex(IR_Vertex_Expr<A> *index) { m_index = index; }
  };

  template <class A>
  class IR_Vertex_Ident final : public IR_Vertex_Expr<A> {
    std::string_view m_name;
    IR_Vertex_Expr<A> *m_what;

  public:
    IR_Vertex_Ident(std::string_view name, IR_Vertex_Expr<A> *what)
        : IR_Vertex_Expr<A>(IR_eIDENT), m_name(name), m_what(what) {}

    auto getWhat() const { return m_what; }
    void setWhat(IR_Vertex_Expr<A> *what) { m_what = what; }

    std::string_view setName(std::string_view name) { m_name = name; }
  };

  template <class A>
  class IR_Vertex_Extern final : public IR_Vertex_Expr<A> {
    std::string_view m_abi_name;
    IR_Vertex_Expr<A> *m_value;

  public:
    IR_Vertex_Extern(IR_Vertex_Expr<A> *value, std::string_view abi_name)
        : IR_Vertex_Expr<A>(IR_eEXTERN), m_abi_name(abi_name), m_value(value) {}

    std::string_view getAbiName() const { return m_abi_name; }
    std::string_view setAbiName(std::string_view abi_name) {
      m_abi_name = abi_name;
    }

    auto getValue() const { return m_value; }
    void setValue(IR_Vertex_Expr<A> *value) { m_value = value; }
  };

  template <class A>
  class IR_Vertex_Local final : public IR_Vertex_Expr<A> {
    std::string_view m_name;
    IR_Vertex_Expr<A> *m_value;
    AbiTag m_abi_tag;
    StorageClass m_storage_class;
    bool m_readonly;

  public:
    IR_Vertex_Local(std::string_view name, IR_Vertex_Expr<A> *value,
                    AbiTag abi_tag, bool readonly = false,
                    StorageClass storage_class = StorageClass::LLVM_StackAlloa)
        : IR_Vertex_Expr<A>(IR_eLOCAL),
          m_name(name),
          m_value(value),
          m_abi_tag(abi_tag),
          m_storage_class(storage_class),
          m_readonly(readonly) {}

    std::string_view setName(std::string_view name) { m_name = name; }

    auto getValue() const { return m_value; }
    void setValue(IR_Vertex_Expr<A> *value) { m_value = value; }

    AbiTag getAbiTag() const { return m_abi_tag; }
    void setAbiTag(AbiTag abi_tag) { m_abi_tag = abi_tag; }

    StorageClass getStorageClass() const { return m_storage_class; }
    void setStorageClass(StorageClass storage_class) {
      m_storage_class = storage_class;
    }

    bool isReadonly() const { return m_readonly; }
    void setReadonly(bool readonly) { m_readonly = readonly; }
  };

  template <class A>
  class IR_Vertex_Ret final : public IR_Vertex_Expr<A> {
    IR_Vertex_Expr<A> *m_expr;

  public:
    IR_Vertex_Ret(IR_Vertex_Expr<A> *expr)
        : IR_Vertex_Expr<A>(IR_eRET), m_expr(expr) {}

    auto getExpr() const { return m_expr; }
    void setExpr(IR_Vertex_Expr<A> *expr) { m_expr = expr; }
  };

  template <class A>
  class IR_Vertex_Brk final : public IR_Vertex_Expr<A> {
  public:
    IR_Vertex_Brk() : IR_Vertex_Expr<A>(IR_eBRK) {}
  };

  template <class A>
  class IR_Vertex_Cont final : public IR_Vertex_Expr<A> {
  public:
    IR_Vertex_Cont() : IR_Vertex_Expr<A>(IR_eSKIP) {}
  };

  template <class A>
  class IR_Vertex_If final : public IR_Vertex_Expr<A> {
    IR_Vertex_Expr<A> *m_cond;
    IR_Vertex_Expr<A> *m_then;
    IR_Vertex_Expr<A> *m_else;

  public:
    IR_Vertex_If(IR_Vertex_Expr<A> *cond, IR_Vertex_Expr<A> *then,
                 IR_Vertex_Expr<A> *else_)
        : IR_Vertex_Expr<A>(IR_eIF),
          m_cond(cond),
          m_then(then),
          m_else(else_) {}

    auto getCond() const { return m_cond; }
    void setCond(IR_Vertex_Expr<A> *cond) { m_cond = cond; }

    auto getThen() const { return m_then; }
    void setThen(IR_Vertex_Expr<A> *then) { m_then = then; }

    auto getElse() const { return m_else; }
    void setElse(IR_Vertex_Expr<A> *else_) { m_else = else_; }
  };

  template <class A>
  class IR_Vertex_While final : public IR_Vertex_Expr<A> {
    IR_Vertex_Expr<A> *m_cond;
    IR_Vertex_Seq<A> *m_body;

  public:
    IR_Vertex_While(IR_Vertex_Expr<A> *cond, IR_Vertex_Seq<A> *body)
        : IR_Vertex_Expr<A>(IR_eWHILE), m_cond(cond), m_body(body) {}

    auto getCond() const { return m_cond; }
    void setCond(IR_Vertex_Expr<A> *cond) { m_cond = cond; }

    auto getBody() const { return m_body; }
    IR_Vertex_Seq<A> *setBody(IR_Vertex_Seq<A> *body) { m_body = body; }
  };

  template <class A>
  class IR_Vertex_For final : public IR_Vertex_Expr<A> {
    IR_Vertex_Expr<A> *m_init;
    IR_Vertex_Expr<A> *m_cond;
    IR_Vertex_Expr<A> *m_step;
    IR_Vertex_Expr<A> *m_body;

  public:
    IR_Vertex_For(IR_Vertex_Expr<A> *init, IR_Vertex_Expr<A> *cond,
                  IR_Vertex_Expr<A> *step, IR_Vertex_Expr<A> *body)
        : IR_Vertex_Expr<A>(IR_eFOR),
          m_init(init),
          m_cond(cond),
          m_step(step),
          m_body(body) {}

    auto getInit() const { return m_init; }
    void setInit(IR_Vertex_Expr<A> *init) { m_init = init; }

    auto getCond() const { return m_cond; }
    void setCond(IR_Vertex_Expr<A> *cond) { m_cond = cond; }

    auto getStep() const { return m_step; }
    void setStep(IR_Vertex_Expr<A> *step) { m_step = step; }

    auto getBody() const { return m_body; }
    void setBody(IR_Vertex_Expr<A> *body) { m_body = body; }
  };

  template <class A>
  class IR_Vertex_Case final : public IR_Vertex_Expr<A> {
    IR_Vertex_Expr<A> *m_cond;
    IR_Vertex_Expr<A> *m_body;

  public:
    IR_Vertex_Case(IR_Vertex_Expr<A> *cond, IR_Vertex_Expr<A> *body)
        : IR_Vertex_Expr<A>(IR_eCASE), m_cond(cond), m_body(body) {}

    auto getCond() { return m_cond; }
    void setCond(IR_Vertex_Expr<A> *cond) { m_cond = cond; }

    auto getBody() { return m_body; }
    void setBody(IR_Vertex_Expr<A> *body) { m_body = body; }
  };

  template <class A>
  using SwitchCases =
      std::vector<IR_Vertex_Case<A> *, Arena<IR_Vertex_Case<A> *>>;

  template <class A>
  class IR_Vertex_Switch final : public IR_Vertex_Expr<A> {
    IR_Vertex_Expr<A> *m_cond;
    IR_Vertex_Expr<A> *m_default;
    SwitchCases<A> m_cases;

  public:
    IR_Vertex_Switch(IR_Vertex_Expr<A> *cond, const SwitchCases<A> &cases,
                     IR_Vertex_Expr<A> *default_)
        : IR_Vertex_Expr<A>(IR_eSWITCH),
          m_cond(cond),
          m_default(default_),
          m_cases(cases) {}

    auto getCond() const { return m_cond; }
    void setCond(IR_Vertex_Expr<A> *cond) { m_cond = cond; }

    auto getDefault() const { return m_default; }
    void setDefault(IR_Vertex_Expr<A> *default_) { m_default = default_; }

    const SwitchCases<A> &getCases() const { return m_cases; }
    SwitchCases<A> &getCases() { return m_cases; }
    void setCases(const SwitchCases<A> &cases) { m_cases = cases; }
    void addCase(IR_Vertex_Case<A> *c) { m_cases.push_back(c); }
  };

  template <class A>
  using Params =
      std::vector<std::pair<IR_Vertex_Type<A> *, std::string_view>,
                  Arena<std::pair<IR_Vertex_Type<A> *, std::string_view>>>;

  template <class A>
  class IR_Vertex_Fn final : public IR_Vertex_Expr<A> {
    std::string_view m_name;
    Params<A> m_params;
    IR_Vertex_Type<A> *m_return;
    std::optional<IR_Vertex_Seq<A> *> m_body;
    bool m_variadic;
    AbiTag m_abi_tag;

  public:
    IR_Vertex_Fn(std::string_view name, const Params<A> &params,
                 IR_Vertex_Type<A> *ret_ty,
                 std::optional<IR_Vertex_Seq<A> *> body, bool variadic,
                 AbiTag abi_tag)
        : IR_Vertex_Expr<A>(IR_eFUNCTION),
          m_name(name),
          m_params(params),
          m_return(ret_ty),
          m_body(body),
          m_variadic(variadic),
          m_abi_tag(abi_tag) {}

    std::string_view setName(std::string_view name) { m_name = name; }

    const Params<A> &getParams() const { return m_params; }
    Params<A> &getParams() { return m_params; }
    void setParams(const Params<A> &params) { m_params = params; }

    auto getReturn() const { return m_return; }
    IR_Vertex_Type<A> *setReturn(IR_Vertex_Type<A> *ret_ty) {
      m_return = ret_ty;
    }

    std::optional<IR_Vertex_Seq<A> *> getBody() const { return m_body; }
    std::optional<IR_Vertex_Seq<A> *> setBody(
        std::optional<IR_Vertex_Seq<A> *> body) {
      m_body = body;
    }

    bool isVariadic() const { return m_variadic; }
    void setVariadic(bool variadic) { m_variadic = variadic; }

    AbiTag getAbiTag() const { return m_abi_tag; }
    AbiTag setAbiTag(AbiTag abi_tag) { m_abi_tag = abi_tag; }
  };

  template <class A>
  class IR_Vertex_Asm final : public IR_Vertex_Expr<A> {
  public:
    IR_Vertex_Asm() : IR_Vertex_Expr<A>(IR_eASM) { qcore_implement(); }
  };

  ///=============================================================================
  /// END: EXPRESSIONS
  ///=============================================================================

  enum class TmpType {
    CALL,
    NAMED_TYPE,
    DEFAULT_VALUE,
  };

  template <class A>
  using CallArguments =
      std::vector<std::pair<std::string_view, IR_Vertex_Expr<A> *>,
                  Arena<std::pair<std::string_view, IR_Vertex_Expr<A> *>>>;

  template <class A>
  struct IR_Vertex_CallArgsTmpNodeCradle {
    IR_Vertex_Expr<A> *base;
    CallArguments<A> args;

    bool operator==(const IR_Vertex_CallArgsTmpNodeCradle<A> &rhs) const {
      return base == rhs.base && args == rhs.args;
    }
  };

  template <class A>
  using TmpNodeCradle =
      std::variant<IR_Vertex_CallArgsTmpNodeCradle<A>, std::string_view>;

  template <class A>
  class IR_Vertex_Tmp final : public IR_Vertex_Type<A> {
    TmpType m_type;
    TmpNodeCradle<A> m_data;

  public:
    IR_Vertex_Tmp(TmpType type, const TmpNodeCradle<A> &data = {})
        : IR_Vertex_Type<A>(IR_tTMP), m_type(type), m_data(data) {}

    TmpType getTmpType() { return m_type; }
    TmpNodeCradle<A> &getData() { return m_data; }
    const TmpNodeCradle<A> &getData() const { return m_data; }
  };

  ///=============================================================================

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
        R = as<IR_Vertex_Ident<A>>()->m_name;
        break;
      }

      case IR_eEXTERN: {
        break;
      }

      case IR_eLOCAL: {
        R = as<IR_Vertex_Local<A>>()->m_name;
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
        R = as<IR_Vertex_Fn<A>>()->m_name;
        break;
      }

      case IR_eASM: {
        qcore_implement();
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
        R = as<IR_Vertex_ConstTy<A>>()->m_item->getName();
        break;
      }

      case IR_tOPAQUE: {
        R = as<IR_Vertex_OpaqueTy<A>>()->m_name;
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
      R[IR_eFUNCTION] = sizeof(IR_Vertex_Fn<A>);
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

namespace ncc::ir {}  // namespace ncc::ir

namespace ncc::ir {
  Expr *createIgn();

  namespace mem {
    extern Brk static_IR_eBRK;
    extern Cont static_IR_eSKIP;
    extern Expr static_IR_eIGN;

  };  // namespace mem

  template <typename T, typename... Args>
  static constexpr inline T *create(Args &&...args) {
    /**
     * Create nodes and minimizes the number of allocations by reusing
     * immutable items.
     */

#define NORMAL_ALLOC(NAME)                                              \
  if constexpr (ty == NAME) {                                           \
    return new (Arena<T>().allocate(1)) T(std::forward<Args>(args)...); \
  }

#define REUSE_ALLOC(NAME)       \
  if constexpr (ty == NAME) {   \
    return &mem::static_##NAME; \
  }

#define CACHE_ALLOC(NAME)                       \
  if constexpr (ty == NAME) {                   \
    return T::get(std::forward<Args>(args)...); \
  }

    constexpr nr_ty_t ty = Expr::getTypeCode<T>();

    NORMAL_ALLOC(IR_eBIN);
    NORMAL_ALLOC(IR_eUNARY);
    CACHE_ALLOC(IR_eINT);
    NORMAL_ALLOC(IR_eFLOAT);
    NORMAL_ALLOC(IR_eLIST);
    NORMAL_ALLOC(IR_eCALL);
    NORMAL_ALLOC(IR_eSEQ);
    NORMAL_ALLOC(IR_eINDEX);
    NORMAL_ALLOC(IR_eIDENT);
    NORMAL_ALLOC(IR_eEXTERN);
    NORMAL_ALLOC(IR_eLOCAL);
    NORMAL_ALLOC(IR_eRET);
    REUSE_ALLOC(IR_eBRK);
    REUSE_ALLOC(IR_eSKIP);
    NORMAL_ALLOC(IR_eIF);
    NORMAL_ALLOC(IR_eWHILE);
    NORMAL_ALLOC(IR_eFOR);
    NORMAL_ALLOC(IR_eCASE);
    NORMAL_ALLOC(IR_eSWITCH);
    NORMAL_ALLOC(IR_eFUNCTION);
    NORMAL_ALLOC(IR_eASM);
    REUSE_ALLOC(IR_eIGN);
    NORMAL_ALLOC(IR_tU1);
    NORMAL_ALLOC(IR_tU8);
    NORMAL_ALLOC(IR_tU16);
    NORMAL_ALLOC(IR_tU32);
    NORMAL_ALLOC(IR_tU64);
    NORMAL_ALLOC(IR_tU128);
    NORMAL_ALLOC(IR_tI8);
    NORMAL_ALLOC(IR_tI16);
    NORMAL_ALLOC(IR_tI32);
    NORMAL_ALLOC(IR_tI64);
    NORMAL_ALLOC(IR_tI128);
    NORMAL_ALLOC(IR_tF16_TY);
    NORMAL_ALLOC(IR_tF32_TY);
    NORMAL_ALLOC(IR_tF64_TY);
    NORMAL_ALLOC(IR_tF128_TY);
    NORMAL_ALLOC(IR_tVOID);
    NORMAL_ALLOC(IR_tPTR);
    NORMAL_ALLOC(IR_tCONST);
    NORMAL_ALLOC(IR_tOPAQUE);
    NORMAL_ALLOC(IR_tSTRUCT);
    NORMAL_ALLOC(IR_tUNION);
    NORMAL_ALLOC(IR_tARRAY);
    NORMAL_ALLOC(IR_tFUNC);
    NORMAL_ALLOC(IR_tTMP);

#undef CACHE_ALLOC
#undef NORMAL_ALLOC
  }

  enum IterMode {
    dfs_pre,
    dfs_post,
    bfs_pre,
    bfs_post,
    children,
  };

  enum class IterOp {
    Proceed,
    Abort,
    SkipChildren,
  };

  typedef std::function<IterOp(Expr *p, Expr **c)> IterCallback;
  typedef std::function<bool(Expr **a, Expr **b)> ChildSelect;

  typedef std::function<IterOp(const Expr *const p, const Expr *const *const c)>
      ConstIterCallback;
  typedef std::function<bool(const Expr *const *const a,
                             const Expr *const *const b)>
      ConstChildSelect;

  namespace detail {
    void dfs_pre_impl(Expr **base, IterCallback cb, ChildSelect cs);
    void dfs_post_impl(Expr **base, IterCallback cb, ChildSelect cs);
    void bfs_pre_impl(Expr **base, IterCallback cb, ChildSelect cs);
    void bfs_post_impl(Expr **base, IterCallback cb, ChildSelect cs);
    void iter_children(Expr **base, IterCallback cb, ChildSelect cs);
  }  // namespace detail

  template <IterMode mode, typename T>
  void iterate(T *&base, IterCallback cb, ChildSelect cs = nullptr) {
    if constexpr (mode == dfs_pre) {
      return detail::dfs_pre_impl((Expr **)&base, cb, cs);
    } else if constexpr (mode == dfs_post) {
      return detail::dfs_post_impl((Expr **)&base, cb, cs);
    } else if constexpr (mode == bfs_pre) {
      return detail::bfs_pre_impl((Expr **)&base, cb, cs);
    } else if constexpr (mode == bfs_post) {
      return detail::bfs_post_impl((Expr **)&base, cb, cs);
    } else if constexpr (mode == children) {
      return detail::iter_children((Expr **)&base, cb, cs);
    } else {
      static_assert(mode != mode, "Invalid iteration mode.");
    }
  }

  template <IterMode mode, typename T>
  void iterate(const T *base, ConstIterCallback cb,
               ConstChildSelect cs = nullptr) {
    T *ref = const_cast<T *>(base);

    const auto const_cb = cb != nullptr ? [&](Expr *p, Expr **c) -> IterOp {
      return cb(static_cast<const Expr *const>(p),
                const_cast<const Expr *const *const>(c));
    }
    : IterCallback(nullptr);

    const auto const_cs = cs != nullptr ? [&](Expr **a, Expr **b) -> bool {
      return cs(const_cast<const Expr *const *const>(a),
                const_cast<const Expr *const *const>(b));
    }
    : ChildSelect(nullptr);

    if constexpr (mode == dfs_pre) {
      return detail::dfs_pre_impl((Expr **)&ref, const_cb, const_cs);
    } else if constexpr (mode == dfs_post) {
      return detail::dfs_post_impl((Expr **)&ref, const_cb, const_cs);
    } else if constexpr (mode == bfs_pre) {
      return detail::bfs_pre_impl((Expr **)&ref, const_cb, const_cs);
    } else if constexpr (mode == bfs_post) {
      return detail::bfs_pre_impl((Expr **)&ref, const_cb, const_cs);
    } else if constexpr (mode == children) {
      return detail::iter_children((Expr **)&ref, const_cb, const_cs);
    } else {
      static_assert(mode != mode, "Invalid iteration mode.");
    }
  }

  std::optional<Expr *> comptime_impl(
      Expr *x, std::optional<std::function<void(std::string_view)>> eprintn =
                   std::nullopt);

  //   template <typename T>
  //   std::optional<T> uint_as(const Expr *x) {
  // #define IS_T(x) std::is_same_v<T, Expr>

  //     qcore_assert(x != nullptr, "evaluate_as(): x is nullptr.");

  //     static_assert(IS_T(std::string) || IS_T(uint64_t),
  //                   "evaluate_as(): T must be either std::string or
  //                   uint64_t.");

  //     Expr *r = comptime_impl(const_cast<Expr *>(x));
  //     if (r == nullptr) {
  //       return std::nullopt;
  //     }

  //     nr_ty_t ty = r->getKind();

  //     if (ty != IR_eINT) {
  //       return std::nullopt;
  //     }

  //     if constexpr (IS_T(std::string)) {
  //       return r->as<Int>()->getValue();
  //     } else if constexpr (IS_T(uint64_t)) {
  //       return r->as<Int>()->getValue();
  //     }

  //     return std::nullopt;

  // #undef IS_T
  //   }

  /** Add source debugging information to an IR node */
  template <typename T>
  static inline T *debug_info(T *N, uint32_t line, uint32_t col) {
    /// TODO: Store source location information
    (void)line;
    (void)col;

    return N;
  }

  template <auto mode = dfs_pre>
  void for_each(const Expr *const v,
                std::function<void(nr_ty_t, const Expr *const)> f) {
    iterate<mode>(v, [&](auto, auto c) -> IterOp {
      f((*c)->getKind(), *c);

      return IterOp::Abort;
    });
  }

  template <auto mode = dfs_pre>
  void transform(Expr *v, std::function<bool(nr_ty_t, Expr **)> f) {
    iterate<mode>(v, [&](auto, auto c) -> IterOp {
      return f((*c)->getKind(), c) ? IterOp::Proceed : IterOp::Abort;
    });
  }

  template <typename T, auto mode = dfs_pre>
  void for_each(const Expr *const v, std::function<void(const T *)> f) {
    iterate<mode>(v, [&](auto, const Expr *const *const c) -> IterOp {
      if ((*c)->getKind() != Expr::getTypeCode<T>()) {
        return IterOp::Proceed;
      }

      f((*c)->as<T>());

      return IterOp::Proceed;
    });
  }

  template <typename T, auto mode = dfs_pre>
  void transform(Expr *v, std::function<bool(T **)> f) {
    iterate<mode>(v, [&](auto, auto c) -> IterOp {
      if ((*c)->getKind() != Expr::getTypeCode<T>()) {
        return IterOp::Proceed;
      }

      return f(reinterpret_cast<T **>(c)) ? IterOp::Proceed : IterOp::Abort;
    });
  }

}  // namespace ncc::ir

#endif
