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
  enum class StringType : uint8_t {
    SingleQuote,
    DoubleQuote,
    RawString,
  };

  class StringLiteral {
    StringData m_value;
    StringType m_type;

  public:
    StringLiteral(StringData value, StringType type) : m_value(std::move(value)), m_type(type) {}

    [[nodiscard]] constexpr auto value() const -> const std::string& { return m_value.get(); }
    [[nodiscard]] constexpr auto type() const -> StringType { return m_type; }

    [[nodiscard]] constexpr auto is_single_quote() const -> bool { return m_type == StringType::SingleQuote; }
    [[nodiscard]] constexpr auto is_double_quote() const -> bool { return m_type == StringType::DoubleQuote; }
    [[nodiscard]] constexpr auto is_raw_string() const -> bool { return m_type == StringType::RawString; }

    [[nodiscard]] constexpr auto size() const -> size_t { return m_value->size(); }
    [[nodiscard]] constexpr auto empty() const -> bool { return m_value->empty(); }

    [[nodiscard]] constexpr auto begin() const { return m_value->cbegin(); }
    [[nodiscard]] constexpr auto end() const { return m_value->cend(); }
  };
}  // namespace nitrate::compiler::lexer
