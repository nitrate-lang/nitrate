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
#include <nitrate-core/FlowPtr.hh>

namespace ncc::alpha::tree {
  class IR_eTUPLE final : public Base {
  public:
    using TupleElements = std::pmr::vector<FlowPtr<Base>>;

    IR_eTUPLE(TupleElements elements) : Base(IRKind::AIR_eTUPLE), m_elements(std::move(elements)) {}
    constexpr IR_eTUPLE(const IR_eTUPLE &) = default;
    constexpr IR_eTUPLE(IR_eTUPLE &&) = default;
    IR_eTUPLE &operator=(const IR_eTUPLE &) = default;
    IR_eTUPLE &operator=(IR_eTUPLE &&) noexcept = default;

    [[nodiscard, gnu::pure]] constexpr auto GetElements() const -> std::span<const FlowPtr<Base>> { return m_elements; }

    [[nodiscard, gnu::pure]] constexpr auto GetElements() -> TupleElements & {
      SetDirtyBit();
      return m_elements;
    }

    [[nodiscard, gnu::pure]] constexpr auto operator->() const -> const TupleElements * { return &m_elements; }

    [[nodiscard, gnu::pure]] constexpr auto operator->() -> TupleElements * {
      SetDirtyBit();
      return &m_elements;
    }

    void SetElements(TupleElements elements) {
      m_elements = std::move(elements);
      SetDirtyBit();
    }

  private:
    TupleElements m_elements;
  };

  class IR_eBIN final : public Base {
  public:
    enum Op : uint8_t {
      Add,       /* Addition */
      Sub,       /* Subtraction */
      Mul,       /* Multiplication */
      sDiv,      /* Signed division */
      uDiv,      /* Unsigned division */
      sMod,      /* Signed modulus */
      uMod,      /* Unsigned modulus */
      BitAnd,    /* Bitwise AND */
      BitOr,     /* Bitwise OR */
      BitXor,    /* Bitwise XOR */
      LShift,    /* Left shift */
      sRShift,   /* Signed shift */
      uRShift,   /* Unsigned right shift */
      LRotate,   /* Rotate left */
      RRotate,   /* Rotate right */
      LogicAnd,  /* Logical AND */
      LogicOr,   /* Logical OR */
      LogicXor,  /* Logical XOR */
      LT,        /* Less than */
      GT,        /* Greater than */
      LE,        /* Less than or equal to */
      GE,        /* Greater than or equal to */
      Eq,        /* Equal to */
      NE,        /* Not equal to */
      Set,       /* Assignment */
      BitcastAs, /* Bitcast */
    };

    constexpr IR_eBIN(Op op, FlowPtr<Base> lhs, FlowPtr<Base> rhs)
        : Base(IRKind::AIR_eBIN), m_op(op), m_lhs(std::move(lhs)), m_rhs(std::move(rhs)) {}
    constexpr IR_eBIN(const IR_eBIN &) = default;
    constexpr IR_eBIN(IR_eBIN &&) = default;
    constexpr IR_eBIN &operator=(const IR_eBIN &) = default;
    constexpr IR_eBIN &operator=(IR_eBIN &&) = default;

    [[nodiscard, gnu::pure]] constexpr auto GetOp() const -> Op { return m_op; }
    [[nodiscard, gnu::pure]] constexpr auto GetLHS() const -> FlowPtr<const Base> { return m_lhs; }
    [[nodiscard, gnu::pure]] constexpr auto GetRHS() const -> FlowPtr<const Base> { return m_rhs; }

    [[nodiscard, gnu::pure]] constexpr auto GetLHS() -> FlowPtr<Base> {
      SetDirtyBit();
      return m_lhs;
    }

    [[nodiscard, gnu::pure]] constexpr auto GetRHS() -> FlowPtr<Base> {
      SetDirtyBit();
      return m_rhs;
    }

    constexpr void SetOp(Op op) {
      m_op = op;
      SetDirtyBit();
    }
    constexpr void SetLHS(FlowPtr<Base> lhs) {
      m_lhs = std::move(lhs);
      SetDirtyBit();
    }
    constexpr void SetRHS(FlowPtr<Base> rhs) {
      m_rhs = std::move(rhs);
      SetDirtyBit();
    }

  private:
    Op m_op;
    FlowPtr<Base> m_lhs, m_rhs;
  };

  class IR_eUNARY final : public Base {
  public:
    enum Op : uint8_t {
      BitNot,     /* Bitwise NOT */
      LogicNot,   /* Logical NOT */
      Bitsizeof,  /* Bit size of */
      Bitalignof, /* Bit alignment of */
    };

    constexpr IR_eUNARY(Op op, FlowPtr<Base> rhs) : Base(IRKind::AIR_eUNARY), m_op(op), m_rhs(std::move(rhs)) {}
    constexpr IR_eUNARY(const IR_eUNARY &) = default;
    constexpr IR_eUNARY(IR_eUNARY &&) = default;
    constexpr IR_eUNARY &operator=(const IR_eUNARY &) = default;
    constexpr IR_eUNARY &operator=(IR_eUNARY &&) = default;

