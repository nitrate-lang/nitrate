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

#pragma once

#include <cstdint>
#include <nitrate-alpha/tree/Base.hh>

namespace ncc::alpha::tree {
  class IR_eINT final : public Base {
  public:
    enum IntType : uint8_t {
      kField8 = 1,
      kField16 = 2,
      kField32 = 4,
      kField64 = 8,
      kField128 = 16,
    };

    using UInt128 = unsigned __int128;
    using Int128 = signed __int128;

  private:
    constexpr explicit IR_eINT(Int128 val, IntType ty)
        : Base(IRKind::AIR_eINT), m_value(PrepareSigned(val, ty)), m_type(ty) {}
    constexpr explicit IR_eINT(UInt128 val, IntType ty)
        : Base(IRKind::AIR_eINT), m_value(PrepareUnsigned(val, ty)), m_type(ty) {}

  public:
    constexpr IR_eINT(const IR_eINT &) = default;
    constexpr IR_eINT(IR_eINT &&) = default;
    constexpr IR_eINT &operator=(const IR_eINT &) = default;
    constexpr IR_eINT &operator=(IR_eINT &&) = default;

    [[nodiscard, gnu::pure]] constexpr static auto CreateSigned(Int128 val, IntType ty) { return IR_eINT(val, ty); }
    [[nodiscard, gnu::pure]] constexpr static auto CreateUnsigned(UInt128 val, IntType ty) { return IR_eINT(val, ty); }

    [[nodiscard, gnu::pure]] constexpr auto GetType() const -> IntType { return m_type; }
    [[nodiscard, gnu::pure]] constexpr auto GetTypeName() const -> std::string_view {
      switch (m_type) {
        case IntType::kField8:
          return "kField8";
        case IntType::kField16:
          return "kField16";
        case IntType::kField32:
          return "kField32";
        case IntType::kField64:
          return "kField64";
        case IntType::kField128:
          return "kField128";
      }
    }

    [[nodiscard, gnu::pure]] constexpr auto GetSigned() const -> Int128 { return static_cast<Int128>(m_value); }
    [[nodiscard, gnu::pure]] constexpr auto GetUnsigned() const -> UInt128 { return m_value; }

    [[nodiscard, gnu::pure]] constexpr auto GetI8() const { return static_cast<int8_t>(GetSigned()); }
    [[nodiscard, gnu::pure]] constexpr auto GetI16() const { return static_cast<int16_t>(GetSigned()); }
    [[nodiscard, gnu::pure]] constexpr auto GetI32() const { return static_cast<int32_t>(GetSigned()); }
    [[nodiscard, gnu::pure]] constexpr auto GetI64() const { return static_cast<int64_t>(GetSigned()); }
    [[nodiscard, gnu::pure]] constexpr auto GetI128() const { return static_cast<Int128>(GetSigned()); }
    [[nodiscard, gnu::pure]] constexpr auto GetU8() const { return static_cast<uint8_t>(GetUnsigned()); }
    [[nodiscard, gnu::pure]] constexpr auto GetU16() const { return static_cast<uint16_t>(GetUnsigned()); }
    [[nodiscard, gnu::pure]] constexpr auto GetU32() const { return static_cast<uint32_t>(GetUnsigned()); }
    [[nodiscard, gnu::pure]] constexpr auto GetU64() const { return static_cast<uint64_t>(GetUnsigned()); }
    [[nodiscard, gnu::pure]] constexpr auto GetU128() const { return static_cast<UInt128>(GetUnsigned()); }

    [[nodiscard, gnu::pure]] constexpr auto IsZero() const -> bool { return GetSigned() == 0; }
    [[nodiscard, gnu::pure]] constexpr auto IsNegative() const -> bool { return GetSigned() < 0; }
    [[nodiscard, gnu::pure]] constexpr auto IsPositive() const -> bool { return GetSigned() > 0; }
    [[nodiscard, gnu::pure]] constexpr auto IsPositiveOrZero() const -> bool { return GetSigned() >= 0; }
    [[nodiscard, gnu::pure]] constexpr auto IsNegativeOrZero() const -> bool { return GetSigned() <= 0; }

