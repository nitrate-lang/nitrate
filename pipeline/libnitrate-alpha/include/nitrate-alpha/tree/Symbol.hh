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
#include <nitrate-core/NullableFlowPtr.hh>

namespace ncc::alpha::tree {
  class IR_eVAR final : public Base {
    FlowPtr<IR_tREF> m_type;
    FlowPtr<Base> m_value;

  public:
    constexpr IR_eVAR(FlowPtr<IR_tREF> type, FlowPtr<Base> value)
        : Base(IRKind::AIR_eVAR), m_type(std::move(type)), m_value(std::move(value)) {}
    constexpr IR_eVAR(const IR_eVAR &) = default;
    constexpr IR_eVAR(IR_eVAR &&) = default;
    constexpr IR_eVAR &operator=(const IR_eVAR &) = default;
    constexpr IR_eVAR &operator=(IR_eVAR &&) noexcept = default;

    [[nodiscard, gnu::pure]] constexpr auto GetType() const -> FlowPtr<const IR_tREF> { return m_type; }
    [[nodiscard, gnu::pure]] constexpr auto GetValue() const -> FlowPtr<const Base> { return m_value; }

    [[nodiscard, gnu::pure]] constexpr auto GetType() -> FlowPtr<IR_tREF> {
      SetDirtyBit();
      return m_type;
    }

    [[nodiscard, gnu::pure]] constexpr auto GetValue() -> FlowPtr<Base> {
      SetDirtyBit();
      return m_value;
    }

    constexpr void SetType(FlowPtr<IR_tREF> type) {
      m_type = std::move(type);
      SetDirtyBit();
    }

    constexpr void SetValue(FlowPtr<Base> value) {
      m_value = std::move(value);
      SetDirtyBit();
    }
  };

  class IR_eFUNCTION final : public Base {
    FlowPtr<IR_tFUNCTION> m_type;
    NullableFlowPtr<IR_eBLOCK> m_body;

  public:
    constexpr IR_eFUNCTION(FlowPtr<IR_tFUNCTION> type, NullableFlowPtr<IR_eBLOCK> body)
        : Base(IRKind::AIR_eFUNCTION), m_type(std::move(type)), m_body(std::move(body)) {}
    constexpr IR_eFUNCTION(const IR_eFUNCTION &) = default;
    constexpr IR_eFUNCTION(IR_eFUNCTION &&) = default;
    constexpr IR_eFUNCTION &operator=(const IR_eFUNCTION &) = default;
    constexpr IR_eFUNCTION &operator=(IR_eFUNCTION &&) noexcept = default;

    [[nodiscard, gnu::pure]] constexpr auto GetType() const -> FlowPtr<const IR_tFUNCTION> { return m_type; }
    [[nodiscard, gnu::pure]] constexpr auto GetBody() const -> NullableFlowPtr<const IR_eBLOCK> { return m_body; }

    [[nodiscard, gnu::pure]] constexpr auto GetType() -> FlowPtr<IR_tFUNCTION> {
      SetDirtyBit();
      return m_type;
    }

    [[nodiscard, gnu::pure]] constexpr auto GetBody() -> NullableFlowPtr<IR_eBLOCK> {
      SetDirtyBit();
      return m_body;
    }

    constexpr void SetType(FlowPtr<IR_tFUNCTION> type) {
      m_type = std::move(type);
      SetDirtyBit();
    }

    constexpr void SetBody(NullableFlowPtr<IR_eBLOCK> body) {
      m_body = std::move(body);
      SetDirtyBit();
    }
  };
}  // namespace ncc::alpha::tree