    [[nodiscard, gnu::pure]] constexpr auto GetOp() const -> Op { return m_op; }
    [[nodiscard, gnu::pure]] constexpr auto GetRHS() const -> FlowPtr<const Base> { return m_rhs; }

    [[nodiscard, gnu::pure]] constexpr auto GetRHS() -> FlowPtr<Base> {
      SetDirtyBit();
      return m_rhs;
    }

    constexpr void SetOp(Op op) {
      m_op = op;
      SetDirtyBit();
    }
    constexpr void SetRHS(FlowPtr<Base> rhs) {
      m_rhs = std::move(rhs);
      SetDirtyBit();
    }

  private:
    Op m_op;
    FlowPtr<Base> m_rhs;
  };

  class IR_eACCESS final : public Base {
    uint32_t m_field;
    FlowPtr<Base> m_aggregate;

  public:
    constexpr IR_eACCESS(FlowPtr<Base> aggregate, uint32_t field)
        : Base(IRKind::AIR_eACCESS), m_field(field), m_aggregate(std::move(aggregate)) {}
    constexpr IR_eACCESS(const IR_eACCESS &) = default;
    constexpr IR_eACCESS(IR_eACCESS &&) = default;
    constexpr IR_eACCESS &operator=(const IR_eACCESS &) = default;
    constexpr IR_eACCESS &operator=(IR_eACCESS &&) = default;

    [[nodiscard, gnu::pure]] constexpr auto GetField() const -> uint32_t { return m_field; }
    [[nodiscard, gnu::pure]] constexpr auto GetAggregate() const -> FlowPtr<const Base> { return m_aggregate; }

    [[nodiscard, gnu::pure]] constexpr auto GetAggregate() -> FlowPtr<Base> {
      SetDirtyBit();
      return m_aggregate;
    }

    constexpr void SetAggregate(FlowPtr<Base> aggregate) {
      m_aggregate = std::move(aggregate);
      SetDirtyBit();
    }

    constexpr void SetField(uint32_t field) {
      m_field = field;
      SetDirtyBit();
    }
  };

  class IR_eINDEX final : public Base {
    FlowPtr<Base> m_base, m_index;

  public:
    constexpr IR_eINDEX(FlowPtr<Base> base, FlowPtr<Base> index)
        : Base(IRKind::AIR_eINDEX), m_base(std::move(base)), m_index(std::move(index)) {}
    constexpr IR_eINDEX(const IR_eINDEX &) = default;
    constexpr IR_eINDEX(IR_eINDEX &&) = default;
    constexpr IR_eINDEX &operator=(const IR_eINDEX &) = default;
    constexpr IR_eINDEX &operator=(IR_eINDEX &&) = default;

    [[nodiscard, gnu::pure]] constexpr auto GetBase() const -> FlowPtr<const Base> { return m_base; }
    [[nodiscard, gnu::pure]] constexpr auto GetIndex() const -> FlowPtr<const Base> { return m_index; }

    [[nodiscard, gnu::pure]] constexpr auto GetBase() -> FlowPtr<Base> {
      SetDirtyBit();
      return m_base;
    }

    [[nodiscard, gnu::pure]] constexpr auto GetIndex() -> FlowPtr<Base> {
      SetDirtyBit();
      return m_index;
    }

    constexpr void SetBase(FlowPtr<Base> base) {
      m_base = std::move(base);
      SetDirtyBit();
    }

    constexpr void SetIndex(FlowPtr<Base> index) {
      m_index = std::move(index);
      SetDirtyBit();
    }
  };

  class IR_eBLOCK final : public Base {
  public:
    using ExpressionList = std::pmr::vector<FlowPtr<Base>>;

    IR_eBLOCK(ExpressionList body) : Base(IRKind::AIR_eBLOCK), m_body(std::move(body)) {}
    constexpr IR_eBLOCK(const IR_eBLOCK &) = default;
    constexpr IR_eBLOCK(IR_eBLOCK &&) = default;
    IR_eBLOCK &operator=(const IR_eBLOCK &) = default;
    IR_eBLOCK &operator=(IR_eBLOCK &&) noexcept = default;

    [[nodiscard, gnu::pure]] constexpr auto GetBodySize() const -> size_t { return m_body.size(); }
    [[nodiscard, gnu::pure]] constexpr auto GetBody() const -> std::span<const FlowPtr<Base>> { return m_body; }

    [[nodiscard, gnu::pure]] constexpr auto GetBody() -> ExpressionList & {
      SetDirtyBit();
      return m_body;
    }

    [[nodiscard, gnu::pure]] constexpr auto operator->() const -> const ExpressionList * { return &m_body; }

    [[nodiscard, gnu::pure]] constexpr auto operator->() -> ExpressionList * {
      SetDirtyBit();
      return &m_body;
    }

    void SetBody(ExpressionList body) {
      m_body = std::move(body);
      SetDirtyBit();
    }

  private:
    ExpressionList m_body;
  };
}  // namespace ncc::alpha::tree
