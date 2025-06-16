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
  enum class IdentifierType : uint8_t {
    Typical,
    Atypical,
  };

  class Identifier {
    StringData m_name;
    IdentifierType m_type;

  public:
    Identifier(StringData name, IdentifierType type) : m_name(std::move(name)), m_type(type) {}

    [[nodiscard]] constexpr auto name() const -> const std::string& { return m_name.get(); }
    [[nodiscard]] constexpr auto type() const -> IdentifierType { return m_type; }

    [[nodiscard]] constexpr auto is_typical() const -> bool { return m_type == IdentifierType::Typical; }
    [[nodiscard]] constexpr auto is_atypical() const -> bool { return m_type == IdentifierType::Atypical; }

    [[nodiscard]] constexpr auto begin() const { return m_name->cbegin(); }
    [[nodiscard]] constexpr auto end() const { return m_name->cend(); }
  };
}  // namespace nitrate::compiler::lexer
