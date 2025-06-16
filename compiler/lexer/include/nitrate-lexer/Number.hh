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
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <cstdint>
#include <nitrate-lexer/StringData.hh>

namespace nitrate::compiler::lexer {
  enum class NumberLiteralType : uint8_t {
    UIntBin,
    UIntOct,
    UIntDec,
    UIntHex,

    UFloatIEEE754,
  };

  class NumberLiteral {
    StringData m_value;
    NumberLiteralType m_type;

  public:
    NumberLiteral(StringData value, NumberLiteralType type) : m_value(std::move(value)), m_type(type) {}

    [[nodiscard]] constexpr auto value() const -> const std::string& { return m_value.get(); }
    [[nodiscard]] constexpr auto type() const -> NumberLiteralType { return m_type; }

    [[nodiscard]] constexpr auto is_uint_bin() const -> bool { return m_type == NumberLiteralType::UIntBin; }
    [[nodiscard]] constexpr auto is_uint_oct() const -> bool { return m_type == NumberLiteralType::UIntOct; }
    [[nodiscard]] constexpr auto is_uint_dec() const -> bool { return m_type == NumberLiteralType::UIntDec; }
    [[nodiscard]] constexpr auto is_uint_hex() const -> bool { return m_type == NumberLiteralType::UIntHex; }
    [[nodiscard]] constexpr auto is_ufloat_ieee754() const -> bool {
      return m_type == NumberLiteralType::UFloatIEEE754;
    }

    [[nodiscard]] constexpr auto begin() const { return m_value->cbegin(); }
    [[nodiscard]] constexpr auto end() const { return m_value->cend(); }
  };
}  // namespace nitrate::compiler::lexer
