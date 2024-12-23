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

#ifndef __NITRATE_NR_NODE_H__
#define __NITRATE_NR_NODE_H__

#ifndef __cplusplus
#error "This header is C++ only."
#endif

#include <nitrate-ir/TypeDecl.h>

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
#include <nitrate-ir/Module.hh>
#include <nitrate-ir/Visitor.hh>
#include <nitrate-lexer/Token.hh>
#include <optional>
#include <ostream>
#include <string>
#include <unordered_set>
#include <variant>
#include <vector>

namespace nr {
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
};  // namespace nr

struct nr_node_t {
public:
  nr_node_t() = default;
};

namespace nr {

#ifdef __NR_NODE_REFLECT_IMPL__
#define QCLASS_REFLECT() public:
#else
#define QCLASS_REFLECT() private:
#endif

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

  class Expr : public nr_node_t {
    QCLASS_REFLECT()

    nr_ty_t m_node_type : 6; /* Typecode of this node. */
    uint32_t m_offset : 32;  /* Offset into source code where node starts. */
    uint32_t m_fileid : 24;  /* File ID of the source file. */

    Expr(const Expr &) = delete;
    Expr &operator=(const Expr &) = delete;

  public:
    constexpr Expr(nr_ty_t ty, uint32_t offset = ncc::lex::QLEX_EOFF,
                   uint32_t fileid = ncc::lex::QLEX_NOFILE)
        : m_node_type(ty), m_offset(offset), m_fileid(fileid) {}

    static constexpr uint32_t getKindSize(nr_ty_t kind);
    constexpr nr_ty_t getKind() const { return m_node_type; }
    static constexpr const char *getKindName(nr_ty_t kind);

    constexpr const char *getKindName() const {
      return getKindName(m_node_type);
    }

    template <typename T>
    static constexpr nr_ty_t getTypeCode() {
      if constexpr (std::is_same_v<T, BinExpr>) {
        return NR_NODE_BINEXPR;
      } else if constexpr (std::is_same_v<T, UnExpr>) {
        return NR_NODE_UNEXPR;
      } else if constexpr (std::is_same_v<T, PostUnExpr>) {
        return NR_NODE_POST_UNEXPR;
      } else if constexpr (std::is_same_v<T, Int>) {
        return NR_NODE_INT;
      } else if constexpr (std::is_same_v<T, Float>) {
        return NR_NODE_FLOAT;
      } else if constexpr (std::is_same_v<T, List>) {
        return NR_NODE_LIST;
      } else if constexpr (std::is_same_v<T, Call>) {
        return NR_NODE_CALL;
      } else if constexpr (std::is_same_v<T, Seq>) {
        return NR_NODE_SEQ;
      } else if constexpr (std::is_same_v<T, Index>) {
        return NR_NODE_INDEX;
      } else if constexpr (std::is_same_v<T, Ident>) {
        return NR_NODE_IDENT;
      } else if constexpr (std::is_same_v<T, Extern>) {
        return NR_NODE_EXTERN;
      } else if constexpr (std::is_same_v<T, Local>) {
        return NR_NODE_LOCAL;
      } else if constexpr (std::is_same_v<T, Ret>) {
        return NR_NODE_RET;
      } else if constexpr (std::is_same_v<T, Brk>) {
        return NR_NODE_BRK;
      } else if constexpr (std::is_same_v<T, Cont>) {
        return NR_NODE_CONT;
      } else if constexpr (std::is_same_v<T, If>) {
        return NR_NODE_IF;
      } else if constexpr (std::is_same_v<T, While>) {
        return NR_NODE_WHILE;
      } else if constexpr (std::is_same_v<T, For>) {
        return NR_NODE_FOR;
      } else if constexpr (std::is_same_v<T, Case>) {
        return NR_NODE_CASE;
      } else if constexpr (std::is_same_v<T, Switch>) {
        return NR_NODE_SWITCH;
      } else if constexpr (std::is_same_v<T, Fn>) {
        return NR_NODE_FN;
      } else if constexpr (std::is_same_v<T, Asm>) {
        return NR_NODE_ASM;
      } else if constexpr (std::is_same_v<T, Expr>) {
        return NR_NODE_IGN;
      } else if constexpr (std::is_same_v<T, U1Ty>) {
        return NR_NODE_U1_TY;
      } else if constexpr (std::is_same_v<T, U8Ty>) {
        return NR_NODE_U8_TY;
      } else if constexpr (std::is_same_v<T, U16Ty>) {
        return NR_NODE_U16_TY;
      } else if constexpr (std::is_same_v<T, U32Ty>) {
        return NR_NODE_U32_TY;
      } else if constexpr (std::is_same_v<T, U64Ty>) {
        return NR_NODE_U64_TY;
      } else if constexpr (std::is_same_v<T, U128Ty>) {
        return NR_NODE_U128_TY;
      } else if constexpr (std::is_same_v<T, I8Ty>) {
        return NR_NODE_I8_TY;
      } else if constexpr (std::is_same_v<T, I16Ty>) {
        return NR_NODE_I16_TY;
      } else if constexpr (std::is_same_v<T, I32Ty>) {
        return NR_NODE_I32_TY;
      } else if constexpr (std::is_same_v<T, I64Ty>) {
        return NR_NODE_I64_TY;
      } else if constexpr (std::is_same_v<T, I128Ty>) {
        return NR_NODE_I128_TY;
      } else if constexpr (std::is_same_v<T, F16Ty>) {
        return NR_NODE_F16_TY;
      } else if constexpr (std::is_same_v<T, F32Ty>) {
        return NR_NODE_F32_TY;
      } else if constexpr (std::is_same_v<T, F64Ty>) {
        return NR_NODE_F64_TY;
      } else if constexpr (std::is_same_v<T, F128Ty>) {
        return NR_NODE_F128_TY;
      } else if constexpr (std::is_same_v<T, VoidTy>) {
        return NR_NODE_VOID_TY;
      } else if constexpr (std::is_same_v<T, PtrTy>) {
        return NR_NODE_PTR_TY;
      } else if constexpr (std::is_same_v<T, ConstTy>) {
        return NR_NODE_CONST_TY;
      } else if constexpr (std::is_same_v<T, OpaqueTy>) {
        return NR_NODE_OPAQUE_TY;
      } else if constexpr (std::is_same_v<T, StructTy>) {
        return NR_NODE_STRUCT_TY;
      } else if constexpr (std::is_same_v<T, UnionTy>) {
        return NR_NODE_UNION_TY;
      } else if constexpr (std::is_same_v<T, ArrayTy>) {
        return NR_NODE_ARRAY_TY;
      } else if constexpr (std::is_same_v<T, FnTy>) {
        return NR_NODE_FN_TY;
      } else if constexpr (std::is_same_v<T, Tmp>) {
        return NR_NODE_TMP;
      } else {
        static_assert(
            !std::is_same_v<T, T>,
            "The requested type target is not supported by this function.");
      }
    }

    constexpr bool isType() const {
      switch (getKind()) {
        case NR_NODE_U1_TY:
        case NR_NODE_U8_TY:
        case NR_NODE_U16_TY:
        case NR_NODE_U32_TY:
        case NR_NODE_U64_TY:
        case NR_NODE_U128_TY:
        case NR_NODE_I8_TY:
        case NR_NODE_I16_TY:
        case NR_NODE_I32_TY:
        case NR_NODE_I64_TY:
        case NR_NODE_I128_TY:
        case NR_NODE_F16_TY:
        case NR_NODE_F32_TY:
        case NR_NODE_F64_TY:
        case NR_NODE_F128_TY:
        case NR_NODE_VOID_TY:
        case NR_NODE_PTR_TY:
        case NR_NODE_CONST_TY:
        case NR_NODE_OPAQUE_TY:
        case NR_NODE_STRUCT_TY:
        case NR_NODE_UNION_TY:
        case NR_NODE_ARRAY_TY:
        case NR_NODE_FN_TY:
        case NR_NODE_TMP:
          return true;
        default:
          return false;
      }
    }

    constexpr bool isLiteral() const {
      return m_node_type == NR_NODE_INT || m_node_type == NR_NODE_FLOAT;
    }

    // Returns "" if the construct is not named.
    constexpr std::string_view getName() const;

