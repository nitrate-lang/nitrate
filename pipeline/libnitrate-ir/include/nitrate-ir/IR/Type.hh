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

#ifndef __NITRATE_IR_GRAPH_TYPE_H__
#define __NITRATE_IR_GRAPH_TYPE_H__

#include <nitrate-ir/IR/Base.hh>

namespace ncc::ir {
  template <class A>
  class GenericU1Ty final : public GenericType<A> {
    friend A;

  public:
    constexpr GenericU1Ty() : GenericType<A>(IR_tU1) {}
  };

  template <class A>
  class GenericU8Ty final : public GenericType<A> {
    friend A;

  public:
    constexpr GenericU8Ty() : GenericType<A>(IR_tU8) {}
  };

  template <class A>
  class GenericU16Ty final : public GenericType<A> {
    friend A;

  public:
    constexpr GenericU16Ty() : GenericType<A>(IR_tU16) {}
  };

  template <class A>
  class GenericU32Ty final : public GenericType<A> {
    friend A;

  public:
    constexpr GenericU32Ty() : GenericType<A>(IR_tU32) {}
  };

  template <class A>
  class GenericU64Ty final : public GenericType<A> {
    friend A;

  public:
    constexpr GenericU64Ty() : GenericType<A>(IR_tU64) {}
  };

  template <class A>
  class GenericU128Ty final : public GenericType<A> {
    friend A;

  public:
    constexpr GenericU128Ty() : GenericType<A>(IR_tU128) {}
  };

  template <class A>
  class GenericI8Ty final : public GenericType<A> {
    friend A;

  public:
    constexpr GenericI8Ty() : GenericType<A>(IR_tI8) {}
  };

  template <class A>
  class GenericI16Ty final : public GenericType<A> {
    friend A;

  public:
    constexpr GenericI16Ty() : GenericType<A>(IR_tI16){};
  };

  template <class A>
  class GenericI32Ty final : public GenericType<A> {
    friend A;

  public:
    constexpr GenericI32Ty() : GenericType<A>(IR_tI32) {}
  };

  template <class A>
  class GenericI64Ty final : public GenericType<A> {
    friend A;

  public:
    constexpr GenericI64Ty() : GenericType<A>(IR_tI64) {}
  };

  template <class A>
  class GenericI128Ty final : public GenericType<A> {
    friend A;

  public:
    constexpr GenericI128Ty() : GenericType<A>(IR_tI128) {}
  };

  template <class A>
  class GenericF16Ty final : public GenericType<A> {
    friend A;

  public:
    constexpr GenericF16Ty() : GenericType<A>(IR_tF16_TY) {}
  };

  template <class A>
  class GenericF32Ty final : public GenericType<A> {
    friend A;

  public:
    constexpr GenericF32Ty() : GenericType<A>(IR_tF32_TY) {}
  };

  template <class A>
  class GenericF64Ty final : public GenericType<A> {
    friend A;

  public:
    constexpr GenericF64Ty() : GenericType<A>(IR_tF64_TY) {}
  };

  template <class A>
  class GenericF128Ty final : public GenericType<A> {
    friend A;

  public:
    constexpr GenericF128Ty() : GenericType<A>(IR_tF128_TY) {}
  };

  template <class A>
  class GenericVoidTy final : public GenericType<A> {
    friend A;

  public:
    constexpr GenericVoidTy() : GenericType<A>(IR_tVOID) {}
  };

  template <class A>
  class GenericPtrTy final : public GenericType<A> {
    friend A;

    FlowPtr<GenericType<A>> m_pointee;
    uint8_t m_native_size;

  public:
    constexpr GenericPtrTy(auto pointee, size_t native_size = 8)
        : GenericType<A>(IR_tPTR),
          m_pointee(pointee),
          m_native_size(native_size) {}

    [[nodiscard]] constexpr auto GetPointee() const { return m_pointee; }
    [[nodiscard]] constexpr auto GetNativeSize() const { return m_native_size; }
  };

  template <class A>
  class GenericConstTy final : public GenericType<A> {
    friend A;

    FlowPtr<GenericType<A>> m_item;

  public:
    constexpr GenericConstTy(auto item)
        : GenericType<A>(IR_tCONST), m_item(item) {}

    [[nodiscard]] constexpr auto GetItem() const { return m_item; }
  };

  template <class A>
  class GenericOpaqueTy final : public GenericType<A> {
    friend A;

    string m_name;

  public:
    constexpr GenericOpaqueTy(auto name)
        : GenericType<A>(IR_tOPAQUE), m_name(name) {}

    [[nodiscard]] constexpr auto GetName() const { return m_name.Get(); }
  };

  template <class A>
  class GenericStructTy final : public GenericType<A> {
    friend A;

    std::span<FlowPtr<GenericType<A>>> m_fields;

  public:
    constexpr GenericStructTy(auto fields)
        : GenericType<A>(IR_tSTRUCT), m_fields(fields) {}