    constexpr auto SetSigned(Int128 val) -> IR_eINT & {
      m_value = PrepareSigned(val, m_type);
      return *this;
    }

    constexpr auto SetUnsigned(UInt128 val) -> IR_eINT & {
      m_value = PrepareUnsigned(val, m_type);
      return *this;
    }

    constexpr auto operator=(Int128 x) -> IR_eINT & { return SetSigned(x); }

  private:
    UInt128 m_value;
    IntType m_type : 5;
    [[maybe_unused]] uint8_t m_pad : 3;

    [[nodiscard, gnu::pure]] constexpr static auto PrepareSigned(Int128 x, IntType ty) -> UInt128 {
      const Int128 lower_bound = -(UInt128(1) << (8 * ty - 1));
      const Int128 upper_bound = (UInt128(1) << (8 * ty - 1)) - 1;
      x = (x < lower_bound) ? lower_bound : x;
      x = (x > upper_bound) ? upper_bound : x;
      if (x < 0) {  // 2's complement
        x = -x;
        x = ~x + 1;
      }

      return static_cast<UInt128>(x);  // TODO: check if this is correct
    }

    [[nodiscard, gnu::pure]] constexpr static auto PrepareUnsigned(UInt128 x, IntType ty) -> UInt128 {
      const auto mask = (UInt128(1) << (8 * ty)) - 1;
      return x & mask;
    }
  } __attribute__((packed));

  class IR_eFLOAT final : public Base {
  public:
    using Float128 = __float128;

    enum FloatType : uint8_t {
      kField16 = 2,
      kField32 = 4,
      kField64 = 8,
      kField128 = 16,
    };

  private:
    constexpr IR_eFLOAT(Float128 val, FloatType ty) : Base(IRKind::AIR_eFLOAT), m_type(ty), m_value(Prepare(val, ty)) {}

  public:
    constexpr IR_eFLOAT(const IR_eFLOAT &) = default;
    constexpr IR_eFLOAT(IR_eFLOAT &&) = default;
    constexpr IR_eFLOAT &operator=(const IR_eFLOAT &) = default;
    constexpr IR_eFLOAT &operator=(IR_eFLOAT &&) = default;

    [[nodiscard, gnu::pure]] constexpr static auto Create(Float128 val, FloatType ty) { return IR_eFLOAT(val, ty); }

    [[nodiscard, gnu::pure]] constexpr auto GetType() const -> FloatType { return m_type; }
    [[nodiscard, gnu::pure]] constexpr auto GetTypeName() const -> std::string_view {
      switch (m_type) {
        case FloatType::kField16:
          return "kField16";
        case FloatType::kField32:
          return "kField32";
        case FloatType::kField64:
          return "kField64";
        case FloatType::kField128:
          return "kField128";
      }
    }

    [[nodiscard, gnu::pure]] constexpr auto GetValue() const -> Float128 { return m_value; }

    [[nodiscard, gnu::pure]] constexpr auto IsZero() const -> bool { return m_value == 0.0; }
    [[nodiscard, gnu::pure]] constexpr auto IsNegative() const -> bool { return m_value < 0.0; }
    [[nodiscard, gnu::pure]] constexpr auto IsPositive() const -> bool { return m_value > 0.0; }
    [[nodiscard, gnu::pure]] constexpr auto IsPositiveOrZero() const -> bool { return m_value >= 0.0; }
    [[nodiscard, gnu::pure]] constexpr auto IsNegativeOrZero() const -> bool { return m_value <= 0.0; }

  private:
    FloatType m_type : 5;
    [[maybe_unused]] uint8_t m_pad : 3;
    Float128 m_value;

    [[nodiscard, gnu::pure]] constexpr static auto Prepare(Float128 x, FloatType ty) -> Float128 {
      /// FIXME: Clip to the range of the type
      return x;
    }
  } __attribute__((packed));
}  // namespace ncc::alpha::tree