    constexpr std::tuple<uint32_t, uint32_t> getLoc() const {
      return {m_offset, m_fileid};
    }

    constexpr std::optional<Type *> getType() const;

    template <typename T>
    static constexpr T *safeCastAs(Expr *ptr) {
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
      return safeCastAs<T>(const_cast<Expr *>(this));
    }

    constexpr Expr *asExpr() { return this; }
    constexpr Type *asType();
    constexpr const Type *asType() const {
      return const_cast<Expr *>(this)->asType();
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
    constexpr bool isSame(const Expr *other) const;

    constexpr void accept(NRVisitor &v) {
      switch (getKind()) {
        case NR_NODE_BINEXPR: {
          v.visit(*as<BinExpr>());
          break;
        }

        case NR_NODE_UNEXPR: {
          v.visit(*as<UnExpr>());
          break;
        }

        case NR_NODE_POST_UNEXPR: {
          v.visit(*as<PostUnExpr>());
          break;
        }

        case NR_NODE_INT: {
          v.visit(*as<Int>());
          break;
        }

        case NR_NODE_FLOAT: {
          v.visit(*as<Float>());
          break;
        }

        case NR_NODE_LIST: {
          v.visit(*as<List>());
          break;
        }

        case NR_NODE_CALL: {
          v.visit(*as<Call>());
          break;
        }

        case NR_NODE_SEQ: {
          v.visit(*as<Seq>());
          break;
        }

        case NR_NODE_INDEX: {
          v.visit(*as<Index>());
          break;
        }

        case NR_NODE_IDENT: {
          v.visit(*as<Ident>());
          break;
        }

        case NR_NODE_EXTERN: {
          v.visit(*as<Extern>());
          break;
        }

        case NR_NODE_LOCAL: {
          v.visit(*as<Local>());
          break;
        }

        case NR_NODE_RET: {
          v.visit(*as<Ret>());
          break;
        }

        case NR_NODE_BRK: {
          v.visit(*as<Brk>());
          break;
        }

        case NR_NODE_CONT: {
          v.visit(*as<Cont>());
          break;
        }

        case NR_NODE_IF: {
          v.visit(*as<If>());
          break;
        }

        case NR_NODE_WHILE: {
          v.visit(*as<While>());
          break;
        }

        case NR_NODE_FOR: {
          v.visit(*as<For>());
          break;
        }

        case NR_NODE_CASE: {
          v.visit(*as<Case>());
          break;
        }

        case NR_NODE_SWITCH: {
          v.visit(*as<Switch>());
          break;
        }

        case NR_NODE_FN: {
          v.visit(*as<Fn>());
          break;
        }

        case NR_NODE_ASM: {
          v.visit(*as<Asm>());
          break;
        }

        case NR_NODE_IGN: {
          v.visit(*as<Expr>());
          break;
        }

        case NR_NODE_U1_TY: {
          v.visit(*as<U1Ty>());
          break;
        }

        case NR_NODE_U8_TY: {
          v.visit(*as<U8Ty>());
          break;
        }

        case NR_NODE_U16_TY: {
          v.visit(*as<U16Ty>());
          break;
        }

        case NR_NODE_U32_TY: {
          v.visit(*as<U32Ty>());
          break;
        }

        case NR_NODE_U64_TY: {
          v.visit(*as<U64Ty>());
          break;
        }

        case NR_NODE_U128_TY: {
          v.visit(*as<U128Ty>());
          break;
        }

        case NR_NODE_I8_TY: {
          v.visit(*as<I8Ty>());
          break;
        }

        case NR_NODE_I16_TY: {
          v.visit(*as<I16Ty>());
          break;
        }

        case NR_NODE_I32_TY: {
          v.visit(*as<I32Ty>());
          break;
        }

        case NR_NODE_I64_TY: {
          v.visit(*as<I64Ty>());
          break;
        }

        case NR_NODE_I128_TY: {
          v.visit(*as<I128Ty>());
          break;
        }

        case NR_NODE_F16_TY: {
          v.visit(*as<F16Ty>());
          break;
        }

        case NR_NODE_F32_TY: {
          v.visit(*as<F32Ty>());
          break;
        }

        case NR_NODE_F64_TY: {
          v.visit(*as<F64Ty>());
          break;
        }

        case NR_NODE_F128_TY: {
          v.visit(*as<F128Ty>());
          break;
        }

        case NR_NODE_VOID_TY: {
          v.visit(*as<VoidTy>());
          break;
        }

        case NR_NODE_PTR_TY: {
          v.visit(*as<PtrTy>());
          break;
        }

        case NR_NODE_CONST_TY: {
          v.visit(*as<ConstTy>());
          break;
        }

        case NR_NODE_OPAQUE_TY: {
          v.visit(*as<OpaqueTy>());
          break;
        }

        case NR_NODE_STRUCT_TY: {
          v.visit(*as<StructTy>());
          break;
        }

        case NR_NODE_UNION_TY: {
          v.visit(*as<UnionTy>());
          break;
        }

        case NR_NODE_ARRAY_TY: {
          v.visit(*as<ArrayTy>());
          break;
        }

        case NR_NODE_FN_TY: {
          v.visit(*as<FnTy>());
          break;
        }

        case NR_NODE_TMP: {
          v.visit(*as<Tmp>());
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

  static_assert(sizeof(Expr) == 8);

  class Type : public Expr {
    friend Expr;

    std::optional<uint64_t> getAlignBits() const;

  public:
    Type(nr_ty_t ty) : Expr(ty) {}

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
      switch (getKind()) {
        case NR_NODE_U1_TY:
        case NR_NODE_U8_TY:
        case NR_NODE_U16_TY:
        case NR_NODE_U32_TY:
        case NR_NODE_U64_TY:
        case NR_NODE_U128_TY:
        case NR_NODE_I8_TY:
        case NR_NODE_I16_TY:
        case NR_NODE_I32_TY:
        case NR_NODE_I64_TY:
        case NR_NODE_I128_TY:
        case NR_NODE_F16_TY:
        case NR_NODE_F32_TY:
        case NR_NODE_F64_TY:
        case NR_NODE_F128_TY:
        case NR_NODE_VOID_TY:
          return true;
        default:
          return false;
      }
    }

    constexpr bool is_array() const { return getKind() == NR_NODE_ARRAY_TY; }

    constexpr bool is_pointer() const { return getKind() == NR_NODE_PTR_TY; }

    constexpr bool is_readonly() const { return getKind() == NR_NODE_CONST_TY; }

    constexpr bool is_function() const { return getKind() == NR_NODE_FN_TY; }

    constexpr bool is_composite() const {
      switch (getKind()) {
        case NR_NODE_STRUCT_TY:
        case NR_NODE_UNION_TY:
        case NR_NODE_ARRAY_TY:
          return true;
        default:
          return false;
      }
    }

    constexpr bool is_union() const { return getKind() == NR_NODE_UNION_TY; }

    constexpr bool is_numeric() const {
      switch (getKind()) {
        case NR_NODE_U1_TY:
        case NR_NODE_U8_TY:
        case NR_NODE_U16_TY:
        case NR_NODE_U32_TY:
        case NR_NODE_U64_TY:
        case NR_NODE_U128_TY:
        case NR_NODE_I8_TY:
        case NR_NODE_I16_TY:
        case NR_NODE_I32_TY:
        case NR_NODE_I64_TY:
        case NR_NODE_I128_TY:
        case NR_NODE_F16_TY:
        case NR_NODE_F32_TY:
        case NR_NODE_F64_TY:
        case NR_NODE_F128_TY:
          return true;
        default:
          return false;
      }
    }

    constexpr bool is_integral() const {
      switch (getKind()) {
        case NR_NODE_U1_TY:
        case NR_NODE_U8_TY:
        case NR_NODE_U16_TY:
        case NR_NODE_U32_TY:
        case NR_NODE_U64_TY:
        case NR_NODE_U128_TY:
        case NR_NODE_I8_TY:
        case NR_NODE_I16_TY:
        case NR_NODE_I32_TY:
        case NR_NODE_I64_TY:
        case NR_NODE_I128_TY:
          return true;
        default:
          return false;
      }
    }

    constexpr bool is_floating_point() const {
      switch (getKind()) {
        case NR_NODE_F16_TY:
        case NR_NODE_F32_TY:
        case NR_NODE_F64_TY:
        case NR_NODE_F128_TY:
          return true;
        default:
          return false;
      }
    }

    constexpr bool is_signed() const {
      switch (getKind()) {
        case NR_NODE_I8_TY:
        case NR_NODE_I16_TY:
        case NR_NODE_I32_TY:
        case NR_NODE_I64_TY:
        case NR_NODE_I128_TY:
        case NR_NODE_F16_TY:
        case NR_NODE_F32_TY:
        case NR_NODE_F64_TY:
        case NR_NODE_F128_TY:
          return true;
        default:
          return false;
      }
    }

    constexpr bool is_unsigned() const {
      switch (getKind()) {
        case NR_NODE_U1_TY:
        case NR_NODE_U8_TY:
        case NR_NODE_U16_TY:
        case NR_NODE_U32_TY:
        case NR_NODE_U64_TY:
        case NR_NODE_U128_TY:
          return true;
        default:
          return false;
      }
    }

    constexpr bool is_void() const { return getKind() == NR_NODE_VOID_TY; }

    constexpr bool is_bool() const { return getKind() == NR_NODE_U1_TY; }
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

  class BinExpr final : public Expr {
    friend Expr;

    QCLASS_REFLECT()

    Expr *m_lhs;
    Expr *m_rhs;
    Op m_op;

  public:
    BinExpr(Expr *lhs, Expr *rhs, Op op)
        : Expr(NR_NODE_BINEXPR), m_lhs(lhs), m_rhs(rhs), m_op(op) {}

    Expr *getLHS() const { return m_lhs; }
    Expr *getRHS() const { return m_rhs; }
    Op getOp() const { return m_op; }

    Expr *setLHS(Expr *lhs) { return m_lhs = lhs; }
    Expr *setRHS(Expr *rhs) { return m_rhs = rhs; }
    Op setOp(Op op) { return m_op = op; }
  };

  class UnExpr final : public Expr {
    friend Expr;

    QCLASS_REFLECT()

    Expr *m_expr;
    Op m_op;

  public:
    UnExpr(Expr *expr, Op op) : Expr(NR_NODE_UNEXPR), m_expr(expr), m_op(op) {}

    Expr *getExpr() const { return m_expr; }
    Op getOp() const { return m_op; }

    Expr *setExpr(Expr *expr) { return m_expr = expr; }
    Op setOp(Op op) { return m_op = op; }
  };

  class PostUnExpr final : public Expr {
    friend Expr;

    QCLASS_REFLECT()

    Expr *m_expr;
    Op m_op;

  public:
    PostUnExpr(Expr *expr, Op op)
        : Expr(NR_NODE_POST_UNEXPR), m_expr(expr), m_op(op) {}

    Expr *getExpr() const { return m_expr; }
    Op getOp() const { return m_op; }

    Expr *setExpr(Expr *expr) { return m_expr = expr; }
    Op setOp(Op op) { return m_op = op; }
  };

  ///=============================================================================
  /// END: EXPRESSIONS CATEGORIES
  ///=============================================================================

  /// ===========================================================================
  /// BEGIN: PRIMITIVE TYPES
  /// ===========================================================================

  class U1Ty final : public Type {
    friend Expr;

    QCLASS_REFLECT()

  public:
    U1Ty() : Type(NR_NODE_U1_TY) {}
  };

  class U8Ty final : public Type {
    friend Expr;

    QCLASS_REFLECT()

  public:
    U8Ty() : Type(NR_NODE_U8_TY) {}
  };

  class U16Ty final : public Type {
    friend Expr;

    QCLASS_REFLECT()

  public:
    U16Ty() : Type(NR_NODE_U16_TY) {}
  };

  class U32Ty final : public Type {
    friend Expr;

    QCLASS_REFLECT()

  public:
    U32Ty() : Type(NR_NODE_U32_TY) {}
  };

  class U64Ty final : public Type {
    friend Expr;

    QCLASS_REFLECT()

  public:
    U64Ty() : Type(NR_NODE_U64_TY) {}
  };

  class U128Ty final : public Type {
    friend Expr;

    QCLASS_REFLECT()

  public:
    U128Ty() : Type(NR_NODE_U128_TY) {}
  };

  class I8Ty final : public Type {
    friend Expr;

    QCLASS_REFLECT()

  public:
    I8Ty() : Type(NR_NODE_I8_TY) {}
  };

  class I16Ty final : public Type {
    friend Expr;

    QCLASS_REFLECT()

  public:
    I16Ty() : Type(NR_NODE_I16_TY){};
  };

  class I32Ty final : public Type {
    friend Expr;

    QCLASS_REFLECT()

  public:
    I32Ty() : Type(NR_NODE_I32_TY) {}
  };

  class I64Ty final : public Type {
    friend Expr;

    QCLASS_REFLECT()

  public:
    I64Ty() : Type(NR_NODE_I64_TY) {}
  };

  class I128Ty final : public Type {
    friend Expr;

    QCLASS_REFLECT()

  public:
    I128Ty() : Type(NR_NODE_I128_TY) {}
  };

  class F16Ty final : public Type {
    friend Expr;

    QCLASS_REFLECT()

  public:
    F16Ty() : Type(NR_NODE_F16_TY) {}
  };

  class F32Ty final : public Type {
    friend Expr;

    QCLASS_REFLECT()

  public:
    F32Ty() : Type(NR_NODE_F32_TY) {}
  };

  class F64Ty final : public Type {
    friend Expr;

    QCLASS_REFLECT()

  public:
    F64Ty() : Type(NR_NODE_F64_TY) {}
  };

  class F128Ty final : public Type {
    friend Expr;

    QCLASS_REFLECT()

  public:
    F128Ty() : Type(NR_NODE_F128_TY) {}
  };

  class VoidTy final : public Type {
    friend Expr;

    QCLASS_REFLECT()

  public:
    VoidTy() : Type(NR_NODE_VOID_TY) {}
  };

  /// ===========================================================================
  /// END: PRIMITIVE TYPES
  /// ===========================================================================

  /// ===========================================================================
  /// BEGIN: COMPLEX TYPES
  /// ===========================================================================

  class PtrTy final : public Type {
    friend Expr;

    QCLASS_REFLECT()

    Type *m_pointee;
    uint8_t m_platform_ptr_size_bytes;

  public:
    PtrTy(Type *pointee, uint8_t platform_size_bytes = 8)
        : Type(NR_NODE_PTR_TY),
          m_pointee(pointee),
          m_platform_ptr_size_bytes(platform_size_bytes) {}

    Type *getPointee() const { return m_pointee; }
    uint8_t getPlatformPointerSizeBytes() const {
      return m_platform_ptr_size_bytes;
    }
  };

  class ConstTy final : public Type {
    friend Expr;

    QCLASS_REFLECT()

    Type *m_item;

  public:
    ConstTy(Type *item) : Type(NR_NODE_CONST_TY), m_item(item) {}

    Type *getItem() const { return m_item; }
  };

  class OpaqueTy final : public Type {
    friend Expr;

    QCLASS_REFLECT()

    std::string_view m_name;

  public:
    OpaqueTy(std::string_view name) : Type(NR_NODE_OPAQUE_TY), m_name(name) {}
  };

  typedef std::vector<Type *, Arena<Type *>> StructFields;

  class StructTy final : public Type {
    friend Expr;

    QCLASS_REFLECT()

    StructFields m_fields;

  public:
    StructTy(const StructFields &fields)
        : Type(NR_NODE_STRUCT_TY), m_fields(fields) {}

    const StructFields &getFields() const { return m_fields; }
  };

  typedef std::vector<Type *, Arena<Type *>> UnionFields;

  class UnionTy final : public Type {
    friend Expr;

    QCLASS_REFLECT()

    UnionFields m_fields;

  public:
    UnionTy(const UnionFields &fields)
        : Type(NR_NODE_UNION_TY), m_fields(fields) {}

    const UnionFields &getFields() const { return m_fields; }
  };

  class ArrayTy final : public Type {
    friend Expr;

    QCLASS_REFLECT()

    Type *m_element;
    size_t m_size;

  public:
    ArrayTy(Type *element, size_t size)
        : Type(NR_NODE_ARRAY_TY), m_element(element), m_size(size) {}

    Type *getElement() const { return m_element; }
    size_t getCount() const { return m_size; }
  };

  enum class FnAttr {
    Variadic,
  };

  typedef std::vector<Type *, Arena<Type *>> FnParams;
  typedef std::unordered_set<FnAttr, std::hash<FnAttr>, std::equal_to<FnAttr>,
                             Arena<FnAttr>>
      FnAttrs;

  class FnTy final : public Type {
    friend Expr;

    QCLASS_REFLECT()

    FnParams m_params;
    FnAttrs m_attrs;
    Type *m_return;
    uint8_t m_platform_ptr_size_bytes;

  public:
    FnTy(const FnParams &params, Type *ret, const FnAttrs &attrs,
         uint8_t platform_ptr_size_bytes = 8)
        : Type(NR_NODE_FN_TY),
          m_params(params),
          m_attrs(attrs),
          m_return(ret),
          m_platform_ptr_size_bytes(platform_ptr_size_bytes) {}

    const FnParams &getParams() const { return m_params; }
    Type *getReturn() const { return m_return; }
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

  class Int final : public Expr {
    friend Expr;

    QCLASS_REFLECT()

    struct map_hash {
      std::size_t operator()(std::pair<uint128_t, uint8_t> const &v) const {
        return std::hash<uint128_t>()(v.first) ^ std::hash<uint8_t>()(v.second);
      }
    };
    static std::unordered_map<std::pair<uint128_t, uint8_t>, Int *, map_hash>
        m_cache;

    unsigned __int128 m_value __attribute__((aligned(16)));
    uint8_t m_size;

    static uint128_t str2u128(std::string_view x);

  public:
    Int(uint128_t val, uint8_t size)
        : Expr(NR_NODE_INT), m_value(val), m_size(size) {}

    Int(std::string_view str, uint8_t size)
        : Expr(NR_NODE_INT), m_value(str2u128(str)) {
      m_size = size;
    }

    static Int *get(uint128_t val, uint8_t size);
    static Int *get(std::string_view str, uint8_t size) {
      return get(str2u128(str), size);
    }

    uint8_t getSize() const { return m_size; }
    uint128_t getValue() const { return m_value; }
    std::string getValueString() const;
  } __attribute__((packed));

  static_assert(sizeof(Int) == 48);

  enum class FloatSize : uint8_t {
    F16,
    F32,
    F64,
    F128,
  };

  class Float final : public Expr {
    friend Expr;

    QCLASS_REFLECT()

    double m_data;
    FloatSize m_size;

    static_assert(sizeof(double) == 8);

  public:
    Float(double dec, FloatSize size)
        : Expr(NR_NODE_FLOAT), m_data{dec}, m_size(size) {}
    Float(std::string_view str) : Expr(NR_NODE_FLOAT) {
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

  static_assert(sizeof(Float) == 17);

  typedef std::vector<Expr *, Arena<Expr *>> ListItems;

  class List final : public Expr {
    friend Expr;

    QCLASS_REFLECT()

    /// FIXME: Implement run-length compression

    ListItems m_items;
    bool m_is_homogenous;

  public:
    List(const ListItems &items, bool is_homogenous)
        : Expr(NR_NODE_LIST), m_items(items), m_is_homogenous(is_homogenous) {}

    auto begin() const { return m_items.begin(); }
    auto end() const { return m_items.end(); }
    size_t size() const { return m_items.size(); }

    Expr *operator[](size_t idx) const { return m_items[idx]; }
    Expr *at(size_t idx) const { return m_items.at(idx); }

    bool isHomogenous() const { return m_is_homogenous; }
  };

  ///=============================================================================
  /// END: LITERALS
  ///=============================================================================

  ///=============================================================================
  /// BEGIN: EXPRESSIONS
  ///=============================================================================

  typedef std::vector<Expr *, Arena<Expr *>> CallArgs;

  class Call final : public Expr {
    friend Expr;

    QCLASS_REFLECT()

    Expr *m_iref; /* Possibly cyclic reference to the target. */
    CallArgs m_args;

  public:
    Call(Expr *ref, const CallArgs &args)
        : Expr(NR_NODE_CALL), m_iref(ref), m_args(args) {}

    Expr *getTarget() const { return m_iref; }
    Expr *setTarget(Expr *ref) { return m_iref = ref; }

    const CallArgs &getArgs() const { return m_args; }
    CallArgs &getArgs() { return m_args; }
    void setArgs(const CallArgs &args) { m_args = args; }

    size_t getNumArgs() { return m_args.size(); }
  };

  typedef std::vector<Expr *, Arena<Expr *>> SeqItems;

  class Seq final : public Expr {
    friend Expr;

    QCLASS_REFLECT()

    SeqItems m_items;

  public:
    Seq(const SeqItems &items) : Expr(NR_NODE_SEQ), m_items(items) {}

    const SeqItems &getItems() const { return m_items; }
    SeqItems &getItems() { return m_items; }
    void setItems(const SeqItems &items) { m_items = items; }
  };

  class Index final : public Expr {
    friend Expr;

    QCLASS_REFLECT()

    Expr *m_expr;
    Expr *m_index;

  public:
    Index(Expr *expr, Expr *index)
        : Expr(NR_NODE_INDEX), m_expr(expr), m_index(index) {}

    Expr *getExpr() const { return m_expr; }
    Expr *setExpr(Expr *expr) { return m_expr = expr; }

    Expr *getIndex() const { return m_index; }
    Expr *setIndex(Expr *index) { return m_index = index; }
  };

  class Ident final : public Expr {
    friend Expr;

    QCLASS_REFLECT()

    std::string_view m_name;
    Expr *m_what;

  public:
    Ident(std::string_view name, Expr *what)
        : Expr(NR_NODE_IDENT), m_name(name), m_what(what) {}

    Expr *getWhat() const { return m_what; }
    Expr *setWhat(Expr *what) { return m_what = what; }

    std::string_view setName(std::string_view name) { return m_name = name; }
  };

  class Extern final : public Expr {
    friend Expr;

    QCLASS_REFLECT()

    std::string_view m_abi_name;
    Expr *m_value;

  public:
    Extern(Expr *value, std::string_view abi_name)
        : Expr(NR_NODE_EXTERN), m_abi_name(abi_name), m_value(value) {}

    std::string_view getAbiName() const { return m_abi_name; }
    std::string_view setAbiName(std::string_view abi_name) {
      return m_abi_name = abi_name;
    }

    Expr *getValue() const { return m_value; }
    Expr *setValue(Expr *value) { return m_value = value; }
  };

  class Local final : public Expr {
    friend Expr;

    QCLASS_REFLECT()

    std::string_view m_name;
    Expr *m_value;
    AbiTag m_abi_tag;
    StorageClass m_storage_class;
    bool m_readonly;

  public:
    Local(std::string_view name, Expr *value, AbiTag abi_tag,
          bool readonly = false,
          StorageClass storage_class = StorageClass::LLVM_StackAlloa)
        : Expr(NR_NODE_LOCAL),
          m_name(name),
          m_value(value),
          m_abi_tag(abi_tag),
          m_storage_class(storage_class),
          m_readonly(readonly) {}

    std::string_view setName(std::string_view name) { return m_name = name; }

    Expr *getValue() const { return m_value; }
    void setValue(Expr *value) { m_value = value; }

    AbiTag getAbiTag() const { return m_abi_tag; }
    void setAbiTag(AbiTag abi_tag) { m_abi_tag = abi_tag; }

    StorageClass getStorageClass() const { return m_storage_class; }
    void setStorageClass(StorageClass storage_class) {
      m_storage_class = storage_class;
    }

    bool isReadonly() const { return m_readonly; }
    void setReadonly(bool readonly) { m_readonly = readonly; }
  };

  class Ret final : public Expr {
    friend Expr;

    QCLASS_REFLECT()

    Expr *m_expr;

  public:
    Ret(Expr *expr) : Expr(NR_NODE_RET), m_expr(expr) {}

    Expr *getExpr() const { return m_expr; }
    Expr *setExpr(Expr *expr) { return m_expr = expr; }
  };

  class Brk final : public Expr {
    friend Expr;

    QCLASS_REFLECT()

  public:
    Brk() : Expr(NR_NODE_BRK) {}
  };

  class Cont final : public Expr {
    friend Expr;

    QCLASS_REFLECT()

  public:
    Cont() : Expr(NR_NODE_CONT) {}
  };

  class If final : public Expr {
    friend Expr;

    QCLASS_REFLECT()

    Expr *m_cond;
    Expr *m_then;
    Expr *m_else;

  public:
    If(Expr *cond, Expr *then, Expr *else_)
        : Expr(NR_NODE_IF), m_cond(cond), m_then(then), m_else(else_) {}

    Expr *getCond() const { return m_cond; }
    Expr *setCond(Expr *cond) { return m_cond = cond; }

    Expr *getThen() const { return m_then; }
    Expr *setThen(Expr *then) { return m_then = then; }

    Expr *getElse() const { return m_else; }
    Expr *setElse(Expr *else_) { return m_else = else_; }
  };

  class While final : public Expr {
    friend Expr;

    QCLASS_REFLECT()

    Expr *m_cond;
    Seq *m_body;

  public:
    While(Expr *cond, Seq *body)
        : Expr(NR_NODE_WHILE), m_cond(cond), m_body(body) {}

    Expr *getCond() const { return m_cond; }
    Expr *setCond(Expr *cond) { return m_cond = cond; }

    Seq *getBody() const { return m_body; }
    Seq *setBody(Seq *body) { return m_body = body; }
  };

  class For final : public Expr {
    friend Expr;

    QCLASS_REFLECT()

    Expr *m_init;
    Expr *m_cond;
    Expr *m_step;
    Expr *m_body;

  public:
    For(Expr *init, Expr *cond, Expr *step, Expr *body)
        : Expr(NR_NODE_FOR),
          m_init(init),
          m_cond(cond),
          m_step(step),
          m_body(body) {}

    Expr *getInit() const { return m_init; }
    Expr *setInit(Expr *init) { return m_init = init; }

    Expr *getCond() const { return m_cond; }
    Expr *setCond(Expr *cond) { return m_cond = cond; }

    Expr *getStep() const { return m_step; }
    Expr *setStep(Expr *step) { return m_step = step; }

    Expr *getBody() const { return m_body; }
    Expr *setBody(Expr *body) { return m_body = body; }
  };

  class Case final : public Expr {
    friend Expr;

    QCLASS_REFLECT()

    Expr *m_cond;
    Expr *m_body;

  public:
    Case(Expr *cond, Expr *body)
        : Expr(NR_NODE_CASE), m_cond(cond), m_body(body) {}

    Expr *getCond() { return m_cond; }
    Expr *setCond(Expr *cond) { return m_cond = cond; }

    Expr *getBody() { return m_body; }
    Expr *setBody(Expr *body) { return m_body = body; }
  };

  typedef std::vector<Case *, Arena<Case *>> SwitchCases;

  class Switch final : public Expr {
    friend Expr;

    QCLASS_REFLECT()

    Expr *m_cond;
    Expr *m_default;
    SwitchCases m_cases;

  public:
    Switch(Expr *cond, const SwitchCases &cases, Expr *default_)
        : Expr(NR_NODE_SWITCH),
          m_cond(cond),
          m_default(default_),
          m_cases(cases) {}

    Expr *getCond() const { return m_cond; }
    Expr *setCond(Expr *cond) { return m_cond = cond; }

    Expr *getDefault() const { return m_default; }
    Expr *setDefault(Expr *default_) { return m_default = default_; }

    const SwitchCases &getCases() const { return m_cases; }
    SwitchCases &getCases() { return m_cases; }
    void setCases(const SwitchCases &cases) { m_cases = cases; }
    void addCase(Case *c) { m_cases.push_back(c); }
  };

  typedef std::vector<std::pair<Type *, std::string_view>,
                      Arena<std::pair<Type *, std::string_view>>>
      Params;

  class Fn final : public Expr {
    friend Expr;

    QCLASS_REFLECT()

    std::string_view m_name;
    Params m_params;
    Type *m_return;
    std::optional<Seq *> m_body;
    bool m_variadic;
    AbiTag m_abi_tag;

  public:
    Fn(std::string_view name, const Params &params, Type *ret_ty,
       std::optional<Seq *> body, bool variadic, AbiTag abi_tag)
        : Expr(NR_NODE_FN),
          m_name(name),
          m_params(params),
          m_return(ret_ty),
          m_body(body),
          m_variadic(variadic),
          m_abi_tag(abi_tag) {}

    std::string_view setName(std::string_view name) { return m_name = name; }

    const Params &getParams() const { return m_params; }
    Params &getParams() { return m_params; }
    void setParams(const Params &params) { m_params = params; }

    Type *getReturn() const { return m_return; }
    Type *setReturn(Type *ret_ty) { return m_return = ret_ty; }

    std::optional<Seq *> getBody() const { return m_body; }
    std::optional<Seq *> setBody(std::optional<Seq *> body) {
      return m_body = body;
    }

    bool isVariadic() const { return m_variadic; }
    void setVariadic(bool variadic) { m_variadic = variadic; }

    AbiTag getAbiTag() const { return m_abi_tag; }
    AbiTag setAbiTag(AbiTag abi_tag) { return m_abi_tag = abi_tag; }
  };

  class Asm final : public Expr {
    friend Expr;

    QCLASS_REFLECT()

  public:
    Asm() : Expr(NR_NODE_ASM) { qcore_implement(); }
  };

  ///=============================================================================
  /// END: EXPRESSIONS
  ///=============================================================================

  enum class TmpType {
    CALL,
    NAMED_TYPE,
    DEFAULT_VALUE,
  };

  using CallArguments = std::vector<std::pair<std::string_view, Expr *>,
                                    Arena<std::pair<std::string_view, Expr *>>>;

  struct CallArgsTmpNodeCradle {
    Expr *base;
    CallArguments args;

    bool operator==(const CallArgsTmpNodeCradle &rhs) const {
      return base == rhs.base && args == rhs.args;
    }
  };

  typedef std::variant<CallArgsTmpNodeCradle, std::string_view> TmpNodeCradle;

  class Tmp final : public Type {
    friend Expr;

    QCLASS_REFLECT()

    TmpType m_type;
    TmpNodeCradle m_data;

  public:
    Tmp(TmpType type, const TmpNodeCradle &data = {})
        : Type(NR_NODE_TMP), m_type(type), m_data(data) {}

    TmpType getTmpType() { return m_type; }
    TmpNodeCradle &getData() { return m_data; }
    const TmpNodeCradle &getData() const { return m_data; }
  };

  ///=============================================================================

  constexpr Type *Expr::asType() {
#ifndef NDEBUG
    if (!isType()) {
      qcore_panicf("Failed to cast a non-type node `%s` to a type node",
                   getKindName());
    }
#endif
    return static_cast<Type *>(this);
  }

  constexpr std::optional<nr::Type *> nr::Expr::getType() const {
    Type *R = static_cast<Type *>(nr_infer(this, nullptr));

    if (R) {
      return R;
    } else {
      return std::nullopt;
    }
  }

  constexpr std::string_view Expr::getName() const {
    std::string_view R = "";

    switch (this->getKind()) {
      case NR_NODE_BINEXPR: {
        break;
      }

      case NR_NODE_UNEXPR: {
        break;
      }

      case NR_NODE_POST_UNEXPR: {
        break;
      }

      case NR_NODE_INT: {
        break;
      }

      case NR_NODE_FLOAT: {
        break;
      }

      case NR_NODE_LIST: {
        break;
      }

      case NR_NODE_CALL: {
        break;
      }

      case NR_NODE_SEQ: {
        break;
      }

      case NR_NODE_INDEX: {
        break;
      }

      case NR_NODE_IDENT: {
        R = as<Ident>()->m_name;
        break;
      }

      case NR_NODE_EXTERN: {
        break;
      }

      case NR_NODE_LOCAL: {
        R = as<Local>()->m_name;
        break;
      }

      case NR_NODE_RET: {
        break;
      }

      case NR_NODE_BRK: {
        break;
      }

      case NR_NODE_CONT: {
        break;
      }

      case NR_NODE_IF: {
        break;
      }

      case NR_NODE_WHILE: {
        break;
      }

      case NR_NODE_FOR: {
        break;
      }

      case NR_NODE_CASE: {
        break;
      }

      case NR_NODE_SWITCH: {
        break;
      }

      case NR_NODE_IGN: {
        break;
      }

      case NR_NODE_FN: {
        R = as<Fn>()->m_name;
        break;
      }

      case NR_NODE_ASM: {
        qcore_implement();
        break;
      }

      case NR_NODE_U1_TY: {
        break;
      }

      case NR_NODE_U8_TY: {
        break;
      }

      case NR_NODE_U16_TY: {
        break;
      }

      case NR_NODE_U32_TY: {
        break;
      }

      case NR_NODE_U64_TY: {
        break;
      }

      case NR_NODE_U128_TY: {
        break;
      }

      case NR_NODE_I8_TY: {
        break;
      }

      case NR_NODE_I16_TY: {
        break;
      }

      case NR_NODE_I32_TY: {
        break;
      }

      case NR_NODE_I64_TY: {
        break;
      }

      case NR_NODE_I128_TY: {
        break;
      }

      case NR_NODE_F16_TY: {
        break;
      }

      case NR_NODE_F32_TY: {
        break;
      }

      case NR_NODE_F64_TY: {
        break;
      }

      case NR_NODE_F128_TY: {
        break;
      }

      case NR_NODE_VOID_TY: {
        break;
      }

      case NR_NODE_PTR_TY: {
        break;
      }

      case NR_NODE_CONST_TY: {
        R = as<ConstTy>()->m_item->getName();
        break;
      }

      case NR_NODE_OPAQUE_TY: {
        R = as<OpaqueTy>()->m_name;
        break;
      }

      case NR_NODE_STRUCT_TY: {
        break;
      }

      case NR_NODE_UNION_TY: {
        break;
      }

      case NR_NODE_ARRAY_TY: {
        break;
      }

      case NR_NODE_FN_TY: {
        break;
      }

      case NR_NODE_TMP: {
        break;
      }
    }

    return R;
  }

  constexpr uint32_t Expr::getKindSize(nr_ty_t type) {
    const std::array<size_t, NR_NODE_COUNT> sizes = []() {
      std::array<size_t, NR_NODE_COUNT> R;
      R.fill(0);

      R[NR_NODE_BINEXPR] = sizeof(BinExpr);
      R[NR_NODE_UNEXPR] = sizeof(UnExpr);
      R[NR_NODE_POST_UNEXPR] = sizeof(PostUnExpr);
      R[NR_NODE_INT] = sizeof(Int);
      R[NR_NODE_FLOAT] = sizeof(Float);
      R[NR_NODE_LIST] = sizeof(List);
      R[NR_NODE_CALL] = sizeof(Call);
      R[NR_NODE_SEQ] = sizeof(Seq);
      R[NR_NODE_INDEX] = sizeof(Index);
      R[NR_NODE_IDENT] = sizeof(Ident);
      R[NR_NODE_EXTERN] = sizeof(Extern);
      R[NR_NODE_LOCAL] = sizeof(Local);
      R[NR_NODE_RET] = sizeof(Ret);
      R[NR_NODE_BRK] = sizeof(Brk);
      R[NR_NODE_CONT] = sizeof(Cont);
      R[NR_NODE_IF] = sizeof(If);
      R[NR_NODE_WHILE] = sizeof(While);
      R[NR_NODE_FOR] = sizeof(For);
      R[NR_NODE_CASE] = sizeof(Case);
      R[NR_NODE_SWITCH] = sizeof(Switch);
      R[NR_NODE_FN] = sizeof(Fn);
      R[NR_NODE_ASM] = sizeof(Asm);
      R[NR_NODE_IGN] = sizeof(Expr);
      R[NR_NODE_U1_TY] = sizeof(U1Ty);
      R[NR_NODE_U8_TY] = sizeof(U8Ty);
      R[NR_NODE_U16_TY] = sizeof(U16Ty);
      R[NR_NODE_U32_TY] = sizeof(U32Ty);
      R[NR_NODE_U64_TY] = sizeof(U64Ty);
      R[NR_NODE_U128_TY] = sizeof(U128Ty);
      R[NR_NODE_I8_TY] = sizeof(I8Ty);
      R[NR_NODE_I16_TY] = sizeof(I16Ty);
      R[NR_NODE_I32_TY] = sizeof(I32Ty);
      R[NR_NODE_I64_TY] = sizeof(I64Ty);
      R[NR_NODE_I128_TY] = sizeof(I128Ty);
      R[NR_NODE_F16_TY] = sizeof(F16Ty);
      R[NR_NODE_F32_TY] = sizeof(F32Ty);
      R[NR_NODE_F64_TY] = sizeof(F64Ty);
      R[NR_NODE_F128_TY] = sizeof(F128Ty);
      R[NR_NODE_VOID_TY] = sizeof(VoidTy);
      R[NR_NODE_PTR_TY] = sizeof(PtrTy);
      R[NR_NODE_CONST_TY] = sizeof(ConstTy);
      R[NR_NODE_OPAQUE_TY] = sizeof(OpaqueTy);
      R[NR_NODE_STRUCT_TY] = sizeof(StructTy);
      R[NR_NODE_UNION_TY] = sizeof(UnionTy);
      R[NR_NODE_ARRAY_TY] = sizeof(ArrayTy);
      R[NR_NODE_FN_TY] = sizeof(FnTy);
      R[NR_NODE_TMP] = sizeof(Tmp);

      return R;
    }();

    return sizes[type];
  }

  constexpr const char *Expr::getKindName(nr_ty_t type) {
    const std::array<const char *, NR_NODE_COUNT> names = []() {
      std::array<const char *, NR_NODE_COUNT> R;
      R.fill("");

      R[NR_NODE_BINEXPR] = "bin_expr";
      R[NR_NODE_UNEXPR] = "unary_expr";
      R[NR_NODE_POST_UNEXPR] = "post_unary_expr";
      R[NR_NODE_INT] = "int";
      R[NR_NODE_FLOAT] = "float";
      R[NR_NODE_LIST] = "list";
      R[NR_NODE_CALL] = "call";
      R[NR_NODE_SEQ] = "seq";
      R[NR_NODE_INDEX] = "index";
      R[NR_NODE_IDENT] = "ident";
      R[NR_NODE_EXTERN] = "extern";
      R[NR_NODE_LOCAL] = "local";
      R[NR_NODE_RET] = "return";
      R[NR_NODE_BRK] = "break";
      R[NR_NODE_CONT] = "continue";
      R[NR_NODE_IF] = "if";
      R[NR_NODE_WHILE] = "while";
      R[NR_NODE_FOR] = "for";
      R[NR_NODE_CASE] = "case";
      R[NR_NODE_SWITCH] = "switch";
      R[NR_NODE_FN] = "fn";
      R[NR_NODE_ASM] = "asm";
      R[NR_NODE_IGN] = "ignore";
      R[NR_NODE_U1_TY] = "u1";
      R[NR_NODE_U8_TY] = "u8";
      R[NR_NODE_U16_TY] = "u16";
      R[NR_NODE_U32_TY] = "u32";
      R[NR_NODE_U64_TY] = "u64";
      R[NR_NODE_U128_TY] = "u128";
      R[NR_NODE_I8_TY] = "i8";
      R[NR_NODE_I16_TY] = "i16";
      R[NR_NODE_I32_TY] = "i32";
      R[NR_NODE_I64_TY] = "i64";
      R[NR_NODE_I128_TY] = "i128";
      R[NR_NODE_F16_TY] = "f16";
      R[NR_NODE_F32_TY] = "f32";
      R[NR_NODE_F64_TY] = "f64";
      R[NR_NODE_F128_TY] = "f128";
      R[NR_NODE_VOID_TY] = "void";
      R[NR_NODE_PTR_TY] = "ptr";
      R[NR_NODE_CONST_TY] = "const";
      R[NR_NODE_OPAQUE_TY] = "opaque";
      R[NR_NODE_STRUCT_TY] = "struct";
      R[NR_NODE_UNION_TY] = "union";
      R[NR_NODE_ARRAY_TY] = "array";
      R[NR_NODE_FN_TY] = "fn_ty";
      R[NR_NODE_TMP] = "tmp";

      return R;
    }();

    return names[type];
  }

  constexpr bool nr::Expr::isSame(const nr::Expr *other) const {
    nr_ty_t kind = getKind();

    if (kind != other->getKind()) {
      return false;
    }

    switch (kind) {
      case NR_NODE_BINEXPR: {
        auto a = as<BinExpr>();
        auto b = other->as<BinExpr>();
        if (a->m_op != b->m_op) {
          return false;
        }
        return a->m_lhs->isSame(b->m_lhs) && a->m_rhs->isSame(b->m_rhs);
      }
      case NR_NODE_UNEXPR: {
        auto a = as<UnExpr>();
        auto b = other->as<UnExpr>();
        if (a->m_op != b->m_op) {
          return false;
        }
        return a->m_expr->isSame(b->m_expr);
      }
      case NR_NODE_POST_UNEXPR: {
        auto a = as<PostUnExpr>();
        auto b = other->as<PostUnExpr>();
        if (a->m_op != b->m_op) {
          return false;
        }
        return a->m_expr->isSame(b->m_expr);
      }
      case NR_NODE_INT: {
        return as<Int>()->getValue() == other->as<Int>()->getValue();
      }
      case NR_NODE_FLOAT: {
        return as<Float>()->getValue() == other->as<Float>()->getValue();
      }
      case NR_NODE_LIST: {
        auto a = as<List>();
        auto b = other->as<List>();
        if (a->m_items.size() != b->m_items.size()) {
          return false;
        }
        for (size_t i = 0; i < a->m_items.size(); i++) {
          if (!a->m_items[i]->isSame(b->m_items[i])) {
            return false;
          }
        }
        return true;
      }
      case NR_NODE_CALL: {
        auto a = as<Call>();
        auto b = other->as<Call>();
        if (!a->m_iref->isSame(b->m_iref)) {
          return false;
        }
        if (a->m_args.size() != b->m_args.size()) {
          return false;
        }
        for (size_t i = 0; i < a->m_args.size(); i++) {
          if (!a->m_args[i]->isSame(b->m_args[i])) {
            return false;
          }
        }
        return true;
      }
      case NR_NODE_SEQ: {
        auto a = as<Seq>();
        auto b = other->as<Seq>();
        if (a->m_items.size() != b->m_items.size()) {
          return false;
        }
        for (size_t i = 0; i < a->m_items.size(); i++) {
          if (!a->m_items[i]->isSame(b->m_items[i])) {
            return false;
          }
        }
        return true;
      }
      case NR_NODE_INDEX: {
        auto a = as<Index>();
        auto b = other->as<Index>();
        if (!a->m_expr->isSame(b->m_expr)) {
          return false;
        }
        if (!a->m_index->isSame(b->m_index)) {
          return false;
        }
        return true;
      }
      case NR_NODE_IDENT: {
        return as<Ident>()->m_name == other->as<Ident>()->m_name;
      }
      case NR_NODE_EXTERN: {
        auto a = as<Extern>();
        auto b = other->as<Extern>();
        if (a->m_abi_name != b->m_abi_name) {
          return false;
        }
        return a->m_value->isSame(b->m_value);
      }
      case NR_NODE_LOCAL: {
        auto a = as<Local>();
        auto b = other->as<Local>();
        if (a->m_name != b->m_name) {
          return false;
        }
        return a->m_value->isSame(b->m_value);
      }
      case NR_NODE_RET: {
        return as<Ret>()->m_expr->isSame(other->as<Ret>()->m_expr);
      }
      case NR_NODE_BRK: {
        return true;
      }
      case NR_NODE_CONT: {
        return true;
      }
      case NR_NODE_IF: {
        auto a = as<If>();
        auto b = other->as<If>();
        if (!a->m_cond->isSame(b->m_cond)) {
          return false;
        }
        if (!a->m_then->isSame(b->m_then)) {
          return false;
        }
        if (!a->m_else->isSame(b->m_else)) {
          return false;
        }
        return true;
      }
      case NR_NODE_WHILE: {
        auto a = as<While>();
        auto b = other->as<While>();
        if (!a->m_cond->isSame(b->m_cond)) {
          return false;
        }
        if (!a->m_body->isSame(b->m_body)) {
          return false;
        }
        return true;
      }
      case NR_NODE_FOR: {
        auto a = as<For>();
        auto b = other->as<For>();
        if (!a->m_init->isSame(b->m_init)) {
          return false;
        }
        if (!a->m_cond->isSame(b->m_cond)) {
          return false;
        }
        if (!a->m_step->isSame(b->m_step)) {
          return false;
        }
        if (!a->m_body->isSame(b->m_body)) {
          return false;
        }
        return true;
      }
      case NR_NODE_CASE: {
        auto a = as<Case>();
        auto b = other->as<Case>();
        if (!a->m_cond->isSame(b->m_cond)) {
          return false;
        }
        if (!a->m_body->isSame(b->m_body)) {
          return false;
        }
        return true;
      }
      case NR_NODE_SWITCH: {
        auto a = as<Switch>();
        auto b = other->as<Switch>();
        if (!a->m_cond->isSame(b->m_cond)) {
          return false;
        }
        if (!a->m_default->isSame(b->m_default)) {
          return false;
        }
        if (a->m_cases.size() != b->m_cases.size()) {
          return false;
        }
        for (size_t i = 0; i < a->m_cases.size(); i++) {
          if (!a->m_cases[i]->isSame(b->m_cases[i])) {
            return false;
          }
        }
        return true;
      }
      case NR_NODE_FN: {
        auto a = as<Fn>();
        auto b = other->as<Fn>();
        if (a->m_name != b->m_name) {
          return false;
        }
        if (!a->m_return->isSame(b->m_return)) {
          return false;
        }
        if (a->m_params.size() != b->m_params.size()) {
          return false;
        }
        for (size_t i = 0; i < a->m_params.size(); i++) {
          if (a->m_params[i].second != b->m_params[i].second) {
            return false;
          }
          if (!a->m_params[i].first->isSame(b->m_params[i].first)) {
            return false;
          }
        }
        if (a->m_body.has_value() && b->m_body.has_value()) {
          if (!a->m_body.value()->isSame(b->m_body.value())) {
            return false;
          }
        } else if (!a->m_body.has_value() ^ b->m_body.has_value()) {
          return false;
        }
        return true;
      }
      case NR_NODE_ASM: {
        qcore_implement();
        break;
      }
      case NR_NODE_IGN: {
        return true;
      }
      case NR_NODE_U1_TY:
      case NR_NODE_U8_TY:
      case NR_NODE_U16_TY:
      case NR_NODE_U32_TY:
      case NR_NODE_U64_TY:
      case NR_NODE_U128_TY:
      case NR_NODE_I8_TY:
      case NR_NODE_I16_TY:
      case NR_NODE_I32_TY:
      case NR_NODE_I64_TY:
      case NR_NODE_I128_TY:
      case NR_NODE_F16_TY:
      case NR_NODE_F32_TY:
      case NR_NODE_F64_TY:
      case NR_NODE_F128_TY:
      case NR_NODE_VOID_TY:
        return true;
      case NR_NODE_PTR_TY: {
        return as<PtrTy>()->m_pointee->isSame(other->as<PtrTy>()->m_pointee);
      }
      case NR_NODE_CONST_TY: {
        return as<ConstTy>()->m_item->isSame(other->as<ConstTy>()->m_item);
      }
      case NR_NODE_OPAQUE_TY: {
        return as<OpaqueTy>()->m_name == other->as<OpaqueTy>()->m_name;
      }
      case NR_NODE_STRUCT_TY: {
        auto a = as<StructTy>();
        auto b = other->as<StructTy>();
        if (a->m_fields.size() != b->m_fields.size()) {
          return false;
        }
        for (size_t i = 0; i < a->m_fields.size(); i++) {
          if (!a->m_fields[i]->isSame(b->m_fields[i])) {
            return false;
          }
        }
        return true;
      }
      case NR_NODE_UNION_TY: {
        auto a = as<UnionTy>();
        auto b = other->as<UnionTy>();
        if (a->m_fields.size() != b->m_fields.size()) {
          return false;
        }
        for (size_t i = 0; i < a->m_fields.size(); i++) {
          if (!a->m_fields[i]->isSame(b->m_fields[i])) {
            return false;
          }
        }
        return true;
      }
      case NR_NODE_ARRAY_TY: {
        auto a = as<ArrayTy>();
        auto b = other->as<ArrayTy>();
        if (!a->m_element->isSame(b->m_element)) {
          return false;
        }
        if (a->m_size != b->m_size) {
          return false;
        }
        return true;
      }
      case NR_NODE_FN_TY: {
        auto a = as<FnTy>();
        auto b = other->as<FnTy>();
        if (a->m_params.size() != b->m_params.size()) {
          return false;
        }
        for (size_t i = 0; i < a->m_params.size(); i++) {
          if (!a->m_params[i]->isSame(b->m_params[i])) {
            return false;
          }
        }
        if (!a->m_return->isSame(b->m_return)) {
          return false;
        }
        if (a->m_attrs != b->m_attrs) {
          return false;
        }
        return true;
      }
      case NR_NODE_TMP: {
        auto a = as<Tmp>();
        auto b = other->as<Tmp>();
        if (a->m_type != b->m_type) {
          return false;
        }

        switch (a->m_type) {
          case nr::TmpType::CALL: {
            const auto &AD = std::get<CallArgsTmpNodeCradle>(a->m_data);
            const auto &BD = std::get<CallArgsTmpNodeCradle>(b->m_data);

            if (AD.args.size() != BD.args.size()) {
              return false;
            }

            if (!AD.base->isSame(BD.base)) {
              return false;
            }

            for (size_t i = 0; i < AD.args.size(); i++) {
              if (AD.args[i].first != BD.args[i].first) {
                return false;
              }

              if (!AD.args[i].second->isSame(BD.args[i].second)) {
                return false;
              }
            }

            return true;
          }

          case nr::TmpType::DEFAULT_VALUE: {
            return std::get<std::string_view>(a->m_data) ==
                   std::get<std::string_view>(b->m_data);
          }

          case nr::TmpType::NAMED_TYPE: {
            return std::get<std::string_view>(a->m_data) ==
                   std::get<std::string_view>(b->m_data);
          }
        }

        qcore_implement();
      }
    }

    qcore_panic("unreachable");
  }

  Expr *createIgn();

  namespace mem {
    extern Brk static_NR_NODE_BRK;
    extern Cont static_NR_NODE_CONT;
    extern Expr static_NR_NODE_IGN;

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

    NORMAL_ALLOC(NR_NODE_BINEXPR);
    NORMAL_ALLOC(NR_NODE_UNEXPR);
    NORMAL_ALLOC(NR_NODE_POST_UNEXPR);
    CACHE_ALLOC(NR_NODE_INT);
    NORMAL_ALLOC(NR_NODE_FLOAT);
    NORMAL_ALLOC(NR_NODE_LIST);
    NORMAL_ALLOC(NR_NODE_CALL);
    NORMAL_ALLOC(NR_NODE_SEQ);
    NORMAL_ALLOC(NR_NODE_INDEX);
    NORMAL_ALLOC(NR_NODE_IDENT);
    NORMAL_ALLOC(NR_NODE_EXTERN);
    NORMAL_ALLOC(NR_NODE_LOCAL);
    NORMAL_ALLOC(NR_NODE_RET);
    REUSE_ALLOC(NR_NODE_BRK);
    REUSE_ALLOC(NR_NODE_CONT);
    NORMAL_ALLOC(NR_NODE_IF);
    NORMAL_ALLOC(NR_NODE_WHILE);
    NORMAL_ALLOC(NR_NODE_FOR);
    NORMAL_ALLOC(NR_NODE_CASE);
    NORMAL_ALLOC(NR_NODE_SWITCH);
    NORMAL_ALLOC(NR_NODE_FN);
    NORMAL_ALLOC(NR_NODE_ASM);
    REUSE_ALLOC(NR_NODE_IGN);
    NORMAL_ALLOC(NR_NODE_U1_TY);
    NORMAL_ALLOC(NR_NODE_U8_TY);
    NORMAL_ALLOC(NR_NODE_U16_TY);
    NORMAL_ALLOC(NR_NODE_U32_TY);
    NORMAL_ALLOC(NR_NODE_U64_TY);
    NORMAL_ALLOC(NR_NODE_U128_TY);
    NORMAL_ALLOC(NR_NODE_I8_TY);
    NORMAL_ALLOC(NR_NODE_I16_TY);
    NORMAL_ALLOC(NR_NODE_I32_TY);
    NORMAL_ALLOC(NR_NODE_I64_TY);
    NORMAL_ALLOC(NR_NODE_I128_TY);
    NORMAL_ALLOC(NR_NODE_F16_TY);
    NORMAL_ALLOC(NR_NODE_F32_TY);
    NORMAL_ALLOC(NR_NODE_F64_TY);
    NORMAL_ALLOC(NR_NODE_F128_TY);
    NORMAL_ALLOC(NR_NODE_VOID_TY);
    NORMAL_ALLOC(NR_NODE_PTR_TY);
    NORMAL_ALLOC(NR_NODE_CONST_TY);
    NORMAL_ALLOC(NR_NODE_OPAQUE_TY);
    NORMAL_ALLOC(NR_NODE_STRUCT_TY);
    NORMAL_ALLOC(NR_NODE_UNION_TY);
    NORMAL_ALLOC(NR_NODE_ARRAY_TY);
    NORMAL_ALLOC(NR_NODE_FN_TY);
    NORMAL_ALLOC(NR_NODE_TMP);

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

  template <typename T>
  std::optional<T> uint_as(const Expr *x) {
#define IS_T(x) std::is_same_v<T, x>

    qcore_assert(x != nullptr, "nr::evaluate_as(): x is nullptr.");

    static_assert(
        IS_T(std::string) || IS_T(uint64_t),
        "nr::evaluate_as(): T must be either std::string or uint64_t.");

    Expr *r = comptime_impl(const_cast<Expr *>(x)).value_or(nullptr);
    if (r == nullptr) {
      return std::nullopt;
    }

    nr_ty_t ty = r->getKind();

    if (ty != NR_NODE_INT) {
      return std::nullopt;
    }

    if constexpr (IS_T(std::string)) {
      return r->as<Int>()->getValue();
    } else if constexpr (IS_T(uint64_t)) {
      return r->as<Int>()->getValue();
    }

    return std::nullopt;

#undef IS_T
  }

  /** Add source debugging information to an IR node */
  template <typename T>
  static inline T *debug_info(T *N, uint32_t line, uint32_t col) {
    /// TODO: Store source location information
    (void)line;
    (void)col;

    return N;
  }
}  // namespace nr

namespace nr {
  template <auto mode = nr::dfs_pre>
  void for_each(const nr::Expr *const v,
                std::function<void(nr_ty_t, const nr::Expr *const)> f) {
    nr::iterate<mode>(v, [&](auto, auto c) -> nr::IterOp {
      f((*c)->getKind(), *c);

      return nr::IterOp::Abort;
    });
  }

  template <auto mode = nr::dfs_pre>
  void transform(nr::Expr *v, std::function<bool(nr_ty_t, nr::Expr **)> f) {
    nr::iterate<mode>(v, [&](auto, auto c) -> nr::IterOp {
      return f((*c)->getKind(), c) ? nr::IterOp::Proceed : nr::IterOp::Abort;
    });
  }

  template <typename T, auto mode = nr::dfs_pre>
  void for_each(const nr::Expr *const v, std::function<void(const T *)> f) {
    nr::iterate<mode>(v,
                      [&](auto, const nr::Expr *const *const c) -> nr::IterOp {
                        if ((*c)->getKind() != nr::Expr::getTypeCode<T>()) {
                          return nr::IterOp::Proceed;
                        }

                        f((*c)->as<T>());

                        return nr::IterOp::Proceed;
                      });
  }

  template <typename T, auto mode = nr::dfs_pre>
  void transform(nr::Expr *v, std::function<bool(T **)> f) {
    nr::iterate<mode>(v, [&](auto, auto c) -> nr::IterOp {
      if ((*c)->getKind() != nr::Expr::getTypeCode<T>()) {
        return nr::IterOp::Proceed;
      }

      return f(reinterpret_cast<T **>(c)) ? nr::IterOp::Proceed
                                          : nr::IterOp::Abort;
    });
  }
}  // namespace nr

#endif