    [[nodiscard]] constexpr auto GetFields() const { return m_fields; }
  };

  template <class A>
  class GenericUnionTy final : public GenericType<A> {
    friend A;

    std::span<FlowPtr<GenericType<A>>> m_fields;

  public:
    constexpr GenericUnionTy(auto fields)
        : GenericType<A>(IR_tUNION), m_fields(fields) {}

    [[nodiscard]] constexpr auto GetFields() const { return m_fields; }
  };

  template <class A>
  class GenericArrayTy final : public GenericType<A> {
    friend A;

    FlowPtr<GenericType<A>> m_element;
    size_t m_size;

  public:
    constexpr GenericArrayTy(auto element, auto size)
        : GenericType<A>(IR_tARRAY), m_element(element), m_size(size) {}

    [[nodiscard]] constexpr auto GetElement() const { return m_element; }
    [[nodiscard]] constexpr auto GetCount() const { return m_size; }
  };

  template <class A>
  class GenericFnTy final : public GenericType<A> {
    friend A;

    std::span<FlowPtr<GenericType<A>>> m_params;
    FlowPtr<GenericType<A>> m_return;
    uint8_t m_native_size;
    bool m_variadic;

  public:
    constexpr GenericFnTy(auto params, auto ret, auto variadic,
                          size_t native_size = 8)
        : GenericType<A>(IR_tFUNC),
          m_params(params),
          m_return(ret),
          m_native_size(native_size),
          m_variadic(variadic) {}

    [[nodiscard]] constexpr auto GetParams() const { return m_params; }
    [[nodiscard]] constexpr auto GetReturn() const { return m_return; }
    [[nodiscard]] constexpr auto IsVariadic() const { return m_variadic; }
    [[nodiscard]] constexpr auto GetNativeSize() const { return m_native_size; }
  };

  template <class A>
  class GenericTmp final : public GenericType<A> {
    friend A;

    TmpType m_type;
    TmpNodeCradle<A> m_data;

  public:
    GenericTmp(TmpType type, TmpNodeCradle<A> data = {})
        : GenericType<A>(IR_tTMP), m_type(type), m_data(data) {}

    auto GetTmpType() { return m_type; }
    [[nodiscard]] auto GetData() const { return m_data; }
  };

  static inline auto GetU1Ty() -> U1Ty* {
    static U1Ty u1;
    return &u1;
  }

  static inline auto GetU8Ty() -> U8Ty* {
    static U8Ty u8;
    return &u8;
  }

  static inline auto GetU16Ty() -> U16Ty* {
    static U16Ty u16;
    return &u16;
  }

  static inline auto GetU32Ty() -> U32Ty* {
    static U32Ty u32;
    return &u32;
  }

  static inline auto GetU64Ty() -> U64Ty* {
    static U64Ty u64;
    return &u64;
  }

  static inline auto GetU128Ty() -> U128Ty* {
    static U128Ty u128;
    return &u128;
  }

  static inline auto GetI8Ty() -> I8Ty* {
    static I8Ty i8;
    return &i8;
  }

  static inline auto GetI16Ty() -> I16Ty* {
    static I16Ty i16;
    return &i16;
  }

  static inline auto GetI32Ty() -> I32Ty* {
    static I32Ty i32;
    return &i32;
  }

  static inline auto GetI64Ty() -> I64Ty* {
    static I64Ty i64;
    return &i64;
  }

  static inline auto GetI128Ty() -> I128Ty* {
    static I128Ty i128;
    return &i128;
  }

  static inline auto GetF16Ty() -> F16Ty* {
    static F16Ty f16;
    return &f16;
  }

  static inline auto GetF32Ty() -> F32Ty* {
    static F32Ty f32;
    return &f32;
  }

  static inline auto GetF64Ty() -> F64Ty* {
    static F64Ty f64;
    return &f64;
  }

  static inline auto GetF128Ty() -> F128Ty* {
    static F128Ty f128;
    return &f128;
  }

  static inline auto GetVoidTy() -> VoidTy* {
    static VoidTy void_ty;
    return &void_ty;
  }

  auto GetPtrTy(FlowPtr<Type> pointee, uint8_t native_size = 8) -> PtrTy*;
  auto GetConstTy(FlowPtr<Type> item) -> ConstTy*;
  auto GetOpaqueTy(string name) -> OpaqueTy*;
  auto GetStructTy(std::span<FlowPtr<Type>> fields) -> StructTy*;
  auto GetUnionTy(std::span<FlowPtr<Type>> fields) -> UnionTy*;
  auto GetArrayTy(FlowPtr<Type> element, size_t size) -> ArrayTy*;
  auto GetFnTy(std::span<FlowPtr<Type>> params, FlowPtr<Type> ret,
               bool variadic, size_t native_size = 8) -> FnTy*;
}  // namespace ncc::ir

#endif
