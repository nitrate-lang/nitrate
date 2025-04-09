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
  class IR_eCALL : public Base {
  public:
    using CallArguments = std::pmr::vector<FlowPtr<Base>>;

    IR_eCALL(FlowPtr<Base> callee, CallArguments arguments)
        : Base(IRKind::AIR_eCALL), m_callee(std::move(callee)), m_arguments(std::move(arguments)) {}
    constexpr IR_eCALL(const IR_eCALL &) = default;
    constexpr IR_eCALL(IR_eCALL &&) = default;
    IR_eCALL &operator=(const IR_eCALL &) = default;
    IR_eCALL &operator=(IR_eCALL &&) noexcept = default;

    [[nodiscard, gnu::pure]] constexpr auto GetCallee() const -> FlowPtr<const Base> { return m_callee; }
    [[nodiscard, gnu::pure]] constexpr auto GetArguments() const -> std::span<const FlowPtr<Base>> {
      return m_arguments;
    }

    [[nodiscard, gnu::pure]] constexpr auto GetCallee() -> FlowPtr<Base> {
      SetDirtyBit();
      return m_callee;
    }

    [[nodiscard, gnu::pure]] constexpr auto GetArguments() -> CallArguments & {
      SetDirtyBit();
      return m_arguments;
    }

    [[nodiscard, gnu::pure]] constexpr auto operator->() const -> const CallArguments * { return &m_arguments; }

    [[nodiscard, gnu::pure]] constexpr auto operator->() -> CallArguments * {
      SetDirtyBit();
      return &m_arguments;
    }

    constexpr void SetCallee(FlowPtr<Base> callee) {
      m_callee = std::move(callee);
      SetDirtyBit();
    }

    void SetArguments(CallArguments arguments) {
      m_arguments = std::move(arguments);
      SetDirtyBit();
    }

  private:
    FlowPtr<Base> m_callee;
    CallArguments m_arguments;
  };

  class IR_eIF final : public Base {
    FlowPtr<Base> m_condition;
    FlowPtr<IR_eBLOCK> m_true_block, m_false_block;

  public:
    constexpr IR_eIF(FlowPtr<Base> condition, FlowPtr<IR_eBLOCK> true_block, FlowPtr<IR_eBLOCK> false_block)
        : Base(IRKind::AIR_eIF),
          m_condition(std::move(condition)),
          m_true_block(std::move(true_block)),
          m_false_block(std::move(false_block)) {}
    constexpr IR_eIF(const IR_eIF &) = default;
    constexpr IR_eIF(IR_eIF &&) = default;
    constexpr IR_eIF &operator=(const IR_eIF &) = default;
    constexpr IR_eIF &operator=(IR_eIF &&) noexcept = default;

    [[nodiscard, gnu::pure]] constexpr auto GetCondition() const -> FlowPtr<const Base> { return m_condition; }
    [[nodiscard, gnu::pure]] constexpr auto GetTrueBlock() const -> FlowPtr<const IR_eBLOCK> { return m_true_block; }
    [[nodiscard, gnu::pure]] constexpr auto GetFalseBlock() const -> FlowPtr<const IR_eBLOCK> { return m_false_block; }

    [[nodiscard, gnu::pure]] constexpr auto GetCondition() -> FlowPtr<Base> {
      SetDirtyBit();
      return m_condition;
    }

    [[nodiscard, gnu::pure]] constexpr auto GetTrueBlock() -> FlowPtr<IR_eBLOCK> {
      SetDirtyBit();
      return m_true_block;
    }

    [[nodiscard, gnu::pure]] constexpr auto GetFalseBlock() -> FlowPtr<IR_eBLOCK> {
      SetDirtyBit();
      return m_false_block;
    }

    constexpr void SetCondition(FlowPtr<Base> condition) {
      m_condition = std::move(condition);
      SetDirtyBit();
    }

    constexpr void SetTrueBlock(FlowPtr<IR_eBLOCK> true_block) {
      m_true_block = std::move(true_block);
      SetDirtyBit();
    }

    constexpr void SetFalseBlock(FlowPtr<IR_eBLOCK> false_block) {
      m_false_block = std::move(false_block);
      SetDirtyBit();
    }
  };

  class IR_eSWITCH final : public Base {
  public:
    using Case = std::pair<FlowPtr<Base>, FlowPtr<IR_eBLOCK>>;
    using CaseList = std::pmr::vector<Case>;

    IR_eSWITCH(FlowPtr<Base> condition, CaseList cases, NullableFlowPtr<IR_eBLOCK> default_block)
        : Base(IRKind::AIR_eSWITCH),
          m_condition(std::move(condition)),
          m_cases(std::move(cases)),
          m_default_block(std::move(default_block)) {}
    constexpr IR_eSWITCH(const IR_eSWITCH &) = default;
    constexpr IR_eSWITCH(IR_eSWITCH &&) = default;
    IR_eSWITCH &operator=(const IR_eSWITCH &) = default;
    IR_eSWITCH &operator=(IR_eSWITCH &&) noexcept = default;

    [[nodiscard, gnu::pure]] constexpr auto GetCondition() const -> FlowPtr<const Base> { return m_condition; }
    [[nodiscard, gnu::pure]] constexpr auto GetCases() const -> std::span<const Case> { return m_cases; }
    [[nodiscard, gnu::pure]] constexpr auto GetDefaultBlock() const -> NullableFlowPtr<const IR_eBLOCK> {
      return m_default_block;
    }

    [[nodiscard, gnu::pure]] constexpr auto GetCondition() -> FlowPtr<Base> {
      SetDirtyBit();
      return m_condition;
    }

    [[nodiscard, gnu::pure]] constexpr auto GetCases() -> CaseList & {
      SetDirtyBit();
      return m_cases;
    }

    [[nodiscard, gnu::pure]] constexpr auto GetDefaultBlock() -> NullableFlowPtr<IR_eBLOCK> {
      SetDirtyBit();
      return m_default_block;
    }

    [[nodiscard, gnu::pure]] constexpr auto operator->() const -> const CaseList * { return &m_cases; }

    [[nodiscard, gnu::pure]] constexpr auto operator->() -> CaseList * {
      SetDirtyBit();
      return &m_cases;
    }

    constexpr void SetCondition(FlowPtr<Base> condition) {
      m_condition = std::move(condition);
      SetDirtyBit();
    }

    void SetCases(CaseList cases) {
      m_cases = std::move(cases);
      SetDirtyBit();
    }

    constexpr void SetDefaultBlock(NullableFlowPtr<IR_eBLOCK> default_block) {
      m_default_block = std::move(default_block);
      SetDirtyBit();
    }

  private:
    FlowPtr<Base> m_condition;
    CaseList m_cases;
    NullableFlowPtr<IR_eBLOCK> m_default_block;
  };

  class IR_eRET final : public Base {
    NullableFlowPtr<Base> m_value;

  public:
    constexpr IR_eRET(NullableFlowPtr<Base> value) : Base(IRKind::AIR_eRET), m_value(std::move(value)) {}
    constexpr IR_eRET(const IR_eRET &) = default;
    constexpr IR_eRET(IR_eRET &&) = default;
    constexpr IR_eRET &operator=(const IR_eRET &) = default;
    constexpr IR_eRET &operator=(IR_eRET &&) noexcept = default;

    [[nodiscard, gnu::pure]] constexpr auto GetValue() const -> NullableFlowPtr<const Base> { return m_value; }

    [[nodiscard, gnu::pure]] constexpr auto GetValue() -> NullableFlowPtr<Base> {
      SetDirtyBit();
      return m_value;
    }

    constexpr void SetValue(NullableFlowPtr<Base> value) {
      m_value = std::move(value);
      SetDirtyBit();
    }
  };

  class IR_eBREAK final : public Base {
  public:
    constexpr IR_eBREAK() : Base(IRKind::AIR_eBREAK) {}
    constexpr IR_eBREAK(const IR_eBREAK &) = default;
    constexpr IR_eBREAK(IR_eBREAK &&) = default;
    constexpr IR_eBREAK &operator=(const IR_eBREAK &) = default;
    constexpr IR_eBREAK &operator=(IR_eBREAK &&) noexcept = default;
  };

  class IR_eCONTINUE final : public Base {
  public:
    constexpr IR_eCONTINUE() : Base(IRKind::AIR_eCONTINUE) {}
    constexpr IR_eCONTINUE(const IR_eCONTINUE &) = default;
    constexpr IR_eCONTINUE(IR_eCONTINUE &&) = default;
    constexpr IR_eCONTINUE &operator=(const IR_eCONTINUE &) = default;
    constexpr IR_eCONTINUE &operator=(IR_eCONTINUE &&) noexcept = default;
  };

  class IR_eWHILE final : public Base {
    FlowPtr<Base> m_condition;
    FlowPtr<IR_eBLOCK> m_body;

  public:
    constexpr IR_eWHILE(FlowPtr<Base> condition, FlowPtr<IR_eBLOCK> body)
        : Base(IRKind::AIR_eWHILE), m_condition(std::move(condition)), m_body(std::move(body)) {}
    constexpr IR_eWHILE(const IR_eWHILE &) = default;
    constexpr IR_eWHILE(IR_eWHILE &&) = default;
    constexpr IR_eWHILE &operator=(const IR_eWHILE &) = default;
    constexpr IR_eWHILE &operator=(IR_eWHILE &&) noexcept = default;

    [[nodiscard, gnu::pure]] constexpr auto GetCondition() const -> FlowPtr<const Base> { return m_condition; }
    [[nodiscard, gnu::pure]] constexpr auto GetBody() const -> FlowPtr<const IR_eBLOCK> { return m_body; }

    [[nodiscard, gnu::pure]] constexpr auto GetCondition() -> FlowPtr<Base> {
      SetDirtyBit();
      return m_condition;
    }

    [[nodiscard, gnu::pure]] constexpr auto GetBody() -> FlowPtr<IR_eBLOCK> {
      SetDirtyBit();
      return m_body;
    }

    constexpr void SetCondition(FlowPtr<Base> condition) {
      m_condition = std::move(condition);
      SetDirtyBit();
    }

    constexpr void SetBody(FlowPtr<IR_eBLOCK> body) {
      m_body = std::move(body);
      SetDirtyBit();
    }
  };

  class IR_eASM final : public Base {
    std::pmr::string m_asm_code;

  public:
    IR_eASM(std::pmr::string asm_code) : Base(IRKind::AIR_eASM), m_asm_code(std::move(asm_code)) {}
    constexpr IR_eASM(const IR_eASM &) = default;
    constexpr IR_eASM(IR_eASM &&) = default;
    IR_eASM &operator=(const IR_eASM &) = default;
    IR_eASM &operator=(IR_eASM &&) noexcept = default;

    [[nodiscard, gnu::pure]] constexpr auto GetAsmCode() const -> std::string_view { return m_asm_code; }

    [[nodiscard, gnu::pure]] constexpr auto GetAsmCode() -> auto & {
      SetDirtyBit();
      return m_asm_code;
    }

    void SetAsmCode(std::pmr::string asm_code) {
      m_asm_code = std::move(asm_code);
      SetDirtyBit();
    }
  };
}  // namespace ncc::alpha::tree
