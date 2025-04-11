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

#include <nitrate-alpha/tree/Base.hh>
#include <nitrate-core/FlowPtr.hh>
#include <span>
#include <vector>

namespace ncc::alpha::tree {
  class IR_tINT final : public Base {
  public:
    enum IntType : uint8_t {
      kU1 = 0b00000000,
      kU8 = 0b00000111,
      kI8 = 0b10000111,
      kU16 = 0b00001111,
      kI16 = 0b10001111,
      kU32 = 0b00011111,
      kI32 = 0b10011111,
      kU64 = 0b00111111,
      kI64 = 0b10111111,
      kU128 = 0b01111111,
      kI128 = 0b11111111,
    };

    constexpr IR_tINT(IntType type) : Base(IRKind::AIR_tINT), m_type(type) {}
    constexpr IR_tINT(const IR_tINT &) = default;
    constexpr IR_tINT(IR_tINT &&) = default;
    constexpr IR_tINT &operator=(const IR_tINT &) = default;
    constexpr IR_tINT &operator=(IR_tINT &&) noexcept = default;

    [[nodiscard, gnu::pure]] constexpr auto GetIntType() const -> IntType { return m_type; }
    [[nodiscard, gnu::pure]] constexpr auto GetBitSize() const -> uint8_t { return (m_type & 0x7f) + 1; }

  private:
    IntType m_type;
  } __attribute__((packed));

  class IR_tFLOAT final : public Base {
    /// TODO: Implement float types
  } __attribute__((packed));

  class IR_tVOID final : public Base {
  public:
    constexpr IR_tVOID() : Base(IRKind::AIR_tVOID) {}
    constexpr IR_tVOID(const IR_tVOID &) = default;
    constexpr IR_tVOID(IR_tVOID &&) = default;
    constexpr IR_tVOID &operator=(const IR_tVOID &) = default;
    constexpr IR_tVOID &operator=(IR_tVOID &&) noexcept = default;
  } __attribute__((packed));

  class IR_tINFER final : public Base {
  public:
    constexpr IR_tINFER() : Base(IRKind::AIR_tINFER) {}
    constexpr IR_tINFER(const IR_tINFER &) = default;
    constexpr IR_tINFER(IR_tINFER &&) = default;
    constexpr IR_tINFER &operator=(const IR_tINFER &) = default;
    constexpr IR_tINFER &operator=(IR_tINFER &&) noexcept = default;
  } __attribute__((packed));

  class IR_tREF final : public Base {
    FlowPtr<Base> m_refee;

  public:
    constexpr IR_tREF(FlowPtr<Base> refee) : Base(IRKind::AIR_tREF), m_refee(std::move(refee)) {}
    constexpr IR_tREF(const IR_tREF &) = default;
    constexpr IR_tREF(IR_tREF &&) = default;
    constexpr IR_tREF &operator=(const IR_tREF &) = default;
    constexpr IR_tREF &operator=(IR_tREF &&) noexcept = default;

    [[nodiscard, gnu::pure]] constexpr auto GetRefee() const -> FlowPtr<const Base> { return m_refee; }
  } __attribute__((packed));

  class IR_tPTR final : public Base {
    FlowPtr<Base> m_pointee;

  public:
    constexpr IR_tPTR(FlowPtr<Base> pointee) : Base(IRKind::AIR_tPTR), m_pointee(std::move(pointee)) {}
    constexpr IR_tPTR(const IR_tPTR &) = default;
    constexpr IR_tPTR(IR_tPTR &&) = default;
    constexpr IR_tPTR &operator=(const IR_tPTR &) = default;
    constexpr IR_tPTR &operator=(IR_tPTR &&) noexcept = default;

    [[nodiscard, gnu::pure]] constexpr auto GetPointee() const -> FlowPtr<const Base> { return m_pointee; }
  } __attribute__((packed));

  class IR_tARRAY final : public Base {
    FlowPtr<Base> m_element_type;
    uint64_t m_size;

  public:
    constexpr IR_tARRAY(FlowPtr<Base> element_type, uint64_t size)
        : Base(IRKind::AIR_tARRAY), m_element_type(std::move(element_type)), m_size(size) {}
    constexpr IR_tARRAY(const IR_tARRAY &) = default;
    constexpr IR_tARRAY(IR_tARRAY &&) = default;
    constexpr IR_tARRAY &operator=(const IR_tARRAY &) = default;
    constexpr IR_tARRAY &operator=(IR_tARRAY &&) noexcept = default;

    [[nodiscard, gnu::pure]] constexpr auto GetElementType() const -> FlowPtr<const Base> { return m_element_type; }
    [[nodiscard, gnu::pure]] constexpr auto GetSize() const -> uint64_t { return m_size; }
  } __attribute__((packed));

  class IR_tTUPLE final : public Base {
  public:
    using TupleElementTypes = std::pmr::vector<FlowPtr<Base>>;

    IR_tTUPLE(TupleElementTypes element_types) : Base(IRKind::AIR_tTUPLE), m_element_types(std::move(element_types)) {}
    constexpr IR_tTUPLE(const IR_tTUPLE &) = default;
    constexpr IR_tTUPLE(IR_tTUPLE &&) = default;
    IR_tTUPLE &operator=(const IR_tTUPLE &) = default;
    IR_tTUPLE &operator=(IR_tTUPLE &&) noexcept = default;

    [[nodiscard, gnu::pure]] constexpr auto GetElementTypes() const -> std::span<const FlowPtr<Base>> {
      return m_element_types;
    }

    [[nodiscard, gnu::pure]] constexpr auto operator->() const -> const TupleElementTypes * { return &m_element_types; }

  private:
    TupleElementTypes m_element_types;
  };

  class IR_tFUNCTION final : public Base {
  public:
    using FunctionArgumentTypes = std::pmr::vector<FlowPtr<Base>>;
    using FunctionAttributes = std::pmr::vector<FlowPtr<Base>>;

    IR_tFUNCTION(FunctionArgumentTypes argument_types, FlowPtr<Base> return_type,
                 FunctionAttributes attributes = FunctionAttributes())
        : Base(IRKind::AIR_tFUNCTION),
          m_return_type(std::move(return_type)),
          m_argument_types(std::move(argument_types)),
          m_attributes(std::move(attributes)) {}
    constexpr IR_tFUNCTION(const IR_tFUNCTION &) = default;
    constexpr IR_tFUNCTION(IR_tFUNCTION &&) = default;
    IR_tFUNCTION &operator=(const IR_tFUNCTION &) = default;
    IR_tFUNCTION &operator=(IR_tFUNCTION &&) noexcept = default;

    [[nodiscard, gnu::pure]] constexpr auto GetReturnType() const -> FlowPtr<const Base> { return m_return_type; }

    [[nodiscard, gnu::pure]] constexpr auto GetArgumentTypes() const -> std::span<const FlowPtr<Base>> {
      return m_argument_types;
    }

    [[nodiscard, gnu::pure]] constexpr auto GetAttributes() const -> std::span<const FlowPtr<Base>> {
      return m_attributes;
    }

  private:
    FlowPtr<Base> m_return_type;
    FunctionArgumentTypes m_argument_types;
    FunctionAttributes m_attributes;
  };
}  // namespace ncc::alpha::tree
